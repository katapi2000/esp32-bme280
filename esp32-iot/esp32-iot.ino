#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFiClientSecure.h>
#include <SPI.h>
#include "config.h"
WiFiClientSecure client;

//wifiのパスワードとか
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWD;

//google script
const char* server = "script.google.com";
const char* key = SCRIPT_ID;

//GPIOピンアサイン master = ESP32, slave = BME280
#define BME_CSB 26
#define BME_SDI 13  // master -> output, slave -> input
#define BME_SDO 12  // master -> input, slave -> output
#define BME_SCK 14

//SPI通信を使う(高速だし)
Adafruit_BME280 bme(BME_CSB, BME_SDI, BME_SDO, BME_SCK); // software SPI

void setup() {
  Serial.begin(115200);
  wifi();

  //BME280が接続されていないとき
  bool status;
  status = bme.begin(0x76);
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
}

void loop() {

  //wifi再接続
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting...");
    wifi();
    delay(1000);
  }
  printValues();
  send_to_google();
  delay(10000); //10秒毎に書き込みする
}

void wifi() {
  WiFiServer server(80);
  // Wi-Fiに接続
  Serial.println("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    // 接続するまで待機する
    delay(1000);
  }
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
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

void send_to_google() {
  String URL = "https://script.google.com/macros/s/";
  URL += key;
  URL += "/exec?";
  URL += "&1_cell=";
  URL += bme.readTemperature() ;
  URL += "&2_cell=";
  URL += bme.readHumidity() ;
  URL += "&3_cell=";
  URL += (bme.readPressure() / 100.0F) ;

  Serial.println(URL);
  // サイトにアクセス
  Serial.println("\nStarting connection to server...");
  if (!client.connect(server, 443))
    Serial.println("Connection failed!");
  else {
    Serial.println("Connected to server!\n");
    client.println("GET " + URL);
    client.stop();
    Serial.println("finish.");
  }
}
