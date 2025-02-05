#include <Arduino.h>
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <FirebaseClient.h>
#include <WiFiClientSecure.h>


#define WIFI_SSID "UW MPSK"
#define WIFI_PASSWORD "u/K5ak6z*e" // Replace with your network password

#define DATABASE_SECRET "AIzaSyDrKIKulzd-9mAwQw1Jh4JG-Bo-aY_-JZA" // Replace with your database URL
#define DATABASE_URL "https://power-18c06-default-rtdb.firebaseio.com/" // Replace with your API key

#define STAGE_INTERVAL 12000 // 12 seconds each stage
#define MAX_WIFI_RETRIES 5 // Maximum number of WiFi connection retries

#define uploadInterval_2HZ 500   // 2 times per second
#define uploadInterval_1HZ 1000  // 1 time per second
#define uploadInterval_05HZ 2000 // Once every 2 seconds
#define uploadInterval_033HZ 3000 // Once every 3 seconds
#define uploadInterval_025HZ 4000 // Once every 4 seconds

WiFiClientSecure ssl;
DefaultNetwork network;
AsyncClientClass client(ssl, getNetwork(network));

FirebaseApp app;
RealtimeDatabase Database;
AsyncResult result;
LegacyToken dbSecret(DATABASE_SECRET);

int uploadInterval = 1000; // 1 seconds each upload
unsigned long sendDataPrevMillis = 0;
int count = 0;
// bool signupOK = false;

// HC-SR04 Pins
const int trigPin = D9;
const int echoPin = D10;

// Define sound speed in cm/usec
const float soundSpeed = 0.034;

//functions
float measureDistance();
void connectToWiFi();
void initFirebase();
void sendDataToFirebase(float distance, int uploadInterval);

void printError(int code, const String &msg)
{
    Firebase.printf("Error, msg: %s, code: %d\n", msg.c_str(), code);
}

void setup(){
  Serial.begin(115200);
  delay(500);
  // while(!Serial){
  //   yield();
  // }

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  
  connectToWiFi();
  unsigned long startTime = millis();

  // Now, turn on Firebase and send data 2 times per second with distance measurements
  Serial.println("Turning on Firebase and sending data 2 times per second...");
  initFirebase();
  startTime = millis();
  while (millis() - startTime < STAGE_INTERVAL)
  {
    float currentDistance = measureDistance();
    sendDataToFirebase(currentDistance, uploadInterval_2HZ);
    delay(100); // Delay between measurements
  }

// Now, turn on Firebase and send data 1 time per second with distance measurements
  Serial.println("Turning on Firebase and sending data 1 time per second...");
  initFirebase();
  startTime = millis();
  while (millis() - startTime < STAGE_INTERVAL)
  {
    float currentDistance = measureDistance();
    sendDataToFirebase(currentDistance, uploadInterval_1HZ);
    delay(100); // Delay between measurements
  }


  // Now, turn on Firebase and send data Once every 2 seconds with distance measurements
  Serial.println("Turning on Firebase and sending data Once every 2 seconds...");
  initFirebase();
  startTime = millis();
  while (millis() - startTime < STAGE_INTERVAL)
  {
    float currentDistance = measureDistance();
    sendDataToFirebase(currentDistance, uploadInterval_05HZ);
    delay(100); // Delay between measurements
  }


// Now, turn on Firebase and send data Once every 3 seconds with distance measurements
  Serial.println("Turning on Firebase and sending data Once every 3 seconds...");
  initFirebase();
  startTime = millis();
  while (millis() - startTime < STAGE_INTERVAL)
  {
    float currentDistance = measureDistance();
    sendDataToFirebase(currentDistance, uploadInterval_033HZ);
    delay(100); // Delay between measurements
  }


// Now, turn on Firebase and send data Once every 4 seconds with distance measurements
  Serial.println("Turning on Firebase and sending data Once every 4 seconds...");
  initFirebase();
  startTime = millis();
  while (millis() - startTime < STAGE_INTERVAL)
  {
    float currentDistance = measureDistance();
    sendDataToFirebase(currentDistance, uploadInterval_025HZ);
    delay(100); // Delay between measurements
  }

  // Go to deep sleep for 12 seconds
  Serial.println("Going to deep sleep for 12 seconds...");
  WiFi.disconnect();
  esp_sleep_enable_timer_wakeup(STAGE_INTERVAL * 1000); // in microseconds
  esp_deep_sleep_start();
  
}

void loop(){
  // This is not used
}

float measureDistance()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  float distance = duration * soundSpeed / 2;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  return distance;
}

void connectToWiFi()
{
  // Print the device's MAC address.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to WiFi");
}

void initFirebase(){
  Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);

    ssl.setInsecure();
#if defined(ESP8266)
    ssl.setBufferSizes(1024, 1024);
#endif

    // Initialize the authentication handler.
    Serial.println("Initializing the app...");
    initializeApp(client, app, getAuth(dbSecret));

    // Binding the authentication handler with your Database class object.
    app.getApp<RealtimeDatabase>(Database);

    // Set your database URL (requires only for Realtime Database)
    Database.url(DATABASE_URL);

    // In sync functions, we have to set the operating result for the client that works with the function.
    client.setAsyncResult(result);
}

void sendDataToFirebase(float distance, int uploadInterval){
  if (millis() - sendDataPrevMillis > uploadInterval || sendDataPrevMillis == 0){
    sendDataPrevMillis = millis();

    Serial.print("Pushing the float value... ");
    String name = Database.push<number_t>(client, "/test/distance", number_t(distance));
    if (client.lastError().code() == 0){
        Firebase.printf("ok, name: %s\n", name.c_str());
        count ++;
    }
  }
}