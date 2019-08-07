#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFiClientSecure.h>
#include <SPI.h>

//arduino ota
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
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

//スリープ時間の定義
#define uS_TO_S_FACTOR 1000000  //マイクロ秒から秒に変換
#define TIME_TO_SLEEP  300 //スリープ時間(秒)

//RTCメモリにスリープ回数を保存
RTC_DATA_ATTR int bootCount = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  ++bootCount;  //ブート回数を増やす
  Serial.println("Boot number: " + String(bootCount));  //ブート回数の表示
  wifi();

  //BME280が接続されていないとき
  bool status;
  status = bme.begin(0x76);
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  //####################
  //ArduinoOTAは削除しない
  //####################
  ArduinoOTA
  .onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  })
  .onEnd([]() {
    Serial.println("\nEnd");
  })
  .onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  })
  .onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();
  //####################

  //### main ###
  main_func();

  //ウェイクアップソースの定義
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  //スリープ実行
  Serial.println("sleep now");
  delay(1000);
  Serial.flush();
  esp_deep_sleep_start();
}

void loop() {
  
}

void main_func() {
  //####################
  //削除しない
  ArduinoOTA.handle();
  //####################

  //wifi再接続
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting...");
    wifi();
    delay(1000);
  }
  printValues();
  send_to_google();
}

void wifi() {
  WiFiServer server(80);
  // Wi-Fiに接続
  Serial.printf("Connecting to %s", ssid);
  WiFi.mode(WIFI_STA);  //wifi子機
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    // 接続するまで待機する
    delay(1000);
  }
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
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
