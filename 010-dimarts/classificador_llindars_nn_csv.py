#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
import pandas as pd # Necessitem pandas per llegir CSV fàcilment

# --- Paràmetres Globals del Classificador de Llindars ---
ADC_RANGE = (0, 1023) # Rang de l'ADC de l'ESP8266

# Nom del fitxer CSV que conté el teu dataset
DATASET_CSV_FILE = "lectures_sensor.csv" 

# Definició de les classes i el seu one-hot encoding
# ASSEGURA'T QUE AQUESTES ETIQUETES COINCIDEIXEN AMB LES DEL TEU CSV
CLASS_LABELS = {
    "low": [1, 0, 0],    # One-hot encoding per a 'Low'
    "medium": [0, 1, 0], # One-hot encoding per a 'Medium'
    "high": [0, 0, 1]    # One-hot encoding per a 'High'
}
CLASS_NAMES = list(CLASS_LABELS.keys())
NUM_CLASSES = len(CLASS_LABELS)

# --- Funció de Càrrega de Dades des de CSV ---
def load_data_from_csv(file_path):
    print(f"Carregant dades des de '{file_path}'...")
    try:
        df = pd.read_csv(file_path)
    except FileNotFoundError:
        print(f"ERROR: El fitxer '{file_path}' no s'ha trobat. Assegura't que és a la mateixa carpeta que l'script.")
        exit()
    except Exception as e:
        print(f"ERROR en llegir el CSV: {e}")
        exit()

    # Assegura't que les columnes 'adc_value' i 'class_label' existeixen
    if 'adc_value' not in df.columns or 'class_label' not in df.columns:
        print("ERROR: El CSV ha de contenir les columnes 'adc_value' i 'class_label'.")
        exit()

    all_data = df['adc_value'].values.reshape(-1, 1) # Una característica
    all_labels_text = df['class_label'].values

    # Convertir les etiquetes de text a one-hot encoding
    all_labels_one_hot = []
    # Usarem una llista per rastrejar els índexs a mantenir si hi ha etiquetes desconegudes
    valid_indices = [] 
    for i, label_text in enumerate(all_labels_text):
        if label_text in CLASS_LABELS:
            all_labels_one_hot.append(CLASS_LABELS[label_text])
            valid_indices.append(i)
        else:
            print(f"AVÍS: Etiqueta de classe desconeguda trobada al CSV: '{label_text}' a la fila {i+2}. Saltant mostra.") # +2 per capçalera + 1 base index
            
    # Filtrar les dades originals per mantenir només les mostres vàlides
    X = all_data[valid_indices]
    Y = np.array(all_labels_one_hot)

    # Barrejar les dades
    permutation = np.random.permutation(len(X))
    X_shuffled = X[permutation]
    Y_shuffled = Y[permutation]
    
    print(f"Dataset carregat amb {X_shuffled.shape[0]} mostres i {X_shuffled.shape[1]} característica.")
    print(f"Les etiquetes estan en format one-hot encoding amb {Y_shuffled.shape[1]} classes.")

    return X_shuffled, Y_shuffled


# --- Normalització de Característiques ---
# Només tenim 1 característica: la lectura de l'ADC
FEATURE_RANGES = {
    'adc_value': (float(ADC_RANGE[0]), float(ADC_RANGE[1]))
}

def normalize_features(features, feature_ranges):
    normalized_features = []
    min_val, max_val = feature_ranges['adc_value']
    
    for val in features:
        if max_val == min_val:
            normalized_features.append(0.0)
        else:
            normalized_features.append((val - min_val) / (max_val - min_val))
    return np.array(normalized_features).reshape(-1, 1) # Mantenir la forma 2D

# --- ENTRENAMENT DE LA XARXA NEURONAL "A PÈL" ---

# Paràmetres de la Xarxa Neuronal
INPUT_DIM = 1  # Només una entrada: la lectura de A0
HIDDEN_DIM = 5 # Una capa oculta molt petita
OUTPUT_DIM = NUM_CLASSES # 3 classes de sortida

