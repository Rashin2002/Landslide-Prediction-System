/*#include <Wire.h>
#include <MPU6050.h>
#include <WiFi.h>
#include <FirebaseESP32.h>

// Wi-Fi credentials
#define WIFI_SSID "Redmi Note 10S"
#define WIFI_PASSWORD "Rash.1111"

// Firebase credentials
#define API_KEY "AIzaSyAHdpgz7C0fVBMuzXiJqQD9cd3j3eZAB6A"
#define DATABASE_URL "https://nodemcutest-226ac-default-rtdb.asia-southeast1.firebasedatabase.app"
#define USER_EMAIL "rashinprashastha2002@gmail.com"
#define USER_PASSWORD "Rashin.1212"

// Firebase and MPU6050 setup
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
MPU6050 mpu;

// Sensor pins
const int soilMoisturePin = 35; // GPIO35 (A0)
const int rainAnalogPin = 34;   // GPIO34 (A1)

// Timing
unsigned long lastSendTime = 0;
const unsigned long interval = 10000; // 10 seconds

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Initialize MPU6050
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed");
    while (1);
  } else {
    Serial.println("MPU6050 connected successfully");
  }

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConnected to Wi-Fi");

  // Firebase config
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Firebase initialized");
}

void loop() {
  // MPU6050 values
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
  mpu.getAcceleration(&ax, &ay, &az);
  mpu.getRotation(&gx, &gy, &gz);

  Serial.print("Accel [g]: X="); Serial.print(ax);
  Serial.print(" Y="); Serial.print(ay);
  Serial.print(" Z="); Serial.print(az);
  Serial.print(" | Gyro [°/s]: X="); Serial.print(gx);
  Serial.print(" Y="); Serial.print(gy);
  Serial.print(" Z="); Serial.println(gz);

  // Soil Moisture
  int soilRaw = analogRead(soilMoisturePin);
  float soilWetness = ((4095.0 - soilRaw) / 4095.0) * 100.0;
  soilWetness = constrain(soilWetness, 0, 100);

  Serial.print("Soil Raw: "); Serial.print(soilRaw);
  Serial.print(" | Soil Wetness %: "); Serial.println(soilWetness);

  // Rainfall Sensor
  int rainRaw = analogRead(rainAnalogPin);
  float rainWetness = ((4095.0 - rainRaw) / 4095.0) * 100.0;
  rainWetness = constrain(rainWetness, 0, 100);

  Serial.print("Rain Raw: "); Serial.print(rainRaw);
  Serial.print(" | Rain Wetness %: "); Serial.println(rainWetness);

  // Send to Firebase every 10 seconds
  if (millis() - lastSendTime >= interval) {
    lastSendTime = millis();

    // MPU6050
    Firebase.setInt(fbdo, "/mpu6050/accel/x", ax);
    Firebase.setInt(fbdo, "/mpu6050/accel/y", ay);
    Firebase.setInt(fbdo, "/mpu6050/accel/z", az);
    Firebase.setInt(fbdo, "/mpu6050/gyro/x", gx);
    Firebase.setInt(fbdo, "/mpu6050/gyro/y", gy);
    Firebase.setInt(fbdo, "/mpu6050/gyro/z", gz);

    // Soil Moisture
    Firebase.setFloat(fbdo, "/soilMoisture/wetnessPercent", soilWetness);

    // Rain Sensor
    Firebase.setFloat(fbdo, "/rainSensor/wetnessPercent", rainWetness);

    // Feedback
    if (fbdo.httpCode() == 200) {
      Serial.println("All sensor data uploaded successfully");
    } else {
      Serial.printf("Firebase upload failed: %s\n", fbdo.errorReason().c_str());
    }
  }

  delay(100); // For serial stability
}*/




#include <Wire.h>
#include <MPU6050.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <HX711_ADC.h>  // For Load Cell (HX711)

// Wi-Fi credentials
#define WIFI_SSID "Redmi Note 10S"
#define WIFI_PASSWORD "Rash.1111"

