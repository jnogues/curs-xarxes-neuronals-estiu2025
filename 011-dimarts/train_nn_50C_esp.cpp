//Inici doc-----------------------------------------------------
//Exercici-003 Versio 06-06-2025
//Sketch per entrenar directament a l'ESP8266
//Curs xarxes neuronals estiu 2025
//I2SB, Institut Industria Sostenible de Barcelona
//Jaume Nogues jnogues@irp.cat
//ESP8266 nodemcu 1.0
//VSC + platformio
//Termostat NN per una consigna de 50C amb dades ja entrenades
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

// Per entrenar el Termostat Neuronal

#include <math.h>
#include <Arduino.h> // Inclou Arduino.h per Serial.print, pinMode, etc.


// Definició de la mida de la xarxa
const int input_neurons = 1;
const int hidden1_neurons = 5;  // Primera capa oculta
const int hidden2_neurons = 3;  // Segona capa oculta
const int output_neurons = 1;
#define learningRate 0.01    //0.1 rapid (21 m),  0.01 lent, 0.001 molt lent
#define minimalError 0.01   //0.01

// Dades d'entrenament (Pots reemplaçar aquest contingut amb el teu dataset més gran)
const float training_DATA[][2] = 
{
        {0.0000, 1.0000},
        {0.0286, 0.9279},
        {0.1143, 0.8292},
        {0.2000, 0.7799},
        {0.2857, 0.7799},
        {0.3429, 0.7305},
        {0.4000, 0.7305},
        {0.4571, 0.6811},
        {0.5143, 0.6811},
        {0.5714, 0.6318},
        {0.6000, 0.5824},
        {0.6286, 0.5331},
        {0.6571, 0.4837},
        {0.6857, 0.4590},
        {0.7143, 0.4344},
        {0.7429, 0.4097},
        {0.7714, 0.3850},
        {0.8000, 0.3603},
        {0.8286, 0.3356},
        {0.8571, 0.3060},
        {0.8857, 0.2468},
        {0.9143, 0.1382},
        {0.9429, 0.0395},
        {1.0000, 0.0000}
};
const int PatternCount = sizeof(training_DATA) / sizeof(training_DATA[0]);


/******************************************************************
 * Node Values - customized per network
 ******************************************************************/
float Input[input_neurons];
float Hidden1[hidden1_neurons];
float Hidden2[hidden2_neurons];
float Output[output_neurons];

/******************************************************************
 * Weights and biases - customized per network
 ******************************************************************/
float weights_input_hidden1[input_neurons][hidden1_neurons];
float bias_hidden1[hidden1_neurons];
float weights_hidden1_hidden2[hidden1_neurons][hidden2_neurons];
float bias_hidden2[hidden2_neurons];
float weights_hidden2_output[hidden2_neurons][output_neurons];
float bias_output[output_neurons]; 


/******************************************************************
 * Error Variables - customized per network
 ******************************************************************/
float Target[output_neurons]; 
float Error;
float output_delta[output_neurons];
float hidden2_delta[hidden2_neurons];
float hidden1_delta[hidden1_neurons];


/******************************************************************
 * Weight Changes (for momentum) - customized per network
 ******************************************************************/
float delta_weights_input_hidden1[input_neurons][hidden1_neurons];
float delta_bias_hidden1[hidden1_neurons];
float delta_weights_hidden1_hidden2[hidden1_neurons][hidden2_neurons];
float delta_bias_hidden2[hidden2_neurons];
float delta_weights_hidden2_output[hidden2_neurons][output_neurons];
float delta_bias_output[output_neurons];

unsigned long TrainingCycle = 0;
int ReportEvery1000 = 1;


/******************************************************************
 * Utility Functions
 ******************************************************************/
float activationFunction(float x) { return 1.0 / (1.0 + expf(-x)); } // Usant expf per float
float activationFunctionDerivative(float x) { return x * (1 - x); }


/******************************************************************
 * Weight Management
 ******************************************************************/
