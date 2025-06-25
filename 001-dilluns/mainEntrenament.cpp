// Exercici-001-entrenament Versi0 25-06-2025
// Entrenament directe a l'ESP8266 d'una Taula de la veritat 
// de 4 entrades 2 sortides 
// Xarxa neuronal 4-8-2
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
#include <math.h>

/******************************************************************
 * Network Configuration - customized per network 
 * All basic settings can be controlled via the Network Configuration
 ******************************************************************/

const int PatternCount = 16;
const int InputNodes = 4;
const int HiddenNodes = 8;
const int OutputNodes = 2;
const float LearningRate = 0.01;//0.3
const float Momentum = 0.8;//0.9
const float InitialWeightMax = 0.5;
const float Success = 0.01;//0.04//0.004;

float Input[PatternCount][InputNodes] = {  
  {0,0,0,0},
  {0,0,0,1},
  {0,0,1,0},
  {0,0,1,1},
  {0,1,0,0},
  {0,1,0,1},
  {0,1,1,0},
  {0,1,1,1},
  
  {1,0,0,0},
  {1,0,0,1},
  {1,0,1,0},
  {1,0,1,1},
  {1,1,0,0},
  {1,1,0,1},
  {1,1,1,0},
  {1,1,1,1},
}; 

float  Target[PatternCount][OutputNodes] = { 
  {0,0},
  {1,1},
  {1,0},
  {0,1},
  {1,1},
  {0,1},
  {0,1},
  {1,1},
  
  {1,0},
  {0,1},
  {0,0},
  {1,1},
  {0,1},
  {1,1},
  {1,1},
  {0,1},    
};

/******************************************************************
 * End Network Configuration
 ******************************************************************/

//init variables
int i, j, p, q, r;
int ReportEvery1000;
int RandomizedIndex[PatternCount];
long  TrainingCycle;
float Rando;
float Error;
float Accum;

float Hidden[HiddenNodes];
float Output[OutputNodes];
float HiddenWeights[InputNodes+1][HiddenNodes];
float OutputWeights[HiddenNodes+1][OutputNodes];
float HiddenDelta[HiddenNodes];
float OutputDelta[OutputNodes];
float ChangeHiddenWeights[InputNodes+1][HiddenNodes];
float ChangeOutputWeights[HiddenNodes+1][OutputNodes];

boolean bOutput[OutputNodes];
unsigned int neuralLoopsCounter=0;


  /*****************************************************************
      Functions
  *****************************************************************/

void printWeightsAsCode() {
  Serial.println("const float HiddenWeights[][HiddenNodes] = {");
  for (int i = 0; i <= InputNodes; i++) {
    Serial.print("  {");
    for (int j = 0; j < HiddenNodes; j++) {
      Serial.print(HiddenWeights[i][j], 6);
      if (j < HiddenNodes - 1) Serial.print(", ");
    }
    Serial.println("},");
  }
  Serial.println("};\n");

  Serial.println("const float OutputWeights[][OutputNodes] = {");
  for (int i = 0; i <= HiddenNodes; i++) {
    Serial.print("  {");
    for (int j = 0; j < OutputNodes; j++) {
      Serial.print(OutputWeights[i][j], 6);
      if (j < OutputNodes - 1) Serial.print(", ");
    }
    Serial.println("},");
  }
  Serial.println("};");
}


void printWeights() {
  Serial.println("Pesos d'entrada a capa amagada:");
  for (int i = 0; i < InputNodes; i++) {
    for (int j = 0; j < HiddenNodes; j++) {
      Serial.print("  HiddenWeights[");
      Serial.print(i);
      Serial.print("][");
      Serial.print(j);
      Serial.print("] = ");
      Serial.println(HiddenWeights[i][j], 6);
    }
  }

  Serial.println("Pesos de capa amagada a sortida:");
  for (int i = 0; i < HiddenNodes; i++) {
    for (int j = 0; j < OutputNodes; j++) {
      Serial.print("  OutputWeights[");
      Serial.print(i);
      Serial.print("][");
      Serial.print(j);
      Serial.print("] = ");
      Serial.println(OutputWeights[i][j], 6);
    }
  }
}

