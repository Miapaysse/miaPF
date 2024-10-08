//grupo 1  Dyner, Payseé, Yedid y Jakubowicz

#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>

#include "time.h"

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#define FLEX1 13
#define FLEX2 12

unsigned long tiempo;

#define WIFI_SSID "iPhone de Mia"
#define WIFI_PASSWORD "Silvermia18"

// Insert Firebase project API Key
#define API_KEY "AIzaSyC1S2t9MWIYYwDYgsjIiLeTxsO-WQzm1qk"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "user@gmail.com"
#define USER_PASSWORD "123456"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://back-on-track-46f4e-default-rtdb.firebaseio.com/"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes
String angPath = "/FLEX1";
String idPath = "/ID";
String timePath = "/timestamp";
String dataPath = "/data";

String data;


// Parent Node (to be updated in every loop)
String parentPath;

int timestamp;
FirebaseJson json;

const char* ntpServer = "pool.ntp.org";

// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
unsigned long ciclo = 0.5;


//WiFiClientSecure client;

String idPersona = "ukmlaNEJSykBmvygrkKm";

int analogicoFlex1 = 0;
long int flex1 = 0;
float tension1 = 0;
float tension2 = 0;
float flex = 0;

float angulosFlex1 = 0;
float angulosFlex2 = 0;
float X1;
float y1;
float z1;
float X2;
float y2;
float z2;

String valorX1;
String valorY1;
String valorZ1;
String valorX2;
String valorY2;
String valorZ2;


float V_min = 1;    //0.2;  // Sustituir con el voltaje medido a 0 grados
float V_max = 4.5;  // Sustituir con el voltaje medido a 180 grados
float V_90 = 2.5;

void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return (0);
  }
  time(&now);
  return now;
}

void setup() {
  Serial.begin(115200);
  pinMode(FLEX1, INPUT);
  pinMode(FLEX2, INPUT);

  initWiFi();
  configTime(0, 0, ntpServer);

  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);
  Serial.println(idPersona);
  // Update database path
  databasePath = "/UsersData/" + uid + "/readings";
}

void loop() {

  analogicoFlex1 = analogRead(FLEX1);
  analogicoFlex2 = analogRead(FLEX2);
  //Serial.println(analogicoFlex1);
  tension1 = obtenerTension(analogicoFlex1);
  tension2 = obtenerTension(analogicoFlex2);
  angulosFlex1 = obtenerAngulo(tension1);
  angulosFlex2 = obtenerAngulo(tension2);

  x1 = 0;
  y1 = 0;
  z1 = 0;
  x2 = 1;
  y2 = 1;
  z2 = 1;

  enviarValores(idPersona, angulosFlex1, angulosFlex2, x1, y1, z1, x2, y2, z2);
}


void obtenerTension(float flex) {
  tension = (flex * 3.3) / 1024;
  //Serial.print("tensión: ");
  //Serial.println(tension);
  return tension;
}

float obtenerAngulo(float tension) {

  //angulo = ((tension - 0.997) / (1.87 - 0.997)) * 180;
  //((tension * (-90)) + 180);

  if (tension >= V_min && tension <= V_max) {
    // Cálculo del ángulo usando interpolación lineal
    angulo = (tension - V_min) / (V_max - V_min) * 180;
  } /*else {
    angulo = 0;  // Valor fuera de rango o error
  }*/

  Serial.print("Angulo: ");
  Serial.println(angulo);

  return angulo;
}



void enviarValores(String id, float angulosFlex1, float angulosFlex2, float valorX1, float valorY1, float valorZ1, float valorX2, float valorY2, float valorZ2) {

  if (Firebase.ready() && (millis() - sendDataPrevMillis > (ciclo * 1000) || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    data = id + "," + String(angulosFlex1) + "," + String(angulosFlex2) + "," + String(valorX1) + "," + String(valorY1) + "," + String(valorZ1) + "," + String(valorX2) + "," + String(valorY2) + "," + String(valorZ2);
    //Get current timestamp
    timestamp = getTime();
    Serial.print("time: ");
    Serial.println(timestamp);


    Serial.print("Flex1: ");
    Serial.println(angulo1);

    parentPath = databasePath + "/" + String(timestamp);
    Serial.println(databasePath);

    json.set(dataPath.c_str(), data);
    json.set(timePath, String(timestamp));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
  }
}