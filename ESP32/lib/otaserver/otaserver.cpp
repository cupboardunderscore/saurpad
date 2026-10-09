#include "otaserver.h"

otaserver::otaserver()
{
    settings.begin(defaultname, false);
    sn = settings.getString("uniqueid");
    bauto = settings.getBool("brightness/auto", true);
    brightness = settings.getInt("brightness", 50);
    drefresh = settings.getInt("int/dispfresh", 500);
    pswitch = settings.getInt("int/pswitch", 25);
    sensors = settings.getInt("int/sens", 1);
    send = settings.getInt("int/send", 60);
    warning = settings.getInt("int/warn", 100);
    server = new AsyncWebServer(80);
    server->on("/", HTTP_GET, [&](AsyncWebServerRequest *request){request->send(200, "text/html", index_ota, [&](const String &var){return repl(var);});});
    server->on("/upload", HTTP_POST, [&](AsyncWebServerRequest *request){request->send(200);}, [&](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){return upload(request, filename, index, data, len, final);});
    server->on("/conf", HTTP_POST, [&](AsyncWebServerRequest *request){request->send(200); resolve(request);});
    server->on("/index.css", HTTP_GET, [](AsyncWebServerRequest *request){request->send(200, "text/css", index_css);});
    server->on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){request->send(SD, "/" + String(defaultname) + "/favicon.png", "image/png");});
    server->on("/apple-touch-icon.png", HTTP_GET, [](AsyncWebServerRequest *request){request->send(SD, "/" + String(defaultname) + "/favicon.png", "image/png");});
    server->begin();
}

String otaserver::repl(const String &var)
{
    if (var == "name")
    {
        return namestyle;
    }
    else if (var == "sn")
    {
        return sn;
    }
    else if (var == "v")
    {
        return ver;
    }
    else if (var == "tem")
    {
        return tem;
    }
    else if (var == "hum")
    {
        return hum;
    }
    else if (var == "lig")
    {
        return lig;
    }
    else if (var == "bat")
    {
        return bat;
    }
    else if (var == "ssid")
    {
        return ssid;
    }
    else if (var == "ip")
    {
        return ip;
    }
    else if (var == "rssi")
    {
        return rssi;
    }
    else if (var == "auto")
    {
        return (bauto)? "checked" : "";
    }
    else if (var == "man")
    {
        return (bauto)? "style=\"display: none;\"" : "";
    }
    else if (var == "manual")
    {
        return String(brightness);
    }
    else if (var == "display")
    {
        return String(drefresh);
    }
    else if (var == "screen")
    {
        return String(pswitch);
    }
    else if (var == "sensors")
    {
        return String(sensors);
    }
    else if (var == "mqtt")
    {
        return String(send);
    }
    else if (var == "warning")
    {
        return String(warning);
    }
    return var;
}

void otaserver::resolve(AsyncWebServerRequest *request)
{
    for (int i = 0; i < request->params(); i++)
    {
        String name = request->getParam(i)->name();
        String value = request->getParam(i)->value();
        if (name == "auto")
        {
            bauto = (value == "true");
            settings.putBool("brightness/auto", (value == "true"));
        }
        else if (name == "manual")
        {
            brightness = value.toInt();
            settings.putInt("brightness", value.toInt());
        }
        else if (name == "display")
        {
            drefresh = value.toInt();
            settings.putInt("int/dispfresh", value.toInt());
        }
        else if (name == "screen")
        {
            pswitch = value.toInt();
            settings.putInt("int/pswitch", value.toInt());
        }
        else if (name == "sensors")
        {
            sensors = value.toInt();
            settings.putInt("int/sens", value.toInt());
        }
        else if (name == "mqtt")
        {
            send = value.toInt();
            settings.putInt("int/send", value.toInt());
        }
        else if (name == "warning")
        {
            warning = value.toInt();
            settings.putInt("int/warn", value.toInt());
        }
    }
}

void otaserver::upload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    Serial.println(request->url());
    if (!index)
    {
        request->_tempFile = SD.open("/" + filename, "w");
    }

    if (len)
    {
        request->_tempFile.write(data, len);
    }

    if (final)
    {
        request->_tempFile.close();
        request->redirect("/");
        if (filename == "firmware.bin")
        {
            ESP.restart();
        }
    }
}

otaserver::~otaserver()
{
    delete(server);
}
