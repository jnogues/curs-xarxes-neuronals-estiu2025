**🧠 Exercici-002: Classificació de Senyals Analògics amb Xarxes Neuronals Quantitzades a l'ESP8266 🎯**
---

Hola a tothom! En aquest exercici, farem un pas més enllà i crearem un sistema intel·ligent que pot classificar automàticament un senyal analògic (com el d'un potenciòmetre o fotorresistència) en tres categories: "Baix" (Low), "Mitjà" (Medium) i "Alt" (High). La màgia recau en l'ús d'una **xarxa neuronal artificial (ANN)** entrenada, evitant la lògica tradicional de `if/else`. A més, introduirem la **quantització de pesos** per optimitzar el rendiment en el microcontrolador!

---
### **Objectiu General de l'Exercici**

L'**Exercici-002** té com a objectiu principal:

* Entrenar una xarxa neuronal per a una tasca de classificació multiclasse (Low/Medium/High).
* Comprendre el procés de **normalització de dades** per a entrades analògiques.
* Utilitzar una eina d'entrenament en Python (`classificador_llindars_nn_csv.py`) que exporta els pesos de la xarxa en format **quantitzat (int8_t)** per a la seva eficient implementació en microcontroladors.
* Implementar la fase d'inferència a l'**ESP8266** utilitzant aquests pesos quantitzats i factors d'escala per realitzar prediccions en temps real.
* Simular l'entrada analògica amb un potenciòmetre o fotorresistència connectat al pin ADC de l'ESP8266.

---
### **El Dataset de Classificació**

Utilitzarem el següent *dataset* per entrenar la nostra xarxa, que mapeja valors d'ADC a categories:

| adc_value | class_label |
|-----------|-------------|
| 0         | low         |
| 10        | low         |
| 50        | low         |
| 110       | low         |
| 120       | low         |
| 280       | low         |
| 290       | low         |
| 310       | medium      |
| 350       | medium      |
| 420       | medium      |
| 480       | medium      |
| 500       | medium      |
| 690       | medium      |
| 710       | high        |
| 750       | high        |
| 890       | high        |
| 950       | high        |
| 1000      | high        |
| 1023      | high        |


Aquestes dades simulen les lectures d'un sensor (per exemple, un potenciòmetre o una fotorresistència) i la seva classificació manual per crear un conjunt d'entrenament.

---
### **Fase 1: Entrenament de la Xarxa Neuronal (Amb Python i Exportació Quantitzada)**

Per a l'entrenament, utilitzarem un script de Python que ens permetrà obtenir un model entrenat de manera eficient i, el més important, exportarà els pesos en un format optimitzat per a microcontroladors.

**Fitxer clau per a l'Entrenament:**

1.  **`classificador_llindars_nn_csv.py`:**
    * Aquest script Python (requereix `numpy` i `pandas`).
    * Llegeix el *dataset* de classificació des d'un fitxer CSV (per defecte `lectures_sensor.csv`, que contindria les dades de dalt).
    * Normalitza l'entrada ADC (rang 0-1023).
    * Defineix l'arquitectura de la xarxa: **1 neurona d'entrada, 5 neurones a la capa oculta, i 3 neurones de sortida** (una per a cada classe "low", "medium", "high", utilitzant *one-hot encoding*).
    * Utilitza la funció d'activació ReLU a la capa oculta i Softmax a la de sortida.
    * Un cop entrenada, la part més rellevant és que **quantitza els pesos a tipus `int8_t` i els exporta** a un fitxer de capçalera C anomenat `nn_params_simple.h`. Aquest fitxer també contindrà les escales (`float`) necessàries per desquantitzar els valors durant la inferència.

**Com procedir amb l'Entrenament (amb Python):**

1.  Crea un fitxer `lectures_sensor.csv` amb el *dataset* proporcionat.
2.  Assegura't de tenir Python, `numpy`, i `pandas` instal·lats (`pip install numpy pandas`).
3.  Executa el script: `python classificador_llindars_nn_csv.py`.
4.  El script mostrarà el procés d'entrenament i, al final, generarà el fitxer `nn_params_simple.h` a la mateixa carpeta. **Aquest fitxer serà essencial per a la fase d'inferència a l'ESP8266.**

---
### **Fase 2: Inferència a l'ESP8266 (Utilitzant Pesos Quantitzats)**

Un cop `nn_params_simple.h` ha estat generat, podem carregar aquests pesos optimitzats al nostre ESP8266 per realitzar la classificació en temps real.

**Fitxers clau per a la Inferència:**

1.  **`nn_params_simple.h`:**
    * Aquest fitxer, generat per l'script de Python, conté les matrius de pesos i biaixos (`W1_simple`, `b1_simple`, `W2_simple`, `b2_simple`) en format `int8_t`, juntament amb els seus respectius factors d'escala en `float`. També defineix les dimensions de la xarxa (`INPUT_DIM_SIMPLE`, `HIDDEN_DIM_SIMPLE`, `OUTPUT_DIM_SIMPLE`) i els noms de les classes.
2.  **Codi C++ per a l'ESP8266 (Sketch de Inferència):** (Aquest codi haurà de ser creat per l'usuari, utilitzant el `nn_params_simple.h` generat)
    * Aquest *sketch* haurà d'incloure `nn_params_simple.h`.
    * Llegirà el valor analògic des del pin ADC (e.g., `A0`) de l'ESP8266 (rang 0-1023).
    * **Normalitzarà** aquest valor d'entrada utilitzant els rangs definits (0-1023), de la mateixa manera que es fa en Python.
    * Realitzarà el càlcul de la xarxa neuronal utilitzant els pesos `int8_t` i les escales `float` de `nn_params_simple.h`. Això implica desquantitzar els pesos abans o durant el càlcul per realitzar les multiplicacions i sumes, i després escalar els resultats intermedis i finals.
    * Interpretarà les tres sortides de la xarxa (`[1,0,0]`, `[0,1,0]`, `[0,0,1]`) per determinar la classe predita ("low", "medium" o "high").
    * Podrà fer alguna acció física (com encendre LEDs o imprimir al Serial Monitor) per indicar la classe detectada.
    * És recomanable utilitzar una llibreria de planificació de tasques com `Tasker` (present en el `platformio.ini`) per a una execució no bloquejant de la inferència.

