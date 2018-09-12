/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
   Modified by Kunsen-an
*/
#include <Arduino.h>

#define LOGLEVEL LOG_LEVEL_NOTICE

#include <ArduinoLog.h> // https://github.com/thijse/Arduino-Log/

#include "hashmap.h"

extern char *getTimestamp();

#define SCAN_INTERVAL 10 /* in second */

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEServer.h>

BLEScan *pBLEScan;
BLEServer *pServer;
BLEAdvertising *pAdvertising;

extern void BLEDevice_init(std::string deviceName);
String deviceName = "MyESP32";

void setupBLEAdvertise()
{
	Log.notice("Start BLE Advertising\n");

	BLEDevice::init(deviceName.c_str());
	Log.notice("BLEDevice::init done '%s'\n", deviceName.c_str());
	pServer = BLEDevice::createServer();
	Log.notice("BLEDevice::createServer() done.'\n");
	pAdvertising = pServer->getAdvertising();
	Log.notice("getAdvertising() done.'\n");
	pAdvertising->start();
	Log.notice("pAdvertising->start() done.'\n");
}

int scanTime = SCAN_INTERVAL; //In seconds

extern int roundCount;
IdMap *pIdMap = NULL;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
	void onResult(BLEAdvertisedDevice advertisedDevice)
	{
		std::string ssid = advertisedDevice.getAddress().toString().c_str();
		int rssi = advertisedDevice.getRSSI();
		std::string name = advertisedDevice.getName().c_str();

		Log.notice("onResult advertisedDevice=%s\n", advertisedDevice.toString().c_str());

		MapElement *element;
		if (pIdMap->count(ssid) == 0)
		{
			element = new BLEMapElement();
			element->round = roundCount;
			element->rssi = rssi;
			(*pIdMap)[ssid] = element;
		}
		else
		{
			element = (*pIdMap)[ssid];
			element->round = roundCount;
			element->rssi = rssi;
		}
		int id = element->id;
		Log.notice("Advertised Device: %d '%s' '%s' RSSI: %d\n", id, ssid.c_str(), name.c_str(), rssi);
	}
};

void setupBLEScanner()
{
	pBLEScan = BLEDevice::getScan(); //create new scan
	pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
	Log.notice("BLE Scanner setup done.\n");
}

void scanBLE(int roundnumber, IdMap *idMap)
{
	roundCount = roundnumber;
	pIdMap = idMap;
	Log.notice("BLE Scanning Round: %d\n", roundCount);

	BLEScanResults foundDevices = pBLEScan->start(scanTime);
	Log.notice("Devices found: %d\n", foundDevices.getCount());

	Log.notice("BLE Scan done.\n");
}

void changeBLEDeviceName(const char *deviceName)
{
	Log.notice("deviceName: %s\n", deviceName);
	esp_err_t errRc;
	errRc = ::esp_ble_gap_set_device_name(deviceName);
	if (errRc != ESP_OK)
	{
		Log.warning("Cannot change deviceName errRC=%d\n", errRc);
		return;
	};
	pAdvertising->start();
}

#include <esp_err.h>
#include <nvs_flash.h>
#include <esp_bt.h>				 // ESP32 BLE
#include <esp_bt_device.h>		 // ESP32 BLE
#include <esp_bt_main.h>		 // ESP32 BLE
#include <esp_gap_ble_api.h>	 // ESP32 BLE
#include <esp_gatts_api.h>		 // ESP32 BLE
#include <esp_gattc_api.h>		 // ESP32 BLE
#include <esp_gatt_common_api.h> // ESP32 BLE
#include <esp_err.h>			 // ESP32 ESP-IDF
//#include <esp_log.h>           // ESP32 ESP-IDF
#include "BLEDevice.h"
#include "BLEClient.h"
#include "BLEUtils.h"
#include "GeneralUtils.h"

/**
 * @brief Initialize the %BLE environment.
 * @param deviceName The device name of the device.
 */
void BLEDevice_init(std::string deviceName)
{
	esp_err_t errRc = ESP_OK;

	if (!btStart())
	{
		errRc = ESP_FAIL;
		return;
	}

	esp_bluedroid_status_t bt_state = esp_bluedroid_get_status();
	if (bt_state == ESP_BLUEDROID_STATUS_UNINITIALIZED)
	{
		errRc = esp_bluedroid_init();
		if (errRc != ESP_OK)
		{
			Log.error("esp_bluedroid_init: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
			return;
		}
	}

	if (bt_state != ESP_BLUEDROID_STATUS_ENABLED)
	{
		errRc = esp_bluedroid_enable();
		if (errRc != ESP_OK)
		{
			Log.error("esp_bluedroid_enable: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
			return;
		}
	}

	errRc = ::esp_ble_gap_set_device_name(deviceName.c_str());
	if (errRc != ESP_OK)
	{
		Log.error("esp_ble_gap_set_device_name: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	};
}