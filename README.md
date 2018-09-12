# BLE_scan_adv_wifi
Platform IDE for Visual Studio Code を開発環境とした ESP32 Arduinoプログラム。

もう少し詳しい説明が[薫染庵 途上日誌](http://kunsen.net/)にある。

MH-ET Live ESP32 Minikit (ESP-WROOM-32)を用いて BLE の advertising packet の送受信を行うと共にWiFiの情報も収集する。収集した BLE および WiFiの RSSI測定データを AWS IoT に MQTT over Websocket で送る。

BLE central としての advertising packet の収集は一定時間おきに、一定の時間だけ行われる。常に advertising packet をスキャンするようになっているわけではない(不完全な)プログラムである。

WiFiManagerを用いて WiFi接続を行う。

収集したデータは内部的にはハッシュマップを用いて管理している。
SSID等の文字列データを一旦整数値に変換して、メッセージサイズを小さくするようにしている。


# 設定
aws-mqtt-websocket.cpp 中の次のインクルードファイルはこのリポジトリに含まれていない。
* myAWSus-east2.h

AWS IoTと通信するために以下のマクロを定義する必要がある。

* MY_AWS_REGION
* MY_AWS_ENDPOINT
* MY_AWS_IAM_KEY
* MY_AWS_IAM_SECRET_KEY

# ファイル
各ファイルの主な役割は以下の通り。

* main.cpp
    * BLEとWiFiの受信した信号の強度(RSSI)を、MQTT でAWS IoT に送るためのメインプログラム
    * [ArduinoJson](https://github.com/bblanchon/ArduinoJson)でJSONの処理をしている。
    * [ArduinoLog](https://github.com/thijse/Arduino-Log/)を使ってデバッグメッセージ出力を制御している。

* wifiManager.cpp
    * [WiFiManager](https://github.com/tzapu/WiFiManager/tree/development) を使って、WiFi接続をする

* aws-mqtt-websocket.cpp
    * AWS IoT と MQTT over Websocketを使って通信するためのコード
    * [aws-mqtt-websockets](https://github.com/odelot/aws-mqtt-websockets) を利用している

* BLEscan.cpp
    * BLE のアドバタイジングパケットのスキャンおよびアドバタイズを行う
    * 次のコードを基にしている
    https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLETests/Arduino/BLE_scan/BLE_scan.ino

* hashmap.h
    * 整数値のID、RSSI値などを管理するハッシュマップの要素の構造を定義している

* wifiScan.cpp
    * WiFi のアクセスポイントの SSID をスキャンし、RSSI などの情報を収集する