// Firebase credentials
#define API_KEY "AIzaSyAHdpgz7C0fVBMuzXiJqQD9cd3j3eZAB6A"
#define DATABASE_URL "https://nodemcutest-226ac-default-rtdb.asia-southeast1.firebasedatabase.app"
#define USER_EMAIL "rashinprashastha2002@gmail.com"
#define USER_PASSWORD "Rashin.1212"

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// MPU6050 object
MPU6050 mpu;

// Sensor pins
const int soilMoisturePin = 35; // GPIO35
const int rainAnalogPin = 34;   // GPIO34

// HX711 pins: DT = GPIO19, SCK = GPIO18
HX711_ADC LoadCell(19, 18);

// Timing
unsigned long lastSendTime = 0;
const unsigned long interval = 10000; // 10 seconds

void setup() {
  Serial.begin(115200);
  Wire.begin(23, 22);

  // MPU6050 initialization
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed");
    while (1);
  } else {
    Serial.println("MPU6050 connected successfully");
  }

  // Wi-Fi connection
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConnected to Wi-Fi");

  // Firebase initialization
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Serial.println("Firebase initialized");

  // HX711 Load Cell initialization
  LoadCell.begin();
  LoadCell.start(2000);  // 2-second delay to tare
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Tare timeout. Check HX711 wiring!");
    while (1);
  }
  LoadCell.setCalFactor(1000.0); // Calibration factor (adjust for 1kg load cell)
  Serial.println("HX711 initialized.");
}

void loop() {
  // MPU6050 reading
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
  mpu.getAcceleration(&ax, &ay, &az);
  mpu.getRotation(&gx, &gy, &gz);

  Serial.print("Accel [g]: X="); Serial.print(ax);
  Serial.print(" Y="); Serial.print(ay);
  Serial.print(" Z="); Serial.print(az);
  Serial.print(" | Gyro [°/s]: X="); Serial.print(gx);
  Serial.print(" Y="); Serial.print(gy);
  Serial.print(" Z="); Serial.println(gz);

  // Soil moisture reading
  int soilRaw = analogRead(soilMoisturePin);
  float soilWetness = ((4095.0 - soilRaw) / 4095.0) * 100.0;
  soilWetness = constrain(soilWetness, 0, 100);
  Serial.print("Soil Raw: "); Serial.print(soilRaw);
  Serial.print(" | Soil Wetness %: "); Serial.println(soilWetness);

  // Rain sensor reading
  int rainRaw = analogRead(rainAnalogPin);
  float rainWetness = ((4095.0 - rainRaw) / 4095.0) * 100.0;
  rainWetness = constrain(rainWetness, 0, 100);
  Serial.print("Rain Raw: "); Serial.print(rainRaw);
  Serial.print(" | Rain Wetness %: "); Serial.println(rainWetness);

  // HX711 load cell reading
  LoadCell.update();
  float weight = LoadCell.getData();
  Serial.print("Weight: ");
  Serial.print(weight, 2);
  Serial.println(" grams");

  // Upload every 10 seconds
  if (millis() - lastSendTime >= interval) {
    lastSendTime = millis();

    // Upload MPU6050 data
    Firebase.setInt(fbdo, "/mpu6050/accel/x", ax);
    Firebase.setInt(fbdo, "/mpu6050/accel/y", ay);
    Firebase.setInt(fbdo, "/mpu6050/accel/z", az);
    Firebase.setInt(fbdo, "/mpu6050/gyro/x", gx);
    Firebase.setInt(fbdo, "/mpu6050/gyro/y", gy);
    Firebase.setInt(fbdo, "/mpu6050/gyro/z", gz);

    // Upload Soil Moisture
    Firebase.setFloat(fbdo, "/soilMoisture/wetnessPercent", soilWetness);

    // Upload Rainfall
    Firebase.setFloat(fbdo, "/rainSensor/wetnessPercent", rainWetness);

    // Upload Load Cell Weight
    Firebase.setFloat(fbdo, "/loadCell/weight", weight);

    // Upload status
    if (fbdo.httpCode() == 200) {
      Serial.println("All sensor data uploaded successfully");
    } else {
      Serial.printf("Firebase upload failed: %s\n", fbdo.errorReason().c_str());
    }
  }

  delay(100); // Serial stability
}



