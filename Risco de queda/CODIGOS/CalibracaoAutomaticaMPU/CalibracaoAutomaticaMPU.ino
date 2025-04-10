#include <Wire.h>
#include <MPU6050_tockn.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Configurações do MPU6050
MPU6050 mpu6050(Wire);

// Configurações Wi-Fi
const char* ssid = "";  // Nome da sua rede Wi-Fi
const char* password = "";    // Senha da sua rede Wi-Fi

// Configurações MQTT
const char* mqtt_server = ""; // IP onde o Mosquitto está rodando
const int mqtt_port = 1883;                 // Porta do Mosquitto
const char* mqtt_topic = "mpu6050/status";  // Tópico para envio de mensagens

WiFiClient espClient;
PubSubClient client(espClient);

// Variáveis de referência dinâmica
float refAccX = 0, refAccY = 0, refAccZ = 0;
float refGyroX = 0, refGyroY = 0, refGyroZ = 0;

// Variável para armazenar o status anterior
String lastStatus = "";

// Função para recalibrar as referências
void calibrateReference() {
  const int numLeituras = 100;
  float sumAccX = 0, sumAccY = 0, sumAccZ = 0;
  float sumGyroX = 0, sumGyroY = 0, sumGyroZ = 0;

  Serial.println("Calibrando novas referências...");
  for (int i = 0; i < numLeituras; i++) {
    mpu6050.update();
    sumAccX += mpu6050.getAccX();
    sumAccY += mpu6050.getAccY();
    sumAccZ += mpu6050.getAccZ();
    sumGyroX += mpu6050.getGyroX();
    sumGyroY += mpu6050.getGyroY();
    sumGyroZ += mpu6050.getGyroZ();
    delay(10);
  }

  refAccX = sumAccX / numLeituras;
  refAccY = sumAccY / numLeituras;
  refAccZ = sumAccZ / numLeituras;
  refGyroX = sumGyroX / numLeituras;
  refGyroY = sumGyroY / numLeituras;
  refGyroZ = sumGyroZ / numLeituras;

  Serial.println("Novas referências calibradas:");
}

void reconnectMQTT() {
  static unsigned long lastAttempt = 0;
  const unsigned long retryInterval = 5000; // 5 segundos entre tentativas

  if (!client.connected() && millis() - lastAttempt >= retryInterval) {
    Serial.println("Tentando reconectar ao MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("Conectado ao MQTT com sucesso!");
    } else {
      Serial.print("Falha na reconexão MQTT. Código de erro: ");
      Serial.println(client.state());
    }
    lastAttempt = millis();
  }
}

void setup() {
  Serial.begin(115200);

  // Inicializar Wi-Fi
  Serial.println("Conectando ao Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");

  // Configurar MQTT
  client.setServer(mqtt_server, mqtt_port);

  // Inicializar MPU6050
  Wire.begin(4, 5);
  mpu6050.begin();

  // Calibrar offsets e referências dinâmicas
  mpu6050.calcGyroOffsets(true);
  calibrateReference();
}

void loop() {
  reconnectMQTT(); // Tenta reconectar ao MQTT, mas sem bloquear o loop principal
  client.loop();

  // Atualizar leituras do MPU6050
  mpu6050.update();

  float accX = mpu6050.getAccX();
  float accY = mpu6050.getAccY();
  float accZ = mpu6050.getAccZ();
  float gyroX = mpu6050.getGyroX();
  float gyroY = mpu6050.getGyroY();
  float gyroZ = mpu6050.getGyroZ();

  // Exibir leituras no Serial Monitor
  Serial.println("===== Leituras do Sensor =====");
  Serial.print("AccX: "); Serial.print(accX);
  Serial.print(" | AccY: "); Serial.print(accY);
  Serial.print(" | AccZ: "); Serial.println(accZ);
  Serial.print("GyroX: "); Serial.print(gyroX);
  Serial.print(" | GyroY: "); Serial.print(gyroY);
  Serial.print(" | GyroZ: "); Serial.println(gyroZ);

  // Determinar o status atual
  String status;
  if (accY < -0.5) { // Limite para detectar queda no eixo Y
    status = "QUEDA DETECTADA";
  } else if (abs(accX - refAccX) < 0.2 && abs(accY - refAccY) < 0.2 && abs(accZ - refAccZ) < 0.2 &&
             abs(gyroX - refGyroX) < 5.0 && abs(gyroY - refGyroY) < 5.0 && abs(gyroZ - refGyroZ) < 5.0) {
    status = "PARADO";
  } else {
    status = "EM MOVIMENTO";
  }

  // Enviar mensagem ao MQTT apenas se o status mudar
  if (status != lastStatus) {
    if (client.connected()) {
      if (client.publish(mqtt_topic, status.c_str())) {
        Serial.println("Mensagem publicada no MQTT: " + status);
      } else {
        Serial.println("Falha ao publicar mensagem no MQTT.");
      }
    }
    lastStatus = status; // Atualizar o status anterior
  }

  // Exibir status no Serial Monitor
  Serial.println("Status: " + status);
  Serial.println("==============================");

  delay(1000);
}
