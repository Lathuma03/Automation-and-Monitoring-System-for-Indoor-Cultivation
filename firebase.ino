#include <WiFi.h>
#include <FirebaseESP32.h>
#include <DHT.h>

// Replace with your Wi-Fi credentials
const char* WIFI_SSID = "vivo Y21";
const char* WIFI_PASSWORD = "lathuma@123";

// Replace with your Firebase credentials
#define FIREBASE_AUTH "Gex47sHejjnzQGRLCZOcmXYYCbmwxKezdfGNhYn0"
#define FIREBASE_HOST "https://sensor-data-e646e-default-rtdb.firebaseio.com/"

// DHT sensor setup
#define DHTPIN 16       // GPIO 4 (D4) for DHT11 data pin
#define DHTTYPE DHT11    // DHT11 sensor type
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Wi-Fi connected successfully");

  // Initialize Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  // Initialize DHT sensor
  dht.begin();
}

void loop() {
  // Declare FirebaseJson object and FirebaseData object
  FirebaseJson json;
  FirebaseData fbdo;

  // Read temperature and humidity from DHT11
  float temperatureCelsius = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperatureCelsius) || isnan(humidity)) {
    Serial.println("Failed to read data from DHT11 sensor!");
    return;
  }

  // Send data to Firebase
  String path = "/dht11";
  json.set("temperature", temperatureCelsius);
  json.set("humidity", humidity);

  if (Firebase.pushJSON(fbdo, path, json)) {
    Serial.println("Data sent to Firebase successfully");
  } else {
    Serial.println("Sending data to Firebase failed!");
    Serial.println(fbdo.errorReason());
  }

  // Adjust the delay to control the frequency of data updates
  delay(5000); // 5 seconds
}
