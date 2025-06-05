//Inici doc-----------------------------------------------------
//Exercici-01 Versi0 05-06-2025
//Curs xarxes neuronals estiu 2025
//I2SB, Institut Industria Sostenible de Barcelona
//Jaume Nogues jnogues@irp.cat
//ESP8266 nodemcu 1.0
//VSC + platformio
//PID basic per jugar amb Kp, Ki i Kd
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
;upload_port = COM3
upload_speed = 512000
board_build.f_cpu = 160000000L

lib_deps =
  
  joysfera/Tasker @ ^2.0.3
  paulstoffregen/OneWire@^2.3.8
  milesburton/DallasTemperature@^4.0.4
  br3ttb/PID @ ^1.2.1


*/ 
//Fi doc-------------------------------------------------------

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PID_v1.h>

// --- Pins ---
#define PIN_TEMP    10
#define PIN_PWM     13
#define PIN_ANALOG  A0

// --- Constants del PID ---
double Kp = 50.0;
double Ki = 5.0;
double Kd = 0.5;

// --- Sensor ---
OneWire oneWire(PIN_TEMP);
DallasTemperature sensors(&oneWire);

// --- Variables PID ---
double inputTemp = 0;
double outputPWM = 0;
double setpoint = 50;

PID myPID(&inputTemp, &outputPWM, &setpoint, Kp, Ki, Kd, DIRECT);

// --- Control setpoint filtrat ---
double lastSetpoint = 50;
const double alphaSetpoint = 0.5; // Suavitza entrada analogica, entre 0.0-1.0, valor all, menys suau, 1.0 sense filtre

// --- Setup ---
void setup() {
  Serial.begin(115200);
  pinMode(PIN_PWM, OUTPUT);

  sensors.begin();
  sensors.setResolution(10); // 0.25 ºC

  myPID.SetOutputLimits(0, 1023);
  myPID.SetMode(AUTOMATIC);
}

// --- Funció: llegir temperatura ---
void readTemperature() {
  sensors.requestTemperatures();
  delay(200); // DS18B20 a 10 bits
  inputTemp = sensors.getTempCByIndex(0);
}

// --- Funció: actualitzar setpoint filtrat ---
void updateSetpoint() {
  int raw = analogRead(PIN_ANALOG);
  double newSetpoint = map(raw, 0, 1023, 30, 70);
  setpoint = alphaSetpoint * newSetpoint + (1 - alphaSetpoint) * lastSetpoint;
  lastSetpoint = setpoint;
}

// --- Funció: control de seguretat i PID ---
void computeControl() {
  if (inputTemp > 70.0) {
    outputPWM = 0;
    if (myPID.GetMode() != MANUAL) myPID.SetMode(MANUAL);
  } else {
    if (myPID.GetMode() != AUTOMATIC) myPID.SetMode(AUTOMATIC);
    myPID.Compute();
  }
  analogWrite(PIN_PWM, (int)outputPWM);
}

// --- Funció: enviar telemetria ---
void sendTelemetry() {
  double error = setpoint - inputTemp;
  Serial.printf(">T:%.2f\n", inputTemp);
  Serial.printf(">setpoint:%.2f\n", setpoint);
  Serial.printf(">pwm:%.0f\n", outputPWM);
  Serial.printf(">error:%.2f\n", error);
  Serial.printf(">Tamb:%.0f\n", 30.0);
  Serial.printf(">Tmax:%.0f\n", 60.0);
}

// --- Loop principal ---
void loop() {
  readTemperature();
  updateSetpoint();
  computeControl();
  sendTelemetry();
  delay(200); // Cicle estable
}
