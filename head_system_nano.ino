#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <ArduinoJson.h>


const int MPU_addr = 0x68;  // I2C address of the MPU-6050
int16_t AcX, AcY, AcZ, GyX, GyY, GyZ, Tmp;
float ax = 0, ay = 0, az = 0, gx = 0, gy = 0, gz = 0, temp = 50;
unsigned long previousMillis = 0;  // will store last time LED was updated
unsigned long currentMillis;
float oxygen = 0, heartRate = 0;
//int data[STORE_SIZE][5]; //array for saving past data
//byte currentIndex=0; //stores current data array index (0-255)
boolean fall = false;      //stores if a fall has occurred
boolean trigger1 = false;  //stores if first trigger (lower threshold) has occurred
boolean trigger2 = false;  //stores if second trigger (upper threshold) has occurred
boolean trigger3 = false;  //stores if third trigger (orientation change) has occurred
String googleMapUrl = "https://www.google.com/maps/place/33.8938,35.5018";
String latitude = "";
String longitude = "";
byte trigger1count = 0;  //stores the counts past since trigger 1 was set true
byte trigger2count = 0;  //stores the counts past since trigger 2 was set true
byte trigger3count = 0;  //stores the counts past since trigger 3 was set true
int angleChange = 0;
int BLINKING_SPEED = 900;
boolean gpsConnected = false;
// int gpsLedState = LOW;
uint32_t tsLastReport = 0;
// int gpsLed = 12;                     //pin D6 where led is connected
float lat = 33.8938, lon = 35.5018;  // create variable for latitude and longitude object
boolean locationFound = false;
SoftwareSerial gpsSerial(3, 4);  //rx,tx
// SoftwareSerial SerialAT(2, 3);
SoftwareSerial espSerial(5, 6);
TinyGPS gps;  // create gps object
// SoftwareSerial SIM900(7, 8);

void setup() {
  // Serial.println("setting!");
  Serial.begin(9600);
  // pinMode(gpsLed, OUTPUT);
  // digitalWrite(gpsLed, gpsLedState);

  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  gpsSerial.begin(9600);  // connect gps sensor
  espSerial.begin(115200);

  // SIM900.begin(9600);
}
void onBeatDetected() {
  Serial.println("Beat!");
}
void loop() {
  mpu_read();
  //2050, 77, 1947 are values for calibration of accelerometer
  // values may be different for you

  ax = (AcX - 2050) / 16384.00;
  ay = (AcY - 77) / 16384.00;
  az = (AcZ - 1947) / 16384.00;

  //270, 351, 136 for gyroscope
  gx = (GyX + 270) / 131.07;
  gy = (GyY - 351) / 131.07;
  gz = (GyZ + 136) / 131.07;
  temp = (Tmp / 340) + 36.53;
  readTemp();
  // calculating Amplitute vactor for 3 axis
  float Raw_AM = pow(pow(ax, 2) + pow(ay, 2) + pow(az, 2), 0.5);
  int AM = Raw_AM * 10;  // as values are within 0 to 1, I multiplied
  // it by for using if else conditions

  Serial.print(F("amplitude vector for 3 axis: "));
  Serial.println(AM);

  // if (trigger3 == true) {
  //   trigger3count++;
  //   //Serial.println(trigger3count);
  //   if (trigger3count >= 2) {
  //     angleChange = pow(pow(gx, 2) + pow(gy, 2) + pow(gz, 2), 0.5);
  //     Serial.print(F("angle change level: "));
  //     Serial.println(angleChange);
  //     if ((angleChange >= 100) && (angleChange <= 450)) {  //if orientation changes remains between 0-10 degrees
  //       fall = true;
  //       trigger3 = false;
  //       trigger3count = 0;
  //       Serial.print(F("angle change level: "));
  //       Serial.println(angleChange);
  //     } else {  //user regained normal orientation
  //       trigger3 = false;
  //       trigger3count = 0;
  //       Serial.println(F("TRIGGER 3 DEACTIVATED"));
  //     }
  //   }
  // }
  // if (trigger2count >= 6) {  //allow 0.5s for orientation change
  //   trigger2 = false;
  //   trigger2count = 0;
  //   Serial.println(F("TRIGGER 2 DECACTIVATED"));
  // }
  // if (trigger1count >= 6) {  //allow 0.5s for AM to break upper threshold
  //   trigger1 = false;
  //   trigger1count = 0;
  //   Serial.println(F("TRIGGER 1 DECACTIVATED"));
  // }
  // if (trigger2 == true) {
  //   trigger2count++;
  //   //angleChange=acos(((double)x*(double)bx+(double)y*(double)by+(double)z*(double)bz)/(double)AM/(double)BM);
  //   angleChange = pow(pow(gx, 2) + pow(gy, 2) + pow(gz, 2), 0.5);
  //   Serial.print(F("angle change level: "));
  //   Serial.println(angleChange);
  //   if (angleChange >= 30 && angleChange <= 400) {  //if orientation changes by between 80-100 degrees
  //     trigger3 = true;
  //     trigger2 = false;
  //     trigger2count = 0;
  //     Serial.print(F("angle change level: "));
  //     Serial.println(angleChange);
  //     Serial.println(F("TRIGGER 3 ACTIVATED"));
  //   }
  // }
  // trigger2 = false;
  if (AM >= 15) {  //if AM breaks upper threshold (3g)
    // trigger2 = true;
    fall = true;
    Serial.println(F("Fall TRIGGER ACTIVATED"));
    // trigger1 = false;
    // trigger1count = 0;
    // }
  }
  
  gpsPeriodicUpdate();
  sendData();
  fall = false;

}

