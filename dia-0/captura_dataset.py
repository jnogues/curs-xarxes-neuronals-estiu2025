import serial
import csv
import time

# --- CONFIGURACIÓ ---
#PORT = 'COM3'         # ⚠️ Canvia-ho a '/dev/ttyUSB0', '/dev/ttySx' o el port correcte del teu sistema
PORT = '/dev/ttyUSB0'
BAUDRATE = 115200
CSV_FILENAME = 'dataset_pid.csv'

# --- INICIALITZACIÓ SERIAL ---
ser = serial.Serial(PORT, BAUDRATE, timeout=1)
print(f"[INFO] Connectat a {PORT}. Esperant dades...")

# --- OBRE FITXER I CAPTURA ---
with open(CSV_FILENAME, mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerow(['error', 'errorPrev', 'outputPWM'])  # Capçalera CSV

    try:
        while True:
            line = ser.readline().decode('utf-8').strip()
            if not line or line.startswith('>'):
                continue  # Ignora línies de Teleplot

            try:
                values = [float(x) for x in line.split(',')]
                if len(values) == 3:
                    writer.writerow(values)
                    print(values)  # Mostra la línia capturada
            except ValueError:
                continue  # Ignora línies incorrectes

    except KeyboardInterrupt:
        print("\n[INFO] Captura finalitzada per l'usuari.")
