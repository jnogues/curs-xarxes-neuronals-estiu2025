//Preparat per a Recollida de Dades i entrenament XN

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PID_v1.h> 
#include <Tasker.h>

//GPIO's
#define DS18B20_PIN 10
#define PWM_PIN     13
#define LED_PIN 16

//Variables PID
double Setpoint = 50.0;  // Temperatura desitjada (°C)
double Input;            // Temperatura mesurada
double Output;           // PWM calculat pel PID

//Constants PID (Aquests són els que has sintonitzat!)
// Utilitza els valors que t'han donat un bon control.
double Kp = 400.0, Ki = 4.0, Kd = 150.0; // Valors d'exemple, substitueix pels teus afinats!

// --- Variables per al càlcul manual de Integral i Derivada (per a la XN) ---
double error_manual = 0;
double integral_manual = 0;
double lastError_manual = 0;
double derivative_manual = 0;
unsigned long lastTime_manual_calc; // Temps per als càlculs manuals

//Objectes globals
OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);
Tasker tasker;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

//Prototips de funcions
void controlTemp(); //Tasca 1
void blinkLED();    //Tasca 2

void setup() 
{
  Serial.begin(115200);
  delay(1000);
  Serial.println(" ");
  Serial.println("Exercici-PID amb dades per XN");
  Serial.println("Recollint: T, Setpoint, Error, Integral, Derivada, PWM");
  pinMode(PWM_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  analogWriteFreq(1000);       // PWM a 1 kHz
  analogWriteRange(1023);      // Rang de 0 a 1023

  sensors.begin();
  sensors.setResolution(12);   // Màxima resolució
  sensors.requestTemperatures();
  delay(800); // Esperar per primera lectura

  // PID configuracio
  myPID.SetOutputLimits(0, 1023);   // Límits físics del PWM
  myPID.SetMode(AUTOMATIC);         // Activa mode automàtic
  myPID.SetSampleTime(800);         // Correspon al nostre interval

  // Tasker configuracio
  tasker.setInterval(blinkLED, 1000);    // Tasca 1 Cada 1000 ms
  tasker.setInterval(controlTemp, 800);  // Tasca 2 Cada 800 ms
  lastTime_manual_calc = millis(); // Inicialitza el temps per als càlculs manuals
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
  // Temps transcorregut per als nostres càlculs manuals (en segons)
  unsigned long now = millis();
  double timeChange = (double)(now - lastTime_manual_calc) / 1000.0;
  lastTime_manual_calc = now; // Actualitza el temps de l'últim càlcul

  Input = sensors.getTempCByIndex(0);

  // --- Càlculs del PID de la llibreria ---
  if (myPID.Compute()) {
    analogWrite(PWM_PIN, (int)Output);  // Aplicar el PWM calculat
  }

  // --- Càlculs manuals d'Error, Integral i Derivada (per a la XN) ---
  error_manual = Setpoint - Input;

  // Calcul integral
  integral_manual += error_manual * timeChange;
  // Límits anti-windup per a la nostra integral manual (ajusta-los si cal)
  double manualIntegralMax = 200.0; // Poden ser diferents als de la llibreria si no els coneixem
  double manualIntegralMin = -200.0;
  if (integral_manual > manualIntegralMax) integral_manual = manualIntegralMax;
  if (integral_manual < manualIntegralMin) integral_manual = manualIntegralMin;

  // Calcul derivada
  derivative_manual = (error_manual - lastError_manual) / timeChange;
  lastError_manual = error_manual; // Actualitza l'error per a la propera iteració


  // --- Telemetria per sèrie (format Teleplot amb tots els camps per a la XN) ---
  Serial.printf(">T:%.2f\n", Input);                 // Temperatura actual
  Serial.printf(">Setpoint:%.2f\n", Setpoint);       // Temperatura desitjada
  Serial.printf(">error:%.2f\n", error_manual);      // Error per la XN
  Serial.printf(">integral:%.2f\n", integral_manual); // Integral per la XN
  Serial.printf(">derivada:%.2f\n", derivative_manual);// Derivada per la XN
  Serial.printf(">pwm:%d\n", (int)Output);           // Sortida PWM que el PID ha aplicat
  Serial.printf(">Tamb:%.2f\n", 30.0);               // Referència (ambient)
  Serial.printf(">Tmin:%.2f\n", Setpoint - 1.0);     // Banda visual inferior
  Serial.printf(">Tmax:%.2f\n", Setpoint + 1.0);     // Banda visual superior

  sensors.requestTemperatures();  // Preparar nova lectura
}