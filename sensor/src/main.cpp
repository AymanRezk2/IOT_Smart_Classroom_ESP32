#include <DHT.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Arduino.h>
#include <NTPClient.h>
#include <Firebase_ESP_Client.h>
#include <Adafruit_Sensor.h>

// Variables 
unsigned long sendDataPrevMillis = 0;

int counter_1 = 0;
int old_counter_1 = 0;

int lightLevel = 0;
int old_lightLevel = 0;

int storedValue = 0;
int counter = 0;

// esp32 pin
#define Flame_PIN 35

#define TRIG_PIN_1 13
#define ECHO_PIN_1 12
#define TRIG_PIN_2 14
#define ECHO_PIN_2 27

#define LDR_PIN 33
#define DHT_TYPE DHT11 
#define DHT_PIN 32

// objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

DHT dht(DHT_PIN, DHT_TYPE);


// conection 
#define WIFI_SSID "Ayman"
#define WIFI_PASSWORD "1234567899"
#define API_KEY "AIzaSyBZ34_7Zd6yzT64om9hiY_aEdM0jQ-t9K4"
#define DATABASE_URL "https://smartclassroom-75af0-default-rtdb.firebaseio.com/"
#define USER_EMAIL "mohamedyasser@gmail.com"
#define USER_PASSWORD "mohamedyasser@gmail.com"

// Function Initialize Wi-Fi connection
void wiFiConnection(){
  Serial.print("Connected to Wi-Fi ...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected to Wi-Fi with IP: ");
  Serial.println(WiFi.localIP());
}

// Function Initialize Firebase
void firebaseConnection(){

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectNetwork(true);
  fbdo.setBSSLBufferSize(4096, 1024);
  fbdo.setResponseSize(2048);
  config.timeout.serverResponse = 10 * 1000;
}

// Function to upload sensor data to Firebase
void uploadSensorData(String path, int value) {
  if (Firebase.RTDB.setInt(&fbdo, path, value)) {
    Serial.print(path);
    Serial.println(value);
  } else {
    Serial.println(fbdo.errorReason().c_str());
  }
}

// Function to read distance from ultrasonic sensor
float readDistanceCM(int TR, int EC) {
  digitalWrite(TR, LOW);
  delayMicroseconds(2);
  digitalWrite(TR, HIGH);
  delayMicroseconds(10);
  digitalWrite(TR, LOW);
  int duration = pulseIn(EC, HIGH);
  return duration * 0.034 / 2;
}

// Function to read Light Level (0 to 100)
int readLightLevel(){
  uint16_t LDR_val = analogRead(LDR_PIN);
  lightLevel = map(LDR_val, 0, 3000, 0, 100);
  return lightLevel;
}

// Function Check if the current sensor value has changed from the stored value
void updateSensorData(String path, int currentValue) {
  if (currentValue != storedValue) {
    storedValue = currentValue;
    // Upload the new value to Firebase
    uploadSensorData(path, currentValue);
  }
}

// Function Flame 
int readFlame(){
  int flame_state = digitalRead(Flame_PIN);

  if (flame_state == HIGH){
    Serial.println("No flame dected => The fire is NOT detected"); }
  else{
    Serial.println("Flame dected => The fire is detected"); }

  return flame_state;
}

// Function DH11
void dh11Setup(){
  dht.begin();  
}
float readDHTHumidity() {
  // Sensor readings may also be up to 2 seconds
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return -1;
  }
  else {
    // Serial.println(h);
    return h;
  }
}
float readDHTTemperature() {
  // Sensor readings may also be up to 2 seconds
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    return -1;
  }
  else {
    // Serial.println(t);
    return t;
  }
}

void setup() {
  Serial.begin(9600);

  // Initialize pins
  pinMode(TRIG_PIN_1, OUTPUT);
  pinMode(ECHO_PIN_1, INPUT);

  pinMode(TRIG_PIN_2, OUTPUT);
  pinMode(ECHO_PIN_2, INPUT);

  pinMode(LDR_PIN, INPUT);

  pinMode(Flame_PIN, INPUT);

  wiFiConnection();
  firebaseConnection();

  dh11Setup();
  
}

void loop(){
  
  // Send data to Firebase at regular intervals
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    delay(500);
  }

  float distance_1 = readDistanceCM(TRIG_PIN_1,ECHO_PIN_1);
  if (distance_1 < 20) {
    counter++;
  }

  float distance_2 = readDistanceCM(TRIG_PIN_2,ECHO_PIN_2);
  if (distance_2 < 20) {
    counter--;
  }

  updateSensorData("altra/count",counter);


  // int state = ;
  updateSensorData("flame/fire",readFlame());

updateSensorData("Humidity/degree",readDHTHumidity());
updateSensorData("Temperature/degree",readDHTTemperature());

updateSensorData("Light/degree",readLightLevel());

Serial.println("-----------------------");

  delay(100);
}
