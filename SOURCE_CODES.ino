#define BLYNK_TEMPLATE_ID "TMPL2bb4JFm6a"
#define BLYNK_TEMPLATE_NAME "HOSPITAL"
#define BLYNK_AUTH_TOKEN "Ncn8UT1TYLp3vnBK77Gy36A-ybdB-rLe"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Keypad.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <DFRobotDFPlayerMini.h>

char ssid[] = "Pixel_2584"; // Replace with your WiFi SSID
char pass[] = "123456789M";    // Replace with your WiFi Password

SoftwareSerial mySerial(19, 18); // Declare pins TX and RX for DFPlayer
DFRobotDFPlayerMini myDFPlayer;  // Create a DFPlayer mini object

SoftwareSerial gsm(17, 16); // RX, TX pins for GSM module
String message;
int currentQueueNumber = 0;
String senderNumber;

LiquidCrystal_I2C lcd2(0x27, 16, 2); // I2C address, columns, rows
LiquidCrystal_I2C lcd(0x26, 16, 2); // Second LCD

const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns

char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {26, 27, 14, 12}; // Connect to the row pinouts of the keypad
byte colPins[COLS] = {13, 32, 33, 25}; // Connect to the column pinouts of the keypad

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

String phoneNumber = "";
int active;
int currentNumber = 1;
bool dfPlayerAvailable = false;

#define PUSH_BUTTON_PIN 4  // Define the pin number for the first push button
#define PUSH_BUTTON_PIN4 5 // Define the pin number for the second push button

void setup() {
  Serial.begin(115200);
  delay(2000); // Allow serial monitor to initialize

  // Initialize LCD and show waiting message
  lcd.init();
  lcd.backlight();
  lcd2.init();
  lcd2.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for WiFi");
  lcd.setCursor(0, 1);
  lcd.print("connection...");
  lcd2.print("NUMBER ON SERVICE: ");

  // Initialize WiFi connection
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi connected");
  
  // Guidance to turn on mobile data
  lcd.setCursor(0, 1);
  lcd.print("Turn on mobile");
  lcd.setCursor(0, 2);
  lcd.print("data to connect");

  // Initialize Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Blynk connected");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Blynk connected");

  Blynk.virtualWrite(V0, "THIS SECTION IS FOR IMPORTANT ANOUNCEMENTS: ");
  Blynk.virtualWrite(V1, 0);

  mySerial.begin(9600);
  if (!myDFPlayer.begin(mySerial)) {    // Use softwareSerial to communicate with MP3
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1. Please recheck the connection!"));
    Serial.println(F("2. Insert the SD card!"));
    dfPlayerAvailable = false;
  } else {
    Serial.println(F("DFPlayer Mini online."));
    myDFPlayer.volume(30); // Set volume level (0-30)
    delay(1000);
    dfPlayerAvailable = true;
  }
  gsm.begin(9600); // Initialize GSM module
  delay(1000); // Wait for the GSM module to initialize
  Serial.println("Initializing GSM module...");
  sendATCommand("AT");            // Check if GSM module is responding
  sendATCommand("AT+CMGF=1");     // Set SMS text mode
  sendATCommand("AT+CNMI=1,2,0,0,0"); // Set to receive live SMS
  Serial.println("Setup complete. Awaiting messages...");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter phone: ");
  lcd2.clear();
  lcd2.setCursor(0,0);
  lcd2.print("NUMBER ON SERVE: ");
  pinMode(PUSH_BUTTON_PIN, INPUT_PULLUP); // Configure the first button with internal pull-up resistor
  pinMode(PUSH_BUTTON_PIN4, INPUT_PULLUP); // Configure the second button with internal pull-up resistor
}

void loop() {
  Blynk.run(); // Continuously run Blynk to process incoming commands and maintain connection

  char customKey = customKeypad.getKey();
  if (customKey != NO_KEY) {
    handleKeyInput(customKey);
  }
  if (gsm.available() > 0) {
    message = gsm.readString();
    Serial.println("Received message: " + message);
    handleGSMInput(message);
  }
  // Handle push button inputs
  if (digitalRead(PUSH_BUTTON_PIN) == LOW) { // Check if the first button is pressed (LOW due to pull-up)
    callNextInQueue();
    delay(100); // Debounce delay
  } else if (digitalRead(PUSH_BUTTON_PIN4) == LOW) { // Check if the second button is pressed (LOW due to pull-up)
    handleQueueBegging();
    delay(100); // Debounce delay
  }

  // Upload data to Blynk
  updateBlynk();
}

