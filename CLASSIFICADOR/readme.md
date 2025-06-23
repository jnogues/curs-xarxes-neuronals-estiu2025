# El Classificador Intel·ligent de Nivells amb ESP8266

---

Aquest projecte t'introduirà al fascinant món de la **Intel·ligència Artificial (IA)** i el **Machine Learning (ML)** aplicat a petits dispositius com l'**ESP8266**. Aprendrem com fer que el nostre microcontrolador "pensi" i prengui decisions basades en dades, en lloc de seguir regles rígides programades a mà.

## 🎯 Objectiu del Projecte

Volem construir un sistema capaç de classificar automàticament el nivell d'un senyal d'entrada (simulat amb un potenciòmetre o fotorresistència) en tres categories: **"Baix" (Low)**, **"Mitjà" (Medium)** i **"Alt" (High)**. La màgia? El nostre ESP8266 aprendrà a fer aquesta classificació utilitzant una **xarxa neuronal**, no amb `if/else` tradicionals!

## ⏱️ Durada Estimada

2-3 sessions (depenent del ritme i el nivell de detall en la discussió de conceptes).

## 💡 Conceptes Clau que Aprendràs

* **Intel·ligència Artificial (IA) / Machine Learning (ML):** Què són i com una màquina pot "aprendre".
* **Xarxes Neuronals:** El "cervell" del nostre sistema.
* **Dataset:** El conjunt de dades amb què la nostra xarxa neuronal aprendrà.
* **Entrenament:** El procés on la xarxa neuronal aprèn del dataset.
* **Inferència / Predicció:** Com la xarxa neuronal fa servir el que ha après per prendre decisions noves.
* **Normalització de Dades:** Per què cal preparar les dades per a la xarxa neuronal.
* **PlatformIO:** L'entorn de desenvolupament per als nostres microcontroladors.

## 📦 Materials Necessaris

* Targeta de desenvolupament **ESP8266** (NodeMCU, Wemos D1 Mini o similar).
* **Potenciòmetre** o **fotorresistència (LDR)**.
* **3 LEDs** (idealment de colors diferents, per exemple, vermell, verd, blau).
* **3 Resistències** per als LEDs (entre 220 Ohm i 330 Ohm).
* **Protoboard** (Breadboard).
* **Cables jumper** (mascle-mascle).
* Ordinador amb **Visual Studio Code** i l'extensió **PlatformIO** instal·lada.
* **Python 3** instal·lat a l'ordinador.

---

## ⚙️ Pas 0: Preparació de l'Entorn i Introducció

