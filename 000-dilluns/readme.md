**⚙️ Guia Ràpida: Instal·lar VSC i Extensions per a ESP8266/ESP32 a Windows 🚀**
---

Hola a tothom! Aquí teniu una guia ràpida i senzilla per configurar el vostre entorn de programació per a ESP8266/ESP32 amb el *framework* d'Arduino.

---
### 1. Visual Studio Code (VSC)
1.  **Descarrega VSC:**
    * Anem al web oficial: <https://code.visualstudio.com/>
    * Baixem l'instal·lador estable per a Windows.
2.  **Instal·la:**
    * Executa l'instal·lador i segueix els passos.
    * **Consell:** Marca la casella "Afegir a PATH" durant la instal·lació (molt útil!).

---
### 2. PlatformIO IDE per a VSC
Aquesta extensió és CLAU per als ESP!
1.  **Obre VSC.**
2.  **Ves a Extensions:** Fes clic a la icona de Extensions a la barra lateral esquerra (és com un quadrat).
3.  **Cerca:** Escriu `PlatformIO IDE`.
4.  **Instal·la:** Clica el botó **Instal·lar**. VSC s'encarregarà de tot!

---
### 3. Extensions Addicionals
Ara afegim eines extra molt útils:
1.  **Serial Monitor:**
    * A la secció de Extensions, cerca `Serial Monitor`.
    * Instal·la l'extensió (hi ha diverses opcions, la de "PlatformIO IDE Serial Monitor" sol anar molt bé!).
2.  **Teleplot:**
    * De la mateixa manera, cerca `Teleplot`.
    * Instal·la-la. Perfecte per visualitzar dades del port sèrie de forma gràfica! 📈

---
### 4. Preparació per a ESP8266/ESP32 (Arduino Framework)
Amb PlatformIO, això és molt senzill:
1.  **Reinicia VSC:** Tanca i torna a obrir VSC un cop hagis instal·lat tot.
2.  **Crea un nou projecte PlatformIO:**
    * Busca la icona de PlatformIO a la barra lateral de VSC (una "formiga" o un endoll).
    * Fes clic a **"Open Project Wizard"** o **"New Project"**.
    * **Tria la teva placa:** Per exemple, `ESP32 Dev Module` o `NodeMCU ESP8266`.
    * **Selecciona el *framework*:** Tria **`Arduino`**.
    * Posa un nom al projecte i tria on desar-lo.
    * PlatformIO descarregarà automàticament tot el que necessites!

---
Amb això ja ho tindries! A programar! Si teniu dubtes, pregunteu! 😉
---
**📝 Exercici-000: Multitasca Cooperativa amb ESP8266 (NodeMCU v2) 🚀**
---

Hola a tothom! Iniciem els exercicis amb una pràctica fonamental per al desenvolupament amb microcontroladors: la **multitasca cooperativa**. Aquest exercici ens introduirà a l'ús de la llibreria `Tasker` per gestionar múltiples tasques de forma eficient en un ESP8266, evitant l'ús de `delay()` que bloqueja el programa.

---
### **Objectiu de l'Exercici**

L'**Exercici-000** té com a finalitat principal demostrar com es pot executar diverses accions "simultàniament" en un microcontrolador (ESP8266) utilitzant un *scheduler* cooperatiu. Concretament, veurem:

* La configuració bàsica d'un projecte PlatformIO per a ESP8266 amb el *framework* d'Arduino.
* L'ús de la llibreria `Tasker` per programar tasques amb intervals de temps definits.
* El control de LEDs amb diferents ritmes d'intermitència.
* L'enviament de dades de telemetria pel port sèrie, preparades per a visualització amb Teleplot.

---
### **Fitxers del Projecte**

Aquest exercici es compon de dos fitxers principals:

1.  **`platformio.ini`**:
    * Defineix l'entorn de construcció per a la placa `nodemcuv2` (ESP8266).
    * Configura el *framework* `arduino`.
    * Estableix la velocitat del monitor sèrie a `115200` bauds.
    * Configura la velocitat d'upload a `512000` bauds.
    * **Molt important:** Declara les dependències de llibreries, incloent `joysfera/Tasker @ ^2.0.3` (el *scheduler* cooperatiu), `paulstoffregen/OneWire@^2.3.8` i `milesburton/DallasTemperature@^4.0.4` (aquestes dues darreres, tot i no usar-se directament en aquest `main.cpp`, estan definides per a futurs usos o configuració base).

2.  **`main.cpp`**:
    * Inclou la llibreria `Tasker.h`.
    * Inicialitza un objecte `Tasker` anomenat `tasker`.
    * Defineix 3 LEDs (pins 0, 2 i 16) i un pin PWM (pin 13).
    * A la funció `setup()`:
        * Inicialitza la comunicació sèrie a 115200 bps.
        * Configura els pins dels LEDs com a sortides i els apaga inicialment.
        * Configura el pin PWM.
        * **Programa quatre tasques** utilitzant `tasker.setInterval()`:
            * `intermitaLed16()` cada 300 ms.
            * `intermitaLed0()` cada 500 ms.
            * `intermitaLed2()` cada 700 ms.
            * `telemetria()` cada 100 ms.
    * A la funció `loop()`:
        * Crida `tasker.loop()` contínuament. Aquesta és la funció que s'encarrega d'executar les tasques programades quan els seus intervals de temps han expirat, sense bloquejar el programa.
    * **Funcions de les tasques:**
        * `intermitaLed16()`, `intermitaLed0()`, `intermitaLed2()`: Canvien l'estat ON/OFF del LED corresponent.
        * `telemetria()`: Llegeix el valor del pin analògic `A0` i l'envia pel port sèrie amb el format `>A0:valor`. Aquest format és ideal per visualitzar amb l'extensió Teleplot de VSC.

---
### **Com provar-ho**

1.  **Crea un nou projecte PlatformIO** a VSC amb la placa `nodemcuv2` i el *framework* `arduino`.
2.  **Copia** el contingut dels fitxers `platformio.ini` i `main.cpp` als seus respectius llocs en el teu projecte.
3.  **Connecta** la teva placa NodeMCU v2 al PC.
4.  **Compila i Puja** el codi a la placa (fletxa dreta a la barra inferior de VSC o icona de "Upload" a la barra de PlatformIO).
5.  **Obre el Serial Monitor** de PlatformIO (icona d'endoll a la barra lateral esquerra de VSC i després "Serial Monitor").
6.  **Obre Teleplot** i visualitza les dades d'A0. Observa com els LEDs de la placa parpellegen a diferents ritmes!

---
**Recorda:** La clau de la multitasca cooperativa és que cada tasca s'executa ràpidament i retorna el control al *scheduler* (`tasker.loop()`), permetent que altres tasques s'executin sense bloquejos. Això és fonamental per a aplicacions en temps real o amb múltiples sensors i actuadors.

Qualsevol dubte o problema, pregunteu! Feliç codificació! 🧑‍💻
