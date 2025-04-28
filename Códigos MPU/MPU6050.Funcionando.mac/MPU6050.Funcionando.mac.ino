#include <Wire.h>
#include <MPU6050_tockn.h>

MPU6050 mpu6050(Wire);

void setup() {
  Serial.begin(115200);

  // Configurar pinos SDA e SCL no ESP8266
  Wire.begin(4, 5);

  mpu6050.begin();
  mpu6050.setGyroOffsets(-4.46, 0.11, 0.14);  // Substitua pelos valores obtidos
  Serial.println("MPU6050 inicializado com offsets aplicados");
}

void loop() {
  mpu6050.update();

  // Obtenha os valores do acelerômetro e do giroscópio
  float aceX = mpu6050.getAccX();
  float aceY = mpu6050.getAccY();
  float aceZ = mpu6050.getAccZ();
  
  float gyroX = mpu6050.getGyroX();
  float gyroY = mpu6050.getGyroY();
  float gyroZ = mpu6050.getGyroZ();

  
  Serial.print("Acelerômetro: ");
  Serial.print("X = ");
  Serial.print(aceX);
  Serial.print(" | Y = ");
  Serial.print(aceY);
  Serial.print(" | Z = ");
  Serial.println(aceZ);

  Serial.print("Giroscópio: ");
  Serial.print("X = ");
  Serial.print(gyroX);
  Serial.print(" | Y = ");
  Serial.print(gyroY);
  Serial.print(" | Z = ");
  Serial.println(gyroZ);
  

  // Verificar se os valores correspondem ao movimento de caminhar, estar sentado, agachado ou cair
  if (abs(aceX - 0.31) < 0.1 && abs(aceY - 0.03) < 0.1 && abs(aceZ - 0.19) < 0.1 &&
      abs(gyroX - 8.40) < 1.0 && abs(gyroY - 6.76) < 1.0 && abs(gyroZ - 6.14) < 1.0) {
    Serial.println("CAMINHANDO");
    
  } 
  else if (abs(aceX - 0.00) < 0.1 && abs(aceY - 0.02) < 0.1 && abs(aceZ - 0.01) < 0.1 &&
           abs(gyroX - 6.06) < 1.0 && abs(gyroY - 1.68) < 1.0 && abs(gyroZ - 1.39) < 1.0) {
    Serial.println("SENTADO");
    
  }
  else if (abs(aceX - 0.00) < 0.1 && abs(aceY - 0.02) < 0.1 && abs(aceZ - 0.11) < 0.1 &&
           abs(gyroX - 13.68) < 1.0 && abs(gyroY - 90.67) < 1.0 && abs(gyroZ - 3.91) < 1.0) {
    Serial.println("AGACHADO");

  }
  else if (abs(aceX - 1.45) < 0.1 && abs(aceY - 2.02) < 0.1 && abs(aceZ - 0.65) < 0.1 &&
           abs(gyroX - 0.00) < 1.0 && abs(gyroY - 100.06) < 1.0 && abs(gyroZ - 43.07) < 1.0) {
    Serial.println("CAIU");
    
  }
  else{
    Serial.println("NORMAL");
  }

  delay(1000);
  Serial.println(" ");
}
