# Guia Detalhado

## Materiais Necessários

- **NodeMCU ESP32**
- **Sensor MPU6050**
- **Cabos jumper**
- **Computador com IDE do Arduino instalada**
- **Acesso à internet via Wi-Fi**
- **MQTT Explorer**

---

## [Acessar esse código](https://github.com/Stenishh/RiscoDeQueda/blob/main/Risco%20de%20queda/CODIGOS/CalibracaoAutomaticaMPU/CalibracaoAutomaticaMPU.ino)

## Passo a Passo

### 1. Instalar a IDE do Arduino

Se você ainda não tem a IDE do Arduino instalada:

1. Acesse o [site oficial do Arduino](https://www.arduino.cc/en/software).
2. Baixe a versão correspondente ao seu sistema operacional.
3. Instale o programa seguindo as instruções fornecidas no site.

### 2. Configurar a Placa NodeMCU ESP32 na IDE do Arduino

1. Abra a IDE do Arduino.
2. Clique em **Arquivo > Preferências**.
3. No campo **URL Adicional para Gerenciadores de Placas**, adicione o seguinte link:
   ```
   https://dl.espressif.com/dl/package_esp32_index.json
   ```
4. Clique em **OK**.
5. Em **Ferramentas > Placa > Gerenciador de Placas**, procure por **esp32** e instale o pacote.
6. Selecione a placa **NodeMCU-32S** em **Ferramentas > Placa**.

### 3. Baixar Bibliotecas Necessárias

1. Clique em **Sketch > Incluir Biblioteca > Gerenciar Bibliotecas**.
2. Pesquise e instale as seguintes bibliotecas:
   - `MPU6050_tockn`
   - `PubSubClient`
   - `WiFi`
3. Aguarde até que todas as bibliotecas sejam instaladas.

### 4. Fazer Conexões Físicas

Conecte o sensor MPU6050 à NodeMCU ESP32 da seguinte forma:

| MPU6050 | NodeMCU ESP32 |
|---------|---------------|
| VCC     | 3V3           |
| GND     | GND           |
| SCL     | GPIO 22       |
| SDA     | GPIO 21       |

- [Datasheet MPU6050](https://components101.com/sensors/mpu6050-module)
- [Datasheet ESP32](https://components101.com/microcontrollers/esp32-devkitc)

### 5. Configurar o Código

1. Abra a IDE do Arduino e copie o código fornecido.
2. Substitua as seguintes informações no código:
   - `ssid`: o nome da sua rede Wi-Fi.
   - `password`: a senha da sua rede Wi-Fi.
   - `mqtt_server`: o IP do seu broker MQTT.

### 6. Configurar o MQTT Explorer no seu computador

1. Baixe e instale o [MQTT Explorer](https://mqtt-explorer.com/).
2. Abra o programa e configure uma nova conexão:
   - **Broker Address**: IP do broker MQTT configurado no código.
   - **Port**: 1883.
3. Clique em **Connect** para verificar a conexão.

### 7. Fazer o Upload do Código para a Placa

1. Conecte a NodeMCU ESP32 ao seu computador usando um cabo USB.
2. Na IDE do Arduino, selecione a porta correta em **Ferramentas > Porta**.
3. Clique no ícone de **Upload** para enviar o código para a placa.
4. Aguarde até que o processo seja concluído.

### 8. Testar o Projeto

1. Abra o **Monitor Serial** na IDE do Arduino (configurado para 115200 baud).
2. Verifique as mensagens exibidas no monitor para garantir que:
   - O dispositivo conectou ao Wi-Fi.
   - As leituras do sensor estão sendo capturadas.
   - O status ("PARADO", "EM MOVIMENTO" ou "QUEDA DETECTADA") está sendo enviado via MQTT.
3. No MQTT Explorer, verifique se os dados estão sendo publicados no tópico configurado (`mpu6050/status`).

### 9. Recalibrar as Referências (Opcional)

Se você deseja recalibrar os valores de referência do sensor:

1. Pressione o botão de reset na placa NodeMCU ESP32.
2. Aguarde enquanto as novas referências são calculadas (o processo leva alguns segundos e as mensagens aparecerão no Monitor Serial).

