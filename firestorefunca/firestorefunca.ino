
#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
// Provide the token generation process info.
#include <addons/TokenHelper.h>

#define POTE 23

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Fibertel WiFi269 2.4GHz"
#define WIFI_PASSWORD "0142901884"

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

float valorPote = 0;



// The Firestore payload upload callback function
void fcsUploadCallback(CFS_UploadStatusInfo info) {
  if (info.status == firebase_cfs_upload_status_init) {
    Serial.printf("\nUploading data (%d)...\n", info.size);
  } else if (info.status == firebase_cfs_upload_status_upload) {
    Serial.printf("Uploaded %d%s\n", (int)info.progress, "%");
  } else if (info.status == firebase_cfs_upload_status_complete) {
    Serial.println("Upload completed ");
  } else if (info.status == firebase_cfs_upload_status_process_response) {
    Serial.print("Processing the response... ");
  } else if (info.status == firebase_cfs_upload_status_error) {
    Serial.printf("Upload failed, %s\n", info.errorMsg.c_str());
  }
}

void setup() {

  pinMode(POTE,INPUT);

  Serial.begin(115200);
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

  // For sending payload callback
  // config.cfs.upload_callback = fcsUploadCallback;

  // You can use TCP KeepAlive in FirebaseData object and tracking the server connection status, please read this for detail.
  // https://github.com/mobizt/Firebase-ESP-Client#about-firebasedata-object
  // fbdo.keepAlive(5, 5, 1);
}

void loop() {

  valorPote = analogRead(POTE);

  // Firebase.ready() should be called repeatedly to handle authentication tasks.

  if (Firebase.ready() && (millis() - dataMillis > 60000 || dataMillis == 0)) {
    dataMillis = millis();

    // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create_Edit_Parse/Create_Edit_Parse.ino
    FirebaseJson content;

    // Note: If new document created under non-existent ancestor documents, that document will not appear in queries and snapshot
    // https://cloud.google.com/firestore/docs/using-console#non-existent_ancestor_documents.

    // We will create the document in the parent path "a0/b?
    // a0 is the collection id, b? is the document id in collection a0.

    String documentPath = "FLEXs/flex" + String(count + 1);

    // If the document path contains space e.g. "a b c/d e f"
    // It should encode the space as %20 then the path will be "a%20b%20c/d%20e%20f"

    content.set("fields/Saludo1/stringValue", "Holi");

    // boolean
    content.set("fields/myBool/booleanValue", true);

    // integer
    content.set("fields/myInteger/integerValue", String(random(500, 1000)));


    String doc_path = "projects/";
    doc_path += FIREBASE_PROJECT_ID;
    doc_path += "/databases/(default)/documents/coll_id/doc_id";  // coll_id and doc_id are your collection id and document id

    // reference
    content.set("fields/myRef/referenceValue", doc_path.c_str());

    // timestamp
    content.set("fields/myTimestamp/timestampValue", "2014-10-02T15:01:23Z");  // RFC3339 UTC "Zulu" format

    // array
    content.set("fields/myArray/arrayValue/values/[0]/stringValue", "test");
    content.set("fields/myArray/arrayValue/values/[1]/integerValue", "20");
    content.set("fields/myArray/arrayValue/values/[2]/booleanValue", true);


    count++;

    Serial.print("Create a document... ");

    if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw()))
      Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
    else
      Serial.println(fbdo.errorReason());
  }
}


