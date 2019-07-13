#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SPI.h>
#include <WiFi.h>
#include <Ambient.h>
#include "time.h"
#include "config.h"

WiFiClient client;
Ambient ambient;

//wifiのパスワードとか
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWD;

//ambient
unsigned int channelId = CHANNELID;
const char* writeKey = WRITE_KEY;

//NTPserver
const char* ntpServer = "ntp.jst.mfeed.ad.jp";    //日本のNTPサーバー
const long  gmtOffset_sec = 9 * 3600;   //時差9h
const int   daylightOffset_sec = 0;   //サマータイムなし

//GPIOピンアサイン master = ESP32, slave = BME280
#define BME_CSB 26
#define BME_SDI 13  // master -> output, slave -> input
#define BME_SDO 12  // master -> input, slave -> output
#define BME_SCK 14

//SPI通信を使う(高速だし)
Adafruit_BME280 bme(BME_CSB, BME_SDI, BME_SDO, BME_SCK); // software SPI

void setup() {
    Serial.begin(9600);

    bool status;
    
    // default settings
    // (you can also pass in a Wire library object like &Wire2)
    status = bme.begin(0x76);  
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }

    //wifi確認用LED
    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);
    
    wifi();

    //init and get the time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    //ambient送信初期化
    // |チャンネルID | ライトキー | WiFiClientの管理データ |
    ambient.begin(channelId, writeKey, &client);
}

void loop() { 
    //wifi接続したら点灯
    digitalWrite(2, HIGH);
    
    //wifi再接続
    while(WiFi.status() != WL_CONNECTED) {
      digitalWrite(2,LOW);  //切断したら消灯
      Serial.println("Reconnecting...");
      delay(1000);
      wifi();
    }
    printLocalTime();
    printValues();
    ambientSend();
    delay(500000);
    Serial.println();
}


void printValues() {
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" ℃");

    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");

    Serial.print("Pressure = ");
    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");
}

void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
}

void wifi() {
  Serial.printf("Connecting to %s", ssid);
  Serial.println();
  WiFi.begin(ssid, password); //wifi apに接続
  while(WiFi.status() != WL_CONNECTED) {  //wifi ap待機
    delay(1000);
    Serial.println("wait...");
  }

  Serial.print("WiFI connected\r IP adress: ");
  Serial.println(WiFi.localIP());
}

void ambientSend() {
  ambient.set(1, bme.readTemperature());  //1,温度
  ambient.set(2, bme.readHumidity()); //2,湿度
  ambient.set(3, bme.readPressure() / 100.0F);  //3,気圧

  ambient.send(); //送信
  Serial.println("sent data");
}

