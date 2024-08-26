//  liprary 
#include <WiFi.h>
#include <time.h>
#include <Keypad.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
#include <Arduino.h>
#include <NTPClient.h>
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>
#include <Firebase_ESP_Client.h>



// Variables 
const byte ROWS = 4;
const byte COLS = 4;

unsigned long getDataPrevMillis = 0;

String password ="1234";

String formattedDate;

int returnData;

// esp32 pin
#define Servo_PIN_in 2
#define Servo_PIN_out 4

// #define Fan_PIN 35

#define BUZZER_PIN 5

#define LED_R_PIN  19
#define LED_Y_PIN  23

byte rowPins[ROWS] = {13, 12, 14, 27};
byte colPins[COLS] = {26, 25, 33, 32};


// Keypad configuration
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// objects
Servo myServo_in;
Servo myServo_out;

LiquidCrystal_I2C lcd(0x27, 16, 2);
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP ,"pool.ntp.org");



// conection 
#define WIFI_SSID "Ayman"
#define WIFI_PASSWORD "1234567899"
#define API_KEY "AIzaSyBZ34_7Zd6yzT64om9hiY_aEdM0jQ-t9K4"
#define DATABASE_URL "https://smartclassroom-75af0-default-rtdb.firebaseio.com/"
#define USER_EMAIL "mohamedyasser@gmail.com"
#define USER_PASSWORD "mohamedyasser@gmail.com"


// Function Initialize Wi-Fi connection
void wiFiConnection(){

  Serial.print("Connected to Wi-Fi...");
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

// Function schake Password to open dore
void keyPadPassword() {
  String inputPassword;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter Password:");

  char key = keypad.getKey();
  while (key != '#') {
    if (key) {
      inputPassword += key;
      lcd.setCursor(inputPassword.length(), 1);
      lcd.print("*");  // Mask input with '*'
    }
    key = keypad.getKey();
    delay(100);
  }

  if (inputPassword == password) {
    lcd.clear();
    lcd.print(" ---- OPEN ---- ");
    myServo_in.write(180);
    while (keypad.getKey() != 'A') {
        delay(100);
    }
  } else {
    lcd.clear();
    lcd.print("  NOT  CORRECT  ");
    delay(1000);
  }
}

// Function Get data from firebase.
int getDataFire(String name){
  if(Firebase.RTDB.getInt(&fbdo, name, &returnData)){
    return returnData;
  }else{
    Serial.println(fbdo.errorReason().c_str());
    return -1;
  }

}

// Function LED 
void led_on(int pin){ digitalWrite(pin, HIGH); }
void led_off(int pin){ digitalWrite(pin, LOW); }

// Function Buzzer
void buzzer_on(){ tone(BUZZER_PIN, 3000); }
void buzzer_off(){ noTone(BUZZER_PIN); }

// // Function Fan
// void fan_on(){ digitalWrite(Fan_PIN, HIGH); }
// void fan_off(){ digitalWrite(Fan_PIN, LOW); }

// Function Time
void timeSetup(){
  timeClient.begin();
  timeClient.setTimeOffset(10800); // Egypt 
}
String readTime(){

  timeClient.update();
  time_t rawTime = timeClient.getEpochTime();
  struct tm * timeInfo = localtime(&rawTime);
  char formattedTime[20];
  strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%d %H:%M:%S", timeInfo);
  return String(formattedTime);
  delay(1000);
  // out format 2024-08-24 15:30:45 
}

// Function LCD
void setupLCD(){
  lcd.init();
  lcd.backlight();
}

void setup() {
  Serial.begin(9600);

  // Initialize pins
  pinMode(LED_R_PIN, OUTPUT);

  pinMode(23, OUTPUT); // Ensure pin 23 is capable of being an output
  // digitalWrite(23, HIGH); // Set pin 23 high

  pinMode(BUZZER_PIN, OUTPUT);
  // pinMode(Fan_PIN, OUTPUT);

  myServo_in.attach(Servo_PIN_in);
  myServo_out.attach(Servo_PIN_out); 

  wiFiConnection();
  firebaseConnection();
  timeSetup();
  setupLCD();
  
}


void loop(){
  
  // Send data to Firebase at regular intervals
  if (Firebase.ready() && (millis() - getDataPrevMillis > 1000 || getDataPrevMillis == 0)) {
    getDataPrevMillis = millis();
    delay(500);
  }

// get data from fire base 
if(getDataFire("flame/fire") == 0){
  led_on(LED_R_PIN);
  buzzer_on();
  myServo_in.write(180);
  myServo_out.write(180);
  delay(10000);
  
}

if(char(keypad.getKey()) == 'D'){
  keyPadPassword();
}

analogWrite(LED_Y_PIN, getDataFire("Light/degree_1") );

// byte speed = map(getDataFire("Fan/degree") , 0 , 3 , 0, 255 );
// analogWrite(speed , Fan_PIN);

myServo_in.write(int(getDataFire("Door/in")));
myServo_out.write(int(getDataFire("Door/out")));

lcd.clear();
lcd.print(readTime());

  delay(100);
}