LEARNING_RATE = 0.05 # Taxa d'aprenentatge
MAX_EPOCHS = 800000    # Nombre màxim d'èpoques (si no s'assoleix l'objectiu de pèrdua)
PATIENCE = 2000       # Nombre d'èpoques per esperar abans d'aturar si la pèrdua no millora
TARGET_LOSS = 0.01    # NOU: Objectiu de pèrdua per aturar l'entrenament abans

def relu(x):
    return np.maximum(0, x)

def relu_derivative(x):
    return (x > 0).astype(float)

def softmax(x):
    e_x = np.exp(x - np.max(x, axis=-1, keepdims=True)) 
    return e_x / np.sum(e_x, axis=-1, keepdims=True)

def initialize_weights(input_dim, hidden_dim, output_dim):
    W1 = np.random.randn(input_dim, hidden_dim) * 0.01
    b1 = np.zeros((1, hidden_dim))
    W2 = np.random.randn(hidden_dim, output_dim) * 0.01
    b2 = np.zeros((1, output_dim))
    return W1, b1, W2, b2

def forward_pass(X, W1, b1, W2, b2):
    Z1 = np.dot(X, W1) + b1
    A1 = relu(Z1)
    Z2 = np.dot(A1, W2) + b2
    A2 = softmax(Z2)
    return Z1, A1, Z2, A2

def compute_loss(Y_pred, Y_true):
    epsilon = 1e-10
    Y_pred = np.clip(Y_pred, epsilon, 1 - epsilon)
    loss = -np.sum(Y_true * np.log(Y_pred)) / len(Y_true)
    return loss

