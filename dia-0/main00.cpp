//Inici doc-----------------------------------------------------
//Exercici-00 Versi0 05-06-2025
//Curs xarxes neuronals estiu 2025
//I2SB, Institut Industria Sostenible de Barcelona
//Jaume Nogues jnogues@irp.cat
//ESP8266 nodemcu 1.0
//VSC + platformio
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



*/ 
//Fi doc-------------------------------------------------------

#include <Arduino.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#define DS1820_PIN 10
#define PWM_PIN 13

OneWire DS1820_oneWire(DS1820_PIN);
DallasTemperature DS1820_sensor(&DS1820_oneWire);
float temperature;
float temperature_max = 60.0;
float temperature_amb = 30.0;


void setup()
{

    Serial.begin(115200);
    delay(1000);
    DS1820_sensor.begin();
    DS1820_sensor.setResolution(10);//9, 10, 11 o 12 bits.  94ms, 190ms, 380ms, 750ms. 0.5°C, 0.25°C, 0.125°C, or 0.0625°C.
    analogWriteRange(1023);
    pinMode(PWM_PIN, OUTPUT);
    analogWrite(PWM_PIN, 0); 
    Serial.println("Exercici-00 Test inicial!"); 
    
}

void loop()
{
  DS1820_sensor.requestTemperatures();
  delay(200); //Cal esperar a conversio, 10 bits 200ms
  temperature = DS1820_sensor.getTempCByIndex(0);  // llegir temperatura en C
  int pwm_manual = analogRead(A0);  // Entrada manual per potenciómetre
  if(temperature > temperature_max) pwm_manual = 0;
  analogWrite(PWM_PIN, pwm_manual);  // Aplicar PWM a la sortida
  
  //Telemetria per Teleplot
  Serial.print(">pwm_manual:");
  Serial.println(pwm_manual);
  Serial.print(">Tamb:");
  Serial.println(temperature_amb);
  Serial.print(">Tmax:");
  Serial.println(temperature_max);
  Serial.print(">T:");
  Serial.println(temperature);
}
