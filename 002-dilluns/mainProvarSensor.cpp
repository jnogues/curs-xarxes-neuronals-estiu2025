// Per provar sensor

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

#include <Arduino.h> // Ja no cal que estigui inclòs, PlatformIO ja ho fa per tu.
#include <OneWire.h>
#include <DallasTemperature.h>

// Defineix els pins
const int ONE_WIRE_BUS = 10; // DS18B20 connectat a GPIO10
const int HEATER_PIN = 13;   // Resistència calefactora connectada a GPIO13 (PWM)

// Inicialització del DS18B20
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);
  delay(1000);
  sensors.begin(); // Inicialitza el sensor DS18B20
  sensors.setResolution(12);//9, 10, 11 o 12 bits.  94ms, 190ms, 380ms, 750ms. 0.5°C, 0.25°C, 0.125°C, or 0.0625°C.
  pinMode(HEATER_PIN, OUTPUT);
  Serial.println("--- Prova de DS18B20 i control de calefactor amb PlatformIO ---");
}

void loop() {
  // Llegeix la temperatura
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0); // Temperatura del primer sensor

  Serial.print("Temperatura: ");
  Serial.print(tempC);
  Serial.println(" °C");

  // Prova la resistència calefactora
  // Encén-la al 50% de potència durant 5 segons
  Serial.println("Encenc calefactor (50% PWM) per 5 segons...");
  analogWrite(HEATER_PIN, 512); // 512 és aproximadament el 50% de 1023
  delay(5000);

  // Apaga la resistència durant 5 segons
  Serial.println("Apago calefactor per 5 segons...");
  analogWrite(HEATER_PIN, 0); // 0 és apagat
  delay(5000);

  // Observa com canvia la temperatura al Serial Monitor.
}