def train_nn(X, Y, W1, b1, W2, b2, learning_rate, max_epochs, patience, target_loss):
    losses = []
    accuracies = []

    best_loss = float('inf')
    epochs_no_improve = 0
    
    best_W1, best_b1, best_W2, best_b2 = W1.copy(), b1.copy(), W2.copy(), b2.copy()

    for epoch in range(max_epochs):
        Z1, A1, Z2, A2 = forward_pass(X, W1, b1, W2, b2)
        loss = compute_loss(A2, Y)
        losses.append(loss)
        
        predictions = np.argmax(A2, axis=1)
        true_labels = np.argmax(Y, axis=1)
        accuracy = np.mean(predictions == true_labels)
        accuracies.append(accuracy)

        # Condició d'Early Stopping per pèrdua objectiu
        if loss <= target_loss:
            print(f"\nObjectiu de pèrdua ({target_loss:.4f}) assolit a l'època {epoch+1}. Aturant entrenament.")
            break 

        if loss < best_loss:
            best_loss = loss
            epochs_no_improve = 0
            best_W1, best_b1, best_W2, best_b2 = W1.copy(), b1.copy(), W2.copy(), b2.copy()
        else:
            epochs_no_improve += 1
            if epochs_no_improve >= patience:
                print(f"\nEarly stopping a l'època {epoch+1} (pèrdua no millora durant {patience} èpoques).")
                break

        dZ2 = A2 - Y 
        dW2 = np.dot(A1.T, dZ2) / len(X)
        db2 = np.sum(dZ2, axis=0, keepdims=True) / len(X)

        dA1 = np.dot(dZ2, W2.T)
        dZ1 = dA1 * relu_derivative(A1) 

        dW1 = np.dot(X.T, dZ1) / len(X)
        db1 = np.sum(dZ1, axis=0, keepdims=True) / len(X)

        W1 -= learning_rate * dW1
        b1 -= learning_rate * db1
        W2 -= learning_rate * dW2
        b2 -= learning_rate * db2
        
        if (epoch + 1) % (max_epochs // 500 or 1) == 0 or (epoch + 1) == 1 or (epoch + 1) == max_epochs:
            print(f"Epoch {epoch+1}/{max_epochs}, Loss: {loss:.4f}, Accuracy: {accuracy:.4f}, Best Loss: {best_loss:.4f}, No Improve: {epochs_no_improve}")

    print(f"\nEntrenament finalitzat. Millor precisió: {np.max(accuracies):.4f} (a l'època amb la millor pèrdua).")
    return best_W1, best_b1, best_W2, best_b2, losses, accuracies

# --- Quantització i Exportació dels Pesos a C/C++ ---

def quantize_weights(weights):
    min_float = np.min(weights)
    max_float = np.max(weights)
    
    min_int = -127.0 
    max_int = 127.0
    
    abs_max = max(abs(min_float), abs(max_float))
    if abs_max == 0:
        scale = 1.0
    else:
        scale = abs_max / max_int 
    
    quantized_weights = np.round(weights / scale).astype(np.int8)
    
    return quantized_weights, scale

def generate_c_header(W1, b1, W2, b2, scale_W1, scale_b1, scale_W2, scale_b2, filename="nn_params_simple.h"):
    """Genera un fitxer de capçalera C amb els pesos i biaixos per al model simple."""
    with open(filename, 'w') as f:
        f.write("#ifndef NN_PARAMS_SIMPLE_H\n")
        f.write("#define NN_PARAMS_SIMPLE_H\n\n")

        f.write(f"// Parameters for the Simple Neural Network model\n")
        f.write(f"#define INPUT_DIM_SIMPLE {INPUT_DIM}\n")
        f.write(f"#define HIDDEN_DIM_SIMPLE {HIDDEN_DIM}\n")
        f.write(f"#define OUTPUT_DIM_SIMPLE {OUTPUT_DIM}\n\n")
        f.write(f"// Class names for reference\n")
        f.write(f"const char* CLASS_NAMES_SIMPLE[] = {{\n")
        for name in CLASS_NAMES:
            f.write(f"    \"{name}\",\n")
        f.write(f"}};\n\n")

        def write_array(name, arr, scale):
            f.write(f"// {name} (shape: {arr.shape}, scale: {scale:.6f})\n")
            f.write(f"const int8_t {name}_simple[{arr.size}] = {{\n    ")
            f.write(", ".join(map(str, arr.flatten())))
            f.write("\n};\n\n")
            f.write(f"const float {name}_simple_scale = {scale:.6f}f;\n\n")

        write_array("W1", W1, scale_W1)
        write_array("b1", b1, scale_b1)
        write_array("W2", W2, scale_W2)
        f.write(f"// b2 (shape: {b2.shape}, scale: {scale_b2:.6f})\n") # Adjusted for proper b2 shape (1, NUM_CLASSES)
        f.write(f"const int8_t b2_simple[{b2.size}] = {{\n    ")
        f.write(", ".join(map(str, b2.flatten())))
        f.write("\n};\n\n")
        f.write(f"const float b2_simple_scale = {sb2:.6f}f;\n\n") # FIX: Use sb2 here

        f.write("#endif // NN_PARAMS_SIMPLE_H\n")
    print(f"Fitxer de capçalera C '{filename}' generat amb èxit.")


# --- Funció de Visualització del Dataset ---
def plot_dataset(X, Y, feature_ranges, class_names):
    print("\n--- Visualitzant el Dataset Carregat des del CSV ---")

    plt.figure(figsize=(10, 6))
    
    # Utilitzem X directament per visualitzar les dades originals del CSV
    
    # Determinar els colors per a cada classe
    colors = ['blue', 'green', 'red'] # Low, Medium, High (ordre basat en CLASS_NAMES)
    
    # Iterar per cada punt de dada i pintar-lo amb el color de la seva classe
    for i in range(len(X)):
        adc_value = X[i][0] # La lectura ADC original del CSV
        label_one_hot = Y[i] # Etiqueta one-hot
        
        class_idx = np.argmax(label_one_hot) # Trobar l'índex de la classe
        
        plt.scatter(adc_value, 0, color=colors[class_idx], alpha=0.6, s=50) # El 0 és per dibuixar-ho en una línia horitzontal

    plt.title('Distribució del Dataset CSV per Classes')
    plt.xlabel('Valor de l\'ADC')
    plt.yticks([]) # Amaga les marques de l'eix Y, ja que totes les dades estan a y=0
    
    # Afegir llindars visuals: Si no tens llindars fixes, pots comentar aquestes línies.
    # Si els tens i coincideixen amb la lògica del CSV, descomenta i ajusta.
    # plt.axvline(x=300, color='gray', linestyle='--', label=f'Llindar Low/Medium (300)')
    # plt.axvline(x=700, color='purple', linestyle='--', label=f'Llindar Medium/High (700)')

    plt.xlim(feature_ranges['adc_value'][0], feature_ranges['adc_value'][1]) # Limitar l'eix X al rang de l'ADC
    plt.grid(True, axis='x', linestyle=':', alpha=0.7) # Només quadrícula a l'eix X

    # Afegir una llegenda manual
    from matplotlib.lines import Line2D
    legend_elements = [
        Line2D([0], [0], marker='o', color='w', label=class_names[0], markerfacecolor=colors[0], markersize=10),
        Line2D([0], [0], marker='o', color='w', label=class_names[1], markerfacecolor=colors[1], markersize=10),
        Line2D([0], [0], marker='o', color='w', label=class_names[2], markerfacecolor=colors[2], markersize=10),
    ]
    # Opcional: Si uses els llindars visuals, afegeix aquesta línia a legend_elements:
    # Line2D([0], [0], color='gray', linestyle='--', label='Llindars definits')
    plt.legend(handles=legend_elements, loc='upper right')

    plt.show()


# --- Execució Principal del Script ---

if __name__ == "__main__":
    # Carregar el dataset des del CSV
    X_train, Y_train = load_data_from_csv(DATASET_CSV_FILE)

    print("\n--- Informació del Dataset Carregat ---")
    print(f"Forma de les característiques (X_train): {X_train.shape}")
    print(f"Forma de les etiquetes (Y_train): {Y_train.shape}")
    
    # Normalitzar les característiques (necessari abans d'entrenar)
    # Aquesta normalització utilitzarà el rang de l'ADC (0-1023)
    X_train_normalized = normalize_features(X_train, FEATURE_RANGES)
    print(f"Forma de les característiques normalitzades: {X_train_normalized.shape}")

    # Visualitzar el dataset carregat (descomenta la línia de sota per activar-ho)
    # plot_dataset(X_train, Y_train, FEATURE_RANGES, CLASS_NAMES)

    print("\n--- Entrenant la Xarxa Neuronal 'a pèl' amb dades del CSV ---")
    W1, b1, W2, b2 = initialize_weights(INPUT_DIM, HIDDEN_DIM, OUTPUT_DIM)

    W1_trained, b1_trained, W2_trained, b2_trained, losses, accuracies = train_nn(
        X_train_normalized, Y_train, W1, b1, W2, b2, LEARNING_RATE, MAX_EPOCHS, PATIENCE, TARGET_LOSS
    )

    # Visualitzar la pèrdua i la precisió durant l'entrenament
    plt.figure(figsize=(10, 5))
    plt.subplot(1, 2, 1)
    plt.plot(losses)
    plt.title('Pèrdua durant l\'Entrenament')
    plt.xlabel('Època')
    plt.ylabel('Pèrdua')
    plt.grid(True)

    plt.subplot(1, 2, 2)
    plt.plot(accuracies)
    plt.title('Precisió durant l\'Entrenament')
    plt.xlabel('Època')
    plt.ylabel('Precisió')
    plt.grid(True)
    plt.tight_layout()
    plt.show()

    print("\n--- Quantitzant i Exportant Pesos a C/C++ ---")
    
    qW1, sW1 = quantize_weights(W1_trained)
    qb1, sb1 = quantize_weights(b1_trained)
    qW2, sW2 = quantize_weights(W2_trained)
    qb2, sb2 = quantize_weights(b2_trained) 

    generate_c_header(
        qW1, qb1, 
        qW2, qb2, 
        sW1, sb1, 
        sW2, sb2,
        filename="nn_params_simple.h" # S'exporta al mateix nom de fitxer per compatibilitat amb el C++
    )
    
    print("\n--- Rangos de Caracteristicas para C/C++ (para la única característica 'adc_value') ---")
    for key, value in FEATURE_RANGES.items():
        print(f"const float FEATURE_MIN_{key} = {value[0]:.6f}f;")
        print(f"const float FEATURE_MAX_{key} = {value[1]:.6f}f;\n")

    print("\nScript completat. El model de classificador de llindars ha estat entrenat amb dades del CSV i els pesos exportats a C/C++.")