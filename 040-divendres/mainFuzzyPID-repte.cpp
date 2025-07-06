// Repte final per fer un fuzzyPID
//

#include <Arduino.h>
#include <OneWire.h>            // Llibreria per al bus OneWire del DS18B20
#include <DallasTemperature.h>  // Llibreria per al sensor DS18B20
#include "Tasker.h"             // gestió de la multitasca

Tasker tasker; // Creem un gestor de taskes de nom tasker

// --- 1. Configuració de Pins i Sensor DS18B20 ---
#define ONE_WIRE_BUS_GPIO 10 // GPIO10 de l'ESP8266 per al DS18B20
#define PWM_GPIO 13          // GPIO13 de l'ESP8266 per al PWM
#define LED_16 16            // GPIO16 led blau nodemcu

OneWire oneWire(ONE_WIRE_BUS_GPIO);
DallasTemperature sensors(&oneWire);

// Variables globals
float current_temp_c = 50.0; // Temperatura "inicial" a 50C
int pwm_value = 0;
float previous_error = 0.0; // Variable per guardar l'error de l'interval anterior
const float SETPOINT_TEMP = 50.0; // El teu setpoint de temperatura

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

// Funció de trapezi genèrica (no usada directament per E/dE, però la deixem)
float trapezoidal_membership(float x, float a, float b, float c, float d) {
  if (x <= a || x >= d) return 0.0;
  if (x >= b && x <= c) return 1.0;
  if (x >= a && x <= b) return (x - a) / (b - a);
  if (x >= c && x <= d) return (d - x) / (d - c);
  return 0.0;
}

// --- 3. Fuzzificació de les Variables d'Entrada ---

// Estructura per als graus de pertinença de la Temperatura
struct TemperatureMemberships {
  float molt_freda;
  float freda;
  float ok;
  float calida;
  float molt_calida;
};

// Estructura per als graus de pertinença de l'Error
struct ErrorMemberships {
  float ng; // Negatiu Gran
  float np; // Negatiu Petit
  float z;  // Zero
  float pp; // Positiu Petit
  float pg; // Positiu Gran
};

// Estructura per als graus de pertinença del Delta_Error
struct DeltaErrorMemberships {
  float n;  // Negatiu
  float z;  // Zero
  float p;  // Positiu
};

// Estructura per emmagatzemar tots els graus de pertinença de les entrades
struct FuzzifiedInputs {
  TemperatureMemberships temp_mf; // Els graus de pertinença de la Temperatura
  ErrorMemberships error_mf;      // Els graus de pertinença de l'Error
  DeltaErrorMemberships delta_error_mf; // Els graus de pertinença del Delta_Error
};


// Funció per fuzzificar totes les entrades
FuzzifiedInputs fuzzify_all_inputs(float temp, float error, float delta_error) {
  FuzzifiedInputs fi;
  
  // Fuzzificació de la Temperatura (basat en la teva última versió, que ara és correcta)
  fi.temp_mf.molt_freda = trapezoidal_left_mf(temp, 48.0, 48.5); 
  fi.temp_mf.freda      = triangular_membership(temp, 48.0, 48.75, 49.5);
  fi.temp_mf.ok         = triangular_membership(temp, 49.0, 50.0, 51.0);
  fi.temp_mf.calida     = triangular_membership(temp, 50.5, 51.25, 52.0);
  fi.temp_mf.molt_calida = trapezoidal_right_mf(temp, 51.5, 52.0); 

  // Fuzzificació de l'Error (E) - Ajusta els punts segons el comportament desitjat
  fi.error_mf.ng = trapezoidal_left_mf(error, -2.0, -1.0);     // Per error <= -2.0 (temp >= 52.0)
  fi.error_mf.np = triangular_membership(error, -1.5, -0.75, 0.0); // Entre -1.5 i 0.0
  fi.error_mf.z  = triangular_membership(error, -0.5, 0.0, 0.5);   // Al voltant de 0.0
  fi.error_mf.pp = triangular_membership(error, 0.0, 0.75, 1.5);   // Entre 0.0 i 1.5
  fi.error_mf.pg = trapezoidal_right_mf(error, 1.0, 2.0);      // Per error >= 2.0 (temp <= 48.0)

  // Fuzzificació del Delta_Error (dE) - Ajusta els punts segons la velocitat del sistema
  fi.delta_error_mf.n = trapezoidal_left_mf(delta_error, -0.1, -0.05);   // Disminueix ràpid
  fi.delta_error_mf.z = triangular_membership(delta_error, -0.075, 0.0, 0.075); // Estable
  fi.delta_error_mf.p = trapezoidal_right_mf(delta_error, 0.05, 0.1);     // Augmenta ràpid

  return fi;
}


