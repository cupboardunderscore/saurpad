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
    events = new AsyncEventSource("/events");
    server->on("/", HTTP_GET, [&](AsyncWebServerRequest *request){request->send(200, "text/html", index_ota, [&](const String &var){return repl(var);});});
    server->on("/upload", HTTP_POST, [&](AsyncWebServerRequest *request){request->send(200);}, [&](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){return upload(request, filename, index, data, len, final);});
    server->on("/conf", HTTP_POST, [&](AsyncWebServerRequest *request){request->send(200); resolve(request);});
    server->on("/index.css", HTTP_GET, [](AsyncWebServerRequest *request){request->send(200, "text/css", index_css);});
    server->on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){request->send(SD, "/" + String(defaultname) + "/favicon.png", "image/png");});
    server->on("/apple-touch-icon.png", HTTP_GET, [](AsyncWebServerRequest *request){request->send(SD, "/" + String(defaultname) + "/apple-touch-icon.png", "image/png");});
    server->on("/favicon-32x32.png", HTTP_GET, [](AsyncWebServerRequest *request){request->send(SD, "/" + String(defaultname) + "/favicon-32x32.png", "image/png");});
    server->on("/favicon-16x16.png", HTTP_GET, [](AsyncWebServerRequest *request){request->send(SD, "/" + String(defaultname) + "/favicon-16x16.png", "image/png");});
    server->addHandler(events);
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
    else if (var == "available")
    {
        HTTPClient http;
        http.setTimeout(5000);
        http.begin(latesturl);
        http.GET();
        String aversion = http.getLocation();
        String cversion(ver);
        http.end();
        aversion = aversion.substring(aversion.lastIndexOf('/') + 1);
        String av(aversion);
        if (aversion.indexOf('.') == aversion.lastIndexOf('.'))
        {
            aversion = aversion + ".0";
        }
        cversion = cversion.substring(0, cversion.indexOf('-'));
        if (cversion.indexOf('.') == cversion.lastIndexOf('.'))
        {
            cversion = cversion + ".0";
        }
        int a[3], c[3];
        a[0] = aversion.substring(0, aversion.indexOf('.')).toInt();
        a[1] = aversion.substring(aversion.indexOf('.') + 1, aversion.lastIndexOf('.') + 1).toInt();
        a[2] = aversion.substring(aversion.lastIndexOf('.') + 1).toInt();
        c[0] = cversion.substring(0, cversion.indexOf('.')).toInt();
        c[1] = cversion.substring(cversion.indexOf('.') + 1, cversion.lastIndexOf('.') + 1).toInt();
        c[2] = cversion.substring(cversion.lastIndexOf('.') + 1).toInt();
        if (a[0] > c[0])
        {
            return " New version " + av + " available!";
        }
        else if (a[0] < c[0])
        {
            return "";
        }
        else
        {
            if (a[1] > c[1])
            {
                return " New version " + av + " available!";
            }
            else if (a[1] < c[1])
            {
                return "";
            }
            else
            {
                if (a[2] > c[2])
                {
                    return " New version " + av + " available!";
                }
                else if (a[2] < c[2])
                {
                    return "";
                }
                else
                {
                    return "";
                }
            }
        }
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
        else if (name == "latest")
        {
            HTTPClient http;
            http.setTimeout(15000);
            http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
            http.begin(downloadurl);
            if (http.GET() == HTTP_CODE_OK)
            {
                int size = http.getSize();
                File file = SD.open("/firmware.bin", "w");
                WiFiClient *stream = http.getStreamPtr();
                uint8_t buffer[1024];
                int bytesWritten = 0;
                stream->setTimeout(2500);
                int progress = 0;
                while (true)
                {
                    int s = stream->available();
                    int temprogress = (bytesWritten * 100)/size;
                    if (temprogress > progress && temprogress%5 == 0)
                    {
                        progress = temprogress;
                        events->send(String(progress).c_str(), "progress", millis());
                    }
                    if (s && bytesWritten < size)
                    {
                        int len = stream->readBytes(buffer, ((s > sizeof(buffer)) ? sizeof(buffer) : s));
                        file.write(buffer, len);
                        bytesWritten += len;
                    }
                    else if (!s && bytesWritten < size)
                    {}
                    else
                    {
                        break;
                    }
                    esp_task_wdt_reset();
                    yield();
                }
                file.close();
                http.end();
                events->send("done", "status", millis());
                ESP.restart();
            }
            else
            {
                http.end();
                events->send("error", "status", millis());
            }
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
    delete(events);
}
