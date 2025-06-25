// Exercici-000 Versi0 25-06-2025
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
;upload_port = COM3
upload_speed = 512000
board_build.f_cpu = 160000000L

lib_deps =
  
  joysfera/Tasker @ ^2.0.3
  paulstoffregen/OneWire@^2.3.8
  milesburton/DallasTemperature@^4.0.4



*/ 


#include <Arduino.h>
#include "Tasker.h"

Tasker tasker; // Creem un gestor de taskes de nom tasker

#define LED0_PIN    0
#define LED2_PIN    2
#define LED16_PIN   16 
#define PWM_PIN     13

// Prototips de funcions, aixi poden estar despres de loop()
void intermitaLed16(); // Tasca 1
void intermitaLed0();  // Tasca 2
void intermitaLed2();  // Tasca 3
void telemetria();     // Tasca 4

void setup()
{
    // Configurem Serial i benvinguda
    Serial.begin(115200);
    delay(1000); // Esperem una estoneta a que el port serie estigui a punt
    Serial.println(" ");
    Serial.println("Comencem......");
    Serial.println("Exercici-000 Test inicial!"); 
    
    pinMode(0,OUTPUT);
    digitalWrite(0, LOW); // Led 0 off
    pinMode(2,OUTPUT);
    digitalWrite(2, LOW); // Led 2 off
    pinMode(16,OUTPUT);
    digitalWrite(16, LOW); // Led 16 off

    analogWriteRange(1023);
    pinMode(PWM_PIN, OUTPUT);
    analogWrite(PWM_PIN, 0); 
    
    // Configuració de les tasques
    tasker.setInterval(intermitaLed16, 300); // Tasca 1
    tasker.setInterval(intermitaLed0, 500);  // Tasca 2
    tasker.setInterval(intermitaLed2, 700);  // Tasca 3
    tasker.setInterval(telemetria, 100);     // Tasca 4
    
}

void loop()
{

    tasker.loop(); // Gestio de les tasques

}



// ************************ Tasques *************************

// Tasca 1
void intermitaLed16()
{
    digitalWrite(16, !digitalRead(16));
}

// Tasca 2
void intermitaLed0()
{
    digitalWrite(0, !digitalRead(0));
}

// Tasca 3
void intermitaLed2()
{
    digitalWrite(2, !digitalRead(2));
}

// Tasca 4 , Telemetria per Teleplot
void telemetria()
{
    Serial.print(">A0:"); Serial.println(analogRead(A0));
}