**🧠 Exercici-001: Resolució de Taula de la Veritat amb Xarxes Neuronals (Fases d'Entrenament i Inferència) 🎯**
---

Hola a tothom! Aquest exercici ens submergirà completament en l'ús de les **Xarxes Neuronals Artificials (ANN)** per resoldre una taula de la veritat, des de l'entrenament fins a la seva execució en un microcontrolador. L'objectiu és que una xarxa neuronal simple aprengui a predir dues sortides basant-se en quatre entrades.

---
### **Objectiu General de l'Exercici**

L'**Exercici-001** té com a finalitat principal:

* Entendre el cicle complet de vida d'una xarxa neuronal: **entrenament** i **inferència**.
* Dissenyar i implementar una xarxa neuronal MLP (Multi-Layer Perceptron) amb arquitectura de **4 neurones d'entrada, 8 a la capa oculta i 2 a la de sortida**.
* Realitzar l'entrenament de la xarxa per resoldre una taula de la veritat predefinida.
* Incrustar els pesos entrenats en un *sketch* per a ESP8266 i realitzar la **inferència en temps real** al microcontrolador, llegint entrades digitals i controlant sortides digitals.
* Utilitzar la llibreria `Tasker` per gestionar la multitasca cooperativa durant la inferència, assegurant un funcionament no bloquejant.

---
### **La Taula de la Veritat a Resoldre**

La xarxa haurà d'aprendre la següent relació entre les entrades (`IN0-IN3`) i les sortides (`OUT0-OUT1`):


IN0,IN1,IN2,IN3,OUT0,OUT1
0,0,0,0,0,0
0,0,0,1,1,1
0,0,1,0,1,0
0,0,1,1,0,1
0,1,0,0,1,1
0,1,0,1,0,1
0,1,1,0,0,1
0,1,1,1,1,1
1,0,0,0,1,0
1,0,0,1,0,1
1,0,1,0,0,0
1,0,1,1,1,1
1,1,0,0,0,1
1,1,0,1,1,1
1,1,1,0,1,1
1,1,1,1,0,1

---


---
### **Fase 1: Entrenament de la Xarxa Neuronal (Obtenció dels Pesos)**

L'objectiu d'aquesta fase és que la xarxa aprengui a reproduir la taula de la veritat ajustant els seus pesos.

**Fitxers clau per a l'Entrenament:**

1.  **`mainEntrenament.cpp` (Entrenament a l'ESP8266):**
    * Codi C++ que implementa l'algorisme de *backpropagation* per entrenar la xarxa directament a l'ESP8266 (NodeMCU v2).
    * Inclou les dades d'entrada i sortida de la taula de la veritat.
    * Durant l'entrenament, s'imprimeixen l'error i els cicles per port sèrie.
    * **Molt important:** Un cop l'entrenament finalitza amb èxit (`Error < Success`), aquest codi imprimeix els pesos finals de la xarxa en format de constants C++ per a les matrius `HiddenWeights` i `OutputWeights`. Aquests pesos són el "coneixement" que ha adquirit la xarxa.

    ```cpp
    // Exemple del format de sortida dels pesos
    const float HiddenWeights[][HiddenNodes] = {
      // ... valors calculats ...
      {2.143316, -2.548144, ..., 1.208608}, // Biaixos
    };

    const float OutputWeights[][OutputNodes] = {
      // ... valors calculats ...
      {0.678717, -1.413864}, // Biaixos
    };
    ```

2.  **`entrenadorPython.py` (Entrenament alternatiu amb Python):**
    * Un script de Python que realitza el mateix procés d'entrenament de la xarxa neuronal.
    * És una alternativa més ràpida i còmoda per obtenir els pesos si l'entrenament a l'ESP8266 és massa lent.
    * També genera els pesos en un format similar al C++ al final de l'entrenament.

**Com procedir amb l'Entrenament:**

1.  **A l'ESP8266 (Recomanat per entendre el procés a fons):**
    * Crea un projecte PlatformIO nou per a `nodemcuv2` (ESP8266).
    * Copia el contingut del `mainEntrenament.cpp` al `src/main.cpp` del projecte.
    * Compila i puja el codi a la teva placa.
    * Obre el Serial Monitor de VSC (a 115200 bauds). Observa com l'error disminueix a cada cicle. Quan l'entrenament finalitzi, **copia els blocs de `const float HiddenWeights[][]` i `const float OutputWeights[][]`** que apareixen per port sèrie. Aquests són els teus pesos entrenats!
2.  **Amb Python (Alternatiu):**
    * Executa `python entrenadorPython.py` al teu terminal (assegura't de tenir `numpy` instal·lat: `pip install numpy`).
    * El script entrenarà la xarxa i imprimirà els pesos finals en la consola. Copia'ls de la mateixa manera.

---
### **Fase 2: Inferència amb els Pesos Entrenats (Execució al Microcontrolador)**

Un cop tenim els pesos, ja no cal re-entrenar la xarxa. Podem carregar aquests pesos a un nou *sketch* i fer que la nostra ESP8266 "predigui" les sortides per a noves entrades.

**Fitxer clau per a la Inferència:**

1.  **`mainInferencia.cpp`:**
    * Aquest codi C++ està dissenyat per a l'ESP8266 (NodeMCU v2).
    * **Conté espais designats** on hauràs de **pegar els pesos** que has obtingut en la fase d'entrenament (`HiddenWeights` i `OutputWeights`).
    * Defineix pins GPIO específics per a les entrades (e.g., `inputPin0` a `inputPin3`) i les sortides (`outputPin0`, `outputPin1`).
    * Utilitza la llibreria `Tasker` per programar una tasca `inferencia()` que s'executa regularment (cada 100 ms).
    * La tasca `inferencia()` llegeix l'estat dels pins d'entrada, passa aquestes dades per la xarxa neuronal utilitzant els pesos incrustats, i escriu les prediccions als pins de sortida corresponents (amb la lògica necessària).
    * També envia l'estat de les entrades i les sortides predites pel port sèrie, permetent la monitorització.

**Com procedir amb la Inferència:**

1.  **Prepara el codi d'inferència:**
    * Crea un nou projecte PlatformIO per a `nodemcuv2`.
    * Copia el contingut del `mainInferencia.cpp` al `src/main.cpp` del projecte.
    * **Enganxa els pesos** que vas obtenir en la fase d'entrenament (ja sigui de l'ESP8266 o de Python) als llocs indicats dins de `mainInferencia.cpp`.
2.  **Conecta i Prova:**
    * Connecta els 4 pins d'entrada a interruptors o jumpers per simular els valors 0 i 1.
    * Connecta els 2 pins de sortida a LEDs o multímetres per observar el resultat.
    * Compila i puja el codi a la placa.
    * Obre el Serial Monitor i observa com, en canviar les entrades, les sortides de la xarxa responen d'acord amb la taula de la veritat entrenada. Un LED a `outputPin0` s'encendrà quan la sortida sigui `LOW` (lògica inversa) i l'altre a `outputPin1` s'encendrà amb `HIGH`.

---
Aquest exercici et donarà una comprensió pràctica de com es dissenyen, entrenen i implementen petites xarxes neuronals en microcontroladors. Un pas gegant per al teu projecte!

Qualsevol dubte o problema en qualsevol de les fases, estic aquí per ajudar! 🧑‍💻