void initWeights() {
    randomSeed(analogRead(A0)); // Usa analogRead per una millor aleatorietat

    // Inicialitzar pesos i biaixos aleatòriament
    for (int i = 0; i < input_neurons; i++) {
        for (int j = 0; j < hidden1_neurons; j++) {
            weights_input_hidden1[i][j] = ((float)random(0, 1000) / 1000.0) - 0.5; // -0.5 a 0.5
            delta_weights_input_hidden1[i][j] = 0.0;
        }
    }
    for (int i = 0; i < hidden1_neurons; i++) {
        bias_hidden1[i] = ((float)random(0, 1000) / 1000.0) - 0.5;
        delta_bias_hidden1[i] = 0.0;
    }

    for (int i = 0; i < hidden1_neurons; i++) {
        for (int j = 0; j < hidden2_neurons; j++) {
            weights_hidden1_hidden2[i][j] = ((float)random(0, 1000) / 1000.0) - 0.5;
            delta_weights_hidden1_hidden2[i][j] = 0.0;
        }
    }
    for (int i = 0; i < hidden2_neurons; i++) {
        bias_hidden2[i] = ((float)random(0, 1000) / 1000.0) - 0.5;
        delta_bias_hidden2[i] = 0.0;
    }

    for (int i = 0; i < hidden2_neurons; i++) {
        for (int j = 0; j < output_neurons; j++) {
            weights_hidden2_output[i][j] = ((float)random(0, 1000) / 1000.0) - 0.5;
            delta_weights_hidden2_output[i][j] = 0.0;
        }
    }
    for (int i = 0; i < output_neurons; i++) {
        bias_output[i] = ((float)random(0, 1000) / 1000.0) - 0.5;
        delta_bias_output[i] = 0.0;
    }
}


void toTerminal() {
    Serial.println("\n// Pesos i biaixos en format C/C++");

    Serial.println("\n// weights_input_hidden1 (shape: [input_neurons][hidden1_neurons])");
    Serial.println("float weights_input_hidden1[input_neurons][hidden1_neurons] = {");
    for (int i = 0; i < input_neurons; i++) {
        Serial.print("    {");
        for (int j = 0; j < hidden1_neurons; j++) {
            Serial.print(weights_input_hidden1[i][j], 6);
            if (j < hidden1_neurons - 1) Serial.print(", ");
        }
        Serial.print("}");
        if (i < input_neurons - 1) Serial.println(",");
        else Serial.println();
    }
    Serial.println("};");

    Serial.println("\n// bias_hidden1 (shape: [hidden1_neurons])");
    Serial.print("float bias_hidden1[hidden1_neurons] = {");
    for (int i = 0; i < hidden1_neurons; i++) {
        Serial.print(bias_hidden1[i], 6);
        if (i < hidden1_neurons - 1) Serial.print(", ");
    }
    Serial.println("};");

    Serial.println("\n// weights_hidden1_hidden2 (shape: [hidden1_neurons][hidden2_neurons])");
    Serial.println("float weights_hidden1_hidden2[hidden1_neurons][hidden2_neurons] = {");
    for (int i = 0; i < hidden1_neurons; i++) {
        Serial.print("    {");
        for (int j = 0; j < hidden2_neurons; j++) {
            Serial.print(weights_hidden1_hidden2[i][j], 6);
            if (j < hidden2_neurons - 1) Serial.print(", ");
        }
        Serial.print("}");
        if (i < hidden1_neurons - 1) Serial.println(",");
        else Serial.println();
    }
    Serial.println("};");

    Serial.println("\n// bias_hidden2 (shape: [hidden2_neurons])");
    Serial.print("float bias_hidden2[hidden2_neurons] = {");
    for (int i = 0; i < hidden2_neurons; i++) {
        Serial.print(bias_hidden2[i], 6);
        if (i < hidden2_neurons - 1) Serial.print(", ");
    }
    Serial.println("};");

    Serial.println("\n// weights_hidden2_output (shape: [hidden2_neurons][output_neurons])");
    Serial.println("float weights_hidden2_output[hidden2_neurons][output_neurons] = {");
    for (int i = 0; i < hidden2_neurons; i++) {
        Serial.print("    {");
        for (int j = 0; j < output_neurons; j++) {
            Serial.print(weights_hidden2_output[i][j], 6);
            if (j < output_neurons - 1) Serial.print(", ");
        }
        Serial.print("}");
        if (i < hidden2_neurons - 1) Serial.println(",");
        else Serial.println();
    }
    Serial.println("};");

    Serial.println("\n// bias_output (shape: [output_neurons])");
    Serial.print("float bias_output[output_neurons] = {");
    for (int i = 0; i < output_neurons; i++) {
        Serial.print(bias_output[i], 6);
        if (i < output_neurons - 1) Serial.print(", ");
    }
    Serial.println("};");
}