void gpsPeriodicUpdate() {
  Serial.println("------  location params are: " + String(lat) + ", " + String(lon));
  currentMillis = millis();
  if (!gpsConnected && currentMillis - previousMillis >= 1000) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

  }
  whilenected) {  // check for gps data
    if (gps.encode(gpsSerial.read()))               // encode gps data
    {
      Serial.println("reading gps values");
      //position current
      gps. (gpsSerial.available() && !gpsConf_get_position(&lat, &lon);  // get latitude and longitude
      latitude = String(lat, 6);
      longitude = String(lon, 6);

      googleMapUrl = "https://www.google.com/maps/place/" + latitude + "," + longitude;
      Serial.println("------  google map location: " + googleMapUrl);
      gpsConnected = true;
      // gpsLedState = HIGH;
      //update location on map
    }
  }
  if (lon > 0 && lat > 0) {
    gpsConnected = true;
    // gpsLedState = HIGH;
  } else {
    gpsConnected = false;
    // gpsLedState = LOW;
  }
  // digitalWrite(gpsLed, gpsLedState);
}


void sendData() {

  StaticJsonDocument<200> json;
  json["lon"] = lon;
  json["lat"] = lat;
  json["fall"] = fall;
  json["temp"] = temp;
  delay(3000);
  serializeJson(json, espSerial);
  Serial.println("json sent: ");
  char str[200];
  serializeJsonPretty(json, str);
  Serial.println(str);
}
void readTemp() {

  // Print the temperature in Celsius
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print(" \xC2\xB0");  // shows degree symbol
  Serial.print("C");
  Serial.print("\n");
}

void mpu_read() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true);  // request a total of 14 registers
  AcX = Wire.read() << 8 | Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY = Wire.read() << 8 | Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ = Wire.read() << 8 | Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp = Wire.read() << 8 | Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX = Wire.read() << 8 | Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY = Wire.read() << 8 | Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ = Wire.read() << 8 | Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
}



// void sendSMS(long heartRate) {
//   Serial.println("sending sms");
//   SIM900.print("AT+CMGF=1\r");
//   delayUpdate(100);

//   SIM900.println("AT+CMGS=\"+96171362075\"");
//   delayUpdate(100);

//   String info = "Temp: " + String(temp, 3) + " C\n"
//                 + "Heart rate: " + String(heartRate) + " \n"
//                 + "Oxygen: " + String(oxygen, 2) + " \n"
//                 + googleMapUrl;
//   SIM900.println(info);
//   delayUpdate(100);

//   SIM900.println((char)26);
//   delayUpdate(100);
//   SIM900.println();
// }

// void readHeartBeatSensor() {
//   if (millis() - tsLastReport > 1000) {
//     Serial.print("Heart rate:");
//     heartRate = pox.getHeartRate();
//     Serial.print(heartRate);
//     Serial.print("bpm / SpO2:");
//     oxygen = pox.getSpO2();
//     Serial.print(oxygen);
//     Serial.println("%");
//     tsLastReport = millis();
//   }
//   delayUpdate(100);
// }

void delayUpdate(int millis) {
  for (int i = 0; i < millis; i++) {
    // pox.update();
    delay(1);
  }
}