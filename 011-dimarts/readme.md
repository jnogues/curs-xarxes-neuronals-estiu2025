**🌡️ Exercici-003: Termòstat Intel·ligent amb Xarxa Neuronal 1-5-3-1 a l'ESP8266 🧠**
---

Hola a tothom! En aquest exercici implementarem un sistema de **control de temperatura** utilitzant el poder de les **Xarxes Neuronals Artificials (ANN)**. En lloc d'un controlador PID clàssic o lògica de llindars, la nostra ESP8266 aprendrà a mantenir una temperatura constant (un *setpoint* de 50°C) ajustant un escalfador mitjançant PWM. Aquest exercici ens introduirà a les xarxes amb **dues capes ocultes** i la seva aplicació en sistemes de control.

---
### **Objectiu General de l'Exercici**

L'**Exercici-003** té com a finalitat principal:

* Comprendre i aplicar el concepte d'un **controlador predictiu basat en Xarxes Neuronals** per a una consigna fixa (50°C).
* Dissenyar i entrenar una xarxa neuronal amb una arquitectura de **1 neurona d'entrada, 5 a la primera capa oculta, 3 a la segona capa oculta i 1 de sortida (1-5-3-1)**.
* Realitzar l'entrenament de la xarxa amb un *dataset* de comportament desitjat utilitzant Python.
* Implementar la **fase d'inferència a l'ESP8266**, on la xarxa, amb els pesos entrenats, llegirà la temperatura real i calcularà el valor PWM a aplicar a un element calefactor.
* Utilitzar el sensor de temperatura digital DS18B20 per a les lectures.
* Controlar una sortida PWM per a la resistència calefactora, simulant un sistema de calefacció.

---
### **La Relació Temperatura-PWM a Aprendre (Dataset)**

La xarxa neuronal aprendrà a mapejar una temperatura d'entrada a un valor PWM de sortida. El *dataset* que utilitzarem per a l'entrenament defineix aquest comportament desitjat al voltant del *setpoint* de 50°C:

| temperature | pwm    |
|-------------|--------|
| 47          | 1023   |
| 47.1        | 950    |
| 47.4        | 850    |
| 47.7        | 800    |
| 48          | 800    |
| 48.2        | 750    |
| 48.4        | 750    |
| 48.6        | 700    |
| 48.8        | 700    |
| 49          | 650    |
| 49.1        | 600    |
| 49.2        | 550    |
| 49.3        | 500    |
| 49.4        | 475    |
| 49.5        | 450    |
| 49.6        | 425    |
| 49.7        | 400    |
| 49.8        | 375    |
| 49.9        | 350    |
| 50          | 320    |
| 50.1        | 260    |
| 50.2        | 150    |
| 50.3        | 50     |
| 50.5        | 10     |

Com podeu veure, a temperatures baixes el PWM és alt (escalfa més), i a temperatures altes (per sobre de 50°C), el PWM disminueix dràsticament.

---
### **Fase 1: Entrenament de la Xarxa Neuronal (Generació dels Pesos)**

L'objectiu d'aquesta fase és generar els pesos i biaixos òptims per a la nostra xarxa 1-5-3-1, que posteriorment incrustarem en el codi de l'ESP8266.

**Fitxers clau per a l'Entrenament:**

1.  **`dataset50C.csv`:**
    * Aquest fitxer CSV contindrà el *dataset* de temperatura i PWM en format de text pla.
2.  **`normalize_csv_to_train_nn.py`:**
    * Aquest script Python és el **primer pas de l'entrenament**.
    * Llegeix el `dataset50C.csv` i **normalitza** els valors de `temperature` i `pwm` al rang [0.0, 1.0]. Aquesta normalització és crucial perquè les xarxes neuronals treballin de forma òptima.
    * Imprimeix a la consola el *dataset* normalitzat en un format de `numpy.array` que es pot copiar i enganxar directament al següent script d'entrenament. També indica els valors mínims i màxims originals utilitzats per a la normalització (per exemple, per a la temperatura: `min = 47.0, max = 50.5`).
3.  **`train_nn_50C.py`:**
    * Aquest és el **cor de l'entrenament**.
    * Conté la implementació d'una xarxa neuronal amb l'arquitectura **1-5-3-1** (amb dues capes ocultes).
    * Inclou les funcions d'activació **sigmoide** i la seva derivada, utilitzades per la propagació cap endavant i la retropropagació.
    * Implementa l'algorisme de **backpropagation** per ajustar iterativament els pesos i biaixos de la xarxa per minimitzar l'error entre les sortides predites i les sortides desitjades del *dataset*.
    * Un cop l'entrenament ha assolit un error prou baix (`minimal_error = 0.01`), l'script **imprimeix els pesos i biaixos optimitzats en un format de constants `float` de C/C++**, llests per ser copiats al *sketch* de l'ESP8266.

