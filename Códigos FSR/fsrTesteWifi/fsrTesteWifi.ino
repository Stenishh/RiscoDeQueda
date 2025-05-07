#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "secretsFSR.h"  // Inclui o arquivo de segredos

// Wi-Fi credentials
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// MQTT broker details private
const char* mqtt_broker = MQTT_BROKER;
const int mqtt_port = 8883;
const char* mqtt_username = MQTT_USER;
const char* mqtt_password = MQTT_PASSWORD;

// MQTT topic for IR sensor
const char* topic_publish_fsr = "esp32/fsr";

// Variables for timing
long previous_time = 0;

// Create instances
WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

void setupMQTT() {
  mqttClient.setServer(mqtt_broker, mqtt_port);
}

void reconnect() {
  Serial.println("Connecting to MQTT Broker...");
  while (!mqttClient.connected()) {
    Serial.println("Reconnecting to MQTT Broker...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    if (mqttClient.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Connected to MQTT Broker.");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

const int fsrPin1 = 36;  // ADC1_CHANNEL_0 corresponde ao GPIO13 (VP)
const int fsrPin2 = 39;  // ADC1_CHANNEL_0 corresponde ao GPIO15 (VP)
const int fsrPin3 = 34;  // ADC1_CHANNEL_0 corresponde ao GPIO14 (VP)

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to Wi-Fi");

  // Initialize secure WiFiClient
  wifiClient.setInsecure(); // Use this only for testing, it allows connecting without a root certificate
  
  setupMQTT();

  pinMode(fsrPin1, INPUT); // Set IR sensor pin as input

  // Define resolução do ADC para 12 bits (padrão no ESP32)
  analogReadResolution(12);  // 0 a 4095

  // Define a atenuação para ler toda a faixa de 0 a 3.3V
  analogSetAttenuation(ADC_11db); // permite leitura até ~3.3V
}

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

  long now = millis();
  if (now - previous_time > 1000) { // Publish every 10 seconds
    previous_time = now;

    int fsrReading1 = analogRead(fsrPin1);
    int fsrReading2 = analogRead(fsrPin2);
    int fsrReading3 = analogRead(fsrPin3);

    String fsrR1_str = String(fsrReading1);

    // Envia os valores dos sensores no formato CSV (valores separados por vírgulas)
    Serial.print(fsrReading1);
    Serial.print(",");
    Serial.print(fsrReading2);
    Serial.print(",");
    Serial.println(fsrReading3);
    mqttClient.publish(topic_publish_fsr, fsrR1_str.c_str());

    delay(200);

  }
}
