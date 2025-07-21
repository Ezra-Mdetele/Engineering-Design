#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Password.h>
#define buzzer 9
#define relay 10
Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

String newPasswordString; //hold the new password
char newPassword[6]; //charater string of newPasswordString
byte a = 6;
bool value = true;

Password password = Password("1111"); //Enter your password

byte maxPasswordLength = 6;
byte currentPasswordLength = 0;
const byte ROWS = 4; // Four rows
const byte COLS = 3; // Four columns


char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'},
};


byte rowPins[ROWS] = {5, 6, 7, 8};
byte colPins[COLS] = {2, 3, 4};

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );


void setup() {
  Serial.begin(9600);
  pinMode(buzzer, OUTPUT);
  pinMode(relay,OUTPUT);
  servo.attach(11);
  servo.write(50);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("PASSWORD BASED");
  lcd.setCursor(0, 1);
  lcd.print("CIRCUIT BREAKER");
  Serial.println("PASSWORD BASED CIRCUIT BREAKER");
  delay(3000);
  lcd.clear();
}

void loop() {
  lcd.setCursor(1, 0);
  lcd.print("ENTER PASSWORD");
Serial.println("ENTER PASSWORD");
  char key = keypad.getKey();
  if (key != NO_KEY) {
    delay(60);
    if (key == '*') {
      resetPassword();
    } else if (key == '#') {
      if (value == true) {
        doorlocked();
        //value = false;
      } else if (value == false) {
        dooropen();
        //value = true;
      }
    } else {
      processNumberKey(key);
    }
  }
}

void processNumberKey(char key) {
  lcd.setCursor(a, 1);
  lcd.print(key);
  a++;
  if (a == 4) {
    a = 4;
  }
  currentPasswordLength++;
  password.append(key);

  if (currentPasswordLength == maxPasswordLength) {
    doorlocked();
  }
}

void dooropen() {
    digitalWrite(buzzer, HIGH);
    digitalWrite(relay,LOW);
    delay(200);
    digitalWrite(buzzer, LOW);
    delay(200);
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);
    delay(200);
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);
    delay(200);
    lcd.setCursor(0, 0);
    lcd.print("WRONG PASSWORD!");
    lcd.setCursor(0, 1);
    lcd.print("PLEASE TRY AGAIN");
    delay(2000);
    lcd.clear();
    a = 4;
  resetPassword();
}

void resetPassword() {
  password.reset();
  currentPasswordLength = 0;
  lcd.clear();
  a = 4;
}
void doorlocked() {
  if (password.evaluate()) {
    digitalWrite(buzzer, HIGH);
    digitalWrite(relay,HIGH);
    delay(300);
    digitalWrite(buzzer, LOW);
    servo.write(110);
    delay(100);
    lcd.setCursor(0, 0);
    lcd.print("CORRECT PASSWORD");
    lcd.setCursor(2, 1);
    lcd.print("ACCCESS GRANTED");
    delay(2000);
    lcd.clear();
    a = 4;
  }
  resetPassword();
}