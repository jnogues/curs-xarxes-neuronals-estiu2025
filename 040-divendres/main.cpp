// Exercici-fuzzylogic Versio 05-07-2025
// Multitasca cooperativa, cooperative scheduler
// Curs xarxes neuronals estiu 2025
// I2SB, Institut Industria Sostenible de Barcelona
// Jaume Nogues jnogues@irp.cat
// ESP8266 nodemcu 1.0
// VSC + platformio
// Contingut platformio.ini:
/*

[env:nodemcuv2]
platform = espressif8266
board = nodemcuv2
;build_flags = -D EEPROM_SIZE=1024
framework = arduino
monitor_speed = 115200
monitor_port = /dev/ttyUSB0
;monitor_port = COM3
upload_speed = 512000
board_build.f_cpu = 160000000L

lib_deps =
  
  joysfera/Tasker @ ^2.0.3
  paulstoffregen/OneWire@^2.3.8
  milesburton/DallasTemperature@^4.0.4


*/

#include <Arduino.h>
#include <OneWire.h>            // Llibreria per al bus OneWire del DS18B20
#include <DallasTemperature.h>  // Llibreria per al sensor DS18B20
#include "Tasker.h"             //gestió d ela multitasca

Tasker tasker; // Creem un gestor de taskes de nom tasker

// --- 1. Configuració de Pins i Sensor DS18B20 ---
#define ONE_WIRE_BUS_GPIO 10 // GPIO10 de l'ESP8266 per al DS18B20
#define PWM_GPIO 13          // GPIO13 de l'ESP8266 per al PWM
#define LED_16 16            // GPIO16 led blau nodemcu

OneWire oneWire(ONE_WIRE_BUS_GPIO);
DallasTemperature sensors(&oneWire);

// Variables
float current_temp_c = 50; // Temperatura "inicial" a 50C
int pwm_value = 0;

// Definició de les categories de potència per a la sortida final (per al Serial Monitor)
const String POWER_LABELS[] = {"Poca Potència", "Potència Mitjana-Baixa", "Potència Mitjana", "Potència Mitjana-Alta", "Molta Potència"};

// --- 2. Funcions de Pertinença (Fuzzificació) ---
// Funcions de pertinença triangular i trapezoidal
float trapezoidal_left_mf(float x, float a, float b) {
  if (x <= a) return 1.0;
  if (x >= a && x <= b) return (b - x) / (b - a);
  return 0.0;
}

float trapezoidal_right_mf(float x, float a, float b) {
  if (x >= b) return 1.0;
  if (x >= a && x <= b) return (x - a) / (b - a);
  return 0.0;
}

float triangular_membership(float x, float a, float b, float c) {
  if (x <= a || x >= c) return 0.0;
  if (x >= a && x <= b) return (x - a) / (b - a);
  if (x >= b && x <= c) return (c - x) / (c - b);
  return 0.0;
}

float trapezoidal_membership(float x, float a, float b, float c, float d) {
  if (x <= a || x >= d) return 0.0;
  if (x >= b && x <= c) return 1.0;
  if (x >= a && x <= b) return (x - a) / (b - a);
  if (x >= c && x <= d) return (d - x) / (d - c);
  return 0.0;
}

// --- 3. Fuzzificació de la Temperatura ---
// Retorna un struct amb els graus de pertinença a cada conjunt difús de temperatura
struct TemperatureMemberships {
  float molt_freda;
  float freda;
  float ok;
  float calida;
  float molt_calida;
};

