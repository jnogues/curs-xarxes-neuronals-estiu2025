# main.py per a ESP32 (MicroPython)
# Per entrenar https://colab.research.google.com/drive/11-q7iKI3ELDgvOzC-MVxOgNGuPSy8s3w?usp=sharing

import json
import math
from machine import Pin, PWM, reset, freq
import onewire, ds18x20, time, math
import uasyncio as asyncio
from umqtt.simple import MQTTClient
import network
import sys

import socket

# Configuració Teleplot
UDP_IP = "192.168.0.95"  # o IP del teu ordinador si cal
UDP_PORT = 47269
udp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# --- Configura WiFi ---
WIFI_SSID = "viscalaterra"
WIFI_PASS = "pitufito*"
MQTT_BROKER = "5.196.88.155"
MQTT_CLIENT_ID = "esp32_termostat"
MQTT_TOPIC_PUB = b"/termostat/out"
MQTT_TOPIC_SUB = b"/termostat/in"

wlan = network.WLAN(network.STA_IF)
client = None
mqtt_error_count = 0
MQTT_ERROR_MAX = 10
temp_actual = 50

def connect_wifi():
    wlan.active(True)
    if not wlan.isconnected():
        print("🔌 Connectant a WiFi...")
        wlan.connect(WIFI_SSID, WIFI_PASS)
        timeout = 0
        while not wlan.isconnected() and timeout < 10:
            time.sleep(2)
            timeout += 1
    if wlan.isconnected():
        print("✅ Connectat a WiFi:", wlan.ifconfig())
    else:
        print("❌ No s'ha pogut connectar a WiFi")
        reset()

def connect_mqtt():
    global client, mqtt_error_count
    try:
        client = MQTTClient(MQTT_CLIENT_ID, MQTT_BROKER)
        client.set_callback(mqtt_callback)
        client.connect()
        client.subscribe(MQTT_TOPIC_SUB)
        print("✅ Connectat a MQTT")
        mqtt_error_count = 0
    except Exception as e:
        print("⚠️ Error connectant a MQTT:", e)
        mqtt_error_count += 1
        if mqtt_error_count >= MQTT_ERROR_MAX:
            print("💥 Massa errors MQTT. Reiniciant...")
            time.sleep(2)
            reset()

def mqtt_callback(topic, msg):
    global K_i
    try:
        message = msg.decode().strip()
        if message.startswith("ki="):
            new_ki = float(message.split("=")[1])
            if 0.0 <= new_ki <= 100.0:
                K_i = new_ki
                print(f"🔧 K_i actualitzat a: {K_i}")
            else:
                print("⚠️ Valor de K_i fora de rang.")
        else:
            print(f"📥 Missatge rebut a {topic.decode()}: {message}")
    except Exception as e:
        print("⚠️ Error interpretant missatge:", e)

connect_wifi()
connect_mqtt()

# --- Configura el DS18B20 ---
ow = onewire.OneWire(Pin(1))
ds = ds18x20.DS18X20(ow)
roms = ds.scan()
if not roms:
    print("❌ No s'ha trobat cap sensor DS18B20. Reiniciant...")
    time.sleep(2)
    reset()
else:
    print("✅ Sensor trobat:", roms)

PWM_PIN = 0
LED_PIN = 8
pwm = PWM(Pin(PWM_PIN))
pwm.freq(1000)
pwm.duty(0)  # Apaga el PWM
time.sleep_ms(100)

led = Pin(LED_PIN, Pin.OUT)

print("  ")
print("Comencem....")
print("machine freq: ", freq()/1000_000 , "MHz")


# --- 1. Funcions de Normalització Min-Max ---
# Aquestes funcions han de ser exactament les mateixes que vas usar a Python per normalitzar
# les dades d'entrada i desnormalitzar les de sortida.
def normalize_minmax(value, data_min, data_max):
    return (value - data_min) / (data_max - data_min)

def denormalize_minmax(normalized_value, data_min, data_max):
    return normalized_value * (data_max - data_min) + data_min

