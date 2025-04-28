const int fsrPin = 36;  // ADC1_CHANNEL_0 corresponde ao GPIO36 (VP)
int fsrReading;
int fsrVoltage;

void setup() {
  Serial.begin(115200);

  // Define resolução do ADC para 12 bits (padrão no ESP32)
  analogReadResolution(12);  // 0 a 4095

  // Define a atenuação para ler toda a faixa de 0 a 3.3V
  analogSetAttenuation(ADC_11db); // permite leitura até ~3.3V
}

void loop() {
  fsrReading = analogRead(fsrPin);

  // Conversão para estimativa de força (ajuste com base na calibração)
  float forca = fsrReading / 320.0;

  // Conversão para milivolts
  fsrVoltage = map(fsrReading, 0, 4095, 0, 3300);

  Serial.print("Leitura: ");
  Serial.print(fsrReading);
  Serial.print("  -> Força estimada: ");
  Serial.print(forca, 2);
  Serial.print(" N");
  Serial.print("  -> Voltagem: ");
  Serial.print(fsrVoltage);
  Serial.println(" mV");

  delay(200);
}