TemperatureMemberships fuzzify_temperature(float temp) {
  TemperatureMemberships tm;
  
  // Els rangs s'ajusten per a les 5 categories, ara el control difús va fins a 51.0C
  // Molt Freda (MF): 46.0 - 47.5 (Pic 46.0 - 46.5)
  //tm.molt_freda = trapezoidal_membership(temp, 48.0, 48.0, 48.0, 49.0); 
  // Molt Freda (MF): 1.0 per sota de 48.0, 0.0 a 48.5
  tm.molt_freda = trapezoidal_left_mf(temp, 48.0, 48.5); 

  // Freda (F): 47.0 - 48.5 (Pic 47.5)
  //tm.freda = triangular_membership(temp, 48.0, 49.0, 50.0);
  // Freda (F): 0.0 a 48.0, 1.0 a 48.75, 0.0 a 49.5
  tm.freda = triangular_membership(temp, 48.0, 48.75, 49.5);

  // OK (OK): 48.0 - 50.0 (Pic 49.0) <-- Ampliada lleugerament per solapar millor
  //tm.ok = triangular_membership(temp, 49.7, 50.0, 50.3);
  // Normal (N): 0.0 a 49.0, 1.0 a 50.0, 0.0 a 51.0
  tm.ok = triangular_membership(temp, 49.0, 50.0, 51.0);

  // Càlida (C): 49.5 - 51.0 (Pic 50.5) <-- El pic es desplaça fins a 50.5, i baixa a 0.0 a 51.0
  //tm.calida = triangular_membership(temp, 50.0, 50.5, 51.0);
  // Càlida (A): 0.0 a 50.5, 1.0 a 51.25, 0.0 a 52.0
  tm.calida = triangular_membership(temp, 50.5, 51.25, 52.0);

  // Molt Càlida (MC): 50.0 - 51.0 (Pic 51.0) <-- Comença a 50.0, arriba a 1.0 a 51.0 i es manté
  // Usarem un trapezi dret on el punt 'c' i 'd' seran el mateix valor final (51.0)
  //tm.molt_calida = trapezoidal_membership(temp, 50.5, 51.0, 51.0, 51.0 + 0.1); // El +0.1 és una petita estratagema per garantir el trapezi dret perfecte.
  // Molt Càlida (MA): 0.0 a 51.5, 1.0 a 52.0 (i es manté 1.0)
  tm.molt_calida = trapezoidal_right_mf(temp, 51.5, 52.0); 

  return tm;
}

// --- 4. Desdifusificació (Centre de Gravetat - Centroid) ---
// Defineix els "pics" (crisp values) per a cada categoria de potència PWM
const float PWM_MOLTA_POTENCIA_CRISP = 1023.0; // 100%
const float PWM_MITJANA_ALTA_CRISP = 500.0;  
const float PWM_MITJANA_CRISP = 250.0;     
const float PWM_MITJANA_BAIXA_CRISP = 100.0; 
const float PWM_POCA_POTENCIA_CRISP = 0.0;    // 0%


int calculate_fuzzy_output(float temp) 
{
  // 1. Fuzzificació
  TemperatureMemberships tm = fuzzify_temperature(temp);

  // 2. Avaluació de Regles i Agregació (Mamdani/Larsen)
  // En aquest cas, el grau d'activació de cada conseqüent és el grau de pertinença de l'antecedent.
  float fire_molta_potencia = tm.molt_freda;
  float fire_mitjana_alta = tm.freda;
  float fire_mitjana = tm.ok;
  float fire_mitjana_baixa = tm.calida;
  float fire_poca_potencia = tm.molt_calida;

  // 3. Desdifusificació (Centroid):
  float numerator =   (fire_molta_potencia * PWM_MOLTA_POTENCIA_CRISP) + 
                      (fire_mitjana_alta * PWM_MITJANA_ALTA_CRISP) +
                      (fire_mitjana * PWM_MITJANA_CRISP) + 
                      (fire_mitjana_baixa * PWM_MITJANA_BAIXA_CRISP) +
                      (fire_poca_potencia * PWM_POCA_POTENCIA_CRISP);
  
  float denominator = fire_molta_potencia + fire_mitjana_alta + fire_mitjana + fire_mitjana_baixa + fire_poca_potencia;

  if (denominator == 0) {
    // Si la temperatura cau en una zona on cap funció de pertinença té grau > 0,
    // o exactament en un punt zero entre dues, el denominador pot ser 0.
    // Retornem 0 o un valor segur per evitar divisions per zero.
    return 0; 
  }

  float crisp_output = numerator / denominator;
  return round(constrain(crisp_output, 0, 1023)); 
}


// --- Funció principal per obtenir el valor PWM de sortida ---
// Engloba la lògica "dura" i la lògica difusa.
int get_pwm_output(float current_temp) 
{
  if (current_temp < 48.0) {
    return 1023; // PWM a màxim per sota de 47C (zona Molt Freda "dura")
  } else if (current_temp > 51.0) { // <--- **AJUSTAT AQUÍ: el control difús arriba fins a 51.0C**
    return 0;    // PWM a zero per sobre de 51.0C (zona Molt Càlida "dura")
  } else {
    // Si estem dins del rang difús (47.0 a 51.0), apliquem la lògica difusa
    return calculate_fuzzy_output(current_temp);
  }
}

