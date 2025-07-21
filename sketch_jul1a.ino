#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <UrlEncode.h>
#include <time.h>

#define DHTPIN 25
#define DHTTYPE DHT22
#define heater1 18
#define heater2 19
#define heater1_feedback 20 
#define heater2_feedback 4
#define currentSensorPin 34
#define CLOCK_INTERRUPT_PIN 27

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);
AsyncWebServer server(80);

float temperature = 0, humidity = 0, current = 0;
bool heater1Failed = false;
volatile bool toggleRequested = false;

const char* ssid = "Pixel_2584";
const char* password = "123456789M";
const char* http_username = "admin";
const char* http_password = "admin123";
String phoneNumber = "+255621789420";
String apiKey = "3828151";

String logData = "DateTime,Temp,Humidity,Current,Heater1,Heater2\n";

void IRAM_ATTR toggleHeater() {
  toggleRequested = true;
}

void sendMessage(String message) {
  String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);
  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST(url);
  Serial.println(httpResponseCode == 200 ? "Message sent successfully" : "Error sending message");
  http.end();
}

String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "00:00";
  char timeStr[16];
  strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
  return String(timeStr);
}

String getFormattedDate() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "1970-01-01";
  char dateStr[16];
  strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &timeinfo);
  return String(dateStr);
}

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
  delay(3000);
}
float getACcurrent1(int pin, int navg1);

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0); lcd.print("Initializing...");

  dht.begin();
  pinMode(heater1, OUTPUT);
  pinMode(heater2, OUTPUT);
  pinMode(heater1_feedback, INPUT);
  pinMode(heater2_feedback, INPUT);
  attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), toggleHeater, FALLING);

  if (!SPIFFS.begin(true)) Serial.println("SPIFFS mount failed");

  setupWiFi();
  configTime(10800, 0, "pool.ntp.org", "time.nist.gov");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
    if (!req->authenticate(http_username, http_password)) return req->requestAuthentication();
    String html = R"rawliteral(
