//Inici doc-----------------------------------------------------
//Exercici-004 Versio 07-06-2025
//Curs xarxes neuronals estiu 2025
//I2SB, Institut Industria Sostenible de Barcelona
//Jaume Nogues jnogues@irp.cat
//ESP8266 nodemcu 1.0
//VSC + platformio
//Termostat NN + PI per una consigna de 50C amb dades ja entrenades
//El dataset50C.csv està a la carpeta extres:
//1r cal normalitzar-lo de 0.0 a 1.0 amb normalize_csv_to_train_nn.py
//2n copiem el dataset normalitzat i l'enganxem a train_nn_50C.py
//3r l'executem per entrenar
//4t copiem pesos resultants per pegar-los en aquest programa 

//Contingut platformio.ini:
/*

[env:nodemcuv2]
platform = espressif8266
board = nodemcuv2
build_flags = -D EEPROM_SIZE=1024
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
#include <math.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 10
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature temperatureDS(&oneWire);

#include "Tasker.h"
Tasker tasker;

/*************************  Definició de la mida de la xarxa ********************************/
const int input_neurons = 1;
const int hidden1_neurons = 5;  // Primera capa oculta
const int hidden2_neurons = 3;  // Segona capa oculta
const int output_neurons = 1;
/****************************************************************************************************/

// Pesos i biaixos en format per copiar al codi Arduino:
float weights_input_hidden1[hidden1_neurons] = {
    2.342184, -3.499474, 7.472236, -8.273430, 4.473033
};

float weights_hidden1_hidden2[hidden1_neurons][hidden2_neurons] = {
    { -1.976171, -1.355278, 1.123294 },
    { 2.381539, 2.035939, -4.614745 },
    { -7.197997, -1.135755, 0.636513 },
    { 1.696426, 4.779124, -11.652722 },
    { -2.341656, -3.843585, 4.390120 }
};

float weights_hidden2_output[hidden2_neurons] = {
    5.758445, 1.828496, -8.763787
};

float bias_hidden1[hidden1_neurons] = {
    -0.743730, 2.314878, -0.355552, 6.797288, -2.909866
};

float bias_hidden2[hidden2_neurons] = {
    -0.653985, -1.483059, -1.660262
};

float bias_output = 
    -0.622661;



/******************* VARIABLES i PROTOTIPS ********************************************************/

float entrada = 0.0;
float sortida = 0.0;
float temperatura = 0.0;
unsigned int pwmOutput = 0;

//Prototips de funcions 
float normalitza(float x, float in_min, float in_max, float out_min, float out_max);
void intermitaLed16();
void llegeixTemperatura();
float sigmoid(float x);
float forward_propagation(float input_value, float hidden1_output[], float hidden2_output[]);

void setup() 
{
    Serial.begin(115200);
    delay(1000);
    analogWriteRange(1023);
    analogWrite(13, 0);
    temperatureDS.begin();
    temperatureDS.setResolution(12);//9, 10, 11 o 12 bits.  94ms, 190ms, 380ms, 750ms. 0.5°C, 0.25°C, 0.125°C, or 0.0625°C.
    Serial.println(" ");
    Serial.println("Comencem.......");  
    pinMode(16,OUTPUT);
    digitalWrite(16, HIGH);
    Serial.print("Petició de temperatura...");
    temperatureDS.requestTemperatures();

    // Configuració tasques
    tasker.setInterval(intermitaLed16, 3000);
    tasker.setInterval(llegeixTemperatura, 800);//Aquest temps ve determinat pel temps de conversio del DS18B20
}

void loop() 
{
  tasker.loop();
}

//****************** TASQUES **************************/
// Tasca 1
void intermitaLed16()
{
  digitalWrite(16, !digitalRead(16));
}

// Tasca 2
void llegeixTemperatura()
{
  temperatura = temperatureDS.getTempCByIndex(0);
  float temperaturaRaw = temperatura;
  if (temperatura < 47.0) temperatura = 47.0;
  else if (temperatura > 50.5) temperatura = 50.5;
  entrada = normalitza(temperatura, 47.0, 50.5, 0.0, 1.0);
 
  float hidden1_output[hidden1_neurons];
  float hidden2_output[hidden2_neurons];
  sortida = forward_propagation(entrada, hidden1_output, hidden2_output);
  
// --- Paràmetres de control ---
const float alpha = 0.9;          // Suavitzat exponencial
const float setpoint = 50.0;      // Temperatura desitjada
const float kp = 0.0;            // Proporcional
const float ki = 0.5;             // Integral

// --- Variables persistents ---
static float pwmFiltrat = 0;
static float integralError = 0;

// --- Càlcul de l'error ---
float error = setpoint - temperatura;
integralError += error;
// Limitació anti-windup
if (integralError > 300.0) integralError = 300.0;
if (integralError < -300.0) integralError = -300.0;


// --- PID corrector ---
float pid = kp * error + ki * integralError;

// --- Aplicació del filtratge exponencial ---
int pwmBase = sortida * 1023;
pwmFiltrat = pwmFiltrat * (1.0 - alpha) + pwmBase * alpha;

// --- Combinació NN + PID ---
int pwmFinal = constrain(pwmFiltrat + pid, 0, 1023);

// --- Sortida final PWM ---
analogWrite(13, pwmFinal);

  
  //Telemetria per Teleplot
  Serial.print(">Tamb:");
  Serial.println(30);
  Serial.print(">Tmin:");
  Serial.println(49);
  Serial.print(">setPoint:");
  Serial.println(50);
  Serial.print(">Tmax:");
  Serial.println(51);
  Serial.print(">T:");
  Serial.println(temperaturaRaw);
  Serial.print(">pwm:");
  Serial.println(pwmFinal);
  Serial.print(">integralError:");
  Serial.println(integralError);

  temperatureDS.requestTemperatures(); //Enviem petició conversió tmeperatura, que triga 800ms, per això aquí al final
  
}

//********************** FUNCIONS *******************************************
// Funció de normalització de 0.0 a 1.0
float normalitza(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Funció d'activació Sigmoide
float sigmoid(float x) {
    return 1 / (1 + exp(-x));
}

// Propagació cap endavant
float forward_propagation(float input_value, float hidden1_output[], float hidden2_output[]) {
    // Primera capa oculta
    for (int i = 0; i < hidden1_neurons; i++) {
        float weighted_sum = input_value * weights_input_hidden1[i] + bias_hidden1[i];
        hidden1_output[i] = sigmoid(weighted_sum);
    }

    // Segona capa oculta
    for (int j = 0; j < hidden2_neurons; j++) {
        float weighted_sum = 0;
        for (int i = 0; i < hidden1_neurons; i++) {
            weighted_sum += hidden1_output[i] * weights_hidden1_hidden2[i][j];
        }
        weighted_sum += bias_hidden2[j];
        hidden2_output[j] = sigmoid(weighted_sum);
    }

    // Capa de sortida
    float output = 0;
    for (int j = 0; j < hidden2_neurons; j++) {
        output += hidden2_output[j] * weights_hidden2_output[j];
    }
    output += bias_output;
    output = sigmoid(output);

    return output;
}