void InputToOutput(float In1, float In2, float In3, float In4)
{
  float TestInput[] = {0,0,0,0};
  TestInput[0] = In1;
  TestInput[1] = In2;
  TestInput[2] = In3;
  TestInput[3] = In4;

  /******************************************************************
    Compute hidden layer activations
  ******************************************************************/

  for ( i = 0 ; i < HiddenNodes ; i++ ) {
    Accum = HiddenWeights[InputNodes][i] ;
    //Serial.print("Accum1="); Serial.println(Accum);//show bias
    for ( j = 0 ; j < InputNodes ; j++ ) {
      Accum += TestInput[j] * HiddenWeights[j][i] ;
    }
    Hidden[i] = 1.0 / (1.0 + exp(-Accum)) ;
  }

  /******************************************************************
    Compute output layer activations and calculate errors
  ******************************************************************/

  for ( i = 0 ; i < OutputNodes ; i++ ) {
    Accum = OutputWeights[HiddenNodes][i] ;
    //Serial.print("Accum2="); Serial.println(Accum);//show bias
    for ( j = 0 ; j < HiddenNodes ; j++ ) {
      Accum += Hidden[j] * OutputWeights[j][i] ;
    }
    Output[i] = 1.0 / (1.0 + exp(-Accum)) ;

    if (Output[i]>=0.5) bOutput[i]= 1;
    if (Output[i] <0.5) bOutput[i]= 0;
    //Serial.print(bOutput[i]);
    //Serial.print(" ");
  }
  //Serial.println(" ");

  //every 1000 loops in neural network, built in led toggle
  neuralLoopsCounter=neuralLoopsCounter+1;
  if (neuralLoopsCounter==100)//1000
  {
    digitalWrite(2,!digitalRead(2));
    neuralLoopsCounter=0;
  }
  //Serial.println(micros()-neuralTime);
}

void toTerminal()
{

  for( p = 0 ; p < PatternCount ; p++ ) { 
    Serial.println(); 
    Serial.print ("  Training Pattern: ");
    Serial.println (p);      
    Serial.print ("  Input ");
    for( i = 0 ; i < InputNodes ; i++ ) {
      Serial.print (Input[p][i], DEC);
      Serial.print (" ");
    }
    Serial.print ("  Target ");
    for( i = 0 ; i < OutputNodes ; i++ ) {
      Serial.print (Target[p][i], DEC);
      Serial.print (" ");
    }
/******************************************************************
* Compute hidden layer activations
******************************************************************/

    for( i = 0 ; i < HiddenNodes ; i++ ) {    
      Accum = HiddenWeights[InputNodes][i] ;
      for( j = 0 ; j < InputNodes ; j++ ) {
        Accum += Input[p][j] * HiddenWeights[j][i] ;
        //Serial.print (HiddenWeights[j][i],1);
        //Serial.print (" ");
      }
      Hidden[i] = 1.0/(1.0 + exp(-Accum)) ;
    }

/******************************************************************
* Compute output layer activations and calculate errors
******************************************************************/

    for( i = 0 ; i < OutputNodes ; i++ ) {    
      Accum = OutputWeights[HiddenNodes][i] ;
      for( j = 0 ; j < HiddenNodes ; j++ ) {
        Accum += Hidden[j] * OutputWeights[j][i] ;
        //Serial.print (OutputWeights[j][i],1);
        //Serial.print (" ");
      }
      Output[i] = 1.0/(1.0 + exp(-Accum)) ; 
    }
    Serial.print ("  Output ");
    for( i = 0 ; i < OutputNodes ; i++ ) {       
      Serial.print (Output[i], 5);
      Serial.print (" ");
    }
  }
}




