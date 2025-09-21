#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ------------------- Pin Config -------------------
#define DHTPIN D4
#define DHTTYPE DHT11
#define MQ2_AO A0          // Analog output for 3.3V
#define BUZZER D8
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SMOKE_THRESHOLD 40  // Adjust after testing

// ------------------- Objects ----------------------
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ------------------- Variables -------------------
unsigned long buzzerStart = 0;
bool buzzerActive = false;
const unsigned long buzzerDuration = 10000; // 10 seconds

void setup() {
  Serial.begin(115200);
  dht.begin();

  if (!bmp.begin()) {
    Serial.println("Could not find BMP180 sensor!");
    while (1);
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 OLED not found!");
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  Serial.println("All sensors initialized...");
}

void loop() {
  // -------- Read Sensors --------
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) Serial.println("DHT11 failed!");

  float bmpTemp = bmp.readTemperature();
  float bmpPressure = bmp.readPressure() / 100.0;  // hPa

  int mq2Analog = analogRead(MQ2_AO);

  // -------- Buzzer Logic --------
  if (mq2Analog > SMOKE_THRESHOLD && !buzzerActive) {
    buzzerActive = true;
    buzzerStart = millis();
    digitalWrite(BUZZER, HIGH);
    Serial.println("Smoke Detected: YES");
  }

  // Turn off buzzer after 10 seconds
  if (buzzerActive && (millis() - buzzerStart >= buzzerDuration)) {
    buzzerActive = false;
    digitalWrite(BUZZER, LOW);
  }

  if (!buzzerActive && mq2Analog <= SMOKE_THRESHOLD) {
    Serial.println("Smoke Detected: NO");
  }

  // -------- Serial Monitor --------
  Serial.println("===== Sensor Readings =====");
  Serial.print("DHT11 -> Temp: "); Serial.print(t); Serial.print("C  Hum: "); Serial.println(h);
  Serial.print("BMP180 -> Temp: "); Serial.print(bmpTemp); Serial.print("C  Pressure: "); Serial.println(bmpPressure);
  Serial.print("MQ2 -> AO: "); Serial.println(mq2Analog);
  Serial.println("===========================");

  // -------- OLED Display --------
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Sensor Test");
  display.println("------------------");

  display.print("DHT11: "); display.print(t); display.print("C  Hum: "); display.println(h);
  display.print("BMP180: "); display.print(bmpTemp); display.print("C  Press: "); display.println(bmpPressure);

  display.print("Smoke: ");
  if (mq2Analog > SMOKE_THRESHOLD) display.println("YES");
  else display.println("NO");
  display.print("AO: "); display.println(mq2Analog);

  display.display();

  delay(1000); // 1-second loop
}