# --- 2. Funció d'Activació Sigmoide ---
# Utilitzem 'math.exp' que és més eficient en MicroPython que 'numpy.exp' (que no existeix).
def sigmoid(x):
    return 1 / (1 + math.exp(-x))

# --- 3. Carregar els Paràmetres de la Xarxa Neuronal ---
# Aquest fitxer 'nn_params_micropython.json' haurà d'estar pujat a l'ESP32.
nn_params = {}
try:
    with open('nn_params_micropython.json', 'r') as f:
        nn_params = json.load(f)
    print("Paràmetres de la xarxa carregats amb èxit.")
except OSError:
    print("Error: No es pot obrir 'nn_params_micropython.json'. Assegura't que el fitxer existeix a l'ESP32.")
    # Si no es pot carregar, inicialitzarem amb valors predeterminats o aturarem l'execució.
    # Per simplificació, aquí només imprimim l'error i el programa potser fallarà després.
except Exception as e:
    print(f"Error en carregar JSON: {e}")

# Extreure els pesos i biaixos per a un accés més fàcil
# Recorda que JSON els carrega com a llistes de Python.
# Si necessitem "matrius", les tractarem com a llistes anidades.

W1 = nn_params.get('weights_input_hidden1', [])
b1 = nn_params.get('bias_hidden1', [])
W2 = nn_params.get('weights_hidden1_hidden2', [])
b2 = nn_params.get('bias_hidden1_hidden2', [])
W3 = nn_params.get('weights_hidden2_output', [])
b3 = nn_params.get('bias_output', [])

# Extreure els paràmetres de normalització
min_X_orig = nn_params.get('min_X_orig', 0.0)
max_X_orig = nn_params.get('max_X_orig', 1.0)
min_y_orig = nn_params.get('min_y_orig', 0.0)
max_y_orig = nn_params.get('max_y_orig', 1.0)


# --- 4. Funció de Predicció de la Xarxa Neuronal ---
# Aquesta funció pren un valor de temperatura i retorna la predicció de PWM.
def predict_pwm(temperature):
    # Pas 1: Normalitzar la temperatura d'entrada
    temp_scaled = normalize_minmax(temperature, min_X_orig, max_X_orig)

    # La xarxa espera un "array" d'entrada. En MicroPython, usem llistes o flotants directes.
    # Per una sola entrada, temp_scaled és un flotant.
    # Adaptarem els càlculs per funcionar amb flotants/llistes simples si les capes tenen una sola neurona.

    # Propagació cap endavant (Forward Pass)
    # Capa 1 (Oculta 1): 1 entrada -> 7 neurones
    # Z1 = np.dot(X_scaled, W1) + b1
    # En MicroPython, com X_scaled és un sol valor per la temperatura
    # Z1 serà una llista amb 7 elements (un per cada neurona de la capa oculta 1)
    Z1 = [0.0] * len(b1[0]) # Inicialitzem una llista per Z1
    for j in range(len(b1[0])): # Itera sobre les 7 neurones de la capa oculta 1
        # La multiplicació de matriu X_scaled * W1[0][j] és simplement temp_scaled * W1[0][j]
        # ja que X_scaled té una única dimensió d'entrada
        Z1[j] = (temp_scaled * W1[0][j]) + b1[0][j] # W1[0] és la fila de pesos per la nostra única entrada
    A1 = [sigmoid(z) for z in Z1] # Apliquem la sigmoide a cada element


    # Capa 2 (Oculta 2): 7 neurones -> 5 neurones
    # Z2 = np.dot(A1, W2) + b2
    Z2 = [0.0] * len(b2[0]) # Inicialitzem una llista per Z2 (5 elements)
    for j in range(len(b2[0])): # Itera sobre les 5 neurones de la capa oculta 2
        sum_val = 0.0
        for i in range(len(A1)): # Suma les contribucions de les 7 neurones d'A1
            sum_val += A1[i] * W2[i][j] # W2[i][j] = pes de neurona i (A1) a neurona j (Z2)
        Z2[j] = sum_val + b2[0][j]
    A2 = [sigmoid(z) for z in Z2] # Apliquem la sigmoide a cada element


    # Capa 3 (Sortida): 5 neurones -> 1 neurona
    # Z3 = np.dot(A2, W3) + b3
    Z3 = [0.0] * len(b3[0]) # Inicialitzem una llista per Z3 (1 element)
    for j in range(len(b3[0])): # Itera sobre l'única neurona de sortida
        sum_val = 0.0
        for i in range(len(A2)): # Suma les contribucions de les 5 neurones d'A2
            sum_val += A2[i] * W3[i][j] # W3[i][j] = pes de neurona i (A2) a neurona j (Z3)
        Z3[j] = sum_val + b3[0][j]
    A3 = [sigmoid(z) for z in Z3] # Apliquem la sigmoide a cada element

    # A3[0] és la predicció PWM normalitzada (ja que només hi ha 1 neurona de sortida)
    pwm_scaled_pred = A3[0]

    # Pas 2: Desnormalitzar la predicció PWM per obtenir el valor real
    pwm_original_pred = denormalize_minmax(pwm_scaled_pred, min_y_orig, max_y_orig)

    return pwm_original_pred