// --- 4. Desdifusificació (Centre de Gravetat - Centroid) ---
// Defineix els "pics" (crisp values) per a cada categoria de potència PWM
const float PWM_MOLTA_POTENCIA_CRISP = 1023.0; // 100%
const float PWM_MITJANA_ALTA_CRISP   = 767.0;  // Aproximadament 75% de 1023
const float PWM_MITJANA_CRISP        = 512.0;  // Aproximadament 50% de 1023
const float PWM_MITJANA_BAIXA_CRISP  = 256.0;  // Aproximadament 25% de 1023
const float PWM_POCA_POTENCIA_CRISP  = 0.0;    // 0%


// Funció per calcular la sortida difusa (PWM)
int calculate_fuzzy_output(float current_temp, float error, float delta_error) 
{
  // 1. Fuzzificació de totes les entrades
  FuzzifiedInputs fi = fuzzify_all_inputs(current_temp, error, delta_error);

  // 2. Avaluació de Regles i Agregació (Mamdani/Larsen)
  // Inicialitzem les activacions de les sortides
  float fire_molta_potencia   = 0.0;
  float fire_mitjana_alta     = 0.0;
  float fire_mitjana          = 0.0;
  float fire_mitjana_baixa    = 0.0;
  float fire_poca_potencia    = 0.0;

  // Matriu de Regles (Exemples, cal completar i ajustar per obtenir el comportament desitjat)
  // Per simplificar i ser més "PID-like", les regles es basen principalment en Error i Delta_Error.
  // La temperatura actual (fi.temp_mf) es pot usar per ajustar si el control és massa agressiu o lent.
  // Utilitzem min() per l'operador AND i max() per l'operador OR.

  // Si E és Negatiu Gran (Temp >> Setpoint)
  fire_poca_potencia = max(fire_poca_potencia, min(fi.error_mf.ng, fi.delta_error_mf.n)); // Molt Calent i baixant -> Molt Poca
  fire_poca_potencia = max(fire_poca_potencia, min(fi.error_mf.ng, fi.delta_error_mf.z)); // Molt Calent i estable -> Molt Poca
  fire_poca_potencia = max(fire_poca_potencia, min(fi.error_mf.ng, fi.delta_error_mf.p)); // Molt Calent i pujant -> Molt Poca

  // Si E és Negatiu Petit (Temp > Setpoint)
  fire_poca_potencia = max(fire_poca_potencia, min(fi.error_mf.np, fi.delta_error_mf.n)); // Lleug. Calent i baixant -> Poca
  fire_poca_potencia = max(fire_poca_potencia, min(fi.error_mf.np, fi.delta_error_mf.z)); // Lleug. Calent i estable -> Poca
  fire_mitjana_baixa = max(fire_mitjana_baixa, min(fi.error_mf.np, fi.delta_error_mf.p)); // Lleug. Calent i pujant -> Mitjana-Baixa

  // Si E és Zero (Temp ~ Setpoint)
  fire_mitjana_baixa = max(fire_mitjana_baixa, min(fi.error_mf.z, fi.delta_error_mf.n)); // En Setpoint i baixant -> Mitjana-Baixa (per no refredar massa)
  fire_poca_potencia = max(fire_poca_potencia, min(fi.error_mf.z, fi.delta_error_mf.z)); // En Setpoint i estable -> Poca (manteniment)
  fire_mitjana_alta  = max(fire_mitjana_alta, min(fi.error_mf.z, fi.delta_error_mf.p)); // En Setpoint i pujant -> Mitjana-Alta (correcció)

  // Si E és Positiu Petit (Temp < Setpoint)
  fire_mitjana_alta = max(fire_mitjana_alta, min(fi.error_mf.pp, fi.delta_error_mf.n)); // Lleug. Fred i baixant -> Mitjana-Alta
  fire_molta_potencia = max(fire_molta_potencia, min(fi.error_mf.pp, fi.delta_error_mf.z)); // Lleug. Fred i estable -> Molta
  fire_molta_potencia = max(fire_molta_potencia, min(fi.error_mf.pp, fi.delta_error_mf.p)); // Lleug. Fred i pujant -> Molta

  // Si E és Positiu Gran (Temp << Setpoint)
  fire_molta_potencia = max(fire_molta_potencia, min(fi.error_mf.pg, fi.delta_error_mf.n)); // Molt Fred i baixant -> Molta
  fire_molta_potencia = max(fire_molta_potencia, min(fi.error_mf.pg, fi.delta_error_mf.z)); // Molt Fred i estable -> Molta
  fire_molta_potencia = max(fire_molta_potencia, min(fi.error_mf.pg, fi.delta_error_mf.p)); // Molt Fred i pujant -> Molta


  // 3. Desdifusificació (Centroid):
  float numerator =   (fire_molta_potencia * PWM_MOLTA_POTENCIA_CRISP) + 
                      (fire_mitjana_alta * PWM_MITJANA_ALTA_CRISP) +
                      (fire_mitjana * PWM_MITJANA_CRISP) + 
                      (fire_mitjana_baixa * PWM_MITJANA_BAIXA_CRISP) +
                      (fire_poca_potencia * PWM_POCA_POTENCIA_CRISP);
  
  float denominator = fire_molta_potencia + fire_mitjana_alta + fire_mitjana + fire_mitjana_baixa + fire_poca_potencia;

  if (denominator == 0) {
    // Si cap regla s'activa o totes les activacions són 0.
    // Retornem 0 o un valor segur, com el PWM actual per evitar canvis bruscos.
    return pwm_value; 
  }

  float crisp_output = numerator / denominator;
  return round(constrain(crisp_output, 0, 1023)); 
}


