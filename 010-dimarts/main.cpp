// Exercici-003-inferencia Versio 25-06-2025
// Inferencia
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
#include "nn_params_simple.h" // Inclou els teus paràmetres de la NN simple generats per Python
#include <math.h> // Per a expf

// --- Definició dels LEDs ---
#define LED_LOW_PIN    0  // GPIO0 per a la classe "low" (lògica negativa)
#define LED_MEDIUM_PIN 2  // GPIO2 per a la classe "medium" (lògica negativa)
#define LED_HIGH_PIN   13 // GPIO13 per a la classe "high" (lògica positiva)

// --- Rang de l'ADC (ha de coincidir amb el Python) ---
#define ADC_RANGE_MIN 0
#define ADC_RANGE_MAX 1023

// --- Constants de rang per normalització (MOLT IMPORTANT: Copia AQUESTS valors del teu Python) ---
// Executa el script Python "classificador_llindars_nn_csv.py", COPIA la sortida del bloc:
// "--- Rangos de Caracteristicas para C/C++ (para la única característica 'adc_value') ---"
// i enganxa-la a continuació, reemplaçant els valors d'exemple.

const float FEATURE_MIN_adc_value = 0.000000f; // <--- REEMPLAÇA AMB ELS TEUS VALORS REALS
const float FEATURE_MAX_adc_value = 1023.000000f; // <--- REEMPLAÇA AMB ELS TEUS VALORS REALS

// --- Funcions Bàsiques de la Xarxa Neuronal en C/C++ ---

float relu(float x) {
    return (x > 0) ? x : 0.0f;
}

void softmax(float* input, int size) {
    float max_val = input[0];
    for (int i = 1; i < size; i++) {
        if (input[i] > max_val) {
            max_val = input[i];
        }
    }

    float sum_exp = 0.0f;
    for (int i = 0; i < size; i++) {
        input[i] = expf(input[i] - max_val); // expf per float, per estabilitat numèrica
        sum_exp += input[i];
    }

    for (int i = 0; i < size; i++) {
        input[i] /= sum_exp;
    }
}

// Multiplicació de matriu per vector (o vector per matriu)
void matrix_vector_multiply(const int8_t* W, float W_scale, const float* X, int input_dim, int output_dim, float* result) {
    for (int j = 0; j < output_dim; j++) {
        float sum = 0.0f;
        for (int i = 0; i < input_dim; i++) {
            sum += ((float)W[i * output_dim + j] * W_scale) * X[i];
        }
        result[j] = sum;
    }
}

void add_bias(float* input_vector, const int8_t* b, float b_scale, int size) {
    for (int i = 0; i < size; i++) {
        input_vector[i] += (float)b[i] * b_scale;
    }
}

// --- Funció de Predicció Principal de la Xarxa Neuronal ---
void predict_nn_simple(float adc_value_normalized, float* output_probabilities) {
    float features_normalized[INPUT_DIM_SIMPLE];
    features_normalized[0] = adc_value_normalized; // La nostra única característica

    float hidden_layer_output[HIDDEN_DIM_SIMPLE];
    matrix_vector_multiply(W1_simple, W1_simple_scale, features_normalized, INPUT_DIM_SIMPLE, HIDDEN_DIM_SIMPLE, hidden_layer_output);
    add_bias(hidden_layer_output, b1_simple, b1_simple_scale, HIDDEN_DIM_SIMPLE);

    for (int i = 0; i < HIDDEN_DIM_SIMPLE; i++) {
        hidden_layer_output[i] = relu(hidden_layer_output[i]);
    }

    float output_layer_raw[OUTPUT_DIM_SIMPLE];
    matrix_vector_multiply(W2_simple, W2_simple_scale, hidden_layer_output, HIDDEN_DIM_SIMPLE, OUTPUT_DIM_SIMPLE, output_layer_raw);
    add_bias(output_layer_raw, b2_simple, b2_simple_scale, OUTPUT_DIM_SIMPLE);

    softmax(output_layer_raw, OUTPUT_DIM_SIMPLE);

    for (int i = 0; i < OUTPUT_DIM_SIMPLE; i++) {
        output_probabilities[i] = output_layer_raw[i];
    }
}

// --- Normalització de la Lectura de l'ADC ---
float normalize_adc_value(float value) {
    if (FEATURE_MAX_adc_value == FEATURE_MIN_adc_value) {
        return 0.0f; 
    }
    return (value - FEATURE_MIN_adc_value) / (FEATURE_MAX_adc_value - FEATURE_MIN_adc_value);
}

