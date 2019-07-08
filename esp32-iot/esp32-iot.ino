/*
MIT
Copyright (c) 2017 Mgo-tec
 */

#include <WiFi.h>
#include <Ambient.h>
#include "ESP32_BME280_SPI.h"
#include "config.h"

WiFiClient client;
Ambient ambient;

//wifiのパスワードとか
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWD;

//ambient
unsigned int channelId = CHANNELID;
const char* writeKey = WRITE_KEY;

//GPIOピンアサイン
const uint8_t SCLK_bme280 = 14;
const uint8_t MOSI_bme280 =13; //Master Output Slave Input ESP32=Master,BME280=slave 
const uint8_t MISO_bme280 =12; //Master Input Slave Output
const uint8_t CS_bme280 = 26; //CS pin

//初期化
ESP32_BME280_SPI bme280spi(SCLK_bme280, MOSI_bme280, MISO_bme280, CS_bme280, 10000000);

void setup(){
  digitalWrite(2,LOW);
  Serial.begin(115200);
  delay(1000); //Take some time to open up the Serial Monitor

  pinMode(2, OUTPUT);

  uint8_t t_sb = 5; //stanby 1000ms
  uint8_t filter = 0; //filter O = off
  uint8_t osrs_t = 4; //OverSampling Temperature x4
  uint8_t osrs_p = 4; //OverSampling Pressure x4
  uint8_t osrs_h = 4; //OverSampling Humidity x4
  uint8_t Mode = 3; //Normal mode
 
  bme280spi.ESP32_BME280_SPI_Init(t_sb, filter, osrs_t, osrs_p, osrs_h, Mode);
  delay(1000);

  wifi();

  ambient.begin(channelId, writeKey, &client);  //ambientの初期化
}

void loop(){
  digitalWrite(2,HIGH);
  //wifi再接続
  while(WiFi.status() != WL_CONNECTED) {
    digitalWrite(2,LOW);
    Serial.println("Reconnecting...");
    delay(1000);
    wifi();
  }
  bme_get();
  delay(300000);
}

//wifi
void wifi() {
  WiFi.begin(ssid, password); //wifi apに接続
  while(WiFi.status() != WL_CONNECTED) {  //wifi ap待機
    delay(1000);
    Serial.println("wait...");  
  }

  Serial.print("WiFI connected\r\nIP adress: ");
  Serial.println(WiFi.localIP());
}

//BME280 測定
void bme_get(){ 
  
  byte temperature = (byte)round(bme280spi.Read_Temperature());
  byte humidity = (byte)round(bme280spi.Read_Humidity());
  uint16_t pressure = (uint16_t)round(bme280spi.Read_Pressure());

  char temp_c[10], hum_c[10], pres_c[10];
  sprintf(temp_c, "%2d ℃", temperature);
  sprintf(hum_c, "%2d ％", humidity);
  sprintf(pres_c, "%4d hPa", pressure);

  Serial.println("-----------------------");
  Serial.print("Temperature = "); Serial.println(temp_c);
  Serial.print("Humidity = "); Serial.println(hum_c);
  Serial.print("Pressure = "); Serial.println(pres_c);
  Serial.println("-----------------------");
  
  Serial.println(temperature);
  Serial.println(humidity);
  Serial.println(pressure);
  
  Serial.flush();

  ambient.set(1, temperature);  //温度を1にセット
  ambient.set(2, humidity);   //湿度を2にセット
  ambient.set(3, pressure);   //気圧を3にセット

  ambient.send(); //送信
}
