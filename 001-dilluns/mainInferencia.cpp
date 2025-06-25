// Exercici-001-inferencia Versio 25-06-2025
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
// Inferencia

#include <Arduino.h>
#include <Tasker.h> 

// Xarxa neuronal 4-8-2 entrenada
const int InputNodes = 4;
const int HiddenNodes = 8;
const int OutputNodes = 2;

/* Entrenament en ESP8266
// Pesos entre les entrades i la capa amagada (inclou biaixos a l'última fila)
const float HiddenWeights[][HiddenNodes] = {
  {0.638287, -6.135964, -0.234393, 3.777576, 4.788980, -5.077071, -1.496645, -4.492221},
  {2.567598, 6.195114, -2.211198, -5.504265, 2.544335, 0.974372, -1.360966, -5.907733},
  {1.075754, 4.854670, -1.169886, 3.513667, -4.721368, 6.007407, 1.764682, 6.167396},
  {2.610736, 6.199662, -2.091509, -5.567939, 2.108255, 1.590481, -1.295955, -5.869940},
  {-1.890016, -2.039222, 1.594169, -1.132664, -2.513614, -0.168414, 1.428392, 2.005616}, // Biaixos
};

// Pesos entre la capa amagada i la de sortida (inclou biaixos a l'última fila)
const float OutputWeights[][OutputNodes] = {
  {-1.768913, 3.709338},
  {10.424750, 4.905071},
  {3.097234, -2.177868},
  {6.499412, -4.566014},
  {-6.613309, 2.088778},
  {-8.127640, 0.981205},
  {2.925395, -1.789570},
  {-9.390575, -3.500240},
  {1.069683, 0.208826}, // Biaixos
};
*/

// Entrenament amb entrenadorPython.py
const float HiddenWeights[][HiddenNodes] = {
  {-5.789902, 0.628230, 3.936206, -4.341748, -1.956181, -4.959378, -4.504487, 1.963532},
  {-6.527766, 4.297890, 3.755985, 1.402896, 0.413783, -4.082824, 3.868259, -5.537065},
  {-6.537889, -2.881077, 2.364517, -2.555442, -1.709425, -3.820374, -5.068625, -2.990155},
  {-6.145655, 4.296244, 3.768181, -3.818015, -1.740317, 2.517283, 3.458445, -5.640432},
  {8.861224, -4.044344, -6.929563, 1.905593, -0.276258, 2.074940, -0.731785, 2.814101}  // Biaixos
};

const float OutputWeights[][OutputNodes] = {
  {11.844481, -2.008877},
  {-8.024986, 3.032267},
  {6.118095, 4.731848},
  {-5.872631, -0.393780},
  {-1.809687, 2.531773},
  {-6.338333, 1.666901},
  {7.720337, 4.782614},
  {-5.073220, -8.119347},
  {-1.645476, 1.064408}  // Biaixos
};




// Assignació explícita de pins d'entrada
const uint8_t inputPin0 = 4;   // Entrada 1
const uint8_t inputPin1 = 5;   // Entrada 2
const uint8_t inputPin2 = 12;  // Entrada 3
const uint8_t inputPin3 = 14;  // Entrada 4

// Assignació explícita de pins de sortida
const uint8_t outputPin0 = 0; // Sortida 1 (lògica inversa)
const uint8_t outputPin1 = 2; // Sortida 2
#define  LED_BLAU  16       // Sortida 16

Tasker tasker;

// Tasca 1
void blinkLED()
{
  digitalWrite(LED_BLAU, !digitalRead(LED_BLAU));
}


// Tasca 2
// Funció que s'executa cada 100 ms per fer la inferència
void inferencia() {
  float input[InputNodes];
  float hidden[HiddenNodes];
  float output[OutputNodes];

  // Llegeix les entrades digitals
  input[0] = digitalRead(inputPin0);
  input[1] = digitalRead(inputPin1);
  input[2] = digitalRead(inputPin2);
  input[3] = digitalRead(inputPin3);

  // Calcula l'activació de la capa amagada (sigmoide)
  for (int i = 0; i < HiddenNodes; i++) {
    float acc = HiddenWeights[InputNodes][i]; // Inicialitza amb el bias
    for (int j = 0; j < InputNodes; j++) {
      acc += input[j] * HiddenWeights[j][i];
    }
    hidden[i] = 1.0 / (1.0 + exp(-acc)); // funció d'activació
  }

  // Calcula l'activació de la capa de sortida (sigmoide)
  for (int i = 0; i < OutputNodes; i++) {
    float acc = OutputWeights[HiddenNodes][i]; // Inicialitza amb el bias
    for (int j = 0; j < HiddenNodes; j++) {
      acc += hidden[j] * OutputWeights[j][i];
    }
    output[i] = 1.0 / (1.0 + exp(-acc)); // funció d'activació
  }

  // Escriu les sortides al GPIO (amb conversió a nivell lògic binari)
  digitalWrite(outputPin0, output[0] >= 0.5 ? LOW : HIGH); // Sortida amb lògica inversa
  digitalWrite(outputPin1, output[1] >= 0.5 ? HIGH : LOW); // Sortida normal

  // Envia per sèrie l'estat de les entrades i sortides
  Serial.print("Entrades: ");
  for (int i = 0; i < InputNodes; i++) {
    Serial.print((int)input[i]);
    Serial.print(" ");
  }
  Serial.print("| Sortides: ");
  for (int i = 0; i < OutputNodes; i++) {
    Serial.print(output[i], 3);
    Serial.print(" ");
  }
  Serial.println();
}

// Inicialitza els pins i programa la tasca recurrent d'inferència
void setup() {
  pinMode(inputPin0, INPUT);
  pinMode(inputPin1, INPUT);
  pinMode(inputPin2, INPUT);
  pinMode(inputPin3, INPUT);

  pinMode(outputPin0, OUTPUT);
  digitalWrite(outputPin0, HIGH);
  pinMode(outputPin1, OUTPUT);
  digitalWrite(outputPin1, HIGH);
  pinMode(LED_BLAU, OUTPUT);
  digitalWrite(LED_BLAU, HIGH);


  Serial.begin(115200); // Inicialitza comunicació sèrie
  delay(1000);
  Serial.println("Sistema iniciat. Inferència cada 100ms.");

  tasker.setInterval(blinkLED, 1000); // Executa inferència cada 1000 ms
  tasker.setInterval(inferencia, 100); // Executa inferència cada 100 ms
  
}

// Bucle principal que manté el sistema en funcionament
void loop() 
{
  tasker.loop();
}