async def tasca_lectura():
    global temp_actual
    while True:
        try:
            ds.convert_temp()
            await asyncio.sleep_ms(800)
            t = ds.read_temp(roms[0])
            if t is not None:
                temp_actual = t
                #print(t)
        except Exception as e:
            print("⚠️ Error lectura:", e)
        await asyncio.sleep(0.1)


async def tasca_control():
    #global temp_actual
    while True:
        if not wlan.isconnected():
            print("🔄 WiFi perdut. Reconnectant...")
            connect_wifi()

        temp = temp_actual
        predicted_pwm_value = predict_pwm(temp)
        #print("pwm = ", predicted_pwm_value)
        print("@")
        try:
            pwm.duty(int(predicted_pwm_value))
        except Exception as e:
            print("⚠️ Error PWM:", e)
            pwm.duty(0)

        await asyncio.sleep(1)


async def tasca_mqtt():
    global mqtt_error_count
    while True:
        try:
            client.check_msg()
            mqtt_error_count = 0
        except Exception as e:
            print("⚠️ Error MQTT:", e)
            mqtt_error_count += 1
            connect_mqtt()
        await asyncio.sleep(0.5)

async def tasca_publicacio():
    while True:
        try:
            msg = f"T={temp_actual:.2f},PWM={pwm.duty()}"
            client.publish(MQTT_TOPIC_PUB, msg)
            #print("📤 Publicat MQTT:", msg)
        except Exception as e:
            print("⚠️ Error publicació MQTT:", e)
        await asyncio.sleep(5)

async def tasca_teleplot():
    while True:
        try:
            msg = (
                f"T:{temp_actual:.2f}\n"
                f"PWM:{pwm.duty()}\n"
                f"Tamb:30.00\n"
                f"Tmax:51.00\n"
                f"Tmin:49.00\n"
                f"setpoint:50.00"
            )
            udp_sock.sendto(msg.encode(), (UDP_IP, UDP_PORT))
        except Exception as e:
            print("⚠️ Error Teleplot:", e)
        await asyncio.sleep(0.5)

async def tasca_led():
    while True:
        led.toggle()
        await asyncio.sleep(1)


async def main():
    await asyncio.gather(
        tasca_lectura(),
        tasca_control(),
        tasca_mqtt(),
        tasca_publicacio(),
        tasca_teleplot(),
        tasca_led()
    )

try:
    asyncio.run(main())
except KeyboardInterrupt:
    print("🛑 Interrupció detectada. Aturant PWM i sortint.")
    pwm.duty(0)  # Apaga el PWM
    time.sleep_ms(100)
    sys.exit()
finally:
    pwm.duty(0)
