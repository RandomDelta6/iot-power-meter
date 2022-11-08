#ifndef _MinimalWiFiManager_H_
#define _MinimalWiFiManager_H_

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <FileOperation.h>

#define WM_PORT 80
#define AP_SSID "Power IoT Meter"
#define AP_PASS "powermeter"

#define SSID_PATH "/ssid"
#define PASS_PATH "/pass"
#define _MAX_ATTEMPTS 10

class MinimalWifiManager
{
    AsyncWebServer *server;
    fs::SPIFFSFS filesystem;
    bool _to_scan = false, _saved = false;
    String scanResults = "";
    String _ssid = "", _pass = "";
    int numberOfNetworks = 0;
    bool bindServer();
    bool setApMode();
    bool connectToStation(bool disableAp);
    void setSSID(String ssid);
    void setPassword(String pass);

public:
    ~MinimalWifiManager();
    bool autoConnect();
    String getSSID();
    String getPassword();
    bool resetSettings();
    void loop();
    void begin(AsyncWebServer *serverObj, fs::SPIFFSFS &fs);
};

#endif