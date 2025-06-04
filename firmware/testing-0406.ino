#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// ===== WiFi Credentials =====
struct WiFiCredentials
{
  const char *ssid;
  const char *password;
};

WiFiCredentials wifiList[] = {
    {"FSB-202", "fsb@123aB"},
    {"FSB-Student", "fsb@student"},
    {"PATO", "EFGH1234!"}};
const int wifiCount = sizeof(wifiList) / sizeof(wifiList[0]);

// ===== API URLs =====
const char *base_url = "https://fesmartlightcontroliotrailwayzero-production.up.railway.app";
const char *api_url = "/api/control";
const char *delay_api_url = "/api/delay";
const char *src_api_url = "/api/src";

// ===== MQTT - HiveMQ Cloud =====
const char *mqtt_server = "21583ec947f6453f8e3793f0b34dde0b.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char *mqtt_user = "testmse21";
const char *mqtt_pass = "Testmse21dn!";
const char *mqtt_topic = "lamp/state";

// ===== Pins =====
const int ledPin = D2;
const int motionPin = D5;

// ===== Clients =====
WiFiClientSecure secureClient;
PubSubClient mqttClient(secureClient);

// ===== Timing Variables =====
unsigned long lastHttpRequestTime = 0;
unsigned long httpInterval = 5000;
unsigned long lastMotionTime = 0;
const unsigned long motionTimeout = 10000;
bool motionActive = false;
String srcValue = "DB";

// ===== Connect to available WiFi =====
void connectToWiFi()
{
  Serial.println("Scanning saved WiFi networks...");
  int n = WiFi.scanNetworks();
  for (int i = 0; i < wifiCount; i++)
  {
    for (int j = 0; j < n; j++)
    {
      if (WiFi.SSID(j) == wifiList[i].ssid)
      {
        Serial.print("Connecting to: ");
        Serial.println(wifiList[i].ssid);
        WiFi.begin(wifiList[i].ssid, wifiList[i].password);
        unsigned long startAttempt = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 15000)
        {
          delay(500);
          Serial.print(".");
        }
        if (WiFi.status() == WL_CONNECTED)
        {
          Serial.println("\nWiFi connected!");
          Serial.print("IP address: ");
          Serial.println(WiFi.localIP());
          return;
        }
        Serial.println("\nConnection failed. Trying next...");
      }
    }
  }
  Serial.println("No known WiFi networks found.");
}

// ===== MQTT Callback =====
void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");

  String message;
  for (unsigned int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    message += (char)payload[i];
  }
  Serial.println();

  Serial.print("Parsed message: ");
  Serial.println(message);

  if (message == "1")
  {
    digitalWrite(ledPin, HIGH);
    Serial.println("LED turned ON by MQTT");
  }
  else if (message == "0")
  {
    digitalWrite(ledPin, LOW);
    Serial.println("LED turned OFF by MQTT");
  }
  else
  {
    Serial.println("Unknown MQTT message");
  }
}

// ===== MQTT Connection =====
void reconnectMQTT()
{
  while (!mqttClient.connected())
  {
    Serial.print("Connecting to MQTT...");
    if (mqttClient.connect("ESP8266Client", mqtt_user, mqtt_pass))
    {
      Serial.println("MQTT connected!");
      if (mqttClient.subscribe(mqtt_topic))
      {
        Serial.print("Subscribed to topic: ");
        Serial.println(mqtt_topic);
      }
      else
      {
        Serial.println("Subscription failed!");
      }
    }
    else
    {
      Serial.print("MQTT connection failed, state: ");
      Serial.println(mqttClient.state());
      delay(5000);
    }
  }
}

// ===== Fetch Delay from API =====
void fetchDelayFromAPI()
{
  if (WiFi.status() != WL_CONNECTED)
    return;

  HTTPClient http;
  String fullUrl = String(base_url) + delay_api_url;
  Serial.print("Calling API: ");
  Serial.println(fullUrl);

  if (!http.begin(secureClient, fullUrl))
  {
    Serial.println("HTTP begin failed!");
    return;
  }

  http.addHeader("Content-Type", "application/json");

  int httpCode = http.GET();
  Serial.print("HTTP response code: ");
  Serial.println(httpCode);

  if (httpCode == 200)
  {
    String payload = http.getString();
    Serial.println("API response:");
    Serial.println(payload);

    StaticJsonDocument<200> doc;
    if (deserializeJson(doc, payload) == DeserializationError::Ok)
    {
      float dt = doc["dt_value"];
      httpInterval = (unsigned long)(dt * 1000);
      Serial.print("Updated delay interval: ");
      Serial.println(httpInterval);
    }
    else
    {
      Serial.println("Failed to parse JSON");
    }
  }
  else
  {
    Serial.print("Bad response: ");
    Serial.println(http.getString());
  }

  http.end();
}

// ===== Fetch Source Control from API =====
void fetchSourceControlFromAPI()
{
  if (WiFi.status() != WL_CONNECTED)
    return;

  HTTPClient http;
  String fullUrl = String(base_url) + src_api_url;
  Serial.print("Calling API: ");
  Serial.println(fullUrl);

  if (!http.begin(secureClient, fullUrl))
  {
    Serial.println("HTTP begin failed!");
    return;
  }

  http.addHeader("Content-Type", "application/json");

  int httpCode = http.GET();
  Serial.print("HTTP response code: ");
  Serial.println(httpCode);

  if (httpCode == 200)
  {
    String payload = http.getString();
    Serial.println("API response:");
    Serial.println(payload);

    StaticJsonDocument<200> doc;
    if (deserializeJson(doc, payload) == DeserializationError::Ok)
    {
      srcValue = doc["src_value"].as<String>();
      Serial.print("Updated source control: ");
      Serial.println(srcValue);
    }
    else
    {
      Serial.println("Failed to parse JSON");
    }
  }
  else
  {
    Serial.print("Bad response: ");
    Serial.println(http.getString());
  }

  http.end();
}

// ===== Handle REST API Poll =====
void handleRestApi()
{
  if (WiFi.status() != WL_CONNECTED)
    return;

  HTTPClient http;
  String fullUrl = String(base_url) + api_url;
  http.begin(secureClient, fullUrl);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.GET();
  Serial.print("HTTP code (control API): ");
  Serial.println(httpCode);

  if (httpCode == 200)
  {
    String payload = http.getString();
    StaticJsonDocument<200> doc;
    if (deserializeJson(doc, payload) == DeserializationError::Ok)
    {
      int command = doc["command"];
      digitalWrite(ledPin, command == 1 ? HIGH : LOW);
    }
    else
    {
      Serial.println("Failed to parse REST JSON");
    }
  }
  else
  {
    Serial.print("REST API error code: ");
    Serial.println(httpCode);
  }

  http.end();
}

// ===== Setup =====
void setup()
{
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(motionPin, INPUT);
  connectToWiFi();

  secureClient.setInsecure(); // Accept all certs (not for production)
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);
  Serial.println("MQTT setup complete.");

  fetchDelayFromAPI();
}

// ===== Main Loop =====
void loop()
{
  fetchSourceControlFromAPI();
  if (srcValue == "MQTT") {
    if (!mqttClient.connected())
      reconnectMQTT();
    mqttClient.loop();
  } 
  else if (srcValue == "MOTION") {
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
  }
  else if (srcValue == "DB") {
    if (millis() - lastHttpRequestTime >= httpInterval) {
      lastHttpRequestTime = millis();
      handleRestApi();
      fetchDelayFromAPI();
    }
  }
}
