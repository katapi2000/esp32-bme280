#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SPI.h>
#include <WiFi.h>
#include "time.h"
#include "config.h"


//wifiのパスワードとか
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWD;

//NTPserver
const char* ntpServer = "ntp.jst.mfeed.ad.jp";    //日本のNTPサーバー
const long  gmtOffset_sec = 9 * 3600;   //時差9h
const int   daylightOffset_sec = 0;   //サマータイムなし



#define BME_CSB 26
#define BME_SDI 13
#define BME_SDO 12
#define BME_SCK 14

//Adafruit_BME280 bme; //I2C

//SPI通信を使う(高速だし)
//Adafruit_BME280 bme(BME_CSB); // hardware SPI
Adafruit_BME280 bme(BME_CSB, BME_SDI, BME_SDO, BME_SCK); // software SPI

void setup() {
    Serial.begin(9600);
    Serial.println(F("BME280 test"));

    bool status;
    
    // default settings
    // (you can also pass in a Wire library object like &Wire2)
    status = bme.begin(0x76);  
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }

    //init and get the time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    
    Serial.println("-- Default Test --");
    delay(1000);

    Serial.println();
}

void loop() { 
    //wifi再接続
    while(WiFi.status() != WL_CONNECTED) {
      digitalWrite(2,LOW);
      Serial.println("Reconnecting...");
      delay(1000);
      wifi();
    }
    printLocalTime();
    printValues();
    delay(3000);
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

    Serial.println();
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
  WiFi.begin(ssid, password); //wifi apに接続
  while(WiFi.status() != WL_CONNECTED) {  //wifi ap待機
    delay(1000);
    Serial.println("wait...");
  }

  Serial.print("WiFI connected\r\nIP adress: ");
  Serial.println(WiFi.localIP());
}