void trainNeuralNetwork()
{
  randomSeed(analogRead(0)); //Collect a random ADC sample for Randomization.
  ReportEvery1000 = 1;
  for( p = 0 ; p < PatternCount ; p++ ) {    
    RandomizedIndex[p] = p ;
  }
/******************************************************************
* Initialize HiddenWeights and ChangeHiddenWeights 
******************************************************************/

  for( i = 0 ; i < HiddenNodes ; i++ ) {    
    for( j = 0 ; j <= InputNodes ; j++ ) { 
      yield();
      ChangeHiddenWeights[j][i] = 0.0 ;
      Rando = float(random(100))/100;
      HiddenWeights[j][i] = 2.0 * ( Rando - 0.5 ) * InitialWeightMax ;
    }
  }
/******************************************************************
* Initialize OutputWeights and ChangeOutputWeights
******************************************************************/

  for( i = 0 ; i < OutputNodes ; i ++ ) {    
    for( j = 0 ; j <= HiddenNodes ; j++ ) {
      yield();
      ChangeOutputWeights[j][i] = 0.0 ;  
      Rando = float(random(100))/100;        
      OutputWeights[j][i] = 2.0 * ( Rando - 0.5 ) * InitialWeightMax ;
    }
  }
  Serial.println("Initial/Untrained Outputs: ");
  toTerminal();
/******************************************************************
* Begin training 
******************************************************************/

  for( TrainingCycle = 1 ; TrainingCycle < 2147483647 ; TrainingCycle++) {    

/******************************************************************
* Randomize order of training patterns
******************************************************************/

    for( p = 0 ; p < PatternCount ; p++) {
      yield();
      q = random(PatternCount);
      r = RandomizedIndex[p] ; 
      RandomizedIndex[p] = RandomizedIndex[q] ; 
      RandomizedIndex[q] = r ;
    }
    Error = 0.0 ;
/******************************************************************
* Cycle through each training pattern in the randomized order
******************************************************************/
    for( q = 0 ; q < PatternCount ; q++ ) {    
      p = RandomizedIndex[q];

/******************************************************************
* Compute hidden layer activations
******************************************************************/

      for( i = 0 ; i < HiddenNodes ; i++ ) {    
        Accum = HiddenWeights[InputNodes][i] ;
        for( j = 0 ; j < InputNodes ; j++ ) {
          Accum += Input[p][j] * HiddenWeights[j][i] ;
        }
        Hidden[i] = 1.0/(1.0 + exp(-Accum)) ;
      }

/******************************************************************
* Compute output layer activations and calculate errors
******************************************************************/

      for( i = 0 ; i < OutputNodes ; i++ ) {    
        Accum = OutputWeights[HiddenNodes][i] ;
        for( j = 0 ; j < HiddenNodes ; j++ ) {
          Accum += Hidden[j] * OutputWeights[j][i] ;
        }
        Output[i] = 1.0/(1.0 + exp(-Accum)) ;   
        OutputDelta[i] = (Target[p][i] - Output[i]) * Output[i] * (1.0 - Output[i]) ;   
        Error += 0.5 * (Target[p][i] - Output[i]) * (Target[p][i] - Output[i]) ;
      }

/******************************************************************
* Backpropagate errors to hidden layer
******************************************************************/

      for( i = 0 ; i < HiddenNodes ; i++ ) {    
        Accum = 0.0 ;
        for( j = 0 ; j < OutputNodes ; j++ ) {
          Accum += OutputWeights[i][j] * OutputDelta[j] ;
        }
        HiddenDelta[i] = Accum * Hidden[i] * (1.0 - Hidden[i]) ;
      }


/******************************************************************
* Update Inner-->Hidden Weights
******************************************************************/


      for( i = 0 ; i < HiddenNodes ; i++ ) {     
        ChangeHiddenWeights[InputNodes][i] = LearningRate * HiddenDelta[i] + Momentum * ChangeHiddenWeights[InputNodes][i] ;
        HiddenWeights[InputNodes][i] += ChangeHiddenWeights[InputNodes][i] ;
        for( j = 0 ; j < InputNodes ; j++ ) { 
          ChangeHiddenWeights[j][i] = LearningRate * Input[p][j] * HiddenDelta[i] + Momentum * ChangeHiddenWeights[j][i];
          HiddenWeights[j][i] += ChangeHiddenWeights[j][i] ;
        }
      }

/******************************************************************
* Update Hidden-->Output Weights
******************************************************************/

      for( i = 0 ; i < OutputNodes ; i ++ ) {    
        ChangeOutputWeights[HiddenNodes][i] = LearningRate * OutputDelta[i] + Momentum * ChangeOutputWeights[HiddenNodes][i] ;
        OutputWeights[HiddenNodes][i] += ChangeOutputWeights[HiddenNodes][i] ;
        for( j = 0 ; j < HiddenNodes ; j++ ) {
          ChangeOutputWeights[j][i] = LearningRate * Hidden[j] * OutputDelta[i] + Momentum * ChangeOutputWeights[j][i] ;
          OutputWeights[j][i] += ChangeOutputWeights[j][i] ;
        }
      }
    }

/******************************************************************
* Every 1000 cycles send data to terminal for display
******************************************************************/
    ReportEvery1000 = ReportEvery1000 - 1;
    if (ReportEvery1000 == 0)
    {
      digitalWrite(16, !digitalRead(16));
      Serial.println(); 
      Serial.println(); 
      Serial.print(">TrainingCycle:");
      Serial.println(TrainingCycle);
      Serial.print (">Error:");
      Serial.println (Error, 5);
      yield();
      //digitalWrite(5,!digitalRead(5));
      //toTerminal();

      if (TrainingCycle==1)
      {
        ReportEvery1000 = 999;
      }
      else
      {
        ReportEvery1000 = 1000;
      }
    }    


/******************************************************************
* If error rate is less than pre-determined threshold then end
******************************************************************/

    if( Error < Success ) break ;  
  }
  Serial.println ();
  Serial.println(); 
  Serial.print ("TrainingCycle: ");
  Serial.print (TrainingCycle);
  Serial.print ("  Error = ");
  Serial.println (Error, 5);

  toTerminal();

  Serial.println ();  
  Serial.println ();
  Serial.println ("Training Set Solved! ");
  Serial.println ("--------"); 
  Serial.println ();
  Serial.println ();  
  ReportEvery1000 = 1;
  printWeights();
  Serial.println(" ");
  Serial.print("Temps d'entrenament: ");
  Serial.print(millis()/1000);
  Serial.println("segons");
  Serial.println(" ");
}


void setup()
{
  pinMode(16,OUTPUT);
  digitalWrite(16, HIGH);

  
  Serial.begin(115200);
  delay(1000);
  Serial.println(" ");
  Serial.println("Starting....");  

  trainNeuralNetwork();
  printWeightsAsCode();

}  

void loop ()
{
     //No cal res
}
