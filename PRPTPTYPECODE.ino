//YWROBOT
//Compatible with the Arduino IDE 1.0
//Library version:1.1
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>    
#include <HTTPClient.h>
#include <UrlEncode.h>
int sensorPin = 34;
int relay= 5;
//int buzzer= 11;
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
const char* ssid = "iPhone";
const char* password = "mkandama";
String phoneNumber = "+255612777321";
String apiKey = "https://api.callmebot.com/whatsapp.php?phone=255612777321&text=This+is+a+test&apikey=6855444";
void sendMessage(String message){

  // Data to send with HTTP POST
  String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);    
  HTTPClient http;
  http.begin(url);

  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  // Send HTTP POST request
  int httpResponseCode = http.POST(url);
  if (httpResponseCode == 200){
    Serial.print("Message sent successfully");
  }
  else{
    Serial.println("Error sending the message");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  http.end();
}
void setup()
{
   pinMode(relay,OUTPUT);
   Serial.begin(115200);
  //pinMode(sensorPin,INPUT);
  //pinMode(buzzer,OUTPUT);
  lcd.init();                      // initialize the lcd 
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  
  lcd.setCursor(0,0);
  lcd.print("SMART BUBBLE");
  lcd.setCursor(2,1);
  lcd.print("REMOVER");
 
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  // Send Message to WhatsAPP
 
   delay(1000);
   lcd.clear();
  
}
void loop()
{
  int sensorValue = analogRead(sensorPin);
  Serial.println(sensorValue);
  delay(10);
  if(sensorValue>1700)
  {
    digitalWrite(relay,HIGH);
    //digitalWrite(buzzer,HIGH);
     lcd.setCursor(0,0);
  lcd.print("TRANSFUSSION: NOT OK  ");
  lcd.setCursor(0,1);
  lcd.print("STATUS: DETECTED     ");
  delay(6000);
  lcd.clear();
  lcd.setCursor(0,0);
   lcd.setCursor(0,3);
  lcd.print("REMOVING BUBBLE");
  sendMessage("TRANSFUSION NOT OKAY, BUBBLE DETECTED");
  delay(6000);
  lcd.clear();
   
  }
  else
  {
     lcd.setCursor(0,0);
  lcd.print("TRANSFUSSION: OK        ");
  lcd.setCursor(0,1);
  lcd.print("STATUS: NOT DETECTED  ");
  sendMessage("TRANSFUSION  OKAY, BUBBLE NOT DETECTED");
  delay(1000);
  lcd.clear();
//digitalWrite(buzzer,LOW);
   digitalWrite(relay,LOW);
  }
  
}
