# ESP32-BME280

## What is this?

ESP32と温湿度・気圧センサモジュールのBME280で、部屋の状態を測定。  
IoTデータ可視化サービス "Ambient" でデータをグラフにします。

## 使い方
``` shell
$ git clone git@github.com:katapi2000/esp32-bme280
$ cd esp32-bme280
```
/esp32-iot/config.h を作成し、以下を記述する。
``` shell
//wifi
#define WIFI_SSID ""	//wifiのssid
#define WIFI_PASSWD ""	//wifiのパスワード

//ambient
#define CHANNELID xxxx	//チャネルID(xxxxを置き換える)
#define WRITE_KEY ""	//ライトキー
```

Ambientの使い方は、[こちら](https://ambidata.io/docs/gettingstarted/)に説明があります。