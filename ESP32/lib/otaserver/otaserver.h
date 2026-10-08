#ifndef OTASERVER_H
#define OTASERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SD.h>
#include <Preferences.h>
#include "defs.hpp"

class otaserver
{
    String sn;
    AsyncWebServer *server;
    Preferences settings;
    String repl(const String &var);
    void upload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    void resolve(AsyncWebServerRequest *request);

public:
    otaserver();
    String tem = "Not initialized";
    String hum = "Not initialized";
    String lig = "Not initialized";
    String bat = "Not initialized";
    String ssid = "Not initialized";
    String ip = "Not initialized";
    String rssi = "Not initialized";
    bool bauto;
    int brightness;
    int drefresh;
    int pswitch;
    int sensors;
    int send;
    int warning;
    ~otaserver();
};
#endif // OTASERVER_H
