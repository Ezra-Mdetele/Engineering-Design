#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <LiquidCrystal_I2C.h>
#include <HX711.h>
#include <time.h>

// Pins
#define LOADCELL_DOUT_PIN 25
#define LOADCELL_SCK_PIN 23
#define LM35_PIN 34
#define IN1 5
#define IN2 15
#define IN3 18
#define IN4 19

// PWM Settings
#define PWM_CH 0
#define PWM_FREQ 5000
#define PWM_RES 8

// Globals
LiquidCrystal_I2C lcd(0x27, 20, 4);
HX711 scale;
WebServer server(80);

const char* ssid = "Pixel_2584";
const char* password = "123456789M";

float tempC = 0, weight = 0;

void setupWiFi() {
  WiFi.begin(ssid, password);
  lcd.setCursor(0, 0); lcd.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("WiFi OK");
  lcd.setCursor(0, 1); lcd.print(WiFi.localIP());
  delay(4000);
}

void setupNTP() {
  configTime(10800, 0, "pool.ntp.org", "time.nist.gov");
}

String getTimeString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "00:00:00";
  char buffer[10];
  strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
  return String(buffer);
}

String getDateString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "1970-01-01";
  char buffer[12];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeinfo);
  return String(buffer);
}

void logData() {
  File file = SPIFFS.open("/log.txt", FILE_APPEND);
  if (file) {
    file.printf("%s %s,%.1f°C,%.1fg,Speed:%d\n", getDateString().c_str(), getTimeString().c_str(), tempC, weight, 255);
    file.close();
  }
}

void handleRoot() {
  String html = R"rawliteral(
  <!DOCTYPE html><html><head>
    <title>ESP32 Conveyor Dashboard</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <script src="https://cdn.jsdelivr.net/npm/canvas-gauges/gauge.min.js"></script>
    <style>
      body { font-family: sans-serif; background: #f7f7f7; text-align: center; padding: 20px; }
      canvas { margin: 10px; }
      button { margin: 10px; padding: 10px 20px; font-size: 16px; }
    </style>
  </head><body>
    <h2>ESP32 Conveyor Dashboard</h2>

    <canvas id="tempGauge"></canvas>
    <canvas id="weightGauge"></canvas>
    <canvas id="speedGauge"></canvas>

    <h4>Time: <span id="time">--</span></h4>
    <h4>WiFi: <span id="status">--</span></h4>

    <form action="/on" method="post"><button>Motor ON</button></form>
    <form action="/off" method="post"><button>Motor OFF</button></form>
    <form action="/download"><button>Download Logs</button></form>
    <form action="/showlogs"><button>Show Logs</button></form>

    <script>
      var tempGauge = new RadialGauge({
        renderTo: 'tempGauge',
        width: 250,
        height: 250,
        units: "°C",
        title: "Temperature",
        minValue: 0,
        maxValue: 100,
        majorTicks: ["0", "10", "20", "30", "40", "50", "60", "70", "80", "90", "100"],
        minorTicks: 2,
        strokeTicks: true,
        highlights: [{ from: 0, to: 30, color: "lightblue" }, { from: 30, to: 60, color: "orange" }],
        colorPlate: "#fff",
        colorNeedle: "red",
        needleType: "arrow",
        needleEnd: 80,
        value: 0
      }).draw();

      var weightGauge = new RadialGauge({
        renderTo: 'weightGauge',
        width: 250,
        height: 250,
        units: "g",
        title: "Weight",
        minValue: 0,
        maxValue: 1000,
        majorTicks: ["0","100","200","300","400","500","600","700","800","900","1000"],
        minorTicks: 2,
        strokeTicks: true,
        colorNeedle: "blue",
        needleType: "arrow",
        needleEnd: 80,
        value: 0
      }).draw();

      var speedGauge = new RadialGauge({
        renderTo: 'speedGauge',
        width: 250,
        height: 250,
        units: "",
        title: "Motor Speed",
        minValue: 0,
        maxValue: 255,
        majorTicks: ["0","50","100","150","200","255"],
        minorTicks: 5,
        strokeTicks: true,
        colorNeedle: "green",
        needleType: "arrow",
        needleEnd: 80,
        value: 255
      }).draw();

      function updateData() {
        fetch("/data")
          .then(r => r.json())
          .then(d => {
            tempGauge.value = d.temp;
            weightGauge.value = d.weight;
            speedGauge.value = d.speed;
            document.getElementById("time").innerText = d.time;
            document.getElementById("status").innerText = d.status;
          });
      }

      setInterval(updateData, 2000);
      window.onload = updateData;
    </script>
  </body></html>
  )rawliteral";
  server.send(200, "text/html", html);
}


void handleData() {
  // Refresh values here instead of in loop()
  tempC = random(22, 29);  // or use analogRead(LM35_PIN)
  if (scale.is_ready()) weight = scale.get_units(5) + 3;

  String json = "{";
  json += "\"temp\":" + String(tempC) + ",";
  json += "\"weight\":" + String(weight) + ",";
  json += "\"time\":\"" + getDateString() + " " + getTimeString() + "\",";
  json += "\"speed\":255,";
  json += "\"status\":\"" + WiFi.localIP().toString() + "\"";
  json += "}";
  server.send(200, "application/json", json);
}


void handleMotorOn() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  delay(4000);
    digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  delay(2000);
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleMotorOff() {
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
    digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  //ledcWrite(PWM_CH, 0);
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleDownload() {
  File file = SPIFFS.open("/log.txt", "r");
  if (file) {
    server.streamFile(file, "text/plain");
    file.close();
  } else {
    server.send(404, "text/plain", "No log file");
  }
}

void handleShowLogs() {
  File file = SPIFFS.open("/log.txt", "r");
  if (!file) {
    server.send(404, "text/plain", "Log file not found");
    return;
  }
  String html = "<html><body><h2>Logs Table</h2><table border='1'><tr><th>Datetime</th><th>Temp</th><th>Weight</th><th>Speed</th></tr>";
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.replace(",", "</td><td>");
    html += "<tr><td>" + line + "</td></tr>";
  }
  html += "</table></body></html>";
  file.close();
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  lcd.init(); lcd.backlight();
  lcd.setCursor(0, 0); lcd.print("Booting...");

  WiFi.mode(WIFI_STA);
  setupWiFi();
  setupNTP();

  if (!SPIFFS.begin(true)) Serial.println("SPIFFS Failed");

  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
    pinMode(IN1, OUTPUT);
  //ledcSetup(PWM_CH, PWM_FREQ, PWM_RES);
  //ledcAttachPin(MOTOR_PWM_PIN, PWM_CH);
  //ledcWrite(PWM_CH, 255); // Full speed

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(); scale.tare();

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/on", HTTP_POST, handleMotorOn);
  server.on("/off", HTTP_POST, handleMotorOff);
  server.on("/download", handleDownload);
  server.on("/showlogs", handleShowLogs);
  server.begin();
}

void loop() {
  server.handleClient();

  // Simulate temperature reading (replace with analogRead if using LM35)
  tempC = random(22, 29);

  if (scale.is_ready()) {
    weight = scale.get_units(5) + 3;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:"); lcd.print(tempC); lcd.print("C ");
  lcd.print("W:"); lcd.print(weight, 0); lcd.print("g");
  
  lcd.setCursor(0, 1);
  lcd.print("Speed: 255 ");
  lcd.print(getTimeString());

  logData();
  handleMotorOn();
  delay(5000);
}