void handleKeyInput(char customKey) {
  if (isDigit(customKey)) {
    phoneNumber += customKey;
    lcd.setCursor(0, 1); // Print on the second row
    lcd.print(phoneNumber);
  } else if (customKey == '*') {
    if (phoneNumber.length() > 0) {
      phoneNumber.remove(phoneNumber.length() - 1);
      lcd.setCursor(0, 1); // Clear the second row
      lcd.print(phoneNumber + " ");
    }
  } else if (customKey == '#') {
    if (phoneNumber.length() == 10) {
      currentQueueNumber++; // Increment queue number for the next person
      sendSMS(phoneNumber, "Your queue number is: " + String(currentQueueNumber));
      phoneNumber = "";
      lcd.clear();
      lcd.print("QUEUE NUMBER SENT: ");
      delay(1000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter phone: ");
      ToBLYNK();
    } else {
      lcd.clear();
      lcd.print("WRONG NUMBER");
      delay(2000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter phone: ");
      phoneNumber = ""; // Clear phone number to start over
    }
  }
}

void handleQueueBegging() { 
  currentQueueNumber++;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("YOUR QUEUE NO: ");
  lcd.setCursor(0, 1);
  lcd.print(currentQueueNumber);
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ENTER PHONE NO: ");
  ToBLYNK();
}

void handleGSMInput(String message) {
  if (message.indexOf("QUEUE") > -1) {
    int numberStartIndex = message.indexOf("+CMGR: ") + 7; // Find start of the message details
    numberStartIndex = message.indexOf("\"", numberStartIndex) + 1; // Find start of the phone number
    int numberEndIndex = message.indexOf("\"", numberStartIndex);
    senderNumber = message.substring(numberStartIndex, numberEndIndex);
    Serial.println("Sender Number: " + senderNumber);
    sendQueueNumberSMS(senderNumber);
  } else if (message.indexOf("STATUS") > -1) {
    int numberStartIndex = message.indexOf("+CMGR: ") + 7; // Find start of the message details
    numberStartIndex = message.indexOf("\"", numberStartIndex) + 1; // Find start of the phone number
    int numberEndIndex = message.indexOf("\"", numberStartIndex);
    senderNumber = message.substring(numberStartIndex, numberEndIndex);
    Serial.println("Sender Number: " + senderNumber);
    sendActivePeopleStatus(senderNumber);
  } else if (message.indexOf("+255772834603") > -1 ||
             message.indexOf("+255654400116") > -1 ||
             message.indexOf("+255747894092") > -1 ||
             message.indexOf("+255613328039") > -1 ||
             message.indexOf("+255627594372") > -1 ||
             message.indexOf("+255756368156") > -1) {
    int messageStartIndex = message.indexOf("\n") + 1; // Find the start of the actual message
    String actualMessage = message.substring(messageStartIndex);
    Serial.println("Actual Message: " + actualMessage);
    Blynk.virtualWrite(V0, "ANNOUNCEMENT: " + actualMessage); // Send the actual message to Blynk V1
  }
}

void callNextInQueue() {
  if(currentQueueNumber < currentNumber){
    ThereAreNoPeople();
  } else {
    ThereArePeople();
  } 
}

void ThereArePeople() {
  lcd2.clear();
  lcd2.setCursor(0, 0);
  lcd2.print("CALLING NUMBER: ");
  lcd2.setCursor(0, 1);
  lcd2.print(currentNumber);
  delay(3000); // Wait 3 seconds for the person to respond or move to the next person
  announceQueueNumber(currentNumber); // Announce the queue number
  delay(4000); // Wait 4 seconds for the person to respond or move to the next person
  NEXTtoBLYNK();
  currentNumber++; // Move to the next queue number
}

void ThereAreNoPeople() {
  lcd2.clear();
  lcd2.setCursor(0, 0);
  lcd2.print("NO BODY ON QUEUE ");
  delay(2000); 
  lcd2.clear();
  lcd2.setCursor(0, 0);
  lcd2.print("NUMBER ON SERVICE: ");
}

void announceQueueNumber(int currentNumber) {
  myDFPlayer.play(currentNumber);                   
  delay(2000);  
}

void sendQueueNumberSMS(String number) {
  currentQueueNumber++;
  sendSMS(number, "Your queue number is: " + String(currentQueueNumber));
  ToBLYNK();
}

void sendActivePeopleStatus(String number) {
  int active = currentQueueNumber - currentNumber + 1;
  sendSMS(number, "Active people in line: " + String(active));
}

void ToBLYNK() {
  int activePeople = currentQueueNumber - currentNumber + 1;
  Blynk.virtualWrite(V1, activePeople); // Upload active people in line to Blynk V1
}

void NEXTtoBLYNK() {
  int people = currentQueueNumber - currentNumber;
  Blynk.virtualWrite(V1, people); // Upload active people in line to Blynk V1
}

void sendSMS(String number, String message) {
  gsm.print("AT+CMGF=1\r");
  delay(100);
  gsm.print("AT+CMGS=\"" + number + "\"\r");
  delay(100);
  gsm.print(message);
  delay(100);
  gsm.write(26);
  delay(1000);
  Serial.println("SMS sent to: " + number);
}

void sendATCommand(String command) {
  gsm.print(command + "\r");
  delay(1000);
  while (gsm.available()) {
    Serial.print(gsm.read());
  }
}

void updateBlynk() {
  int activePeople = currentQueueNumber - currentNumber + 1;
  Blynk.virtualWrite(V1, activePeople); // Update active people in line on Blynk V1
}
