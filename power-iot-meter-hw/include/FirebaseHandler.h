#ifndef _FIREBASE_HANDLER_H_
#define _FIREBASE_HANDLER_H_

#include <Firebase.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <util.h>
#include "FileOperation.h"

#define API F("AIzaSyA2ZWrZjc6WUU3f4L5bRcnbwtD-Yaf45Cw")
#define DB_URL F("power-iot-meter-default-rtdb.asia-southeast1.firebasedatabase.app")
#define EMAIL_DOM "@gmail.com"
#define CONTROL_TOPIC "controls"
#define UPDATE_DURATION 10000U // ms
#define UID_PATH "/fbuid"

class FirebaseHandler
{
public:
    String chipId;
    bool isConnected;

    void begin(fs::SPIFFSFS &filesystem)
    {
        this->filesystem = filesystem;
    }

    bool firebase_connect()
    {
        isConnected = connect_firebase();
        String uid = this->getUid();
        uid.trim();
        String prevUid = FileOperation::readFile(this->filesystem, UID_PATH);
        prevUid.trim();

        if (prevUid != uid) // if doesn't match
        {
            // uid update
            FileOperation::writeFile(this->filesystem, UID_PATH, uid);
        }
        return isConnected;
    }

    void firebase_listen(
        FirebaseData::StreamEventCallback streamCallback,
        FirebaseData::StreamTimeoutCallback streamTimeoutCallback)
    {
        __stream.setResponseSize(2048);
        if (!Firebase.RTDB.beginStream(&__stream, CONTROL_TOPIC))
        {
            Serial.printf("sream begin error, %s\n\n", __stream.errorReason().c_str());
            delay(5000);
        }
        Serial.println(F("Listening"));
        Firebase.RTDB.setStreamCallback(&__stream, streamCallback, streamTimeoutCallback);
    }

    void firebase_loop()
    {
        isConnected = Firebase.isTokenExpired();
        if (!isConnected)
        {
            connect_firebase();
        }

        if (millis() - __last > UPDATE_DURATION)
        {
            // TODO: updae packet from here
            __last = millis();
        }
    }

    String fetchData(String topic)
    {
        return Firebase.RTDB.getString<String>(&__stream, topic) ? __stream.to<String>() : __stream.errorReason().c_str(); // changes from getJSON and <FirebaseJson>
    }

    bool updateData(String topic, FirebaseJson *jsonObj)
    {
        return Firebase.RTDB.updateNode(&__stream, topic, jsonObj);
    }

    String getUid()
    {
        return __auth.token.uid.c_str();
    }

private:
    FirebaseConfig __config;
    FirebaseAuth __auth;
    FirebaseData __stream;
    bool __first_init = false;
    uint32_t __last = 0;
    fs::SPIFFSFS filesystem;

    void init_firebase_credentials()
    {
        __config.api_key = API;
        __config.database_url = DB_URL;
        __auth.user.email = chipId + String(EMAIL_DOM);
        __auth.user.password = chipId;
    }

    bool signUp()
    {
        return Firebase.signUp(
            &__config,
            &__auth,
            __auth.user.email,
            __auth.user.password);
    }

    bool stream_http_status()
    {
        return this->__stream.httpConnected();
    }

    int stream_http_code()
    {
        return this->__stream.httpCode();
    }

    String stream_error_reason()
    {
        return this->__stream.errorReason();
    }

    /**
     * @brief
     * this separate function will encapsulate repeating code later on during refactoring
     * @return true on success
     * @return false on failure
     */
    bool connect_firebase()
    {
        if (!__first_init)
        {
            chipId = getChipId();
            init_firebase_credentials();
            Firebase.reconnectWiFi(true);
            __config.token_status_callback = tokenStatusCallback;
            __config.max_token_generation_retry = 5;
            // Recommend for ESP8266 stream, adjust the buffer size to match your stream data size
#if defined(ESP8266)
            __stream.setBSSLBufferSize(512 /* Rx in bytes, 512 - 16384 */, 512 /* Tx in bytes, 512 - 16384 */);
#endif
            __first_init = true;
        }
        Firebase.begin(&__config, &__auth);
        Serial.println("Getting User UID");
        Serial.println(__auth.token.uid.c_str());
        if ((__auth.token.uid) != "") // auth uid present
            return true;

        Serial.println(F("Not present in database, signing up"));
        if (this->signUp())
        {
            Serial.println(__auth.token.uid.c_str());
            Firebase.begin(&__config, &__auth);
            return true;
        }

        Serial.println(F("User creation failed"));
        return false; // for recursive approach on the caller
    }
};
FirebaseHandler firebaseHandler;

#endif