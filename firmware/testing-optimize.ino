#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// ===== WiFi Credentials =====
struct WiFiCredentials {
  const char* ssid;
  const char* password;
};

WiFiCredentials wifiList[] = {
  {"FSB-202", "fsb@123aB"},
  {"FSB-Student", "fsb@student"},
  {"PATO", "EFGH1234!"}
};
const int wifiCount = sizeof(wifiList) / sizeof(wifiList[0]);

// ===== API URLs (Updated to Railway App) =====
const char* base_url = "https://fesmartlightcontroliotrailwayzero-production.up.railway.app";
const char* api_url = "/api/control";
const char* delay_api_url = "/api/delay";

// ===== MQTT - HiveMQ Cloud =====
const char* mqtt_server = "21583ec947f6453f8e3793f0b34dde0b.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "testmse21";
const char* mqtt_pass = "Testmse21dn!";
const char* mqtt_topic = "lamp/state";

// ===== Pins =====
const int ledPin = D2;     // GPIO4
const int motionPin = D5;  // GPIO14

// ===== Clients =====
WiFiClientSecure secureClient;
WiFiClient httpClient;
PubSubClient mqttClient(secureClient);

// ===== Timing Variables =====
unsigned long lastHttpRequestTime = 0;
unsigned long httpInterval = 5000;
unsigned long lastMotionTime = 0;
const unsigned long motionTimeout = 10000;
bool motionActive = false;

// ===== Connect to available WiFi =====
void connectToWiFi() {
  Serial.println("Dang tim WiFi da luu...");
  int n = WiFi.scanNetworks();
  for (int i = 0; i < wifiCount; i++) {
    for (int j = 0; j < n; j++) {
      if (WiFi.SSID(j) == wifiList[i].ssid) {
        Serial.print("Dang ket noi toi: ");
        Serial.println(wifiList[i].ssid);
        WiFi.begin(wifiList[i].ssid, wifiList[i].password);
        unsigned long startAttempt = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 15000) {
          delay(500);
          Serial.print(".");
        }
        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("\nDa ket noi WiFi!");
          Serial.print("IP: ");
          Serial.println(WiFi.localIP());
          return;
        }
        Serial.println("\nThat bai ket noi. Thu WiFi khac...");
      }
    }
  }
  Serial.println("Khong tim thay WiFi phu hop.");
}

// ===== MQTT Callback =====
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) message += (char)payload[i];

  Serial.print("MQTT: ");
  Serial.println(message);

  if (message == "1") {
    digitalWrite(ledPin, HIGH);
  } else if (message == "0") {
    digitalWrite(ledPin, LOW);
  }
}

// ===== MQTT Connection =====
void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Dang ket noi MQTT...");
    if (mqttClient.connect("ESP8266Client", mqtt_user, mqtt_pass)) {
      Serial.println("Da ket noi MQTT!");
      mqttClient.subscribe(mqtt_topic);
    } else {
      Serial.print("Loi MQTT: ");
      Serial.println(mqttClient.state());
      delay(5000);
    }
  }
}

// ===== Fetch Delay from API =====
void fetchDelayFromAPI() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String fullUrl = String(base_url) + delay_api_url;
  Serial.print("Goi GET: ");
  Serial.println(fullUrl);

  http.begin(secureClient, fullUrl);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.GET();

  Serial.print("Ma phan hoi delay API: ");
  Serial.println(httpCode);

  if (httpCode == 200) {
    String payload = http.getString();
    Serial.print("Phan hoi delay API: ");
    Serial.println(payload);

    StaticJsonDocument<200> doc;
    if (deserializeJson(doc, payload) == DeserializationError::Ok) {
      float dt = doc["dt_value"];
      httpInterval = (unsigned long)(dt * 1000);
      Serial.print("Cap nhat delay moi: ");
      Serial.println(httpInterval);
    } else {
      Serial.println("Loi parse JSON delay");
    }
  } else {
    Serial.print("Phan hoi khong hop le: ");
    Serial.println(http.getString());  // In body lỗi
  }

  http.end();
}


// ===== Handle REST API Poll =====
void handleRestApi() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String fullUrl = String(base_url) + api_url;
  http.begin(secureClient, fullUrl);
  http.addHeader("Content-Type", "application/json");
  
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    StaticJsonDocument<200> doc;
    if (deserializeJson(doc, payload) == DeserializationError::Ok) {
      int command = doc["command"];
      digitalWrite(ledPin, command == 1 ? HIGH : LOW);
    } else {
      Serial.println("Loi parse REST JSON");
    }
  } else {
    Serial.print("Loi REST HTTP: ");
    Serial.println(httpCode);
  }
  http.end();
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  // pinMode(motionPin, INPUT);
  connectToWiFi();

  // secureClient.setInsecure(); // Accept all certs (dev only!)
  // mqttClient.setServer(mqtt_server, mqtt_port);
  // mqttClient.setCallback(mqttCallback);

  // fetchDelayFromAPI();
}

// ===== Main Loop =====
void loop() {
  if (!mqttClient.connected()) reconnectMQTT();
  mqttClient.loop();

  int motionValue = digitalRead(motionPin);
  if (motionValue == HIGH) {
    if (!motionActive) {
      digitalWrite(ledPin, HIGH);
      mqttClient.publish(mqtt_topic, "motion ON");
      motionActive = true;
    }
    lastMotionTime = millis();
  } else if (motionActive && millis() - lastMotionTime > motionTimeout) {
    digitalWrite(ledPin, LOW);
    mqttClient.publish(mqtt_topic, "motion OFF");
    motionActive = false;
  }

  if (millis() - lastHttpRequestTime >= httpInterval) {
    lastHttpRequestTime = millis();
    handleRestApi();
    fetchDelayFromAPI();
  }
}
