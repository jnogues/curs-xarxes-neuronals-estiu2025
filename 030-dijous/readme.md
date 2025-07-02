Super dijous.
* Es tracta d'implementar la xarxa neuronal en un ESP32 i micropython
* Instal·larem com a IDE [Thonny](https://thonny.org/)
* Després cal flashejar l'ESP32 amb la imtge de micropython.  
* Per entrenar la xarxa tenim un [Google Colab](https://colab.research.google.com/drive/11-q7iKI3ELDgvOzC-MVxOgNGuPSy8s3w?usp=sharing)
* Hem de descarregar al dispositiu tots els fitxers.
* El resultat és un fitxer de nom: nn_params_micropython.json
* I ara ja podem correr els scripts.
* El dataset, tot i que ja està a l'script, és:

| temperatura | pwm  |
| ----------- | ---- |
| 47          | 1023 |
| 47.1        | 950  |
| 47.4        | 850  |
| 47.7        | 800  |
| 48.0        | 800  |
| 48.2        | 750  |
| 48.4        | 750  |
| 48.6        | 700  |
| 48.8        | 700  |
| 49.0        | 650  |
| 49.1        | 600  |
| 49.2        | 550  |
| 49.3        | 500  |
| 49.4        | 475  |
| 49.5        | 450  |
| 49.6        | 425  |
| 49.7        | 400  |
| 49.8        | 375  |
| 49.9        | 350  |
| 50.0        | 320  |
| 50.1        | 260  |
| 50.2        | 150  |
| 50.3        | 50   |
| 50.5        | 10   |


# Guia d'Instal·lació de Thonny IDE a Windows per a MicroPython

Aquesta guia t’explica com instal·lar pas a pas l’IDE **Thonny**, que és una eina senzilla i ideal per programar dispositius amb **MicroPython**, com l’ESP32 o el Raspberry Pi Pico.

## ✅ Requisits previs

- Un ordinador amb **Windows 10 o superior**
- Connexió a internet
- Privilegis per instal·lar programari

---

## 🧩 Pas 1: Descarrega Thonny

1. Ves a la pàgina oficial de Thonny:
   👉 [https://thonny.org](https://thonny.org)

2. A la pàgina principal, fes clic al botó **"Download for Windows"**.
   - S'està descarregant un fitxer `.exe` com ara: `thonny-xxx.exe`

---

## 💾 Pas 2: Instal·la Thonny

1. Un cop descarregat el fitxer `.exe`, fes **doble clic** per executar-lo.
2. Accepta els termes de llicència.
3. Tria la ubicació d’instal·lació o deixa la predeterminada.
4. Deixa seleccionades les opcions per crear una drecera al menú d'inici i/o escriptori.
5. Fes clic a **"Install"** i espera que finalitzi la instal·lació.

---

## 🚀 Pas 3: Executa Thonny

1. Un cop instal·lat, busca "Thonny" al menú d'inici de Windows i obre’l.
2. Es mostrarà una interfície senzilla d’editor de codi.

---

## 🔌 Pas 4: Configura Thonny per a MicroPython

1. Ves al menú **"Tools" (Eines)** > **"Options…" (Opcions)**.
2. A la pestanya **"Interpreter"**:
   - Selecciona **"MicroPython (ESP32 / ESP8266)"** o el dispositiu que utilitzis.
   - Connecta el dispositiu al PC amb un cable USB.
   - Selecciona el port sèrie correcte (per exemple `COM3` o `COM5`).
3. Fes clic a **"OK"**.

---

## ✅ Pas 5: Comença a programar

Ara ja pots escriure codi MicroPython i executar-lo directament sobre el teu dispositiu des de Thonny!

Exemple:

```python
print("Hola, món des de MicroPython!")



