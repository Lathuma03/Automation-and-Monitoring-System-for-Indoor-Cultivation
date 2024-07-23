#include <FirebaseESP32.h>
#include <Wire.h>
#include <WiFi.h>
#include "DHT.h"
#include <LiquidCrystal_I2C.h>

#define FIREBASE_HOST "https://mushroom-indoor-farming-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define WIFI_SSID "vivo Y21"
#define WIFI_PASSWORD "lathuma@123"
#define FIREBASE_AUTH "AIzaSyBrmo-ASK3u62-o2-3-4biLSI_wXpZZLPs"
#define DHTPIN 14 // Pin number
#define DHTTYPE DHT11 // Sensor type

DHT dht(DHTPIN, DHTTYPE);
FirebaseData firebaseData; // Firebase object
int relay1 = 33; // Relay pins
int relay2 = 25; // Relay pins
int relay3 = 26; // Relay pins
int relay4 = 27; // Relay pins

// LCD Display Setup
LiquidCrystal_I2C lcd(0x3F, 16, 2);

void setup() {
  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  dht.begin();

  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);

  // Initialize the LCD
  lcd.init();
  lcd.backlight();
}

void loop() {
  float humidity = dht.readHumidity() - 3;
  float temperature = dht.readTemperature() - 1;

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    Firebase.setString(firebaseData, "/farmlink/live/ERROR", "Failed to read from DHT sensor!");
    delay(5000);
  } else {
    Serial.print("temperature: ");
    Serial.print(temperature, 3);
    Serial.print("°C");
    Serial.print(" humidity: ");
    Serial.print(humidity, 3);
    Serial.print("%");
    Serial.println();

    // Update live readings
    Firebase.setFloat(firebaseData, "/farmlink/live/temperature", temperature);
    Firebase.setFloat(firebaseData, "/farmlink/live/humidity", humidity);

    // Check emergency condition
    bool isEmergencyOn = false; // Default: emergency is off

    // Check emergency status from Firebase
    Firebase.getString(firebaseData, "/farmlink/live/emergency");
    if (firebaseData.dataType() == "string") {
      isEmergencyOn = (firebaseData.stringData() == "ON");
    }

    // Update emergency status in Firebase
    Firebase.setString(firebaseData, "/farmlink/live/emergency", isEmergencyOn ? "ON" : "OFF");

    // Control devices based on conditions
    bool isSprinklerOn = false;
    bool isCoolerOn = false;
    bool isExhaustOn = false;
    bool isPumpOn = false;

    if (isEmergencyOn) {
      // If emergency is on, turn off all relays
      isSprinklerOn = false;
      isCoolerOn = false;
      isExhaustOn = false;
      isPumpOn = false;
    } 
    else {
      // Normal operation conditions
      if (temperature > 28 && humidity < 70) {
        isSprinklerOn = true;
        isCoolerOn = true;
        isExhaustOn = true;
        isPumpOn = true;
        Serial.println("Sprinkler, Cooler, and Exhaust are ON");
        delay(4000);  
      } else if (temperature > 30 && (humidity > 75 || humidity < 70)) {
        isSprinklerOn = true;
        isCoolerOn = true;
        isExhaustOn = true;
        isPumpOn = true;
        Serial.println("Sprinkler, Cooler, and Exhaust are ON");
        delay(4000);
      } else if (temperature > 23 && temperature < 25) {
        isSprinklerOn = false;
        isCoolerOn = false;
        isExhaustOn = false;
        isPumpOn = false;
        Serial.println("Cooler and Sprinkler are OFF");
        delay(4000);
      }
    }

    // Update relay statuses
    digitalWrite(relay1, isSprinklerOn ? LOW : HIGH);
    digitalWrite(relay2, isCoolerOn ? LOW : HIGH);
    digitalWrite(relay3, isExhaustOn ? LOW : HIGH);
    digitalWrite(relay4, isPumpOn ? LOW : HIGH);

    // Store data in "data set" with a push() every 2 minutes
    static unsigned long previousDatasetUpdateTime = millis();
    static int datasetUpdateInterval = 120000; // 2 minutes in milliseconds

    if (millis() - previousDatasetUpdateTime >= datasetUpdateInterval) {
      String dataSetPath = "/farmlink/data_set";
      FirebaseJson jsonData;
      jsonData.add("temperature", temperature);
      jsonData.add("humidity", humidity);
      jsonData.add("sprinkler", isSprinklerOn ? "ON" : "OFF");
      jsonData.add("cooler", isCoolerOn ? "ON" : "OFF");
      jsonData.add("exhaust", isExhaustOn ? "ON" : "OFF");
      jsonData.add("pump", isPumpOn ? "ON" : "OFF");
      Firebase.pushJSON(firebaseData, dataSetPath, jsonData);

      previousDatasetUpdateTime = millis();
    }

    // Display data on the LCD every 5 seconds
    lcd.setCursor(0, 0);
    lcd.print("Temp : ");
    lcd.print(temperature, 2);
    lcd.print((char)223);
    lcd.print("C");

    lcd.setCursor(0, 1);
    lcd.print("Humd : ");
    lcd.print(humidity, 2);
    lcd.print("%");

    // Delay for 5 seconds
    delay(5000);
  }
}
