#include <WiFi.h>
#include <MQTTClient.h>
#include <Preferences.h>
#include <vector>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>
#include <algorithm>
#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include <Update.h>
#include <Wire.h>
#include <RTClib.h>
#include <ezTime.h>
#include <Adafruit_MAX1704X.h>
#include <Adafruit_SHT4x.h>
#include <Adafruit_VEML7700.h>
#include <Adafruit_PCF8575.h>
#include <I2SSpeaker.h>
#include <MP3Player.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>

#include "defs.hpp"
#include "wifisetup.h"
#include "devices.h"
#include "lvgl_v8_port.h"
#include "ui.h"
#include "images.h"
#include "config.hpp"
#include "otaserver.h"

WiFiClient network;
MQTTClient mqtt = MQTTClient(2048);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
Preferences settings;
esp_panel::board::Board *board;
RTC_DS3231 rtc;
Timezone loc;
Adafruit_MAX17048 bat;
Adafruit_SHT4x th;
Adafruit_VEML7700 lght;
Adafruit_PCF8575 expd;
TaskHandle_t sens;
TaskHandle_t sned;
TaskHandle_t warw;
TaskHandle_t blnk;
TaskHandle_t btn;
I2SSpeaker *speak = new I2SSpeaker(I2S_DOUT, I2S_BCLK, I2S_LRC);
SemaphoreHandle_t i2cmutex = NULL;

std::vector<devices> device;
std::vector<String> warn;
std::vector<String> wanl;

unsigned long lastpageswitch = 0;
unsigned long lastdisplayrefresh = 0;
unsigned long lastvolumechange = 0;
const int volinterval = 3;
int devicecount = 0;
int currentpage = 0;
lv_obj_t *name[8];
lv_obj_t *area[8];
lv_obj_t *state[8];
lv_obj_t *updated[8];
lv_obj_t *button[8];
bool done = false;
int dayofyear = -1;
float batt = 0;
float lx = 0;
int rssi = 0;
float chr8 = 0;
String blonk = "";
double volume = 0.5;
double lastvolume = 0.5;
bool b1c = true, b2c = true, b1l = true, b2l = true;
unsigned long held1 = 0, held2 = 0;
String lastupdatedsens = "";
bool h12 = false;

otaserver *ota;
devices *dv;

class led
{
    int pin;
    bool status;
public:
    led(int pin)
    {
        this->pin = pin;
        this->status = false;
        expd.pinMode(pin, OUTPUT);
        expd.digitalWrite(pin, HIGH);
    }
    bool toggle()
    {
        bool rt;
        if (xSemaphoreTake(i2cmutex, 100))
        {
            if (this->status)
            {
                rt = expd.digitalWrite(pin, HIGH);
            }
            else
            {
                rt = expd.digitalWrite(pin, LOW);
            }
            xSemaphoreGive(i2cmutex);
        }
        return rt;
    }
    bool on()
    {
        bool rt;
        if (xSemaphoreTake(i2cmutex, 5000))
        {
            rt = expd.digitalWrite(pin, LOW);
            xSemaphoreGive(i2cmutex);
        }
        return rt;
    }
    bool off()
    {
        bool rt;
        if (xSemaphoreTake(i2cmutex, 10000))
        {
            rt = expd.digitalWrite(pin, HIGH);
            xSemaphoreGive(i2cmutex);
        }
        return rt;
    }
};

led *r;
led *g;
led *b;