<!DOCTYPE html><html><head>
  <title>Radiant Warmer Dashboard</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script src="https://cdn.jsdelivr.net/npm/canvas-gauges/gauge.min.js"></script>
  <style>
    body { font-family: sans-serif; background: #f7f7f7; text-align: center; padding: 20px; }
    canvas { margin: 10px; }
    .indicator { padding: 5px 10px; border-radius: 5px; margin: 5px; display:inline-block; }
    .on { background: green; color: white; }
    .off { background: gray; color: white; }
    table, th, td { border: 1px solid black; border-collapse: collapse; padding: 4px; }
  </style>
</head><body>
  <h2>Radiant Warmer Web Dashboard</h2>
  <canvas id="tempGauge"></canvas>
  <canvas id="humGauge"></canvas>
  <canvas id="currGauge"></canvas>
  <h4>Time: <span id="datetime">--</span></h4>
  <div>
    <span id="heater1state" class="indicator">Heater 1: Unknown</span>
    <span id="heater2state" class="indicator">Heater 2: Unknown</span>
  </div>
  <br><a href="/download">Download Logs</a>
  <h3>Log Data (latest 20 entries)</h3>
  <div id="logtable"></div>
  <script>
    var tempGauge = new RadialGauge({ renderTo: 'tempGauge', width: 250, height: 250, units: "°C", title: "Temperature", minValue: 0, maxValue: 60, majorTicks: ["0", "10", "20", "30", "40", "50", "60"], minorTicks: 2, highlights: [{ from: 0, to: 30, color: "lightblue" }, { from: 30, to: 45, color: "orange" }, { from: 45, to: 60, color: "red" }], needleType: "arrow", colorNeedle: "red", needleEnd: 80, value: 0 }).draw();
    var humGauge = new RadialGauge({ renderTo: 'humGauge', width: 250, height: 250, units: "%", title: "Humidity", minValue: 0, maxValue: 100, majorTicks: ["0", "20", "40", "60", "80", "100"], minorTicks: 2, colorNeedle: "blue", needleType: "arrow", needleEnd: 80, value: 0 }).draw();
    var currGauge = new RadialGauge({ renderTo: 'currGauge', width: 250, height: 250, units: "A", title: "Current", minValue: 0, maxValue: 10, majorTicks: ["0", "2", "4", "6", "8", "10"], minorTicks: 2, colorNeedle: "green", needleType: "arrow", needleEnd: 80, value: 0 }).draw();
    function fetchData() {
      fetch("/data").then(r => r.json()).then(d => {
        tempGauge.value = d.temp;
        humGauge.value = d.hum;
        currGauge.value = d.curr;
        document.getElementById("datetime").innerText = d.time;
        document.getElementById("heater1state").className = 'indicator ' + (d.h1 == 1 ? 'on' : 'off');
        document.getElementById("heater1state").innerText = 'Heater 1: ' + (d.h1 == 1 ? 'ON' : 'OFF');
        document.getElementById("heater2state").className = 'indicator ' + (d.h2 == 1 ? 'on' : 'off');
        document.getElementById("heater2state").innerText = 'Heater 2: ' + (d.h2 == 1 ? 'ON' : 'OFF');
      });
      fetch("/logs.txt").then(res => res.text()).then(text => {
        const rows = text.trim().split('\n').slice(-20);
        let html = "<table><tr>";
        const headers = rows[0].split(",");
        for (let h of headers) html += `<th>${h}</th>`;
        html += "</tr>";
        for (let i = 1; i < rows.length; i++) {
          html += "<tr>";
          let cells = rows[i].split(",");
          for (let c of cells) html += `<td>${c}</td>`;
          html += "</tr>";
        }
        html += "</table>";
        document.getElementById('logtable').innerHTML = html;
      });
    }
    setInterval(fetchData, 3000);
    window.onload = fetchData;
  </script>
</body></html>
)rawliteral";
    req->send(200, "text/html", html);
  });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *req) {
    String json = "{";
    json += "\"temp\":" + String(temperature) + ",";
    json += "\"hum\":" + String(humidity) + ",";
    json += "\"curr\":" + String(current) + ",";
    json += "\"h1\":" + String(digitalRead(heater1)) + ",";
    json += "\"h2\":" + String(digitalRead(heater2)) + ",";
    json += "\"time\":\"" + getFormattedDate() + " " + getFormattedTime() + "\"";
    json += "}";
    req->send(200, "application/json", json);
  });

  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *req) {
    if (!req->authenticate(http_username, http_password)) return req->requestAuthentication();
    req->send(SPIFFS, "/logs.txt", "text/plain");
  });

  server.begin();
}

void loop() {
  if (toggleRequested) toggleRequested = false;

  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  current = getACcurrent1(currentSensorPin, 300);
  heater1Failed = (digitalRead(heater1_feedback) == LOW);

  if ((temperature + current) < 20) {
    digitalWrite(heater1, LOW);
  } else {
    digitalWrite(heater1, HIGH);
  }

  if (temperature > 40) {
    digitalWrite(heater1, LOW);
    digitalWrite(heater2, LOW);
    sendMessage("[ALERT] Temperature exceeded 40C, heaters shut off.");
  } else if (heater1Failed || temperature < 37) {
    digitalWrite(heater1, LOW);
    digitalWrite(heater2, HIGH);
    sendMessage("[ALERT] Heater 1 failed or temp < 37C, switching to Heater 2.");
  } else {
    digitalWrite(heater2, LOW);
  }

  h
  lcd.print("T:"); lcd.print(temperature, 1); lcd.print("C H:"); lcd.print(humidity, 0);
  lcd.setCursor(0, 1);
  lcd.print("C:"); lcd.print(current, 2); lcd.print("A "); lcd.print(getFormattedTime());

  logData += getFormattedDate() + " " + getFormattedTime() + "," + String(temperature) + "," +
             String(humidity) + "," + String(current) + "," +
             String(digitalRead(heater1)) + "," + String(digitalRead(heater2)) + "\n";

  File file = SPIFFS.open("/logs.txt", FILE_APPEND);
  if (file) {
    file.print(logData);
    file.close();
    logData = "";
  }

  delay(3000);
}

float getACcurrent1(int pin, int navg1) {
  long acc = 0;
  for (int i = 0; i < navg1; i++) {
    long adc = analogRead(pin) - 512;
    acc += (adc * adc);
  }
  return sqrt(acc / navg1) * 0.0264;
}
