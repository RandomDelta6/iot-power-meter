#include "MinimalWifiManager.h"

MinimalWifiManager::~MinimalWifiManager()
{
    this->server->end();
    this->filesystem.end();
}

String MinimalWifiManager::getSSID()
{
    return FileOperation::readFile(this->filesystem, SSID_PATH);
}

void MinimalWifiManager::setSSID(String ssid)
{
    this->_ssid = ssid;
    FileOperation::writeFile(this->filesystem, SSID_PATH, ssid);
}

String MinimalWifiManager::getPassword()
{
    return FileOperation::readFile(this->filesystem, PASS_PATH);
}

void MinimalWifiManager::setPassword(String pass)
{
    this->_pass = pass;
    FileOperation::writeFile(this->filesystem, PASS_PATH, pass);
}

bool MinimalWifiManager::autoConnect()
{
    /**
     * @brief
     * attempts connection - done
     * checks failure - done
     * if no connection details present - done
     * check ssid.txt and pass.txt - done
     * if wrong credential - done
     * stop station mode
     * start ap mode
     * take credentisls, attempts to connect
     * after successful connection save credential to ssid.txt and pass.txt
     */
    Serial.println("Attempting connection.");
    if (this->connectToStation(false))
    {
        Serial.println("Connection successful!");
        return true;
    }
    Serial.println("Connection failed, creating AP..");
    bool success = true;
    // this part executes if connection fails
    success &= WiFi.disconnect();
    success &= this->setApMode();
    this->bindServer();
    while (!this->_saved)
    { // busy-wait loop for synchronous approach
        this->loop();
    }
    success &= this->connectToStation(true);
    Serial.print("Connected to: ");
    if (WiFi.status() == WL_CONNECTED)
        ;
    Serial.println(WiFi.localIP());
    return success;
}

bool MinimalWifiManager::resetSettings()
{
    WiFi.disconnect();
    WiFi.persistent(false);
    this->_ssid.clear();
    this->_pass.clear();
    this->setSSID(this->_ssid);
    this->setPassword(this->_pass);
    this->_saved = false;
    Serial.println("Successfully erased configs!");
    return false; // FIXME: introduce success flag
}

bool MinimalWifiManager::bindServer()
{
    Serial.println("Binding server on \"/\", \"/scan\", \"/save\" routes");
    this->server->serveStatic("/", SPIFFS, "/");
    this->server
        ->on(
            "/",
            HTTP_GET,
            [this](AsyncWebServerRequest *req)
            { this->_to_scan = true;
              req->send(this->filesystem, "/index.html", "text/html"); });

    this->server
        ->on(
            "/scan",
            HTTP_GET,
            [this](AsyncWebServerRequest *req)
            {
                if (!this->numberOfNetworks)
                {
                    // req->send(200, "text/plain", n + " network(s) found");
                    req->send(200, "text/html", String(this->numberOfNetworks) + " network(s) found <br><br>" + scanResults);
                    _to_scan = true;
                }
                req->send(200, "text/plain", scanResults);
                this->_to_scan = true;
            });

    this->server
        ->on("/save",
             HTTP_POST,
             [this](AsyncWebServerRequest *req)
             {
                 int params = req->params();
                 for (int i = 0; i < params; i++)
                 {
                     AsyncWebParameter *param = req->getParam(i);
                     if (param->isPost())
                     {
                         if (param->name() == String("ssid"))
                         {
                             Serial.print("ssid: ");
                             this->_ssid = param->value();
                             Serial.println(_ssid);
                         }
                         if (param->name() == String("pass"))
                         {
                             Serial.print("pass: ");
                             this->_pass = param->value();
                             Serial.println(_pass);
                         }
                     }
                 }
                 req->send(200, "text/html", "{\"ssid\":\"" + _ssid + "\", \"pass\":\"" + _pass + "\"}"); // FIXME: create proper user-friendly html response
                 this->setSSID(_ssid);
                 this->setPassword(_pass);
                 this->_saved = true;
             });

    this->server->begin();
    return false; // FIXME: introduce success flag
}

bool MinimalWifiManager::setApMode()
{
    bool success = true;
    success &= WiFi.mode(WIFI_AP);
    success &= WiFi.softAP(AP_SSID, AP_PASS);
    return success;
}

bool MinimalWifiManager::connectToStation(bool disableAp = false)
{
    bool success = true;
    if (disableAp)
    {
        success &= WiFi.softAPdisconnect();
    }
    // take credentials from saved files
    if (this->_ssid == "") // variables not set
    {                      // fetch variables from files
        this->_ssid = this->getSSID();
        this->_pass = this->getPassword();
    }
    if (this->_ssid == "") // if ssid is unset
    {
        return false;
    }
    success &= WiFi.mode(WIFI_STA);
    success &= WiFi.begin(this->_ssid.c_str(), this->_pass.c_str());
    int attempts = 0;
    while (attempts < _MAX_ATTEMPTS && WiFi.status() != WL_CONNECTED)
    {
        attempts++;
        Serial.print("attempt: ");
        Serial.println(attempts);
        delay(1000);
    }
    if (attempts >= _MAX_ATTEMPTS && WiFi.status() != WL_CONNECTED) // wrong credentials or wifi not present
    {
        Serial.println("Attempts exceeded..");
        return false;
    }
    success &= WiFi.setAutoConnect(true);
    success &= WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    return true;
}

void MinimalWifiManager::loop()
{
    if (this->_to_scan)
    {
        this->numberOfNetworks = WiFi.scanNetworks();
        if (!this->numberOfNetworks)
        {
            return;
        }
        this->scanResults.clear();
        DynamicJsonDocument doc(1024);
        JsonArray aps = doc.to<JsonArray>();
        for (int i = 0; i < numberOfNetworks; ++i)
        {
            // Print SSID and RSSI for each network found
            JsonObject ap = aps.createNestedObject();
            ap["id"] = i + 1;
            ap["ssid"] = WiFi.SSID(i);
            ap["rssi"] = WiFi.RSSI(i);
            ap["enc"] = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*";
        }
        serializeJson(aps, this->scanResults);
        this->_to_scan = false;
    }
}

void MinimalWifiManager::begin(AsyncWebServer *serverObj, fs::SPIFFSFS &fs)
{
    this->server = serverObj;
    this->filesystem = fs;
    FileOperation::initFlashFileSystem(this->filesystem);
}