#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

/****** WiFi Connection Details *******/
const char* ssid = "Tesla IoT";
const char* password = "fsL6HgjN";

/******* MQTT Broker Connection Details *******/
const char* mqtt_server = "e484f92d045b4deab9f074f91ece9b0f.s2.eu.hivemq.cloud";
const char* mqtt_username = "MusaOpdracht";
const char* mqtt_password = "Maassluis66";
const int mqtt_port = 8883;
const char* mqtt_topic = "visitor count";

/**** Secure WiFi Connectivity Initialization *****/
WiFiClientSecure espClient;

/**** MQTT Client Initialization Using WiFi Connection *****/
PubSubClient client(espClient);

const int trigPin = D1;
const int echoPin = D2;
const int distanceThreshold = 25; // Afstand in centimeters om te activeren

int visitorCount = 0;
bool visitorDetected = false;

void setup_wifi() {
  delay(10);
  Serial.print("\nConnecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected\nIP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void publishMessage(const char* topic, String payload, boolean retained) {
  if (client.publish(topic, payload.c_str(), retained))
    Serial.println("Message published [" + String(topic) + "]: " + payload);
}

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(9600);
  while (!Serial)
    ;
  setup_wifi();
  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected())
    reconnect();
  client.loop();

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 10000000); // Verhoogde ultrasone pulsduur naar 5 seconden
  int distance = duration * 0.034 / 2;

  //Serial.print("Distance: ");
  //Serial.println(distance);

  if (distance <= distanceThreshold && !visitorDetected) {
    visitorCount++;

    DynamicJsonDocument doc(128);
    doc["visitor count "] = visitorCount;

    char mqtt_message[128];
    serializeJson(doc, mqtt_message);

    publishMessage(mqtt_topic, mqtt_message, true);

    visitorDetected = true;
  } else if (distance > distanceThreshold) {
    visitorDetected = false;
  }

  delay(100); // Verlaagde de vertraging tussen metingen naar 500 milliseconden
}