1.  **Configura VS Code i PlatformIO:** Assegura't que tens Visual Studio Code i l'extensió PlatformIO correctament instal·lats i funcionals.
2.  **Instal·la les Llibreries de Python:** Obre el terminal del teu ordinador i executa la següent comanda per instal·lar les llibreries necessàries:
    ```bash
    pip install numpy matplotlib pandas
    ```
    (Si tens `python` i `python3` instal·lats, pot ser que hagis d'usar `pip3 install ...`.)

---

## 📊 Pas 1: El Dataset - Les Dades d'Aprenentatge del Nostre "Cervell"

Perquè la nostra xarxa neuronal aprengui a classificar els nivells, necessita exemples. Aquests exemples els guardarem en un fitxer `.csv` (Comma Separated Values).

### `lectures_sensor.csv`

Crea un fitxer anomenat `lectures_sensor.csv` a la mateixa carpeta on tindràs el teu script Python. Aquest fitxer tindrà dues columnes: `adc_value` (la lectura del sensor) i `class_label` (a quina categoria pertany aquesta lectura).

**Exemple de contingut per a `lectures_sensor.csv`:**

```csv
adc_value,class_label
50,low
120,low
280,low
350,medium
480,medium
690,medium
750,high
890,high
1000,high
290,low
710,high

## ⚡ Pas 3: El nostre "Cervell" a l'ESP8266 (C++ a PlatformIO)

Ara que tenim el "cervell" (els pesos de la xarxa neuronal) en un format C++, el pujarem al nostre ESP8266.

### 🔌 Connexions Físiques

Connecta els components al teu ESP8266 de la següent manera:

* **Potenciòmetre/LDR:**
    * Un extrem a **GND**.
    * L'altre extrem a **3.3V**.
    * El pin central (o el pin de sortida de l'LDR amb la resistència) al pin **A0** de l'ESP8266.

* **LED per a "LOW" (lògica negativa):**
    * Càtode (pata curta) del LED al pin **GPIO0**.
    * Ànode (pata llarga) del LED a una resistència de 220-330 Ohm.
    * L'altre extrem de la resistència a **3.3V**.
    *(Recorda que GPIO0 pot tenir un comportament especial en l'arrencada de l'ESP8266, però per a un LED de sortida normalment funciona bé una vegada ha arrencat.)*

* **LED per a "MEDIUM" (lògica negativa):**
    * Càtode (pata curta) del LED al pin **GPIO2**.
    * Ànode (pata llarga) del LED a una resistència de 220-330 Ohm.
    * L'altre extrem de la resistència a **3.3V**.

* **LED per a "HIGH" (lògica positiva):**
    * Ànode (pata llarga) del LED al pin **GPIO13**.
    * Càtode (pata curta) del LED a una resistència de 220-330 Ohm.
    * L'altre extrem de la resistència a **GND**.

### `src/main.cpp`

Aquest és el codi que s'executarà al teu ESP8266. Substitueix el contingut del teu `src/main.cpp` al projecte PlatformIO per aquest:

```cpp
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
// y engancha-la a continuació, reemplaçant els valors d'exemple.

const float FEATURE_MIN_adc_value = 0.000000f; // <--- REEMPLAÇA AMB ELS TEUS VALORS REALS DEL PYTHON
const float FEATURE_MAX_adc_value = 1023.000000f; // <--- REEMPLAÇA AMB ELS TEUS VALORS REALS DEL PYTHON

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
        input[i] = expf(input[i] - max_val);
        sum_exp += input[i];
    }

    for (int i = 0; i < size; i++) {
        input[i] /= sum_exp;
    }
}

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
    features_normalized[0] = adc_value_normalized;

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
    if (predicted_class_idx == 0) { // Si és "low" (correspon a l'índex 0 de CLASS_NAMES_SIMPLE)
        digitalWrite(LED_LOW_PIN, LOW); // LOW per encendre LED lògica negativa
        Serial.println("  -> LOW LED ON");
    } else if (predicted_class_idx == 1) { // Si és "medium" (correspon a l'índex 1)
        digitalWrite(LED_MEDIUM_PIN, LOW); // LOW per encendre LED lògica negativa
        Serial.println("  -> MEDIUM LED ON");
    } else if (predicted_class_idx == 2) { // Si és "high" (correspon a l'índex 2)
        digitalWrite(LED_HIGH_PIN, HIGH); // HIGH per encendre LED lògica positiva
        Serial.println("  -> HIGH LED ON");
    }
}


// --- Configuració de PlatformIO i Loop Principal ---