**Com procedir amb l'Entrenament (amb Python):**

1.  **Crea el fitxer `dataset50C.csv`** amb les dades de la taula superior (`temperature,pwm`).
2.  **Executa `normalize_csv_to_train_nn.py`**:
    ```bash
    python normalize_csv_to_train_nn.py
    ```
    Copia la sortida (el bloc de `training_data = np.array([...])`) a la secció corresponent de `train_nn_50C.py`.
3.  **Executa `train_nn_50C.py`**:
    ```bash
    python train_nn_50C.py
    ```
    El script entrenarà la xarxa. Quan l'entrenament finalitzi, copiaràs els blocs de codi C++ amb els pesos i biaixos (ex: `float weights_input_hidden1[]`, `float bias_hidden1[]`, etc.).

---
### **Fase 2: Inferència a l'ESP8266 (Control de Temperatura en Temps Real)**

Amb els pesos i biaixos entrenats, la nostra ESP8266 pot utilitzar la xarxa neuronal per controlar la temperatura de manera autònoma.

**Fitxers clau per a la Inferència:**

1.  **`platformio.ini` (Configuració del Projecte):**
    * Defineix l'entorn de construcció per a la placa `nodemcuv2` amb el *framework* `arduino`.
    * Inclou les llibreries `Tasker` (per a multitasca), `OneWire` i `DallasTemperature` (per al sensor DS18B20).
    * Configura la velocitat del monitor sèrie a 115200 bauds.
2.  **`main.cpp` (Sketch de Inferència a l'ESP8266):**
    * Aquest és el codi que s'executarà a la placa.
    * **Conté els pesos i biaixos de la xarxa 1-5-3-1 incrustats directament com a constants `float`**. Aquests són els valors que has obtingut de `train_nn_50C.py`.
    * Configura el sensor DS18B20 (connectat al pin D2/GPIO4) i el pin PWM (D7/GPIO13).
    * Implementa la funció de **normalització** d'entrada (`normalitza()`) i la funció **sigmoide** (`sigmoid()`), que són coherents amb les utilitzades durant l'entrenament.
    * La funció `forward_propagation()` realitza els càlculs de la xarxa neuronal amb les dues capes ocultes i retorna la sortida.
    * La tasca `llegeixTemperatura()` (gestionada per `Tasker` cada 800ms):
        * Llegeix la temperatura del DS18B20.
        * Normalitza la temperatura al rang [0.0, 1.0] utilitzant els límits 47.0°C i 50.5°C.
        * Executa la `forward_propagation` per obtenir el valor de PWM desitjat.
        * Converteix la sortida normalitzada del PWM (0.0-1.0) al rang 0-1023 i l'aplica al `PWM_PIN`.
        * Envia telemetria per port sèrie (temperatura, PWM aplicat).

**Com procedir amb la Inferència a l'ESP8266:**

1.  **Crea un nou projecte PlatformIO** per a la placa `nodemcuv2`.
2.  Copia el contingut del `platformio.ini` proporcionat al fitxer `platformio.ini` del teu projecte.
3.  Copia el contingut del `main.cpp` proporcionat al `src/main.cpp` del teu projecte.
4.  **Enganxa els pesos i biaixos** que has obtingut de `train_nn_50C.py` als llocs corresponents dins del `main.cpp`.
5.  **Connecta** un sensor DS18B20 (amb la seva resistència *pull-up*) al pin D2 (GPIO4) de l'ESP8266.
6.  **Connecta** una resistència calefactora (amb un transistor o mòdul relé/MOSFET, si cal) al pin D7 (GPIO13). (Per a proves, es pot connectar un LED amb la seva resistència per visualitzar el PWM).
7.  Compila i puja el codi a l'ESP8266.
8.  Obre el Serial Monitor de PlatformIO (a 115200 bauds) per observar la temperatura, la sortida de la xarxa i el PWM aplicat. Observa com el sistema intenta mantenir la temperatura prop dels 50°C.

---
**Punts Clau de l'Exercici-003:**

* **Control basat en ANN:** Un enfocament intel·ligent per a tasques de control, aprenent un comportament complex a partir de dades.
* **Xarxa de dues capes ocultes (1-5-3-1):** Una arquitectura més potent per a problemes no lineals.
* **Cicle complet:** Des de la generació de dades i l'entrenament Python fins a la implementació en temps real a l'ESP8266.
* **Importància de la normalització:** Crucial per a l'entrenament i la inferència.

---
Aquest exercici és una gran demostració del potencial de les xarxes neuronals en sistemes encastats per a aplicacions de control! Qualsevol dubte o problema en qualsevol fase, pregunteu! 🚀