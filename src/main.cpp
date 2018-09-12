using namespace std;

#include <Arduino.h>

#define LOGLEVEL LOG_LEVEL_NOTICE

#include <ArduinoJson.h>
#include <ArduinoLog.h> // https://github.com/thijse/Arduino-Log/

#include <string.h>
#include <unordered_map>
#include "hashmap.h"

//#include "bt/include/esp_bt.h"
#include <BLEDevice.h>

#define JSON_BUFFER_SIZE 2048

#define REPORT_INTERVAL (15 * 1000) /* ms */

extern void setupWiFi();
extern void scanWiFi(int round, IdMap *idmap);
extern char *getTimestamp();

extern void setupMQTT();
extern void loopMQTT();
extern int publishJson(JsonObject &jsonObject);

extern void setupBLEAdvertise();
extern void setupBLEScanner();
extern void scanBLE(int round, IdMap *idmap);
extern void changeBLEDeviceName(const char *deviceName);

const char *getDeviceType()
{
  return "ESP32";
}

int MapElement::idNumber;
IdMap idmap;
int lastIdNumber;
int roundCount = 0;

void publishIdTuple(int num, const char *ssid, const char *type)
{
  StaticJsonBuffer<JSON_BUFFER_SIZE> jsonBuffer;
  JsonObject &tupleObject = jsonBuffer.createObject();

  tupleObject["ID"] = num;
  tupleObject["SSID"] = ssid;
  tupleObject["Type"] = type;

  // publish
  publishJson(tupleObject);
}

void addRSSI(JsonArray &rssiArray, int num, int rssi)
{
  JsonObject &object = rssiArray.createNestedObject();
  object["ID"] = num;
  object["RSSI"] = rssi;
}

void printObject(JsonObject &object)
{
  char buf[JSON_BUFFER_SIZE];
  object.printTo(buf);
  Serial.println(buf);
}

void publishRSSIs()
{
  StaticJsonBuffer<2048> jsonBuffer;
  JsonObject &rssiObject = jsonBuffer.createObject();
  rssiObject["Timestamp"] = getTimestamp();
  rssiObject["Round"] = roundCount;

  // new (id, ssid, type) tuple
  for (auto iter = idmap.begin(); iter != idmap.end(); iter++)
  {
    const char *ssid = (iter->first).c_str();
    MapElement *element = (MapElement *)(iter->second);
    if (element->id > lastIdNumber)
    { // new id
      publishIdTuple(element->id, ssid, element->type());
    }
  }

  // ID array
  JsonArray &idArray = rssiObject.createNestedArray("IDs");
  for (auto iter = idmap.begin(); iter != idmap.end(); iter++)
  {
    MapElement *element = (MapElement *)(iter->second);

    if (element->round == roundCount)
    {
      idArray.add(element->id);
    }
  }
  // RSSI array
  JsonArray &rssiArray = rssiObject.createNestedArray("nRSSIs");
  int num = 0;
  for (auto iter = idmap.begin(); iter != idmap.end(); iter++)
  {
    MapElement *element = (MapElement *)(iter->second);

    if (element->round == roundCount)
    { // updated
      rssiArray.add(-element->rssi);  // invert sign for short representation
      num++;
    }
  }
  rssiObject["Found"] = num;

  // publish
  publishJson(rssiObject);
}

void printIdMap()
{
  for (auto iter = idmap.begin(); iter != idmap.end(); iter++)
  {
    const char *ssid = (iter->first).c_str();
    MapElement *element = (MapElement *)(iter->second);

    Log.notice("id=%d, ssid=%s, round=%d, rssi=%d, type=%s\n",
               element->id, ssid, element->round, element->rssi, element->type());
  }
}

void setup()
{
  Serial.begin(115200);
  Log.begin(LOGLEVEL, &Serial);
  delay(2000);

  Log.notice("free heap=%d\n", ESP.getFreeHeap());
  Log.notice("setup WiFi\n");
  setupWiFi();

  Log.notice(">esp_bt_controller_mem_release free heap=%d\n", ESP.getFreeHeap());
  ::esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
  Log.notice("<esp_bt_controller_mem_release free heap=%d\n", ESP.getFreeHeap());

  Log.notice("setup BLE\n");
  setupBLEAdvertise();
  setupBLEScanner();

  Log.notice("setup MQTT");
  setupMQTT();

  Log.notice("setup done.\n");
}

int counter = 0;
long lastTime = millis() - REPORT_INTERVAL;

void loop()
{
  loopMQTT();

  long currentTime = millis();
  if (currentTime < (lastTime + REPORT_INTERVAL))
  {
    delay(100);
    return;
  }

  // round
  lastIdNumber = MapElement::getIdNumber();
  roundCount++;

  Log.notice("Round: %d, Timestamp: %s\n", roundCount, getTimestamp());

  String deviceName;
  deviceName = "MyESP32:";
  deviceName += counter++;

  changeBLEDeviceName(deviceName.c_str());
  Log.notice("deviceName: %s\n", deviceName.c_str());

  // scan
  scanBLE(roundCount, &idmap);
  scanWiFi(roundCount, &idmap);

  // print IdMap for Debugging
  // printIdMap();

  // publish scan results
  publishRSSIs();
}