void setup()
{
  Serial.begin(115200);
  delay(100);
  Serial.println("\n--- Iniciant entrenament de la Xarxa Neuronal en C++ ---");
  Serial.println("Arquitectura: 1 (input) - 5 (hidden1) - 3 (hidden2) - 1 (output)");
  Serial.print("Learning Rate: "); Serial.println(learningRate);
  Serial.print("Minimal Error: "); Serial.println(minimalError, 6);
  Serial.print("Total Patrons: "); Serial.println(PatternCount);
  Serial.println(" ");
  pinMode(16, OUTPUT); // Pin 16 per indicar progrés
  digitalWrite(16, LOW); // LED apagat

  initWeights();


  // Entrenament de la xarxa
  while(1) {
    TrainingCycle++;

    // For each training pattern
    Error = 0.0;
    for (int p = 0; p < PatternCount; p++) {
      Input[0] = training_DATA[p][0]; // Input de temperatura
      Target[0] = training_DATA[p][1]; // Output de PWM

      // Forward propagation
      // Capa oculta 1
      for (int i = 0; i < hidden1_neurons; i++) {
        float sum = Input[0] * weights_input_hidden1[0][i] + bias_hidden1[i];
        Hidden1[i] = activationFunction(sum);
      }

      // Capa oculta 2
      for (int i = 0; i < hidden2_neurons; i++) {
        float sum = 0.0;
        for (int j = 0; j < hidden1_neurons; j++) {
          sum += Hidden1[j] * weights_hidden1_hidden2[j][i];
        }
        sum += bias_hidden2[i];
        Hidden2[i] = activationFunction(sum);
      }

      // Capa de sortida
      for (int i = 0; i < output_neurons; i++) {
        float sum = 0.0;
        for (int j = 0; j < hidden2_neurons; j++) {
          sum += Hidden2[j] * weights_hidden2_output[j][i];
        }
        sum += bias_output[i];
        Output[i] = activationFunction(sum);
      }

      // Backpropagation
      // Calcula l'error total
      for (int i = 0; i < output_neurons; i++) {
        Error += (Target[i] - Output[i]) * (Target[i] - Output[i]);
      }

      // Output deltas
      for (int i = 0; i < output_neurons; i++) {
        output_delta[i] = (Target[i] - Output[i]) * activationFunctionDerivative(Output[i]);
      }

      // Hidden2 deltas
      for (int i = 0; i < hidden2_neurons; i++) {
        float sum = 0.0;
        for (int j = 0; j < output_neurons; j++) {
          sum += output_delta[j] * weights_hidden2_output[i][j];
        }
        hidden2_delta[i] = sum * activationFunctionDerivative(Hidden2[i]);
      }

      // Hidden1 deltas
      for (int i = 0; i < hidden1_neurons; i++) {
        float sum = 0.0;
        for (int j = 0; j < hidden2_neurons; j++) {
          sum += hidden2_delta[j] * weights_hidden1_hidden2[i][j];
        }
        hidden1_delta[i] = sum * activationFunctionDerivative(Hidden1[i]);
      }

      // Update weights and biases with momentum (momentum factor hardcoded to 0.9)
      // Actualitza pesos hidden2 -> output
      for (int i = 0; i < hidden2_neurons; i++) {
        for (int j = 0; j < output_neurons; j++) {
          delta_weights_hidden2_output[i][j] = learningRate * Hidden2[i] * output_delta[j] + 0.9 * delta_weights_hidden2_output[i][j];
          weights_hidden2_output[i][j] += delta_weights_hidden2_output[i][j];
        }
      }
      for (int i = 0; i < output_neurons; i++) {
        delta_bias_output[i] = learningRate * output_delta[i] + 0.9 * delta_bias_output[i];
        bias_output[i] += delta_bias_output[i];
      }

      // Actualitza pesos hidden1 -> hidden2
      for (int i = 0; i < hidden1_neurons; i++) {
        for (int j = 0; j < hidden2_neurons; j++) {
          delta_weights_hidden1_hidden2[i][j] = learningRate * Hidden1[i] * hidden2_delta[j] + 0.9 * delta_weights_hidden1_hidden2[i][j];
          weights_hidden1_hidden2[i][j] += delta_weights_hidden1_hidden2[i][j];
        }
      }
      for (int i = 0; i < hidden2_neurons; i++) {
        delta_bias_hidden2[i] = learningRate * hidden2_delta[i] + 0.9 * delta_bias_hidden2[i];
        bias_hidden2[i] += delta_bias_hidden2[i];
      }

      // Actualitza pesos input -> hidden1
      for (int i = 0; i < input_neurons; i++) {
        for (int j = 0; j < hidden1_neurons; j++) {
          delta_weights_input_hidden1[i][j] = learningRate * Input[i] * hidden1_delta[j] + 0.9 * delta_weights_input_hidden1[i][j];
          weights_input_hidden1[i][j] += delta_weights_input_hidden1[i][j];
        }
      }
      for (int i = 0; i < hidden1_neurons; i++) {
        delta_bias_hidden1[i] = learningRate * hidden1_delta[i] + 0.9 * delta_bias_hidden1[i];
        bias_hidden1[i] += delta_bias_hidden1[i];
      }
      
      yield(); // Cedeix el control al sistema operatiu
    }


    /******************************************************************
    * Display training progress on the serial monitor.
    ******************************************************************/
    if (TrainingCycle % ReportEvery1000 == 0) {
      digitalWrite(16, !digitalRead(16)); // Toggle LED per indicar progrés
      Serial.println(); 
      Serial.println(); 
      Serial.print (">TrainingCycle:");
      Serial.println(TrainingCycle);
      Serial.print (">Error:");
      Serial.println (Error, 5);
      yield();

      // Ajusta la freqüència de report si és el primer cicle per evitar molta informació
      if (TrainingCycle == 1) {
        ReportEvery1000 = 999;
      } else {
        ReportEvery1000 = 1000;
      }
    }    


/******************************************************************
* If error rate is less than pre-determined threshold then end
******************************************************************/
    if( Error < minimalError ) break ;  
  }
  Serial.println ();
  Serial.println(); 
  Serial.print ("TrainingCycle: ");
  Serial.print (TrainingCycle);
  Serial.print ("  Error = ");
  Serial.println (Error, 5);

  toTerminal(); // Imprimeix els pesos finals

  Serial.println ();  
  Serial.println ();
  Serial.println ("Training Set Solved! ");
  Serial.println ("--------"); 
  Serial.println ();
  Serial.println ();  
  ReportEvery1000 = 1;
  // printWeights(); // Aquesta funció sembla ser la mateixa que toTerminal()
  Serial.println(" ");
  Serial.print("Temps d'entrenament: ");
  Serial.print(millis()/1000);
  Serial.println("segons");
  Serial.println(" ");
}


void loop()
{
  // El loop principal no fa res, ja que l'entrenament es fa completament a setup()
}

