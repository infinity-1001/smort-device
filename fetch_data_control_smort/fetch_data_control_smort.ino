#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <ESP32Servo.h>
#include <WiFiUdp.h>

// WiFi and Firestore Settings
const char* ssid = "SmartMedic";
const char* password = "sm@1234567";
const char* apiKey = "AIzaSyAfxhAhUlpxYdIJ6wV1RWWMExyeZOYoeg";
const char* baseURL = "https://firestore.googleapis.com/v1/projects/smort-care/databases/(default)/documents/";

// NTP Client Setup
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000);

// Servo and Buzzer Setup
Servo myServo;
const int servoPin = 13;
const int buzzerPin = 12;

// Line Following Sensor and Motor Setup
const int leftSensorPin = 27;
const int rightSensorPin = 26;
const int leftMotorPin1 = 5;
const int leftMotorPin2 = 18;
const int rightMotorPin1 = 19;
const int rightMotorPin2 = 21;

// Additional Sensor Pins
const int bringWaterPin = 23; // Adjust to your actual pin
const int thankyouPin = 25;   // Adjust to your actual pin

// Medication Data Structure
struct Medication {
  int sectionNumber;
  String time;
};
Medication medications[10];
int medicationCount = 0;

// Servo Positions Array for Each Section
const int servoPositions[] = {36, 64, 90, 117, 145, 180};
const int numSections = sizeof(servoPositions) / sizeof(servoPositions[0]);

// Timing Variables
unsigned long lastFetchTime = 0;
const unsigned long fetchInterval = 5 * 60 * 1000; // 5 minutes

// Line Following Mode Flag
bool lineFollowingMode = false;

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
   
  // WiFi Connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Start NTP Client
  timeClient.begin();

  // Setup Servo, Buzzer, and Additional Sensors
  myServo.attach(servoPin);
  pinMode(buzzerPin, OUTPUT);
  pinMode(bringWaterPin, INPUT);
  pinMode(thankyouPin, INPUT);

  // Setup Line Following Sensors and Motors
  pinMode(leftSensorPin, INPUT);
  pinMode(rightSensorPin, INPUT);
  pinMode(leftMotorPin1, OUTPUT);
  pinMode(leftMotorPin2, OUTPUT);
  pinMode(rightMotorPin1, OUTPUT);
  pinMode(rightMotorPin2, OUTPUT);

  // Fetch initial Firestore data
  fetchFirestoreData();
}

void fetchFirestoreData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(baseURL) + "smort/smort_test/medications?key=" + apiKey;

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("Fetched medication data: " + payload);

      DynamicJsonDocument doc(4096);
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        JsonArray medicationsArray = doc["documents"].as<JsonArray>();
        medicationCount = 0;
        for (JsonObject medication : medicationsArray) {
          JsonObject fields = medication["fields"].as<JsonObject>();
          medications[medicationCount].sectionNumber = fields["sectionNumber"]["integerValue"].as<int>();
          medications[medicationCount].time = fields["time"]["stringValue"] | "N/A";
          medicationCount++;
        }
      } else {
        Serial.println("JSON parsing failed");
      }
    }
    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}

void loop() {
  timeClient.update();
  String currentTime = timeClient.getFormattedTime();
  Serial.println("Current Time: " + currentTime);

  // Check if bringWater or thankyou is triggered
  if (digitalRead(bringWaterPin) == HIGH || digitalRead(thankyouPin) == HIGH) {
    lineFollowingMode = true;
    digitalWrite(buzzerPin,HIGH);
    resumeMotion(); // Initiate motion
  }

  // Check for scheduled medication dispensing
  for (int i = 0; i < medicationCount; i++) {
    if (isTimeMatching(currentTime, medications[i].time)) {
      Serial.print("Time for medication at section: ");
      Serial.println(medications[i].sectionNumber);

      // Activate line following mode, beep, and move to start following the line
      lineFollowingMode = true;
      digitalWrite(buzzerPin, HIGH);
      rotateServo(medications[i].sectionNumber);
      resumeMotion(); // Initiate motion before line following

      // Follow the line until reaching the destination
      while(lineFollowingMode) {
        followLine();
      }
      delay(60000);
    }
  }

  // Execute line-following if mode is activated
  if (lineFollowingMode) {
    followLine();
  }

  // Update medications every 5 minutes
  unsigned long currentMillis = millis();
  if (currentMillis - lastFetchTime >= fetchInterval) {
    lastFetchTime = currentMillis;
    fetchFirestoreData();
  }
  delay(1000);
}

void resumeMotion() {
  moveForward();
  delay(3000);
  followLine();
}

bool isTimeMatching(String currentTime, String medicationTime) {
  int currentSeconds = timeToSeconds(currentTime);
  int medicationSeconds = timeToSeconds(medicationTime);
  return abs(currentSeconds - medicationSeconds) <= 5;
}

int timeToSeconds(String time) {
  int hours, minutes, seconds;
  sscanf(time.c_str(), "%d:%d:%d", &hours, &minutes, &seconds);
  return hours * 3600 + minutes * 60 + seconds;
}

void followLine() {
  int leftSensor = digitalRead(leftSensorPin);
  int rightSensor = digitalRead(rightSensorPin);

  if (leftSensor == HIGH && rightSensor == HIGH) {
    // Stop if both sensors detect black (reached destination)
    stopCar();
    digitalWrite(buzzerPin, LOW);
    lineFollowingMode = false; // Stop line-following mode
  } else if (leftSensor == HIGH && rightSensor == LOW) {
    turnLeft();
  } else if (leftSensor == LOW && rightSensor == HIGH) {
    turnRight();
  } else {
    moveForward();
  }
}

void moveForward() {
  digitalWrite(leftMotorPin1, LOW);
  digitalWrite(leftMotorPin2, HIGH);
  digitalWrite(rightMotorPin1, HIGH);
  digitalWrite(rightMotorPin2, LOW);
}

void turnRight() {
  digitalWrite(leftMotorPin1, LOW);
  digitalWrite(leftMotorPin2, HIGH);
  digitalWrite(rightMotorPin1, LOW);
  digitalWrite(rightMotorPin2, LOW);
}

void turnLeft() {
  digitalWrite(leftMotorPin1, LOW);
  digitalWrite(leftMotorPin2, LOW);
  digitalWrite(rightMotorPin1, HIGH);
  digitalWrite(rightMotorPin2, LOW);
}

void stopCar() {
  digitalWrite(leftMotorPin1, LOW);
  digitalWrite(leftMotorPin2, LOW);
  digitalWrite(rightMotorPin1, LOW);
  digitalWrite(rightMotorPin2, LOW);
}


void rotateServo(int sectionNumber) {
  if (sectionNumber >= 1 && sectionNumber <= numSections) {
    int servoPosition = servoPositions[sectionNumber - 1];
    myServo.write(servoPosition);
    Serial.print("Rotating to section: ");
    Serial.println(sectionNumber);
    delay(1000);
  } else {
    Serial.println("Invalid section number");
  }
}