void updatedisplay(int page = currentpage)
{
    lvgl_port_lock(-1);
    int x = 0;
    int y = 0;
    while (y < page*8 && x < device.size())
    {
        if (device[x].friendly_name != "" || device[x].get_state() != "0 None" || device[x].name == "sensor." + String(defaultname) + "_battery")
        {
            y++;
        }
        x++;
    }
    y = 0;
    while (x < device.size() && y < 8)
    {
        if (device[x].name == "sensor." + String(defaultname) +  "_battery")
        {
            device[x].set_state(batt);
            device[x].set_updated(lastupdatedsens + "+00:00");
        }
        else if (device[x].name == "sensor." + String(defaultname) +  "_illuminance")
        {
            device[x].set_state(lx);
            device[x].set_updated(lastupdatedsens + "+00:00");
        }
        if (device[x].friendly_name == "" || device[x].get_state() == "0 None" || device[x].name == "sensor." + String(defaultname) + "_battery")
        {
            x++;
            continue;
        }
        DateTime now;
        if (xSemaphoreTake(i2cmutex, 1000))
        {
            now = rtc.now();
            xSemaphoreGive(i2cmutex);
        }
        lv_obj_clear_flag(button[y], LV_OBJ_FLAG_HIDDEN);
        if (device[x].get_warning(now.unixtime()))
        {
            lv_obj_set_style_bg_color(button[y], lv_color_hex(0xffff0000), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        else
        {
            lv_obj_set_style_bg_color(button[y], lv_color_hex(0xff2196f3), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        lv_label_set_text(name[y], device[x].friendly_name.c_str());
        lv_label_set_text(area[y], device[x].area_name.c_str());
        lv_label_set_text(state[y], device[x].get_state().c_str());
        lv_label_set_text(updated[y], device[x].get_updated(now.unixtime()).c_str());
        y++;
        x++;
    }
    while (y < 8)
    {
        lv_obj_add_flag(button[y], LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(name[y], " ");
        lv_label_set_text(area[y], " ");
        lv_label_set_text(state[y], " ");
        lv_label_set_text(updated[y], " ");
        y++;
    }
    currentpage = page;
    lvgl_port_unlock();
}

void sendtoclass(String message, devices &dev)
{
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, message);
    if (error)
    {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        Serial.println(message);
        return;
    }
    String state = doc["state"];
    String device_class = doc["device_class"];
    String unit_of_measurement = doc["unit_of_measurement"];
    String friendly_name = doc["friendly_name"];
    String last_updated = doc["last_updated"];
    String area_name = doc["area_name"];
    String warning = doc["warning"];

    if (state != "null")
    {
        if (state == "on" || state == "open" || state == "opening")
        {
            dev.set_state(true);
        }
        else if (state == "off" || state == "closed")
        {
            dev.set_state(false);
        }
        else if (state != "unknown" && state != "unavailable")
        {
            dev.set_state(state.toDouble());
        }
    }
    if (friendly_name != "null")
    {
        dev.friendly_name = friendly_name;
    }
    if (unit_of_measurement != "null")
    {
        String newmessage = unit_of_measurement;
        newmessage.replace("\\u00b3", "\u00b3");
        dev.unit_of_measurement = newmessage;
    }
    if (last_updated != "null")
    {
        dev.set_updated(last_updated);
    }
    if (device_class != "null")
    {
        dev.device_class = device_class;
    }
    if (warning != "null")
    {
        if (warning == "on")
        {
            dev.set_warning('1');
        }
        else if (warning == "off")
        {
            dev.set_warning('0');
        }
        else if (warning == "remove")
        {
            dev.set_warning('-');
        }
    }
    if (area_name != "null")
    {
        dev.area_name = area_name;
    }
    /*else
    {
        Serial.print(dev.name);
        Serial.print(": ");
        Serial.print(action);
        Serial.print(" - ");
        Serial.println(message);
    }*/
    std::stable_sort(device.begin(), device.end());
}

void messageHandler(String &topic, String &message)
{
    if (topic == "homeassistant/status" && message == "online")
    {
        mqtt.publish(String(defaultname) + "/done", "done");
        mqtt.publish("homeassistant/device/" + String(defaultname) + "/config", jsn(), false, 1);
    }
    else if (topic == "homeassistant/done" && message == "done")
    {
        done = true;
        updatedisplay();
    }
    if (topic.startsWith("homeassistant"))
    {        
        String newtopic = topic;
        newtopic.remove(0, newtopic.indexOf('/') + 1);
        String devname = newtopic.substring(0, newtopic.indexOf('/'));

        for (int i = 0; i < device.size(); i++)
        {
            if (device[i].name == devname)
            {
                if (message == "remove")
                {
                    device.erase(device.begin()+i);
                    return;
                }
                sendtoclass(message, device[i]);
                return;
            }
        }
        if (devname == "homeassistant.homeassistant" || devname == "status.status" || devname == "binary_sensor." + String(defaultname) + "_charging" || message == "remove")
        {
            return;
        }
        devices temp;
        temp.name = devname;
        sendtoclass(message, temp);
        device.push_back(temp);
        return;
    }
}

void setupdisplay()
{
    board = new esp_panel::board::Board();
    board->init();
    if (board->getIO_Expander() != nullptr)
    {
        static_cast<esp_panel::drivers::BusI2C *>(board->getTouch()->getBus())->configI2C_HostSkipInit();
        board->getIO_Expander()->skipInitHost();
    }
#if LVGL_PORT_AVOID_TEARING_MODE
    auto lcd = board->getLCD();
    // When avoid tearing function is enabled, the frame buffer number should be set in the board driver
    lcd->configFrameBufferNumber(LVGL_PORT_DISP_BUFFER_NUM);
#if ESP_PANEL_DRIVERS_BUS_ENABLE_RGB && CONFIG_IDF_TARGET_ESP32S3
    auto lcd_bus = lcd->getBus();
    /**
     * As the anti-tearing feature typically consumes more PSRAM bandwidth, for the ESP32-S3, we need to utilize the
     * "bounce buffer" functionality to enhance the RGB data bandwidth.
     * This feature will consume `bounce_buffer_size * bytes_per_pixel * 2` of SRAM memory.
     */
    if (lcd_bus->getBasicAttributes().type == ESP_PANEL_BUS_TYPE_RGB)
    {
        static_cast<esp_panel::drivers::BusRGB *>(lcd_bus)->configRGB_BounceBufferSize(lcd->getFrameWidth() * 10);
    }
#endif
#endif
    assert(board->begin());

    lvgl_port_init(board->getLCD(), board->getTouch());
    /* Lock the mutex due to the LVGL APIs are not thread-safe */
    lvgl_port_lock(-1);
    ui_init();
    lvgl_port_unlock();
    name[0] = objects.name_0;
    name[1] = objects.name_1;
    name[2] = objects.name_2;
    name[3] = objects.name_3;
    name[4] = objects.name_4;
    name[5] = objects.name_5;
    name[6] = objects.name_6;
    name[7] = objects.name_7;
    area[0] = objects.area_0;
    area[1] = objects.area_1;
    area[2] = objects.area_2;
    area[3] = objects.area_3;
    area[4] = objects.area_4;
    area[5] = objects.area_5;
    area[6] = objects.area_6;
    area[7] = objects.area_7;
    state[0] = objects.state_0;
    state[1] = objects.state_1;
    state[2] = objects.state_2;
    state[3] = objects.state_3;
    state[4] = objects.state_4;
    state[5] = objects.state_5;
    state[6] = objects.state_6;
    state[7] = objects.state_7;
    updated[0] = objects.updated_0;
    updated[1] = objects.updated_1;
    updated[2] = objects.updated_2;
    updated[3] = objects.updated_3;
    updated[4] = objects.updated_4;
    updated[5] = objects.updated_5;
    updated[6] = objects.updated_6;
    updated[7] = objects.updated_7;
    button[0] = objects.button_0;
    button[1] = objects.button_1;
    button[2] = objects.button_2;
    button[3] = objects.button_3;
    button[4] = objects.button_4;
    button[5] = objects.button_5;
    button[6] = objects.button_6;
    button[7] = objects.button_7;
}

void sensorsloop(void *pvParameters)
{
    while (true)
    {
        unsigned long delay = millis();
        rssi = WiFi.RSSI();
        DateTime now;
        float batV;
        if (xSemaphoreTake(i2cmutex, 10000))
        {
            batt = bat.cellPercent();
            batV = bat.cellVoltage();
            chr8 = bat.chargeRate();
            lx = lght.readLux(VEML_LUX_AUTO);
            now = rtc.now();
            xSemaphoreGive(i2cmutex);
        }
        /*if (batt < 50 && chr8 < 0 && batV < 3.8)
        {
            float batcur = batt;
            batt = 97.96*batV*batV - 603.88*batV + 930.62;
            if (batt < batcur)
            {
                batt = batcur;
            }
            if (batV <= 3)
            {
                batt = 0;
            }
        }*/
        if (batt > 100)
        {
            batt = 100;
        }
        else if (batt < 0)
        {
            batt = 0;
        }
        lastupdatedsens = now.timestamp();
        ota->bat = String(batt) + "%%";
        ota->lig = String(lx) + " lx";
        ota->rssi = String(rssi);
        delay = millis() - delay;
        int interval = ota->sensors*1000;
        if (delay < interval)
        {
            delay = interval;
        }
        vTaskDelay(delay / portTICK_PERIOD_MS);
    }
}

void sendloop(void *pvParameters)
{
    while (!done)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    while (true)
    {
        sensors_event_t tem, hum;
        if (xSemaphoreTake(i2cmutex, 10000))
        {
            th.getEvent(&hum, &tem);
            xSemaphoreGive(i2cmutex);
        }
        ota->tem = String(tem.temperature) + " °C";
        ota->hum = String(hum.relative_humidity) + "%%";
        String pub = "{\"temperature\": " + String(tem.temperature) + ", \"humidity\": " + String(hum.relative_humidity) + ", \"illuminance\": " + String(lx) + ", \"battery\": " + String(batt) + ", \"battery_charging\": " + String((chr8 > 0)? "\"ON\"" : "\"OFF\"") + "}";
        Serial.println(pub);
        mqtt.publish(String(defaultname) + "/state", pub);
        mqtt.publish("bV", String(bat.cellVoltage(), 4));
        vTaskDelay((ota->send * 1000) / portTICK_PERIOD_MS);
    }
}

void warningloop(void *pvParameters)
{
    while (true)
    {
        if (!wanl.empty())
        {
            String temp = wanl[0];
            wanl.erase(wanl.begin());
            devices tmp;
            for (auto i: device)
            {
                if (i.name == temp)
                {
                    tmp = i;
                    break;
                }
            }
            if (tmp.device_class == "")
            {
                blonk = "default";
            }
            else
            {
                blonk = tmp.device_class;
            }
            lv_obj_clear_flag(objects.warn, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(objects.warn_name, tmp.friendly_name.c_str());
            lv_label_set_text(objects.warn_area, tmp.area_name.c_str());
            lv_label_set_text(objects.warn_state, tmp.get_state().c_str());

            int delay = 5;
            String temparea = tmp.area_name;
            temparea.replace(" ", "_");
            temparea.toLowerCase();
            if (SD.exists("/" + String(defaultname) + "/rooms/" + temparea + ".mp3") && !(temparea == "garage" && (tmp.device_class == "garage" || tmp.device_class == "garage_door")))
            {
                MP3Decoder::MP3Info info;
                MP3Player::playFile("/" + String(defaultname) + "/rooms/" + temparea + ".mp3", volume);
                MP3Player::getFileInfo("/" + String(defaultname) + "/rooms/" + temparea + ".mp3", &info);
                speak->clear();
                delay = delay - info.duration;
            }
            if (SD.exists("/" + String(defaultname) + "/" + tmp.device_class + ".mp3"))
            {
                MP3Decoder::MP3Info info;
                MP3Player::playFile("/" + String(defaultname) + "/" + tmp.device_class + ".mp3", volume);
                MP3Player::getFileInfo("/" + String(defaultname) + "/" + tmp.device_class + ".mp3", &info);
                speak->clear();
                delay = delay - info.duration;
            }
            else
            {
                speak->playTone(880, 1000, volume);
                speak->clear();
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                speak->playTone(880, 1000, volume);
                speak->clear();
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                speak->playTone(880, 1000, volume);
                speak->clear();
                delay = 0;
            }
            if (delay < 0)
            {
                delay = 0;
            }
            vTaskDelay((delay * 1000) / portTICK_PERIOD_MS);
			blonk = "";
            lv_obj_add_flag(objects.warn, LV_OBJ_FLAG_HIDDEN);
        }
        vTaskDelay(ota->warning / portTICK_PERIOD_MS);
    }
}

void blinkloop(void *pvParameters)
{
    while (true)
    {
        if (blonk != "")
        {
            if (SD.exists("/" + String(defaultname) + "/" + blonk + ".txt"))
            {
                File bln = SD.open("/" + String(defaultname) + "/" + blonk + ".txt");
                String temp = bln.readString();
				bln.close();
                char *split = strtok((char *)temp.c_str(), "\n");
                while (split != NULL)
                {
					if (blonk == "")
					{
                        r->off();
                        g->off();
                        b->off();
                        delete(split);
                        break;
					}
                    temp = String(split);
                    Serial.println(temp);
                    if (temp.indexOf("r") >= 0)
                    {
                        r->on();
                    }
                    else
                    {
                        r->off();
                    }
                    if (temp.indexOf("g") >= 0)
                    {
                        g->on();
                    }
                    else
                    {
                        g->off();
                    }
                    if (temp.indexOf("b") >= 0)
                    {
                        b->on();
                    }
                    else
                    {
                        b->off();
                    }
                    vTaskDelay(temp.substring(0, temp.indexOf(",")).toInt() / portTICK_PERIOD_MS);
                    split = strtok(NULL, "\n");
                }
                vTaskDelay(100 / portTICK_PERIOD_MS);
                r->off();
                g->off();
                b->off();
                delete(split);
            }
            else
            {
                r->on();
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                r->off();
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                g->on();
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                g->off();
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                b->on();
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                b->off();
            }
            blonk = "";
        }
        vTaskDelay(ota->warning / portTICK_PERIOD_MS);
    }
}

void buttonloop(void *pvParameters)
{
    while (true)
    {
        b1c = expd.digitalRead(P7);
        b2c = expd.digitalRead(P6);
        if (b1c == false && b1l == false && (millis() - held1 >= 5000))
        {
            settings.putBool("setup", false);
            speak->stop();
            ESP.restart();
        }
        if (b1c == true && b1l == false)
        {
            if (millis() - held1 >= 500)
            {
                speak->stop();
                ESP.restart();
            }
            else if (volume < 0.999)
            {
                volume += 0.0625;
                MP3Player::setVolume(volume);
            }
        }
        else if (b1c == false && b1l == true)
        {
            held1 = millis();
        }
        if (b2c == true && b2l == false)
        {
            if (millis() - held2 >= 500)
            {
                lastpageswitch = 0;
            }
            else if (volume > 0.001)
            {
                volume -= 0.0625;
                MP3Player::setVolume(volume);
            }
        }
        else if (b2c == false && b2l == true)
        {
            held2 = millis();
        }
        b1l = b1c;
        b2l = b2c;
        auto backlight = board->getBacklight();
        int currbright = backlight->getBrightness();
        int settbright = ota->brightness;
        if (ota->bauto)
        {
            if (((int)lx > currbright) && currbright != 100)
            {
                backlight->setBrightness(currbright + 1);
            }
            else if (((int)lx < currbright) && currbright != 10)
            {
                backlight->setBrightness(currbright - 1);
            }
        }
        else if (currbright != settbright)
        {
            backlight->setBrightness(settbright);
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void setup()
{
    Serial.begin(115200);
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    Wire.begin(I2C_SDA, I2C_SCL);
    rtc.begin();
    bat.begin();
    th.begin();
    lght.begin();
    expd.begin();

    expd.pinMode(P6, INPUT);
    expd.pinMode(P7, INPUT);
    r = new led(P10);
    g = new led(P12);
    b = new led(P11);

    speak->init(24000);
    speak->start();
    MP3Player::init(speak);

    String prnt;
    settings.begin("thingy", false);
    volume = settings.getDouble("volume", volume);
    lastvolume = volume;
    setupdisplay();
    if (SD.begin(SD_CS))
    {
        File firmware = SD.open("/firmware.bin");
        if (firmware)
        {
            loadScreen(SCREEN_ID_UPDATE);
            Update.onProgress([](size_t currSize, size_t totalSize){lv_label_set_text(objects.updatetext, (String((float)currSize/1000) + "/" + String((float)totalSize/1000) + " kB").c_str()); lv_bar_set_value(objects.updatebar, ((float)currSize/(float)totalSize)*100, LV_ANIM_ON);});
            Update.begin(firmware.size(), U_FLASH);
            Update.writeStream(firmware);
            firmware.close();
            if (Update.end())
            {
                lv_label_set_text(objects.updatetext, "done c:");
            }
            /*while (true)
            {
                if (!expd.digitalRead(P7))
                {
                    SD.remove("/firmware.bin");
                    break;
                }
                if (!expd.digitalRead(P6))
                {
                    break;
                }
                delay(10);
            }*/
            SD.remove("/firmware.bin");
            delay(200);
            ESP.restart();
        }
    }
    if (!settings.isKey("wifi/ssid") || !settings.isKey("mqtt/address") || !settings.getBool("setup"))
    {
        settings.putBool("setup", false);
        wifisetup set;
        String qr;
        loadScreen(SCREEN_ID_WIFI);
        lv_label_set_text(objects.ssid, ("ssid = \"" + String(defaultname) + "\"").c_str());
        lv_label_set_text(objects.pass, ("pass = \"" + set.pass + "\"").c_str());
        qr = "WIFI:T:WPA;S:" + String(defaultname) + ";P:" + set.pass + ";;";
        lv_qrcode_update(objects.wifi_qr, qr.c_str(), strlen(qr.c_str()));
        lv_obj_clear_flag(objects.wifi_qr, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(objects.ip, ("ip address = \"" + set.ip + "\"").c_str());
        while (!set.done)
        {
            if (!expd.digitalRead(P7))
            {
                break;
            }
        }
        settings.putBool("setup", true);
        ESP.restart();
    }

    prnt = "ssid = \"" + settings.getString("wifi/ssid") + "\"";
    Serial.println(prnt);
    lv_label_set_text(objects.startup_text, prnt.c_str());
    if (settings.isKey("wifi/pass"))
    {
        prnt = "pass = \"" + settings.getString("wifi/pass") + "\"";
        Serial.println(prnt);
        lv_label_set_text(objects.startup_subtext, prnt.c_str());
        WiFi.begin(settings.getString("wifi/ssid"), settings.getString("wifi/pass"));
    }
    else
    {
        WiFi.begin(settings.getString("wifi/ssid"));
    }
    while (WiFi.status() != WL_CONNECTED)
    {
        if (!expd.digitalRead(P7))
        {
            settings.putBool("setup", false);
            ESP.restart();
        }
        if (Serial.available())
        {
            String temp = Serial.readString();
            if (temp == "\\")
            {
                settings.putBool("setup", false);
                ESP.restart();
            }
        }
        delay(500);
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    MDNS.begin(defaultname);
    MDNS.addService("_http", "_tcp", 80);
    ota = new otaserver;
    ota->ssid = WiFi.SSID();

    prnt = "mqtt = \"" + settings.getString("mqtt/address") + ":" + settings.getInt("mqtt/port") + "\"";
    Serial.println(prnt);
    lv_label_set_text(objects.startup_text, prnt.c_str());

    prnt = "username = " + settings.getString("mqtt/user") + ", password = " + settings.getString("mqtt/pass");
    lv_label_set_text(objects.startup_subtext, prnt.c_str());
    mqtt.begin(settings.getString("mqtt/address").c_str(), settings.getInt("mqtt/port"), network);
    mqtt.onMessage(messageHandler);
    lv_label_set_text(objects.startup_subtext, prnt.c_str());
    while (!mqtt.connect(defaultname, settings.getString("mqtt/user").c_str(), settings.getString("mqtt/pass").c_str()))
    {
        if (!WiFi.isConnected())
        {
            speak->stop();
            ESP.restart();
        }
        if (!expd.digitalRead(P7))
        {
            settings.putBool("setup", false);
            ESP.restart();
        }
        if (Serial.available())
        {
            String temp = Serial.readString();
            if (temp == "\\")
            {
                settings.putBool("setup", false);
                ESP.restart();
            }
        }
        delay(500);
        Serial.print(".");
    }
    if (!mqtt.connected())
    {
        Serial.println("MQTT broker Timeout!");
        return;
    }
    Serial.println("");
    if (mqtt.subscribe("#"))
    {
        Serial.println("Subscribed to topic");
    }
    else
    {
        Serial.print("Failed to subscribe to topic");
        ESP.restart();
    }
    Serial.println("MQTT broker Connected!");
    rtc.disable32K();
    th.setPrecision(SHT4X_HIGH_PRECISION);
    th.setHeater(SHT4X_NO_HEATER);
    timeClient.begin();
    timeClient.update();
    if (timeClient.isTimeSet())
    {
        rtc.adjust(timeClient.getEpochTime());
    }
    timeClient.end();
    loc.setLocation(settings.getString("tz", "Etc/UTC"));
    h12 = settings.getBool("12h");
    mqtt.publish("homeassistant/device/" + String(defaultname) + "/config", jsn(), false, 1);
    mqtt.publish(String(defaultname) + "/done", "done");
    i2cmutex = xSemaphoreCreateMutex();
    if (i2cmutex == NULL)
    {
        ESP.restart();
    }
    xTaskCreatePinnedToCore(sensorsloop, "sens", 10000, NULL, 0, &sens, 0);
    xTaskCreatePinnedToCore(sendloop, "sned", 10000, NULL, 0, &sned, 0);
    xTaskCreatePinnedToCore(warningloop, "warw", 10000, NULL, 0, &warw, 0);
    xTaskCreatePinnedToCore(blinkloop, "blnk", 10000, NULL, 0, &blnk, 0);
    xTaskCreatePinnedToCore(buttonloop, "btn", 10000, NULL, 0, &btn, 0);
    loadScreen(SCREEN_ID_MAIN);
}

void loop()
{
    if (volume != lastvolume)
    {
        lvgl_port_lock(-1);
        settings.putDouble("volume", volume);
        lastvolume = volume;
        lv_bar_set_value(objects.volume, volume * 100, LV_ANIM_ON);
        lv_obj_add_flag(objects.clock, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(objects.volume, LV_OBJ_FLAG_HIDDEN);
        lvgl_port_unlock();
        lastvolumechange = millis();
    }
    if (millis() - lastvolumechange > volinterval*1000)
    {
        lvgl_port_lock(-1);
        lv_obj_add_flag(objects.volume, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(objects.clock, LV_OBJ_FLAG_HIDDEN);
        lvgl_port_unlock();
    }
    DateTime now;
    if (xSemaphoreTake(i2cmutex, 5000))
    {
        now = rtc.now();
        xSemaphoreGive(i2cmutex);
    }
    if (!mqtt.connected())
    {
        speak->stop();
        ESP.restart();
    }
    delay(10);
    mqtt.loop();
    UTC.setTime(now.unixtime());
    lvgl_port_lock(-1);
    if (volume == 0)
    {
        lv_obj_clear_flag(objects.mute, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(objects.mute, LV_OBJ_FLAG_HIDDEN);
    }
    lv_label_set_text(objects.clock, (String((h12)? loc.hourFormat12() : loc.hour()) + ((loc.second()%2 == 0)? " " : ":") + ((loc.minute() < 10)? ("0" + String(loc.minute())) : String(loc.minute())) + (h12)? (" " + String(loc.isPM()? "pm" : "am")) : "").c_str());
    if (loc.dayOfYear() != dayofyear)
    {
        dayofyear = loc.dayOfYear();
        lv_label_set_text(objects.date, (dayShortStr(loc.weekday()) + " " + String(loc.day()) + ". " + String(loc.month()) + ".").c_str());
    }
    if (rssi >= -60)
    {
        lv_img_set_src(objects.wifistatus, &img_wifi_3);
    }
    else if (rssi >= -80)
    {
        lv_img_set_src(objects.wifistatus, &img_wifi_2);
    }
    else
    {
        lv_img_set_src(objects.wifistatus, &img_wifi_1);
    }
    lv_label_set_text(objects.battery, (String((int)round(batt)) + "%").c_str());
    if (batt >= 95)
    {
        lv_img_set_src(objects.batterystatus, &img_battery_full);
    }
    else if (batt >= 64)
    {
        lv_img_set_src(objects.batterystatus, &img_battery_three_quarters);
    }
    else if (batt >= 37)
    {
        lv_img_set_src(objects.batterystatus, &img_battery_half);
    }
    else if (batt >= 6)
    {
        lv_img_set_src(objects.batterystatus, &img_battery_quarter);
    }
    else
    {
        lv_img_set_src(objects.batterystatus, &img_battery_empty);
    }
    if (chr8 > 0)
    {
        lv_obj_clear_flag(objects.batterycharge, LV_OBJ_FLAG_HIDDEN);
        if (chr8 > 25)
        {
            lv_img_set_src(objects.batterycharge, &img_bolt_full);
        }
        else
        {
            lv_img_set_src(objects.batterycharge, &img_bolt);
        }
    }
    else
    {
        lv_obj_add_flag(objects.batterycharge, LV_OBJ_FLAG_HIDDEN);
    }
    lvgl_port_unlock();
    devicecount = 0;
    std::vector<String> warntmp;
    for (auto i: device)
    {
        if (i.friendly_name != "" && i.get_state() != "0 None")
        {
            if (i.name != "sensor." + String(defaultname) + "_battery")
            {
                devicecount++;
            }
            if (i.get_warning(now.unixtime()))
            {
                warntmp.push_back(i.name);
            }
        }
    }
    for (auto i: warntmp)
    {
        bool same = false;
        for (auto j : warn)
        {
            if (i == j)
            {
                same = true;
                break;
            }
        }
        if (!same)
        {
            wanl.push_back(i);
        }
    }
    warn.clear();
    warn = warntmp;
    int interval = ota->pswitch*1000;
    if ((millis() - lastpageswitch > interval) && devicecount > 8 && done)
    {
        if (devicecount%8 == 0 && devicecount / 8 > (currentpage + 1))
        {
            updatedisplay(currentpage + 1);
        }
        else if (devicecount%8 == 0 && devicecount / 8 <= (currentpage + 1))
        {
            updatedisplay(0);
        }
        else if (devicecount%8 != 0 && (currentpage + 1)*8 > devicecount)
        {
            updatedisplay(0);
        }
        else if (devicecount%8 != 0 && devicecount / 8 >= (currentpage + 1))
        {
            updatedisplay(currentpage + 1);
        }
        else
        {
            updatedisplay(0);
        }
        lastpageswitch = millis();
    }
    else if ((millis() - lastpageswitch > interval) && done)
    {
        updatedisplay();
        lastpageswitch = millis();
    }
    else if ((millis() - lastdisplayrefresh > ota->drefresh) && done)
    {
        updatedisplay();
        lastdisplayrefresh = millis();
    }
}
