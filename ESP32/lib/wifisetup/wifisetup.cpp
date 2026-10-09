#include "wifisetup.h"

wifisetup::wifisetup()
{
    settings.begin(defaultname, false);
    String uniqueid = settings.getString("uniqueid");
    String prnt;
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    n = WiFi.scanNetworks();
    w = new String[n];
    ws = new bool[n];
    int wc[n];
    for (int i = 0; i < n; i++)
    {
        w[i] = WiFi.SSID(i);
        ws[i] = !(WiFi.encryptionType(i) == WIFI_AUTH_OPEN);
        wc[i] = WiFi.channel(i);
    }
    for (int i = 0; i < n; i++)
    {
        for (int ii = 0; ii < i; ii++)
        {
            if (w[i] == w[ii])
            {
                w[i] = "";
            }
        }
    }
    int channel = -1;
    for (int i = 1; i <= 11; i++)
    {
        bool same = false;
        for (int ii = 0; ii < n; ii++)
        {
            if (wc[ii] == i)
            {
                same = true;
            }
        }
        if (same)
        {
            continue;
        }
        else
        {
            channel = i;
            break;
        }
    }
    if (channel == -1)
    {
        int count[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        int min = 100;
        for (int i = 0; i < n; i++)
        {
            count[wc[i]-1]++;
        }
        for (int i = 0; i < 11; i++)
        {
            if (count[i] < min)
            {
                min = count[i];
                channel = i+1;
            }
        }
    }
    WiFi.scanDelete();
    server = new AsyncWebServer(80);

    String chars = "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz1234567890";
    for (int i = 0; i < 8; i++)
    {
        int t = random(0, chars.length());
        pass = pass + chars[t];
    }
    Serial.println("ssid = \"" + String(defaultname) + "_" + uniqueid + "\"");
    Serial.println("pass = \"" + pass + "\"");

    WiFi.softAP(String(defaultname) + "_" + uniqueid, pass, channel);
    Serial.println("ip address = \"" + WiFi.softAPIP().toString() + "\"");
    ip = WiFi.softAPIP().toString();
    MDNS.begin(String(defaultname) + "_" + uniqueid);
    MDNS.addService("_http", "_tcp", 80);

    server->on("/", HTTP_GET, [&](AsyncWebServerRequest *request){request->send(200, "text/html", index_set, [&](const String &var){return repl(var);});});
    server->on("/", HTTP_POST, [&](AsyncWebServerRequest *request){request->send(200); resolve(request);});
    server->on("/index.css", HTTP_GET, [](AsyncWebServerRequest *request){request->send(200, "text/css", index_css);});
    server->begin();
}

String wifisetup::repl(const String &var)
{
    if (var == "name")
    {
        return namestyle;
    }
    else if (var == "options")
    {
        String temp;
        for (int i = 0; i < n; i++)
        {
            if (w[i] == "")
            {
                continue;
            }
            if (ws[i])
            {
                temp += "<option value=\"" + String(i) + "\">" + w[i] + "</option>";
            }
            else
            {
                temp += "<option value=\"" + String(i) + "n\">" + w[i] + "</option>";
            }
            
        }
        return temp;
    }
    return var;
}

void wifisetup::resolve(AsyncWebServerRequest *request)
{
    for (int i = 0; i < request->params(); i++)
    {
        String name = request->getParam(i)->name();
        String value = request->getParam(i)->value();
        if (name == "networks")
        {
            if (value == "other")
            {
                continue;
            }
            if (value.indexOf("o") >= 0)
            {
                value = value.substring(0, value.indexOf("o"));
            }
            settings.putString("wifi/ssid", w[value.toInt()]);
            if (!ws[value.toInt()])
            {
                settings.remove("wifi/pass");
            }
        }
        else if (name == "ssid")
        {
            settings.putString("wifi/ssid", value);
        }
        else if (name == "pass")
        {
            if (value == "")
            {
                settings.remove("wifi/pass");
            }
            else
            {
                settings.putString("wifi/pass", value);
            }
        }
        else if (name == "ip")
        {
            settings.putString("mqtt/address", value);
        }
        else if (name == "port")
        {
            settings. putInt("mqtt/port", value.toInt());
        }
        else if (name == "user")
        {
            settings.putString("mqtt/user", value);
        }
        else if (name == "mqttpass")
        {
            settings.putString("mqtt/pass", value);
        }
        else if (name == "tz")
        {
            settings.putString("tz", value);
        }
        else if (name == "12h")
        {
            settings.putBool("12h", (value == "true"));
        }
        done = true;
    }
    request->redirect("/");
}

wifisetup::~wifisetup()
{
    delete(server);
}
