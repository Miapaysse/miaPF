#include <Arduino.h>

#include <WiFi.h>

#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>


#define POTE 23



/* 1. Define the WiFi credentials */
#define WIFI_SSID "iPhone de Mia"
#define WIFI_PASSWORD "Silvermia18"

/* 2. Define the API Key */
#define API_KEY "AIzaSyCbc6e5Zx-3KZA-HpMn0RRKOX6TcObuJ6o"

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "firestoreyesp32"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "mia@gmail.com"
#define USER_PASSWORD "123456"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long dataMillis = 0;
int count = 0;

int valorPote = 0;
float valoresPote[3];

void setup() {

  Serial.begin(115200);

  pinMode(POTE, INPUT);


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
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
  config.wifi.clearAP();
  config.wifi.addAP(WIFI_SSID, WIFI_PASSWORD);
#endif

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
  crearDoc("FLEXs/flex1");
  crearDoc("FLEXs/flex2");
  crearDoc("GYROs/gyro1");
  crearDoc("GYROs/gyro2");
  update("FLEXs/flex1", "Saludo1");
}

void crearDoc(String path0) {

  if (Firebase.ready()) {


    // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create_Edit_Parse/Create_Edit_Parse.ino
    FirebaseJson content;

    // Note: If new document created under non-existent ancestor documents, that document will not appear in queries and snapshot
    // https://cloud.google.com/firestore/docs/using-console#non-existent_ancestor_documents.

    // We will create the document in the parent path "a0/b?
    // a0 is the collection id, b? is the document id in collection a0.

    String documentPath = path0;


    // If the document path contains space e.g. "a b c/d e f"
    // It should encode the space as %20 then the path will be "a%20b%20c/d%20e%20f"

    content.set("fields/Saludo1/stringValue", "Holi");

    

    String doc_path = "projects/";
    doc_path += FIREBASE_PROJECT_ID;
    doc_path += "/databases/(default)/documents/coll_id/doc_id";  // coll_id and doc_id are your collection id and document id

    Serial.print("Create a document... ");

    if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw()))
      Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
    else {
      Serial.println(fbdo.errorReason());
    }

    return;
  }
}

void update(String path1, String path2) {
  if (Firebase.ready() && (millis() - dataMillis > 2000 || dataMillis == 0)) {
    dataMillis = millis();
    count++;


    Serial.print("Commit a document (append array)... ");

    // The dyamic array of write object firebase_firestore_document_write_t.
    std::vector<struct firebase_firestore_document_write_t> writes;

    // A write object that will be written to the document.
    struct firebase_firestore_document_write_t transform_write;

    // Set the write object write operation type.
    // firebase_firestore_document_write_type_update,
    // firebase_firestore_document_write_type_delete,
    // firebase_firestore_document_write_type_transform
    transform_write.type = firebase_firestore_document_write_type_transform;

    // Set the document path of document to write (transform)
    transform_write.document_transform.transform_document_path = path1;

    // Set a transformation of a field of the document.
    struct firebase_firestore_document_write_field_transforms_t field_transforms;

    // Set field path to write.
    field_transforms.fieldPath = path2;

    // Set the transformation type.
    // firebase_firestore_transform_type_set_to_server_value,
    // firebase_firestore_transform_type_increment,
    // firebase_firestore_transform_type_maaximum,
    // firebase_firestore_transform_type_minimum,
    // firebase_firestore_transform_type_append_missing_elements,
    // firebase_firestore_transform_type_remove_all_from_array
    field_transforms.transform_type = firebase_firestore_transform_type_append_missing_elements;

    // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create_Edit_Parse/Create_Edit_Parse.ino
    FirebaseJson content;

    //String txt = "Hello World! " + String(count);
    content.set("values/[0]/integerValue", String(rand()).c_str());


    // Set the transformation content.
    field_transforms.transform_content = content.raw();

    // Add a field transformation object to a write object.
    transform_write.document_transform.field_transforms.push_back(field_transforms);

    // Add a write object to a write array.
    writes.push_back(transform_write);

    if (Firebase.Firestore.commitDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, writes /* dynamic array of firebase_firestore_document_write_t */, "" /* transaction */))
      Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
    else
      Serial.println(fbdo.errorReason());
  }
}
