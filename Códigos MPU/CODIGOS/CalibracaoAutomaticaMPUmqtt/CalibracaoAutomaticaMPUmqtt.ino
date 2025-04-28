#include <Wire.h>
#include <MPU6050_tockn.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Configurações Wi-Fi
const char* ssid = "WLL-Inatel";
const char* password = "inatelsemfio";

// Configurações MQTT
const char* mqtt_server = "192.168.68.108";
const int mqtt_port = 1883;
const char* mqtt_topic = "mpu6050/status";

WiFiClient espClient;
PubSubClient client(espClient);

// Configurações do MPU6050 - defina o endereço correto
const uint8_t MPU6050_ADDRESS = 0x68;  // Endereço padrão, pode ser 0x69 se AD0 estiver HIGH
MPU6050 mpu6050(Wire);

// Variáveis de referência dinâmica
float refAccX = 0, refAccY = 0, refAccZ = 0;
float refGyroX = 0, refGyroY = 0, refGyroZ = 0;
String lastStatus = "";

// Limiares para detecção de movimento
const float ACC_THRESHOLD = 0.05;
const float GYRO_THRESHOLD = 1.0;
const float FALL_THRESHOLD = 0.5;  // Ajuste o limiar de queda

bool mpu6050Connected = false;

// Função para verificar quais dispositivos I2C estão conectados
void scanI2CDevices() {
  byte error, address;
  int nDevices = 0;
  
  Serial.println("Escaneando dispositivos I2C...");
  
  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("Dispositivo I2C encontrado no endereço 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      Serial.println();
      
      nDevices++;
      
      // Se encontrarmos o MPU6050 no endereço esperado
      if (address == MPU6050_ADDRESS) {
        Serial.println("MPU6050 encontrado no endereço correto!");
      }
    }
    else if (error == 4) {
      Serial.print("Erro desconhecido no endereço 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }
  
  if (nDevices == 0) {
    Serial.println("Nenhum dispositivo I2C encontrado! Verifique as conexões.");
  } else {
    Serial.print("Encontrados ");
    Serial.print(nDevices);
    Serial.println(" dispositivos I2C.");
  }
}

bool checkMPUConnection() {
  // Tentar se comunicar com o MPU6050
  Wire.beginTransmission(MPU6050_ADDRESS);
  byte error = Wire.endTransmission();
  
  if (error == 0) {
    Serial.println("MPU6050 conectado com sucesso!");
    return true;
  } else {
    Serial.print("Erro ao conectar ao MPU6050. Código de erro: ");
    Serial.println(error);
    return false;
  }
}

void calibrateReference() {
  const int numLeituras = 50;
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

  // Verificar se os valores são todos zeros
  if (sumAccX == 0 && sumAccY == 0 && sumAccZ == 0 &&
      sumGyroX == 0 && sumGyroY == 0 && sumGyroZ == 0) {
    Serial.println("ERRO: Todos os valores são zero. O sensor não está funcionando corretamente!");
    return;
  }

  refAccX = sumAccX / numLeituras;
  refAccY = sumAccY / numLeituras;
  refAccZ = sumAccZ / numLeituras;
  refGyroX = sumGyroX / numLeituras;
  refGyroY = sumGyroY / numLeituras;
  refGyroZ = sumGyroZ / numLeituras;

  Serial.println("Novas referências calibradas!");
  Serial.print("Referências: AccX=");
  Serial.print(refAccX);
  Serial.print(", AccY=");
  Serial.print(refAccY);
  Serial.print(", AccZ=");
  Serial.println(refAccZ);
}

void reconnectMQTT() {
  static unsigned long lastAttempt = 0;
  const unsigned long retryInterval = 5000;

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
  delay(3000);  // Delay maior para garantir tempo para abrir o monitor serial
  Serial.println("\n\n--- Iniciando sistema ---");

  // Inicializar I2C com os pinos corretos para NodeMCU
  Wire.begin(D2, D1);  // SDA = D2 (GPIO4), SCL = D1 (GPIO5)
  
  // Escanear dispositivos I2C para diagnóstico
  scanI2CDevices();
  
  // Verificar conexão com MPU6050
  mpu6050Connected = checkMPUConnection();
  
  if (mpu6050Connected) {
    // Inicializar MPU6050 apenas se estiver conectado
    Serial.println("Inicializando MPU6050...");
    mpu6050.begin();
    Serial.println("MPU6050 inicializado!");
    
    // Calibrar offsets e referências dinâmicas
    Serial.println("Calculando offsets do giroscópio...");
    mpu6050.calcGyroOffsets(true);
    Serial.println("Offsets calculados!");
    
    calibrateReference();
  } else {
    Serial.println("PROBLEMA: MPU6050 não conectado. Verifique as conexões:");
    Serial.println("1. O MPU6050 está alimentado corretamente? (3.3V para NodeMCU)");
    Serial.println("2. SDA está conectado ao pino D2 do NodeMCU?");
    Serial.println("3. SCL está conectado ao pino D1 do NodeMCU?");
    Serial.println("4. GND está conectado ao GND do NodeMCU?");
    Serial.println("5. O sensor está danificado?");
  }

  // Inicializar Wi-Fi
  Serial.println("Conectando ao Wi-Fi...");
  WiFi.begin(ssid, password);
  
  unsigned long wifiStartTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifiStartTime < 20000) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    
    // Configurar MQTT
    client.setServer(mqtt_server, mqtt_port);
  } else {
    Serial.println("\nFalha ao conectar Wi-Fi. Continuando sem conexão.");
  }
  
  Serial.println("--- Setup concluído ---");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    reconnectMQTT();
    client.loop();
  }

  if (!mpu6050Connected) {
    // Tentar reconectar ao MPU6050 a cada 5 segundos
    static unsigned long lastAttempt = 0;
    if (millis() - lastAttempt > 5000) {
      Serial.println("Tentando conectar ao MPU6050 novamente...");
      mpu6050Connected = checkMPUConnection();
      if (mpu6050Connected) {
        Serial.println("MPU6050 conectado! Reinicializando...");
        mpu6050.begin();
        mpu6050.calcGyroOffsets(true);
        calibrateReference();
      }
      lastAttempt = millis();
    }
    delay(1000);
    return;
  }

  // Atualizar leituras do MPU6050
  mpu6050.update();

  float accX = mpu6050.getAccX();
  float accY = mpu6050.getAccY();
  float accZ = mpu6050.getAccZ();
  float gyroX = mpu6050.getGyroX();
  float gyroY = mpu6050.getGyroY();
  float gyroZ = mpu6050.getGyroZ();

  // Verificar se todos os valores são zero (possível problema de comunicação)
  if (accX == 0 && accY == 0 && accZ == 0 && 
      gyroX == 0 && gyroY == 0 && gyroZ == 0) {
    Serial.println("ALERTA: Todos os valores são zero. Possível problema de comunicação!");
    mpu6050Connected = false;
    return;
  }

  // Calcular as diferenças em relação às referências
  float diffAccX = abs(accX - refAccX);
  float diffAccY = abs(accY - refAccY);
  float diffAccZ = abs(accZ - refAccZ);
  float diffGyroX = abs(gyroX - refGyroX);
  float diffGyroY = abs(gyroY - refGyroY);
  float diffGyroZ = abs(gyroZ - refGyroZ);
  
  // Calcular a magnitude total da aceleração
  float accMagnitude = sqrt(diffAccX*diffAccX + diffAccY*diffAccY + diffAccZ*diffAccZ);
  float gyroMagnitude = sqrt(diffGyroX*diffGyroX + diffGyroY*diffGyroY + diffGyroZ*diffGyroZ);

  // Exibir leituras no Serial Monitor
  Serial.println("\n===== Leituras do Sensor =====");
  Serial.print("AccX: "); Serial.print(accX, 4);  // Aumentar precisão decimal para debug
  Serial.print(" | AccY: "); Serial.print(accY, 4);
  Serial.print(" | AccZ: "); Serial.println(accZ, 4);
  Serial.print("GyroX: "); Serial.print(gyroX, 4);
  Serial.print(" | GyroY: "); Serial.print(gyroY, 4);
  Serial.print(" | GyroZ: "); Serial.println(gyroZ, 4);
  Serial.print("Magnitude Acc: "); Serial.print(accMagnitude, 4);
  Serial.print(" | Magnitude Gyro: "); Serial.println(gyroMagnitude, 4);

  // Determinar o status atual com base nas novas métricas
  String status;
  
  if (accMagnitude > FALL_THRESHOLD) {
    status = "QUEDA DETECTADA";
  } 
  else if (accMagnitude > ACC_THRESHOLD || gyroMagnitude > GYRO_THRESHOLD) {
    status = "EM MOVIMENTO";
  }
  else {
    status = "PARADO";
  }

  // Exibir diferenças e limiares para debug
  Serial.print("Diferenças: ACC="); 
  Serial.print(accMagnitude, 4); 
  Serial.print(" (limiar "); 
  Serial.print(ACC_THRESHOLD); 
  Serial.print("), GYRO="); 
  Serial.print(gyroMagnitude, 4); 
  Serial.print(" (limiar "); 
  Serial.print(GYRO_THRESHOLD); 
  Serial.println(")");

  // Enviar mensagem ao MQTT apenas se o status mudar
  if (status != lastStatus) {
    if (WiFi.status() == WL_CONNECTED && client.connected()) {
      String mqttMessage = "Status: " + status + " | Magnitude Acc: " + String(accMagnitude, 4) + " | Magnitude Gyro: " + String(gyroMagnitude, 4);
      if (client.publish(mqtt_topic, mqttMessage.c_str())) {
        Serial.println("Mensagem publicada no MQTT: " + mqttMessage);
      } else {
        Serial.println("Falha ao publicar mensagem no MQTT.");
      }
    }
    lastStatus = status;
  }

  // Exibir status no Serial Monitor
  Serial.println("Status: " + status);
  Serial.println("==============================");

  delay(500);  // Reduzido para uma detecção mais rápida
}
