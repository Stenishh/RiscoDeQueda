#include <Wire.h>
#include <MPU6050_tockn.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Configuração do MPU6050
MPU6050 mpu6050(Wire);

// Configurações Wi-Fi
const char* ssid = "WLL-Inatel";         // Insira o nome da sua rede Wi-Fi
const char* password = "inatelsemfio";   // Insira a senha da sua rede Wi-Fi

// Configurações MQTT
const char* mqtt_server = "test.mosquitto.org"; // Substitua pelo IP do seu MacBook
const int mqtt_port = 1883;                // Porta desicriptada padrão do broker MQTT
const char* mqtt_topic = "mpu6050/status"; // Tópico MQTT para publicar os estados

WiFiClient espClient;
PubSubClient client(espClient);

// Função para reconectar ao broker MQTT
void reconnect() {
  // Loop até que esteja conectado
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    // Tente se conectar
    if (client.connect("ESP8266Client")) { // Nome único do cliente MQTT
      Serial.println("conectado!");
      // Você pode se inscrever em tópicos aqui se desejar
      // client.subscribe("outTopic");
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos...");
      // Aguarde 5 segundos antes de tentar novamente
      delay(5000);
    }
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
  Serial.println("\nWi-Fi conectado! Endereço IP: ");
  Serial.println(WiFi.localIP());

  // Configurar MQTT
  client.setServer(mqtt_server, mqtt_port);
  // Se desejar, defina um callback para receber mensagens
  // client.setCallback(callback);

  // Configurar pinos SDA e SCL no ESP8266
  Wire.begin(4, 5); // GPIO4 (D2) e GPIO5 (D1) no NodeMCU

  mpu6050.begin();
  mpu6050.setGyroOffsets(-4.46, 0.11, 0.14);  // Substitua pelos valores obtidos
  Serial.println("MPU6050 inicializado com offsets aplicados");
}

void loop() {
  // Verificar conexão com o broker
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Atualizar dados do MPU6050
  mpu6050.update();
  float aceX = mpu6050.getAccX();
  float aceY = mpu6050.getAccY();
  float aceZ = mpu6050.getAccZ();

  float gyroX = mpu6050.getGyroX();
  float gyroY = mpu6050.getGyroY();
  float gyroZ = mpu6050.getGyroZ();

  String status;

  // Detectar estado
  if (abs(aceX - 0.31) < 0.1 && abs(aceY - 0.03) < 0.1 && abs(aceZ - 0.19) < 0.1 &&
      abs(gyroX - 8.40) < 1.0 && abs(gyroY - 6.76) < 1.0 && abs(gyroZ - 6.14) < 1.0) {
    status = "CAMINHANDO";
  } 
  else if (abs(aceX - 0.00) < 0.1 && abs(aceY - 0.02) < 0.1 && abs(aceZ - 0.01) < 0.1 &&
           abs(gyroX - 6.06) < 1.0 && abs(gyroY - 1.68) < 1.0 && abs(gyroZ - 1.39) < 1.0) {
    status = "SENTADO";
  }
  else if (abs(aceX - 0.00) < 0.1 && abs(aceY - 0.02) < 0.1 && abs(aceZ - 0.11) < 0.1 &&
           abs(gyroX - 13.68) < 1.0 && abs(gyroY - 90.67) < 1.0 && abs(gyroZ - 3.91) < 1.0) {
    status = "AGACHADO";
  }
  else if (abs(aceX - 1.45) < 0.1 && abs(aceY - 2.02) < 0.1 && abs(aceZ - 0.65) < 0.1 &&
           abs(gyroX - 0.00) < 1.0 && abs(gyroY - 100.06) < 1.0 && abs(gyroZ - 43.07) < 1.0) {
    status = "CAIU";
  }
  else {
    status = "NORMAL";
  }

  // Publicar no MQTT
  if (status.length() > 0) {
    client.publish(mqtt_topic, status.c_str());
    Serial.println("Publicado no MQTT: " + status);
  }

  delay(1000);
}