void setup() {
    Serial.begin(115200); // Velocitat de baudis per comunicació serial
    while (!Serial && !Serial.availableForWrite()) {
        delay(10);
    }
    Serial.println("\n--- ESP8266 Simple NN Classifier Ready! ---");
    Serial.println("Reading A0, predicting class, and controlling LEDs...");

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
    int raw_adc_value = analogRead(A0);
    float normalized_adc_value = normalize_adc_value((float)raw_adc_value);

    float output_probabilities[OUTPUT_DIM_SIMPLE]; 
    predict_nn_simple(normalized_adc_value, output_probabilities);

    int predicted_class_idx = 0;
    float max_prob = -1.0f;
    for (int i = 0; i < OUTPUT_DIM_SIMPLE; i++) {
        if (output_probabilities[i] > max_prob) {
            max_prob = output_probabilities[i];
            predicted_class_idx = i;
        }
    }

    // --- Sortida per Teleplot ---
    Serial.print("#");
    Serial.print("raw_adc:"); Serial.print(raw_adc_value);
    Serial.print(",norm_adc:"); Serial.print(normalized_adc_value, 4);
    
    for (int i = 0; i < OUTPUT_DIM_SIMPLE; i++) {
        Serial.print(",");
        Serial.print(CLASS_NAMES_SIMPLE[i]);
        Serial.print(":");
        Serial.print(output_probabilities[i], 4);
    }
    Serial.print(",predicted_idx:"); Serial.print(predicted_class_idx);
    Serial.print(",confidence:"); Serial.print(max_prob, 4);
    Serial.println();

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
    
    // Controlar els LEDs segons la predicció
    controlLeds(predicted_class_idx);
    
    delay(500);
}

**Instruccions per als alumnes:**

1.  Crea un nou projecte PlatformIO per a l'ESP8266 (si no el tens ja).
2.  Copia el fitxer `nn_params_simple.h` generat pel script Python des de la carpeta del Python a la carpeta `include` del teu projecte PlatformIO.
3.  Obre el fitxer `src/main.cpp` del teu projecte PlatformIO i enganxa el codi anterior.
4.  **Molt important:** Localitza la secció `--- Constants de rang per normalització ---` al `main.cpp` i **substitueix els valors d'exemple** de `FEATURE_MIN_adc_value` i `FEATURE_MAX_adc_value` amb els valors que vas copiar del terminal de Python al pas 2.
5.  A la barra inferior de VS Code, fes clic a la icona de **Build** (l'engranatge) per compilar el projecte. Si no hi ha errors, el codi està llest per pujar.
6.  Connecta el teu ESP8266 a l'ordinador.
7.  Fes clic a la icona de **Upload** (la fletxa cap amunt) per pujar el codi al teu ESP8266.

---

## ✨ Pas 4: Prova el teu Classificador Intel·ligent!

1.  Un cop el codi s'hagi pujat correctament, **tanca el monitor serial de PlatformIO** si el tens obert (és important per a Teleplot).
2.  Obre **Teleplot** al teu navegador web (o l'aplicació d'escriptori).
3.  A Teleplot, selecciona el **port serial correcte** (el del teu ESP8266) i la **velocitat de baudis (115200)**.
4.  Clica "Connect".
5.  **Ara, varia el valor d'entrada analògica** del teu potenciòmetre o LDR. Observa com:
    * Les lectures de l'ADC canvien al gràfic.
    * Les probabilitats de les classes "low", "medium", "high" es modifiquen.
    * **El LED corresponent a la classe predita s'encén!**

**Enhorabona!** Has fet que el teu ESP8266 aprengui a classificar nivells utilitzant una xarxa neuronal. Això és IA en un dispositiu petit!

---

## 🧐 Per Anar Més Enllà (Opcional)

* **Juga amb el `TARGET_LOSS`:** Prova a posar un valor més baix (p. ex., `0.005`) al `classificador_llindars_nn_csv.py` i veu si la xarxa s'entrena més temps i la precisió millora. Què passa si el poses massa baix?
* **Canvia `HIDDEN_DIM`:** Modifica el número de neurones a la capa oculta (`HIDDEN_DIM`) al Python (p. ex., a 3 o a 10). Com afecta a l'entrenament i als resultats?
* **Crea un Dataset Propio:** Si tens temps, recopila les teves pròpies dades amb el sensor real i actualitza el `lectures_sensor.csv`. Aquest és el pas més realista en un projecte de ML!
* **Altres Sensors:** Quins altres sensors podríem classificar amb una xarxa neuronal? (Temperatura, humitat, so, etc.)

