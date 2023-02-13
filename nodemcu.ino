/*************************************************************

  This example shows how value can be pushed from Arduino to
  the Blynk App.

  NOTE:
  BlynkTimer provides SimpleTimer functionality:
    http://playground.arduino.cc/Code/SimpleTimer

  App project setup:
    Value Display widget attached to Virtual Pin V5
 *************************************************************/

// Template ID, Device Name and Auth Token are provided by the Blynk.Cloud
// See the Device Info tab, or Template settings
#define BLYNK_TEMPLATE_ID "TMPLXOPjzmve"
#define BLYNK_DEVICE_NAME "template1"
#define BLYNK_AUTH_TOKEN "rNSV28WBAPSwnBmwem93Bxg5D8k9v0FE"
#define I2C_BUS_SPEED 400000UL

// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <UrlEncode.h>
#include "Adafruit_GFX.h"
#include "OakOLED.h"
#include <ArduinoJson.h>
#define MAX30100_I2C_ADDRESS 0x57
char auth[] = BLYNK_AUTH_TOKEN;
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "micha";
char pass[] = "00000000";
// char ssid[] = "NO_CONNECTION!";
// char pass[] = "M27101992%";

int urgentLed = 0;
int pushButton = 16;
int wifiLed = 2;  //pin D3 where led is connected
// #define wifiLed D4
double lat = 0;
double lon = 0;
// double lat = 33.8938, lon = 35.5018;
boolean fall = false;

//variables
String googleMapUrl = "https://www.google.com/maps/place/33.8938,35.5018";
float oxygen = 0.00, heartRate = 0.00;
float temp = 20.18;
String phoneNumber = "+96176397142";  //96176397142
String apiKey = "3616061";            //3616061 //6261627
int BLINKING_SPEED = 900;
// constants won't change:
unsigned long currentMillis;
String FALL_DETECTION_EMAIL_BODY = "A fall is detected at: " + googleMapUrl;
PulseOximeter pox;
uint32_t tsLastReport = 0;

WidgetLCD lcd(V1);

void onBeatDetected() {
  // Serial.println("Beat!");
}

void setup() {

  Serial.begin(115200);
  pinMode(wifiLed, OUTPUT);
  pinMode(pushButton, INPUT);  //D0
  pinMode(urgentLed, OUTPUT);  //D3
  // Debug console
  // WiFi.begin(ssid, pass);
  // Serial.println("Connecting");

  // while (WiFi.status() != WL_CONNECTED) {
  //   digitalWrite(wifiLed, LOW);
  //   delayUpdate(BLINKING_SPEED);
  //   digitalWrite(wifiLed, HIGH);
  //   Serial.print(".");
  // }

  // Serial.println("");
  // Serial.print("Connected to WiFi network with IP Address: ");
  // Serial.println(WiFi.localIP());

  // while (!Serial) {
  //   ;  // wait for serial port to connect. Needed for native USB port only
  // }
  // pinMode(16, OUTPUT);
  Blynk.begin(auth, ssid, pass);

  if (!pox.begin()) {
    Serial.println(F("PulseOximeter failed begin"));
    for (;;)
      ;
  } else {
    Serial.println(F("PulseOximeter begin"));
    pox.setOnBeatDetectedCallback(onBeatDetected);
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
}

void loop() {
  pox.update();
  Blynk.run();

  if (digitalRead(pushButton) == HIGH) {
    digitalWrite(urgentLed, HIGH);
    sendWhatsapp();
  } else {
    digitalWrite(urgentLed, LOW);
  }
  if (millis() - tsLastReport > 1000) {
    Serial.print("Heart rate:");
    heartRate = pox.getHeartRate();
    Serial.print(heartRate);
    Serial.print(" bpm / SpO2:");
    oxygen = pox.getSpO2();
    Serial.print(oxygen);
    Serial.println("%");
    Blynk.virtualWrite(V2, oxygen);//send value to app
    Blynk.virtualWrite(V3, heartRate);
    tsLastReport = millis();
  }

  if (Serial.available()) {
    pox.update();
    StaticJsonDocument<200> dataObj;
    deserializeJson(dataObj, Serial);

    if (dataObj["lon"] > 0) {
      lon = dataObj["lon"];
    }
    if (dataObj["lat"] > 0) lat = dataObj["lat"];
    if (dataObj["temp"] > 0) {
      temp = dataObj["temp"];
    }
    fall = dataObj["fall"];
    pox.update();
    String longitude = String(lon, 6);
    String latitude = String(lat, 6);

    googleMapUrl = "google.com/maps/place/" + latitude + "," + longitude;
    dataObj["googleMapUrl"] = googleMapUrl;

    char str[200];
    Serial.println("received data object: ");
    serializeJsonPretty(dataObj, str);
    pox.update();
    Serial.println(str);
  }

  // delayUpdate(500);
  updateCloudInfo();
}


void reconnectBlynk() {
  if (!Blynk.connected()) {
    Serial.println("Lost connection");
    if (Blynk.connect()) Serial.println("Reconnected");
    else Serial.println("Not reconnected");
  }
}

void updateCloudInfo() {
  delayUpdate(150);
  if (temp > 0) {
    Blynk.virtualWrite(V0, temp);
    delayUpdate(150);
  }
  Blynk.virtualWrite(V2, oxygen);
  // delayUpdate(150);
  Blynk.virtualWrite(V3, heartRate);
  delayUpdate(150);
  lcd.clear();
  // delayUpdate(150);
  if (lon > 0 && lat > 0) {
    String line1 = String("lat: ") + lat;
    String line2 = String("lng: ") + lon;
    // delayUpdate(150);
    lcd.print(0, 0, line1);
    delayUpdate(150);
    lcd.print(0, 1, line2);
    //myMap.location(2, gps.location.lat(), gps.location.lng(), GPSLabel);
  } else {
    lcd.print(0, 0, "GPS lost");
  }
  delayUpdate(150);
  if (fall) {
    sendWhatsapp();
  }
  delayUpdate(150);
}


void sendWhatsapp() {
  delayUpdate(150);
  Serial.println("sending whatsapp msg");
  googleMapUrl = "google.com/maps/place/" + String(lat, 6) + "," + String(lon, 6);
  String info = "Heart: " + String(heartRate) + " bpm,"
                + "Oxygen: " + String(oxygen) + ", "
                + "Location: " + googleMapUrl;
  Serial.print("sms data: ");
  Serial.println(info);

  // Data to send with HTTP POST
  String url = "http://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&text=" + urlEncode(info) + "&apikey=" + apiKey;
  Serial.print("api url: ");
  Serial.println(url);
  WiFiClient client;
  HTTPClient http;
  http.begin(client, url);
  delayUpdate(150);
  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Send HTTP POST request
  int httpResponseCode = http.POST(url);
  delayUpdate(150);
  if (httpResponseCode == 200) {
    Serial.println("Message sent successfully");
  } else {
    Serial.println("Error sending the message");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
  }

  // delayUpdate(150);
  http.end();

  Blynk.logEvent("fall_detection", FALL_DETECTION_EMAIL_BODY);
  delayUpdate(150);
}

void delayUpdate(int millis) {
  for (int i = 0; i < millis; i++) {
    pox.update();
    delay(1);
  }
}