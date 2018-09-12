#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoLog.h> // https://github.com/thijse/Arduino-Log/

#include "hashmap.h"

void scanWiFi(int roundNumber, IdMap *pIdMap)
{
    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Log.notice("WiFi scan done.\n");
    if (n == 0)
    {
        Log.notice("No networks found.\n");
    }
    else
    {
        Log.notice("%d networks found.\n", n);
        for (int i = 0; i < n; ++i)
        {
            std::string ssid = WiFi.SSID(i).c_str();
            int rssi = WiFi.RSSI(i);

            MapElement *element;
            if (pIdMap->count(ssid) == 0)
            {
                element = new WiFiMapElement();
                element->round = roundNumber;
                element->rssi = rssi;
                (*pIdMap)[ssid] = element;
            }
            else
            {
                element = (*pIdMap)[ssid];
                element->round = roundNumber;
                element->rssi = rssi;
            }
            int id = element->id;
            // Print SSID and RSSI for each network found
            Log.notice("%d: %s (%d)%s\n",
                       id, ssid.c_str(), rssi, (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
            delay(1);
        }
    }
}