// --- Funció per mapejar el valor PWM a la categoria de String (per al Serial Monitor) ---
String get_power_category_label(int pwm_value) 
{
  if (pwm_value >= 900) return POWER_LABELS[4]; // Molta Potència
  else if (pwm_value >= 650) return POWER_LABELS[3]; // Potència Mitjana-Alta
  else if (pwm_value >= 400) return POWER_LABELS[2]; // Potència Mitjana
  else if (pwm_value >= 150) return POWER_LABELS[1]; // Potència Mitjana-Baixa
  else return POWER_LABELS[0]; // Poca Potència
}


// Tasca 1, fuzzifiquem cada 1 segon
void inferenciaFuzzy()
{
  current_temp_c = sensors.getTempCByIndex(0);
  digitalWrite(LED_16, !digitalRead(LED_16));

  if (current_temp_c == DEVICE_DISCONNECTED_C) {
    Serial.println("Error: No es pot llegir la temperatura del DS18B20!");
  } 
  else 
  {
  pwm_value = get_pwm_output(current_temp_c);
  analogWrite(PWM_GPIO, pwm_value);

 // Telemetria per Teleplot
  Serial.print(">T:");
  Serial.println(current_temp_c, 2);
  Serial.print(">setpoint:");
  Serial.println(50);
  Serial.print(">Tmax:");
  Serial.println(51);
  Serial.print(">Tmin:");
  Serial.println(49);
  Serial.print(">PWM:");
  Serial.println(pwm_value);

  String power_label = get_power_category_label(pwm_value);
  Serial.print("Power Lable:");
  Serial.println(power_label);
  }

  sensors.requestTemperatures(); // Envia la comanda per llegir la temperatura
}


// --- Configuració de l'Arduino (setup) ---
void setup() 
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nESP8266 Iniciat amb Lògica Difusa (DS18B20 + PWM) - Implementació Manual amb 5 funcions de pertinença (Límit 51.0C)");

  pinMode(PWM_GPIO, OUTPUT);
  analogWrite(PWM_GPIO, 0);
  pinMode(LED_16, OUTPUT);
  digitalWrite(LED_16, HIGH); // led 16 apagat
  
  sensors.begin();
  sensors.setResolution(12);//9, 10, 11 o 12 bits.  94ms, 190ms, 380ms, 750ms. 0.5°C, 0.25°C, 0.125°C, or 0.0625°C.
  sensors.requestTemperatures(); // Envia la comanda per llegir la temperatura, 1a peticio

  Serial.println("\n--- Proves del Sistema de Control ---");
  // Proves manuals per depuració 
  Serial.print("Temp 47.5 C -> PWM: "); Serial.println(get_pwm_output(47.5));
  Serial.print("Temp 48.0 C -> PWM: "); Serial.println(get_pwm_output(48.0)); 
  Serial.print("Temp 49.0 C -> PWM: "); Serial.println(get_pwm_output(49.0)); 
  Serial.print("Temp 49.5 C -> PWM: "); Serial.println(get_pwm_output(49.5));
  Serial.print("Temp 49.7 C -> PWM: "); Serial.println(get_pwm_output(49.7)); 
  Serial.print("Temp 50.0 C -> PWM: "); Serial.println(get_pwm_output(50.0)); 
  Serial.print("Temp 50.3 C -> PWM: "); Serial.println(get_pwm_output(50.3)); 
  Serial.print("Temp 50.5 C -> PWM: "); Serial.println(get_pwm_output(50.5)); 
  Serial.print("Temp 51.0 C -> PWM: "); Serial.println(get_pwm_output(51.0)); 
  Serial.print("Temp 51.5 C -> PWM: "); Serial.println(get_pwm_output(51.5));
  delay(2000); 

  tasker.setInterval(inferenciaFuzzy, 1000); // Tasca 1

}

// --- Bucle principal (loop) ---
void loop() 
{
  tasker.loop(); // Gestio de les tasques  
}
