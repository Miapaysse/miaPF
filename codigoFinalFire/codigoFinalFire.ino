#include <Arduino.h>


#include <WiFi.h>


#include <Firebase_ESP_Client.h>


// Provide the token generation process info.
#include <addons/TokenHelper.h>


#include <addons/RTDBHelper.h>


#include "time.h"


#define FLEX1 36
#define FLEX2 39


//MAQUINA
#define INNIT 0
#define ESPERA 1
#define LECTURA 2
#define ENVIO 3


#define TIEMPOLECTURA 1
#define CICLO 3


/* 1. Define the WiFi credentials */
#define WIFI_SSID "iPhone de Mia"
#define WIFI_PASSWORD "Silvermia18"


/* 2. Define the API Key */
#define API_KEY "AIzaSyC1S2t9MWIYYwDYgsjIiLeTxsO-WQzm1qk"


/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "back-on-track-46f4e"


/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "user1@gmail.com"
#define USER_PASSWORD "123456"


// Define Firebase Data object
FirebaseData fbdo;


FirebaseAuth auth;
FirebaseConfig config;


unsigned long dataMillis = 0;
unsigned long dataMillisUpdate = 0;
unsigned long dataMillisGet = 0;
unsigned long dataMillisCreate = 0;
unsigned long tiempoUltimoCambio = 0;
int count = 0;
int contador = 0;


int timestamp;


int estado = 0;
String idPersona = "ukmlaNEJSykBmvygrkKm";


int analogicoFlex1 = 0;
float tension1 = 0;
float anguloFlex1 = 0;


int analogicoFlex2 = 0;
float tension2 = 0;
float anguloFlex2 = 0;


float x1 = 1;
//float y1 = 1;
float z1 = 1;
float x2 = 2;
float y2 = 2;
float z2 = 2;


float V_min = 0.5;   // Sustituir con el voltaje medido a 0 grados
float V_max = 1.25;  // Sustituir con el voltaje medido a 180 grados
float V_90 = 0.85;




bool taskCompleted = false;


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
  //configTime(0, 0, ntpServer);


  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);




  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();


  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);


  /* Assign the api key (required) */
  config.api_key = API_KEY;


  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;


  // The WiFi credentials are required for Pico W
  // due to it does not have reconnect feature.


  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h


  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);


  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);


  // Limit the size of response payload to be collected in FirebaseData
  fbdo.setResponseSize(2048);


  Firebase.begin(&config, &auth);


  // You can use TCP KeepAlive in FirebaseData object and tracking the server connection status, please read this for detail.
  // https://github.com/mobizt/Firebase-ESP-Client#about-firebasedata-object
  // fbdo.keepAlive(5, 5, 1);
}


void loop() {
  maquina();
}


void maquina() {
  switch (estado) {
    case INNIT:
      count = 0;
      crearDoc(idPersona + "/flex1");
      crearDoc(idPersona + "/flex2");
      crearDoc(idPersona + "/x1");
      crearDoc(idPersona + "/y1");
      crearDoc(idPersona + "/z1");
      crearDoc(idPersona + "/x2");
      crearDoc(idPersona + "/y2");
      crearDoc(idPersona + "/z2");


      estado = ESPERA;
      break;


    case ESPERA:
      Serial.println("espera");


      if (Firebase.ready() && (millis() - dataMillisUpdate > (CICLO * 1000) || dataMillisUpdate == 0)) {
        dataMillisUpdate = millis();
        estado = LECTURA;
        contador++;
      }


      break;


    case LECTURA:


      Serial.println("lectura");


      analogicoFlex1 = analogRead(FLEX1);
      //Serial.println(analogicoFlex1);
      tension1 = obtenerTension(analogicoFlex1);
      anguloFlex1 = obtenerAngulo(tension1);




      analogicoFlex2 = analogRead(FLEX2);
      //Serial.println(analogicoFlex2);
      tension2 = obtenerTension(analogicoFlex2);
      anguloFlex2 = obtenerAngulo(tension2);


      update(idPersona + "/flex1", "valoresFlex1", anguloFlex1);
      update(idPersona + "/flex2", "valoresFlex2", anguloFlex2);
      update(idPersona + "/x1", "valoresX1", anguloFlex2);
      update(idPersona + "/y1", "valoresy1", anguloFlex2);
      update(idPersona + "/z1", "valoresz1", anguloFlex2);
      update(idPersona + "/x2", "valoresX2", anguloFlex2);
      update(idPersona + "/y2", "valoresy2", anguloFlex2);
      update(idPersona + "/z2", "valoresz2", anguloFlex2);
     


      //LECTURA GIROSCOPIOS
      delay(200);
      estado = ESPERA;




      break;
    case ENVIO:
      Serial.println("Envío");


      delay(200);
      enviarValores(idPersona + "/flex1", "valoresFlex1", anguloFlex1);


      estado = ESPERA;
      break;
  }
}


float obtenerTension(float flex) {
  float tension = 0;
  tension = (flex / 4096) * 3.3;
  //Serial.print("tensión: ");
  //Serial.println(tension);
  return tension;
}


