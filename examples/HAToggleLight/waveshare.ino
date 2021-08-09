/***
 * dieses program liest sensorwerte von home assistant über dessen REST API und gibt die werte auf einem waveshare epaper aus
 * 2021 by martin schlatter, schwetzingen, germany
 */
 
#include <WiFi.h>
#include <HARestAPI.h>

#define ARDUINOJSON_DECODE_UNICODE 1
#include <ArduinoJson.h>

// According to the board, cancel the corresponding macro definition
#define LILYGO_T5_V213
// #define LILYGO_T5_V22
// #define LILYGO_T5_V24
// #define LILYGO_T5_V28
// #define LILYGO_T5_V102                           
// #define LILYGO_T5_V266
// #define LILYGO_EPD_DISPLAY         //T-Display 1.02 inch epaper   //Depend  https://github.com/adafruit/Adafruit_NeoPixel
// #define LILYGO_EPD_DISPLAY_154

#include <boards.h>
#include <GxEPD.h>
#include <SD.h>
#include <FS.h>
//#include "font1.h"

#if defined(LILYGO_T5_V102) || defined(LILYGO_EPD_DISPLAY)
#include <GxGDGDEW0102T4/GxGDGDEW0102T4.h> //1.02" b/w
#include <Adafruit_NeoPixel.h>             //Depend  https://github.com/adafruit/Adafruit_NeoPixel
#elif defined(LILYGO_T5_V266)
#include <GxDEPG0266BN/GxDEPG0266BN.h>    // 2.66" b/w   form DKE GROUP
#elif defined(LILYGO_T5_V213)
#include <GxDEPG0213BN/GxDEPG0213BN.h>    // 2.13" b/w  form DKE GROUP
#else
#endif

#include GxEPD_BitmapExamples

// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

GxIO_Class io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
GxEPD_Class display(io, EPD_RSET, EPD_BUSY);

#if defined(_HAS_SDCARD_) && !defined(_USE_SHARED_SPI_BUS_)
SPIClass SDSPI(VSPI);
#endif

#if defined(_GxGDEW0213Z16_H_) || defined(_GxGDEW029Z10_H_) || defined(_GxGDEW027C44_H_) ||defined(_GxGDEW0154Z17_H_) || defined(_GxGDEW0154Z04_H_) || defined(_GxDEPG0290R_H_)
#define _HAS_COLOR_
#endif

#if defined(LILYGO_EPD_DISPLAY)
Adafruit_NeoPixel strip(RGB_STRIP_COUNT, RGB_STRIP_PIN, NEO_GRBW + NEO_KHZ800);
#endif /*LILYGO_EPD_DISPLAY_102*/

WiFiClient client;
HARestAPI ha(client);

const char* ha_ip = "192.168.178.17";
uint16_t ha_port = 8123;
const char* ha_token = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3MiwiZXhwIjoxOTQzNzIzMDMwfQ.0WshlsKt3AB2DwrK3T3PgLm7ZRyj1a8NUfMTkNexJPg"; //long-lived password. On HA, Profile > Long-Lived Access Tokens > Create Token
const char* ssid = "UPC999999";
const char* password = "dsdkjkj23kjk2434";

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  ha.setHAServer(ha_ip, ha_port);
  ha.setHAPassword(ha_token);

#if defined(LILYGO_EPD_DISPLAY)
    pinMode(EPD_POWER_ENABLE, OUTPUT);
    digitalWrite(EPD_POWER_ENABLE, HIGH);
    delay(50);
    // strip test
    strip.begin();
    strip.show();
    strip.setBrightness(200);
    int i = 0;
    while (i < 5) {
        uint32_t color[] = {0xFF0000, 0x00FF00, 0x0000FF, 0x000000};
        strip.setPixelColor(0, color[i]);
        strip.show();
        delay(1000);
        i++;
    }
    strip.setPixelColor(0, 0);
    strip.show();
#endif /*LILYGO_EPD_DISPLAY*/
    
#if defined(LILYGO_EPD_DISPLAY_102)
    pinMode(EPD_POWER_ENABLE, OUTPUT);
    digitalWrite(EPD_POWER_ENABLE, HIGH);
#endif /*LILYGO_EPD_DISPLAY_102*/
#if defined(LILYGO_T5_V102)
    pinMode(POWER_ENABLE, OUTPUT);
    digitalWrite(POWER_ENABLE, HIGH);
#endif /*LILYGO_T5_V102*/
  

    SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);

    display.init();
    display.setTextColor(GxEPD_BLACK);
    display.setRotation(1);
}

String getSensor(String entity_id) {
  DynamicJsonDocument doc(1024);
  char jresult[501];
  
  // put your main code here, to run repeatedly:
  int len = ha.sendGetHA("/api/states/"+entity_id,"",jresult);
  DeserializationError err = deserializeJson(doc, jresult);
  if (err) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(err.c_str());
  }
  const char* state = doc["state"];
  //Serial.println(state);
  return state;
}

void loop() {
  display.fillScreen(GxEPD_WHITE);
  //display.setFont(&FreeMonoBold9pt7b);
  display.setFont(&FreeSans9pt7b);
  display.setCursor(0, 0);
  display.println();

  String s1 = getSensor("sensor.tuersensor1");
  String s2 = getSensor("sensor.tuersensor2");
  String s3 = getSensor("sensor.tuersensor3");
  String s4 = getSensor("sensor.tuersensor4");
  Serial.println("--------------------------");
  Serial.println(s1);
  Serial.println(s2);
  Serial.println(s3);
  Serial.println(s4);
  Serial.println("--------------------------");
  s1.replace("°","*");
  display.println(s1);
  display.println(s2);
  display.println(s3);
  display.println(s4);
  display.update();
  display.powerDown();
  delay(60000);
  
}

