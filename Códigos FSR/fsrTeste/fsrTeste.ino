const int fsrPin1 = 36;  // ADC1_CHANNEL_0 corresponde ao GPIO36 (VP)
const int fsrPin2 = 39;  // ADC1_CHANNEL_0 corresponde ao GPIO36 (VP)
const int fsrPin3 = 34;  // ADC1_CHANNEL_0 corresponde ao GPIO36 (VP)

void setup() {
  Serial.begin(115200);

  // Define resolução do ADC para 12 bits (padrão no ESP32)
  analogReadResolution(12);  // 0 a 4095

  // Define a atenuação para ler toda a faixa de 0 a 3.3V
  analogSetAttenuation(ADC_11db); // permite leitura até ~3.3V
}

void loop() {
  int fsrReading1 = analogRead(fsrPin1);
  int fsrReading2 = analogRead(fsrPin2);
  int fsrReading3 = analogRead(fsrPin3);

  // Envia os valores dos sensores no formato CSV (valores separados por vírgulas)
  Serial.print(fsrReading1);
  Serial.print(",");
  Serial.print(fsrReading2);
  Serial.print(",");
  Serial.println(fsrReading3);

  delay(200);
}
