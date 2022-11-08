#include <Arduino.h>
#include <SPI.h>
#include <SPIFFS.h>
#include <MinimalWifiManager.h>
#include <ArduinoJson.h>
#include <FirebaseHandler.h>
#include <HardwarePropModule.h>

#define DEBUGGING

#define SCHEDULE(and_logic, num, delay) \
  if (and_logic && (millis() - last_time[num] > delay))
#define SCHEDULES 2
#define TSK_FETCH 200 // ms
#define TSK_SYNC 5000

String fetchPowerOutagesFromFirebase();
void pushStatsToFirebase(packet_t *);

AsyncWebServer server(80);
MinimalWifiManager wifi;
String power_outages;
packet_t data;
bool power_cut_occurred = false;

void setup()
{
  Serial.begin(115200);
  Serial.println("setup()");
  wifi.begin(&server, SPIFFS);
  // // wifi.resetSettings();
  wifi.autoConnect();
  set_time_from_ntp();
  firebaseHandler.begin(SPIFFS);
  firebaseHandler.firebase_connect();
  // attach();
  // FileOperation::listDir(SPIFFS);
  Serial.println("Exiting setup().. \nEntering loop()...");
}

void loop()
{
  static unsigned long last_time[SCHEDULES] = {0};
  wifi.loop();
  /**
   * @brief
   * Create a scheduler for Firebase fetch and push
   * fetch
   * calculate
   * push
   *
   */
  SCHEDULE(true, 0, TSK_FETCH)
  {
    bool power_good = fetchSensorData(&data);
    if (power_good)
    {
      if (power_cut_occurred)
      {
        Serial.println("Power restored...");
        String end_time = getTimestamp();
        String start_time = powerCutTimeFetch(SPIFFS);
        FirebaseJson json;
        json.setJsonData(fetchPowerOutagesFromFirebase());
        json.add<String, String>(start_time, end_time);
        firebaseHandler.updateData("/" + firebaseHandler.getUid() + "/power_outages", &json);
        power_cut_occurred = false;
      }
    }
    else
    {
      if (!power_cut_occurred)
      {
        Serial.println("Power cut occurred...");
        powerCutTimeStore(SPIFFS);
        power_cut_occurred = true;
      }
    }
    last_time[0] = millis();
  }

  SCHEDULE(true, 1, TSK_SYNC)
  {
    if(!power_cut_occurred)
      pushStatsToFirebase(&data);
    last_time[1] = millis();
  }
  // // on power cut
  // if (checkPowerCut() && !power_cut_occurred)
  // {
  //   Serial.println("Power cut occurred...");
  //   powerCutTimeStore(SPIFFS);
  //   power_cut_occurred = true;
  // }

  // // on power restore
  // if (power_cut_occurred && !checkPowerCut())
  // {
  //   Serial.println("Power restored...");
  //   String end_time = getTimestamp();
  //   String start_time = powerCutTimeFetch(SPIFFS);
  //   FirebaseJson json;
  //   json.setJsonData(fetchPowerOutagesFromFirebase());
  //   json.add<String, String>(start_time, end_time);
  //   firebaseHandler.updateData("/" + firebaseHandler.getUid() + "/power_outages", &json);
  //   power_cut_occurred = false;
  // }
}

String fetchPowerOutagesFromFirebase()
{
  power_outages = firebaseHandler.fetchData("/" + firebaseHandler.getUid() + "/power_outages");
#ifdef DEBUGGING
  Serial.println(power_outages);
#endif
  return power_outages;
}

void pushStatsToFirebase(packet_t *data)
{
  FirebaseJson json;
  // prepare to send all the data
  json.add<String, double>("apparent_power", data->apparent_power);
  json.add<String, double>("active_power", data->active_power);
  json.add<String, double>("voltage_rms", data->voltage_rms);
  json.add<String, double>("current_rms", data->current_rms);
  json.add<String, double>("power_factor", data->power_factor);
  json.add<String, double>("frequency", data->frequency);
  json.add<String, double>("total_energy", data->energy);
  json.add<String, String>("timestamp", data->timestamp);
  bool success = firebaseHandler.updateData("/" + firebaseHandler.getUid(), &json);
  Serial.println(success);
#ifdef DEBUGGING
  Serial.println(json.raw());
#endif
}

void updatePowerOutageLog(); // prepare json, check meter1 schema for reference