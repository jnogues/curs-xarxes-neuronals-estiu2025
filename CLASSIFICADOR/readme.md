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



