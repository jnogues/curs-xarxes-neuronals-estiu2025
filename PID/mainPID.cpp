// Inici doc-----------------------------------------------------
// Exercici-PID Versio2 05-06-2025
// Curs xarxes neuronals estiu 2025
// I2SB, Institut Industria Sostenible de Barcelona
// Jaume Nogues jnogues@irp.cat
// ESP8266 nodemcu 1.0
// VSC + platformio
// PID basic per jugar amb Kp, Ki i Kd
// Una o dos resistències de 10 ohm aïllades amb porexpan
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
  br3ttb/PID @ ^1.2.1

*/
// Fi doc-----------------------------------------------------

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

//Constants PID
double Kp = 400.0, Ki = 4.0, Kd = 150.0;//400.0 4.0 150.0 Va molt be
//double Kp = 400.0, Ki = 3.5, Kd = 180.0;//400.0 4.0 150.0 També va bé

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
  Serial.println("Exercici-PID");
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
  Input = sensors.getTempCByIndex(0);

  // Executar sempre el càlcul del PID
  if (myPID.Compute()) {
    analogWrite(PWM_PIN, (int)Output);  // Aplicar sempre el PWM calculat
  }

  // Debug per sèrie
  Serial.print(">T:"); Serial.println(Input);
  Serial.print(">PWM:"); Serial.println(Output);
  Serial.print(">Tamb:"); Serial.println(30);
  Serial.print(">setPoint:"); Serial.println(50);

  sensors.requestTemperatures();  // Preparar nova lectura
}
