import tkinter as tk
from tkinter import ttk
import serial
import threading

class SensorInterface:
    def __init__(self, master):
        self.master = master
        self.master.title("Palmilha Inteligente - Monitoramento de Sensores FSR")
        
        # Configuração da interface gráfica
        self.frame = ttk.Frame(self.master, padding="10")
        self.frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        # Labels e barras de progresso para os sensores
        self.sensor_labels = []
        self.sensor_bars = []
        for i in range(3):
            label = ttk.Label(self.frame, text=f"Sensor {i+1}")
            label.grid(row=i, column=0, sticky=tk.W, padx=5, pady=5)
            self.sensor_labels.append(label)
            
            progress = ttk.Progressbar(self.frame, orient="horizontal", length=300, mode="determinate")
            progress.grid(row=i, column=1, padx=5, pady=5)
            self.sensor_bars.append(progress)
        
        # Variável para armazenar os valores dos sensores
        self.sensor_values = [0] * 3
        
        # Configuração da conexão serial
        try:
            self.serial_port = serial.Serial('/dev/ttyACM0', 115200, timeout=1)  # Alterado para 115200
        except serial.SerialException:
            self.serial_port = None
            print("Erro ao conectar à porta serial.")
        
        # Inicia a thread para leitura de dados
        if self.serial_port:
            self.running = True
            self.read_thread = threading.Thread(target=self.read_serial_data, daemon=True)
            self.read_thread.start()
        
        # Atualiza a interface gráfica periodicamente
        self.master.after(100, self.update_ui)
    
    def read_serial_data(self):
        """Lê dados da porta serial."""
        while self.running:
            try:
                # Lê a linha e tenta decodificar
                line = self.serial_port.readline()
                if line:
                    try:
                        line = line.decode('utf-8').strip()  # Tenta decodificar como UTF-8
                        values = line.split(',')  # Divide os valores pelos delimitadores de vírgula
                        if len(values) == 3:
                            try:
                                # Tenta converter os valores para inteiros
                                self.sensor_values = [int(value) for value in values]
                            except ValueError:
                                print("Erro: dados não numéricos recebidos:", values)
                        else:
                            print("Erro: formato de dados inesperado:", line)
                    except UnicodeDecodeError:
                        print("Erro na decodificação: ignorando byte inválido")
                        continue  # Ignora essa linha e vai para a próxima
            except Exception as e:
                print(f"Erro na leitura serial: {e}")
    
    def update_ui(self):
        """Atualiza os valores dos sensores na interface e exibe no console."""
        # Normaliza os valores para a faixa de 0 a 100
        max_value = 4095  # Valor máximo do ADC de 12 bits
        normalized_values = [min(100, (value / max_value) * 100) for value in self.sensor_values]
        
        # Atualiza as barras de progresso com os valores normalizados
        for i, value in enumerate(normalized_values):
            self.sensor_bars[i]['value'] = value
        
        # Exibe os valores de cada barra no console
        print(f"Sensor 1: {normalized_values[0]}%, Sensor 2: {normalized_values[1]}%, Sensor 3: {normalized_values[2]}%")
        
        # Chama a atualização a cada 100ms
        self.master.after(100, self.update_ui)
    
    def close(self):
        """Fecha a aplicação e encerra a thread."""
        self.running = False
        if self.serial_port:
            self.serial_port.close()
        self.master.destroy()

# Criação da interface
if __name__ == "__main__":
    root = tk.Tk()
    app = SensorInterface(root)
    root.protocol("WM_DELETE_WINDOW", app.close)
    root.mainloop()