**Com procedir amb la Inferència a l'ESP8266:**

1.  Crea un nou projecte PlatformIO per a la placa `nodemcuv2`.
2.  Copia el fitxer `nn_params_simple.h` generat per l'script de Python a la carpeta `src/` del teu projecte PlatformIO.
3.  Crea un fitxer `main.cpp` a la carpeta `src/` i escriu el codi per a la inferència:
    * Inclou `<Arduino.h>`, `<Tasker.h>` i `nn_params_simple.h`.
    * Configura el pin ADC per a la lectura del sensor.
    * Defineix els pins de sortida per indicar la classificació.
    * Implementa la funció de *forward pass* de la xarxa neuronal utilitzant els pesos i escales de `nn_params_simple.h`. Recorda que els càlculs hauran de tenir en compte la quantització.
    * Configura una tasca amb `Tasker` que llegeixi el sensor, executi la inferència i actualitzi les sortides.
    * El `platformio.ini` ja inclou la dependència per `Tasker`.
4.  Compila i puja el codi a l'ESP8266.
5.  Conecta un potenciòmetre o fotorresistència al pin ADC (A0) de l'ESP8266 i observa com la xarxa classifica el seu nivell en temps real!

---
**Punts Clau d'aquest Exercici:**

* **Aplicació real:** Classificació de dades de sensors amb ANN.
* **Eficiència:** Ús de pesos quantitzats (`int8_t`) per optimitzar l'ús de memòria i la velocitat de càlcul en el microcontrolador.
* **Cicle complet:** Des de la preparació del *dataset* i l'entrenament en Python fins a la implementació eficient en el dispositiu *edge*.
* **Arquitectura:** 1 entrada (ADC normalitzat), 5 ocultes, 3 sortides (classes Low/Medium/High).

---
Estem preparats per portar les nostres xarxes neuronals un pas més enllà en el món de l'IoT! Qualsevol dubte o problema amb la implementació, ja sabeu on trobar-me! 🚀

