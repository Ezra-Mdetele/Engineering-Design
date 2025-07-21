#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"

// === LCD & RTC Setup ===
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS1307 rtc;

// === LDR Pin ===
const int ldrPin = A3;

// === Traffic Light Pins (Grouped by Direction) ===
const int red2 = 5, amber2 = 6, green2 = 7;
const int red3 = 2, amber3 = 3, green3 = 4;
const int red4 = 8, amber4 = 9, green4 = 10;
const int red5 = A0, amber5 = A1, green5 = A2;

int ldrValue = 0;
int currentGroup = 1;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  lcd.init(); lcd.backlight();

  if (!rtc.begin()) {
    Serial.println("RTC not found! Halting.");
    while (1);
  }

  // Uncomment to set RTC time:
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  pinMode(ldrPin, INPUT);
  
  pinMode(red2, OUTPUT); pinMode(amber2, OUTPUT); pinMode(green2, OUTPUT);
  pinMode(red3, OUTPUT); pinMode(amber3, OUTPUT); pinMode(green3, OUTPUT);
  pinMode(red4, OUTPUT); pinMode(amber4, OUTPUT); pinMode(green4, OUTPUT);
  pinMode(red5, OUTPUT); pinMode(amber5, OUTPUT); pinMode(green5, OUTPUT);

 
}

void loop() {
  DateTime now = rtc.now();
  ldrValue = analogRead(ldrPin);
  int groupDuration = getGroupDuration(now, ldrValue);

  displayLCD(now, ldrValue, currentGroup, groupDuration);
 // getGroupDuration(DateTime now, int ldr);
  // Cycle through groups based on group numbera
  

}

int getGroupDuration(DateTime now, int ldr) {
  bool isDay = ldr > 500;
  if (isDay) {
   digitalWrite(red2,HIGH);
 digitalWrite(red3,HIGH);
 digitalWrite(red4,HIGH);
  digitalWrite(red2,HIGH);
 digitalWrite(red3,HIGH);
 digitalWrite(red4,HIGH);
  delay(5000);
 digitalWrite(red3,LOW);
 digitalWrite(green3,HIGH);
 delay(10000);
 digitalWrite(green3,LOW);
 digitalWrite(amber3,HIGH);
 delay(1000);
 digitalWrite(green3,LOW);
 digitalWrite(amber3,LOW);
 digitalWrite(red3,HIGH);
digitalWrite(red2,LOW);
delay(200);
digitalWrite(green2,HIGH);
delay(10000);
digitalWrite(green2,LOW);
digitalWrite(amber2,HIGH);
delay(2000);
digitalWrite(amber2,LOW);
digitalWrite(red2,HIGH);
digitalWrite(red4,LOW);
delay(200);
digitalWrite(green4,HIGH);
delay(20000);
digitalWrite(green4,LOW);
digitalWrite(amber4,HIGH);
delay(3000);
digitalWrite(amber4,LOW);
digitalWrite(red4,HIGH);
delay(200);
 
  } else {
     digitalWrite(red2,HIGH);
 digitalWrite(red3,HIGH);
 digitalWrite(red4,HIGH);
  delay(10000);
 digitalWrite(red3,LOW);
 digitalWrite(green3,HIGH);
 delay(10000);
 digitalWrite(green3,LOW);
 digitalWrite(amber3,HIGH);
 delay(1000);
 digitalWrite(green3,LOW);
 digitalWrite(amber3,LOW);
 digitalWrite(red3,HIGH);
digitalWrite(red2,LOW);
delay(200);
digitalWrite(green2,HIGH);
delay(10000);
digitalWrite(green2,LOW);
digitalWrite(amber2,HIGH);
delay(1000);
digitalWrite(amber2,LOW);
digitalWrite(red2,HIGH);
digitalWrite(red4,LOW);
delay(200);
digitalWrite(green4,HIGH);
delay(10000);
digitalWrite(green4,LOW);
digitalWrite(amber4,HIGH);
delay(1000);
digitalWrite(amber4,LOW);
digitalWrite(red4,HIGH);
//delay(200);
    return 30;  // night: fixed 30s for all
  }
}

void displayLCD(DateTime now, int ldr, int group, int duration) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(now.year()); lcd.print("/"); lcd.print(now.month()); lcd.print("/"); lcd.print(now.day());
  lcd.setCursor(0, 1);
  lcd.print("LDR:"); lcd.print(ldr);
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  lcd.print(now.hour()); lcd.print(":"); lcd.print(now.minute());
  lcd.setCursor(0, 1);
  lcd.print("Grp: "); lcd.print(group);
  lcd.print(" Dur: "); lcd.print(duration); lcd.print("s");
  delay(2000);
}
