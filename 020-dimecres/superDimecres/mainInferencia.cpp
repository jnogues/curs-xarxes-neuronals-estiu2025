//Inici doc-----------------------------------------------------
//Exercici-PID Versio Gemini 24-06-2025
//Curs xarxes neuronals estiu 2025
//I2SB, Institut Industria Sostenible de Barcelona
//Jaume Nogues jnogues@irp.cat
//ESP8266 nodemcu 1.0
//VSC + platformio
//PID per entrenar i inferencia amb XN
//Dos resistències de 10 ohm 
//
//Contingut platformio.ini:
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
  br3ttb/PID @ ^1.2.1

*/


#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Tasker.h>
#include "neural_network.h" // Inclou la teva xarxa neuronal


//GPIO's
#define DS18B20_PIN 10
#define PWM_PIN 13  // GPIO16
#define LED_PIN 16   // GPIO2

// Variables per al sensor de temperatura
OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);

// Variables globals per a la XN (corresponen a les entrades del model entrenat)
float current_temp_value;
float setpoint_value = 50.0; // Posa aquí el setpoint desitjat per al teu control
float error_nn, prev_error_nn = 0, integral_nn = 0, derivada_nn = 0; // Inputs per a la NN
// --- Constants per a l'Anti-Windup de la Integral ---
// Aquests valors s'han d'ajustar. Pots inferir-los de la teva data_entrenament.csv,
// observant el rang màxim i mínim de la columna 'integral'.
const float MAX_INTEGRAL = 200.0f; // Ajusta aquest valor segons el rang de les teves dades
const float MIN_INTEGRAL = -200.0f; // Ajusta aquest valor


// Objecte Tasker
Tasker tasker;

// Declaració de les tasques
void blinkLED();
void controlTemp();

// Funció per limitar el PWM entre 0 i 1023 (important per seguretat)
float clamp_pwm(float value) {
  if (value < 0.0f) return 0.0f;
  if (value > 1023.0f) return 1023.0f;
  return value;
}

void setup() 
{
  Serial.begin(115200);
  delay(1000);
  Serial.println(" ");
  Serial.println("Exercici-NN_PID_Control"); // Canviat el nom
  pinMode(PWM_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  analogWriteFreq(1000);       // PWM a 1 kHz
  analogWriteRange(1023);      // Rang de 0 a 1023

  sensors.begin();
  sensors.setResolution(12);   // Màxima resolució
  sensors.requestTemperatures();
  delay(800); // Esperar per primera lectura


  // Tasker configuracio
  tasker.setInterval(blinkLED, 1000);    // Tasca 1 Cada 1000 ms
  tasker.setInterval(controlTemp, 800);  // Tasca 2 Cada 800 ms
}

void loop() 
{
  tasker.loop();  // Mantenir Tasker actiu
}


//********************* TASQUES ************************* */

// Tasca 1
void blinkLED()
{
  digitalWrite(LED_PIN, !digitalRead(LED_PIN));
}

// Tasca 2
void controlTemp() 
{
  
  current_temp_value = sensors.getTempCByIndex(0); 

  if (isnan(current_temp_value)) { // Comprova si la lectura és vàlida
      Serial.println("Error: No es pot llegir la temperatura!");
      // Opcional: Podries intentar amb l'últim PWM vàlid o un valor per defecte si ho desitges
      return; 
  }

  // Càlcul de les entrades per a la Xarxa Neuronal
  // Aquests càlculs d'error, integral i derivada han de coincidir amb
  // els que vas usar per generar les dades d'entrenament del teu model
  error_nn = setpoint_value - current_temp_value;
  
  // Control de la integral per evitar 'wind-up'. Aquest valor és crític
  // que estigui ben ajustat per coincidir amb l'escala de la teva integral
  // en les dades d'entrenament.
  integral_nn += error_nn * (800.0f / 1000.0f); // SampleTime de 800ms

  // *********** LÒGICA ANTI-WINDUP ***********
  if (integral_nn > MAX_INTEGRAL) {
      integral_nn = MAX_INTEGRAL;
  } else if (integral_nn < MIN_INTEGRAL) {
      integral_nn = MIN_INTEGRAL;
  }
  // *******************************************

  derivada_nn = (error_nn - prev_error_nn) / (800.0f / 1000.0f); // SampleTime de 800ms
  prev_error_nn = error_nn;

  // Prepara les entrades per a la Xarxa Neuronal
  float nn_inputs[NN_INPUT_DIM] = {
      current_temp_value,
      error_nn,
      integral_nn,
      derivada_nn
  };

  // Realitza la predicció amb la Xarxa Neuronal
  float predicted_pwm = predict_nn(nn_inputs);

  // Limita la sortida del PWM al rang físic (0-1023)
  predicted_pwm = clamp_pwm(predicted_pwm);
  
  analogWrite(PWM_PIN, (int)predicted_pwm); // Aplica el valor predit del PWM

  // Imprimeix valors per depuració al Serial
  Serial.print(">T:"); Serial.println(current_temp_value);
  Serial.print(">Tamb:"); Serial.println(30);
  Serial.print(">Tmax:"); Serial.println(51);
  Serial.print(">Tmin:"); Serial.println(49);
  Serial.print(">Setpoint:"); Serial.println(setpoint_value);
  Serial.print(">Error_NN:"); Serial.println(error_nn);
  Serial.print(">Integral_NN:"); Serial.println(integral_nn);
  Serial.print(">Derivada_NN:"); Serial.println(derivada_nn);
  Serial.print(">PWM_predit:"); Serial.println((int)predicted_pwm);

  sensors.requestTemperatures();
}