#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ------------------- Pin Config -------------------
#define DHTPIN D4
#define DHTTYPE DHT11
#define MQ2_AO A0
#define BUZZER D8
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SMOKE_THRESHOLD 40  // Adjust after testing

// ------------------- Objects ----------------------
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

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
  display.setTextSize(2);  // Bigger font
  display.setTextColor(SSD1306_WHITE);

  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  Serial.println("Sensors initialized...");
}

void loop() {
  // -------- Read Sensors --------
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float bmpPressure = bmp.readPressure() / 100.0;  // hPa
  int mq2Analog = analogRead(MQ2_AO);

  if (isnan(h) || isnan(t)) {
    Serial.println("DHT11 failed!");
    return;
  }

  // -------- Rain Probability --------
  int rainProb = map((int)h + (1013 - (int)bmpPressure), 0, 200, 0, 100);
  if (rainProb < 0) rainProb = 0;
  if (rainProb > 100) rainProb = 100;

  // -------- Smoke Detection (Buzzer only) --------
  if (mq2Analog > SMOKE_THRESHOLD) {
    digitalWrite(BUZZER, HIGH);
    Serial.println("Smoke Detected!");
    delay(5000); 
  } else {
    digitalWrite(BUZZER, LOW);
  }

  // -------- OLED Display --------
  display.clearDisplay();
  display.setCursor(0, 0);

  display.print("T:"); display.print(t); display.println("C");
  display.print("H:"); display.print(h); display.println("%");
  display.print("P:"); display.print(bmpPressure); display.println("h");
  display.print("R:"); display.print(rainProb); display.println("%");

  display.display();

  delay(2000); // Update every 2s
}
