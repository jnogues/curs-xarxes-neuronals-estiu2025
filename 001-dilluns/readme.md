**🧠 Exercici-001: Resolució de Taula de la Veritat amb Xarxes Neuronals (Fase d'Entrenament) 🎯**
---

Hola a tothom! En aquest exercici ens endinsem al món de les **Xarxes Neuronals Artificials (ANN)** per resoldre un problema clàssic: una taula de la veritat. L'objectiu és entrenar una xarxa perquè aprengui la relació entre quatre entrades i dues sortides. Aquesta és la primera part, centrada en l'entrenament de la xarxa.

---
### **Objectiu de l'Exercici**

L'**Exercici-001** té com a objectiu entrenar una xarxa neuronal per reproduir una taula de la veritat específica. Aprendrem a:

* Configurar i entendre una xarxa neuronal simple (Multi-Layer Perceptron - MLP) amb una arquitectura de **4 neurones d'entrada, 8 a la capa oculta i 2 a la de sortida**.
* Implementar un algorisme de *backpropagation* per ajustar els pesos de la xarxa.
* Realitzar l'entrenament tant en un **ESP8266** (directament al microcontrolador) com, alternativament, en un **script de Python** per a un entrenament més ràpid i flexible.
* Obtenir els pesos finals de la xarxa un cop aquesta ha estat entrenada amb èxit, que serviran per a la fase d'inferència posterior.

---
### **La Taula de la Veritat a Resoldre**

Aquesta és la taula de la veritat que la nostra xarxa haurà d'aprendre:
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
### **Fitxers del Projecte (Fase d'Entrenament)**

Per a aquesta fase, disposem de dos enfocaments per a l'entrenament:

1.  **`mainEntrenament.cpp` (Entrenament a l'ESP8266):**
    * Codi C++ que s'executarà directament a l'ESP8266 (NodeMCU v2).
    * Conté la implementació del *backpropagation* "des de zero" amb funcions de sigmoide.
    * Defineix la configuració de la xarxa (`InputNodes=4`, `HiddenNodes=8`, `OutputNodes=2`).
    * Inclou les dades d'entrada (`Input`) i sortida (`Target`) de la taula de la veritat.
    * Durant l'entrenament, imprimeix l'error i els cicles d'entrenament per port sèrie.
    * **Un cop entrenada**, la funció `printWeightsAsCode()` genera els pesos de la xarxa en un format de codi C++ (arrays) per poder-los copiar i utilitzar en la fase d'inferència.

2.  **`entrenadorPython.py` (Entrenament alternatiu amb Python):**
    * Un script de Python que realitza exactament el mateix procés d'entrenament que el codi C++.
    * Utilitza llibreries com `numpy` per a operacions matricials, que acceleren considerablement el càlcul.
    * És útil per provar configuracions ràpidament o entrenar la xarxa fora del microcontrolador si l'entrenament a bord és massa lent.
    * També exporta els pesos finals en un format similar al C++ per la seva posterior utilització.

---
### **Com procedir**

1.  **Entrenament a l'ESP8266:**
    * Crea un projecte PlatformIO nou amb la placa `nodemcuv2` i el *framework* `arduino`.
    * Copia el contingut del `mainEntrenament.cpp` al teu `src/main.cpp`.
    * Compila i puja el codi a la teva placa.
    * Obre el Serial Monitor de VSC i observa el procés d'entrenament. Quan l'error sigui prou baix, el procés s'aturarà i s'imprimiran els pesos finals.
    * **Copia aquests pesos**, ja que els necessitaràs per al següent exercici (la fase d'inferència!).

2.  **Entrenament amb Python (alternatiu o complementari):**
    * Executa `entrenadorPython.py` en el teu ordinador (assegura't de tenir Python i `numpy` instal·lats).
    * Observa el procés d'entrenament a la consola. També et donarà els pesos finals un cop el criteri de `Success` sigui complert.

---
**Punts Clau:**

* **Arquitectura:** 4 neurones d'entrada, 8 a la capa oculta, 2 a la de sortida.
* **Algorisme:** *Backpropagation* per a l'aprenentatge.
* **Objectiu de l'entrenament:** Que la xarxa reprodueixi amb precisió les sortides de la taula de la veritat per a cada combinació d'entrades.
* **Resultat esperat:** Un conjunt de pesos per a les connexions entre capes (i els biaixos) que permetran a la xarxa funcionar correctament en la fase d'inferència.

---
Estem preparats per endinsar-nos en el món de l'aprenentatge automàtic en microcontroladors! Qualsevol dubte amb el procés d'entrenament, pregunteu!

---