// --- Funció per controlar els LEDs ---
void controlLeds(int predicted_class_idx) {
    // Apaguem tots els LEDs primer per assegurar-nos que només n'hi ha un encès
    digitalWrite(LED_LOW_PIN, HIGH);    // HIGH per apagar LED lògica negativa (GPIO0)
    digitalWrite(LED_MEDIUM_PIN, HIGH); // HIGH per apagar LED lògica negativa (GPIO2)
    digitalWrite(LED_HIGH_PIN, LOW);    // LOW per apagar LED lògica positiva (GPIO13)

    // Encenem el LED corresponent segons la classe predita
    if (predicted_class_idx == 0) { // Si és "low"
        digitalWrite(LED_LOW_PIN, LOW); // LOW per encendre LED lògica negativa
        Serial.println("  -> LOW LED ON");
    } else if (predicted_class_idx == 1) { // Si és "medium"
        digitalWrite(LED_MEDIUM_PIN, LOW); // LOW per encendre LED lògica negativa
        Serial.println("  -> MEDIUM LED ON");
    } else if (predicted_class_idx == 2) { // Si és "high"
        digitalWrite(LED_HIGH_PIN, HIGH); // HIGH per encendre LED lògica positiva
        Serial.println("  -> HIGH LED ON");
    }
}


// --- Configuració de PlatformIO i Loop Principal ---

void setup() {
    Serial.begin(115500); // Velocitat de baudis lleugerament ajustada per bones pràctiques (115200 és comú)
    delay(1000);
    Serial.println("Exercici-003");
    Serial.println("Sistema iniciat. Inferència cada 2s.");
    
    // Configuració dels pins dels LEDs com a sortides
    pinMode(LED_LOW_PIN, OUTPUT);
    pinMode(LED_MEDIUM_PIN, OUTPUT);
    pinMode(LED_HIGH_PIN, OUTPUT);

    // Inicialment, apaguem tots els LEDs
    digitalWrite(LED_LOW_PIN, HIGH);    // Lògica negativa: HIGH = OFF
    digitalWrite(LED_MEDIUM_PIN, HIGH); // Lògica negativa: HIGH = OFF
    digitalWrite(LED_HIGH_PIN, LOW);    // Lògica positiva: LOW = OFF
}

void loop() {
    // Llegir el valor analògic de A0 (ADC del ESP8266)
    int raw_adc_value = analogRead(A0);

    // Normalitzar el valor de l'ADC
    float normalized_adc_value = normalize_adc_value((float)raw_adc_value);

    // Array per guardar les probabilitats de sortida
    float output_probabilities[OUTPUT_DIM_SIMPLE]; 

    // Fer la predicció amb la xarxa neuronal
    predict_nn_simple(normalized_adc_value, output_probabilities);

    // Trobar la classe predita (la que té la probabilitat més alta)
    int predicted_class_idx = 0;
    float max_prob = -1.0f;
    for (int i = 0; i < OUTPUT_DIM_SIMPLE; i++) {
        if (output_probabilities[i] > max_prob) {
            max_prob = output_probabilities[i];
            predicted_class_idx = i;
        }
    }

    // --- Sortida per Serial ---
    Serial.print("#");
    Serial.print("raw_adc:  "); Serial.print(raw_adc_value);
    Serial.print(",  norm_adc:  "); Serial.print(normalized_adc_value, 4);
    
    for (int i = 0; i < OUTPUT_DIM_SIMPLE; i++) {
        Serial.print(",  ");
        Serial.print(CLASS_NAMES_SIMPLE[i]);
        Serial.print(":");
        Serial.print(output_probabilities[i], 4);
    }
    Serial.print(",  predicted_idx:"); Serial.print(predicted_class_idx);
    Serial.print(",  confidence:"); Serial.print(max_prob, 4);
    Serial.println();

    /*
    // --- Sortida per al Monitor Serial normal (per depuració) ---
    Serial.print("Raw ADC: "); Serial.print(raw_adc_value);
    Serial.print(", Norm ADC: "); Serial.print(normalized_adc_value, 4);
    Serial.print(", Probabilities: [");
    for (int i = 0; i < OUTPUT_DIM_SIMPLE; i++) {
        Serial.print(CLASS_NAMES_SIMPLE[i]);
        Serial.print(":");
        Serial.print(output_probabilities[i], 4); 
        if (i < OUTPUT_DIM_SIMPLE - 1) Serial.print(", ");
    }
    Serial.println("]");
    Serial.printf("Predicted Class: %s (Confidence: %.2f%%)\n", CLASS_NAMES_SIMPLE[predicted_class_idx], max_prob * 100.0f);
    */

    // Controlar els LEDs segons la predicció
    controlLeds(predicted_class_idx);
    
    delay(2000); // Llegeix el sensor i fa la predicció cada 0.5 segons
}