#include <Wire.h>
#include <MPU6050_tockn.h>

MPU6050 mpu6050(Wire);

void setup() {
  Serial.begin(115200);

  // Configurar pinos SDA e SCL no ESP8266
  // GPIO 4 (D2) é o SDA, GPIO 5 (D1) é o SCL no ESP8266
  Wire.begin(4, 5);

  mpu6050.begin();
  mpu6050.setGyroOffsets(-4.46, 0.11, 0.14);  // Substitua pelos valores obtidos
  Serial.println("MPU6050 inicializado com offsets aplicados");
}

void loop() {
  mpu6050.update();

  Serial.print("Acelerômetro: ");
  Serial.print("X = ");
  Serial.print(mpu6050.getAccX());
  Serial.print(" | Y = ");
  Serial.print(mpu6050.getAccY());
  Serial.print(" | Z = ");
  Serial.println(mpu6050.getAccZ());

  Serial.print("Giroscópio: ");
  Serial.print("X = ");
  Serial.print(mpu6050.getGyroX());
  Serial.print(" | Y = ");
  Serial.print(mpu6050.getGyroY());
  Serial.print(" | Z = ");
  Serial.println(mpu6050.getGyroZ());

  delay(1000);
  Serial.println(" ");
}