// --- Funció principal per obtenir el valor PWM de sortida ---
// Engloba la lògica "dura" i la lògica difusa.
// Amb un Fuzzy PID, la lògica "dura" als extrems sol ser menys necessària,
// ja que el controlador difús ja hauria de portar la temperatura als extrems.
// No obstant això, es pot mantenir per seguretat o per reacció ultra-ràpida.
int get_pwm_output(float current_temp, float current_error, float delta_error) 
{
  // Mantenim la lògica "dura" per reaccionar ràpidament si la temperatura s'extralimita molt
  if (current_temp < 47.0) { // Molt freda, segur que cal escalfar al màxim
    return 1023; 
  } else if (current_temp > 53.0) { // Molt calenta, segur que cal apagar
    return 0;    
  } else {
    // Si estem dins del rang més controlable, apliquem la lògica difusa PID
    return calculate_fuzzy_output(current_temp, current_error, delta_error);
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
    // Càlcul de l'Error i Delta_Error
    float current_error = SETPOINT_TEMP - current_temp_c;
    float delta_error = current_error - previous_error;

    // Actualitzar l'error anterior per a la propera iteració
    // Aquesta actualització és CRÍTICA per al control PID difús
    previous_error = current_error; 

    pwm_value = get_pwm_output(current_temp_c, current_error, delta_error);
    analogWrite(PWM_GPIO, pwm_value);

    // Telemetria per Teleplot
    Serial.print(">T:");
    Serial.println(current_temp_c, 2);
    Serial.print(">setpoint:");
    Serial.println(SETPOINT_TEMP); // Ara el setpoint és una constant
    Serial.print(">Error:");
    Serial.println(current_error, 2);
    Serial.print(">Delta_Error:");
    Serial.println(delta_error, 3); // Més precisió per a dE
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
  Serial.println("\nESP8266 Iniciat amb Lògica Difusa (DS18B20 + PWM) - Controlador PID Difús");

  pinMode(PWM_GPIO, OUTPUT);
  analogWrite(PWM_GPIO, 0); // Assegura que el PWM comença a 0
  pinMode(LED_16, OUTPUT);
  digitalWrite(LED_16, HIGH); // led 16 apagat (normalment HIGH = OFF per LED_BUILTIN)
  
  sensors.begin();
  sensors.setResolution(12); // Resolució de 12 bits per més precisió
  sensors.requestTemperatures(); // Primera petició de lectura de temperatura
  // La primera lectura de temperatura es farà a la primera execució d'inferenciaFuzzy
  // i s'inicialitzarà previous_error amb el primer càlcul d'error.

  Serial.println("\n--- Proves Inicials del Sistema de Control PID Difús ---");
  // Per a proves manuals amb un Fuzzy PID, cal simular l'error i el delta_error
  // Ja no es pot provar només amb la temperatura.
  // Aquests valors seran calculats automàticament en el bucle principal.
  Serial.println("El comportament es veurà millor al monitor serial en temps real.");
  Serial.println("Monitoritza les variables E i dE per entendre les regles.");
  delay(2000); 

  // Inicialitzem previous_error amb la primera lectura, per evitar un delta_error molt gran al principi
  sensors.requestTemperatures();
  delay(800);
  current_temp_c = sensors.getTempCByIndex(0);
  previous_error = SETPOINT_TEMP - current_temp_c;


  tasker.setInterval(inferenciaFuzzy, 1000); // Executa la tasca cada 1 segon
}

// --- Bucle principal (loop) ---
void loop() 
{
  tasker.loop(); // Gestio de les tasques  
}