float obtenerAngulo(float tension) {


  float angulo = 0;


  //angulo = ((tension - 0.997) / (1.87 - 0.997)) * 180;
  //((tension * (-90)) + 180);
  if (tension >= V_min && tension <= V_max) {
    // Cálculo del ángulo usando interpolación lineal
    angulo = (tension - V_min) / (V_max - V_min) * 180;
  } else {
    angulo = 0;  // Valor fuera de rango o error
  }


  //Serial.print("Angulo: ");
  //Serial.println(angulo);


  return angulo;
}




void crearDoc(String path) {


  if (Firebase.ready() && (millis() - dataMillisCreate > 2000 || dataMillisCreate == 0)) {
    dataMillisCreate = millis();


    // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create_Edit_Parse/Create_Edit_Parse.ino
    FirebaseJson content;


    String documentPath = path;


    // If the document path contains space e.g. "a b c/d e f"
    // It should encode the space as %20 then the path will be "a%20b%20c/d%20e%20f"
    content.set("fields/valoresFlex1/stringValue", "Inicio");


    String doc_path = "projects/";
    doc_path += FIREBASE_PROJECT_ID;
    doc_path += "/databases/(default)/documents/coll_id/doc_id";  // coll_id and doc_id are your collection id and document id


    // reference
    //content.set("fields/myRef/referenceValue", doc_path.c_str());


    // timestamp
    // content.set("fields/myTimestamp/timestampValue", "2014-10-02T15:01:23Z");  // RFC3339 UTC "Zulu" format


    // array
    // content.set("fields/myArray/arrayValue/values/[0]/stringValue", "test");
    //content.set("fields/myArray/arrayValue/values/[1]/integerValue", "20");
    //content.set("fields/myArray/arrayValue/values/[2]/booleanValue", true);




    Serial.print("Create a document... ");


    if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw()))
      Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
    else {
      Serial.println(fbdo.errorReason());
    }


    return;
  }
}


void update(String path1, String path2, float medicion) {


  count++;


  Serial.print("Commit a document (append array)... ");


  // The dyamic array of write object firebase_firestore_document_write_t.
  std::vector<struct firebase_firestore_document_write_t> writes;


  // A write object that will be written to the document.
  struct firebase_firestore_document_write_t transform_write;


  transform_write.type = firebase_firestore_document_write_type_transform;


  // Set the document path of document to write (transform)
  transform_write.document_transform.transform_document_path = path1;


  // Set a transformation of a field of the document.
  struct firebase_firestore_document_write_field_transforms_t field_transforms;


  // Set field path to write.
  field_transforms.fieldPath = path2;




  field_transforms.transform_type = firebase_firestore_transform_type_append_missing_elements;


  // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create_Edit_Parse/Create_Edit_Parse.ino
  FirebaseJson content;


  String muestraMedicion = String(count) + ", " + String(medicion);
  content.set("values/[1]/stringValue", muestraMedicion);
  //content.set("fields/myArray/arrayValue/values/[1]/integerValue", "20");


  // Set the transformation content.
  field_transforms.transform_content = content.raw();


  // Add a field transformation object to a write object.
  transform_write.document_transform.field_transforms.push_back(field_transforms);


  // Add a write object to a write array.
  writes.push_back(transform_write);


  // Add a write object to a write array.
  writes.push_back(transform_write);
  if (Firebase.Firestore.commitDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, writes /* dynamic array of firebase_firestore_document_write_t */, "" /* transaction */))
    Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
  else
    Serial.println(fbdo.errorReason());
}


void enviarValores(String path1, String path2, float medicion) {


  Serial.print("Commit a document (append array)... ");


  // The dyamic array of write object firebase_firestore_document_write_t.
  std::vector<struct firebase_firestore_document_write_t> writes;


  // A write object that will be written to the document.
  struct firebase_firestore_document_write_t transform_write;


  transform_write.type = firebase_firestore_document_write_type_transform;


  // Set the document path of document to write (transform)
  transform_write.document_transform.transform_document_path = path1;


  // Set a transformation of a field of the document.
  struct firebase_firestore_document_write_field_transforms_t field_transforms;


  // Set field path to write.
  field_transforms.fieldPath = path2;




  field_transforms.transform_type = firebase_firestore_transform_type_append_missing_elements;


  // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create_Edit_Parse/Create_Edit_Parse.ino
  FirebaseJson content;


  String muestraMedicion = String(count) + ", " + String(medicion);
  content.set("values/[1]/stringValue", muestraMedicion);
  //content.set("fields/myArray/arrayValue/values/[1]/integerValue", "20");


  // Set the transformation content.
  field_transforms.transform_content = content.raw();


  // Add a field transformation object to a write object.
  transform_write.document_transform.field_transforms.push_back(field_transforms);


  // Add a write object to a write array.
  writes.push_back(transform_write);


  // Add a write object to a write array.
  writes.push_back(transform_write);
  if (Firebase.Firestore.commitDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, writes /* dynamic array of firebase_firestore_document_write_t */, "" /* transaction */))
    Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
  else
    Serial.println(fbdo.errorReason());
}




void getDoc(String documentPath, String mask) {


  if (Firebase.ready() && (millis() - dataMillisGet > 7000 || dataMillisGet == 0)) {
    dataMillisGet = millis();


    Serial.print("Get a document... ");


    if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), mask.c_str()))
      Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
    else
      Serial.println(fbdo.errorReason());
  }
}

