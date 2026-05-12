#ifndef WIFISETUP_H
#define WIFISETUP_H

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include "defs.hpp"

class wifisetup
{
    AsyncWebServer *server;
    Preferences settings;
    int n;
    String *w;
    bool *ws;
    String repl(const String &var);
    void resolve(AsyncWebServerRequest *request);

public:
    String pass;
    String ip;
    bool done = false;
    wifisetup();
    ~wifisetup();
};
#endif // WIFISETUP_H
