import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import mean_squared_error, r2_score
import matplotlib.pyplot as plt

# --- 0. Configuració General ---
# Assegura't que aquest fitxer CSV no conté la columna 'Setpoint'
input_filename = 'dades_entrenament.csv' # O 'dades_entrenament_sense_setpoint.csv' si el vas generar
PWM_MIN = 0
PWM_MAX = 1023

# --- 1. Implementació de la Xarxa Neuronal "a pèl" ---
class SimpleNeuralNetwork:
    def __init__(self, input_size, hidden_size, output_size, learning_rate=0.01, momentum=0.9, random_seed=42):
        self.learning_rate = learning_rate
        self.momentum = momentum

        #np.random.seed(random_seed) # Per resultats reproduïbles

        # Pesos i biaixos per a la capa oculta (Input -> Hidden)
        # W1: (input_size, hidden_size)
        # b1: (1, hidden_size)
        # *** AJUST CRÍTIC AQUÍ: INICIALITZACIÓ MÉS PETITA ***
        self.W1 = np.random.randn(input_size, hidden_size) * 0.001 # Molt més petit per evitar explosió
        self.b1 = np.zeros((1, hidden_size))

        # Pesos i biaixos per a la capa de sortida (Hidden -> Output)
        # W2: (hidden_size, output_size)
        # b2: (1, output_size)
        # *** AJUST CRÍTIC AQUÍ: INICIALITZACIÓ MÉS PETITA ***
        self.W2 = np.random.randn(hidden_size, output_size) * 0.001 # Molt més petit per evitar explosió
        self.b2 = np.zeros((1, output_size))

        # Velocitats per al moment (momentum)
        self.vW1 = np.zeros_like(self.W1)
        self.vb1 = np.zeros_like(self.b1)
        self.vW2 = np.zeros_like(self.W2)
        self.vb2 = np.zeros_like(self.b2)
    
    # Funcions d'activació
    def sigmoid(self, x):
        return 1 / (1 + np.exp(-x))

    def sigmoid_derivative(self, x):
        s = self.sigmoid(x)
        return s * (1 - s)
    
    def relu(self, x):
        return np.maximum(0, x)

    def relu_derivative(self, x):
        return (x > 0).astype(float)

    # Forward Pass
    def forward(self, X):
        # Capa oculta
        self.Z1 = np.dot(X, self.W1) + self.b1
        #self.A1 = self.relu(self.Z1) # Sortida de la capa oculta (després de ReLU)
        self.A1 = self.sigmoid(self.Z1)
        # Capa de sortida
        self.Z2 = np.dot(self.A1, self.W2) + self.b2
        self.A2 = self.Z2 # Predicció final (activació lineal per regressió)
        return self.A2

    # Backward Pass (Retropropagació)
    def backward(self, X, y, y_pred):
        m = X.shape[0] # Nombre de mostres

        dZ2 = (y_pred - y) / m

        dW2 = np.dot(self.A1.T, dZ2)
        db2 = np.sum(dZ2, axis=0, keepdims=True)

        dA1 = np.dot(dZ2, self.W2.T)
        #dZ1 = dA1 * self.relu_derivative(self.Z1) # Aplicar la derivada de la funció d'activació (ReLU)
        dZ1 = dA1 * self.sigmoid_derivative(self.Z1)

        dW1 = np.dot(X.T, dZ1)
        db1 = np.sum(dZ1, axis=0, keepdims=True)

        return dW1, db1, dW2, db2

    # Actualització de pesos amb Momentum
    def update_weights(self, dW1, db1, dW2, db2):
        self.vW1 = self.momentum * self.vW1 - self.learning_rate * dW1
        self.vb1 = self.momentum * self.vb1 - self.learning_rate * db1
        self.vW2 = self.momentum * self.vW2 - self.learning_rate * dW2
        self.vb2 = self.momentum * self.vb2 - self.learning_rate * db2

        self.W1 += self.vW1
        self.b1 += self.vb1
        self.W2 += self.vW2
        self.b2 += self.vb2

    def train(self, X_train, y_train, X_test, y_test, epochs, target_mse=None, early_stopping_patience=1000, verbose_interval=1000):
        history = {'loss': [], 'mse_train': [], 'mse_test': [], 'r2_train': [], 'r2_test': []}
        best_mse_test = float('inf') # Inicialitzar amb un valor molt alt
        patience_counter = 0

        y_train_reshaped = y_train.reshape(-1, 1)

        print(f"\nIniciant entrenament 'a pèl'...")
        print(f"Capes: Entrada ({self.W1.shape[0]}) -> Oculta ({self.W1.shape[1]}) -> Sortida ({self.W2.shape[1]})")
        print(f"Learning Rate: {self.learning_rate}, Momentum: {self.momentum}")
        print(f"Èpoques màximes: {epochs}, Tolerància MSE test: {target_mse}, Paciència early stopping: {early_stopping_patience}")


        for epoch in range(epochs):
            y_pred_train = self.forward(X_train)

            loss = mean_squared_error(y_train_reshaped, y_pred_train)
            history['loss'].append(loss)

            dW1, db1, dW2, db2 = self.backward(X_train, y_train_reshaped, y_pred_train)
            self.update_weights(dW1, db1, dW2, db2)

            mse_train = mean_squared_error(y_train_reshaped, y_pred_train)
            r2_train = r2_score(y_train_reshaped, y_pred_train)

            y_pred_test = self.forward(X_test)
            mse_test = mean_squared_error(y_test, y_pred_test)
            r2_test = r2_score(y_test, y_pred_test)

            history['mse_train'].append(mse_train)
            history['mse_test'].append(mse_test)
            history['r2_train'].append(r2_train)
            history['r2_test'].append(r2_test)

            if (epoch + 1) % verbose_interval == 0:
                print(f"Època {epoch + 1}/{epochs}, Pèrdua (MSE): {loss:.4f}, MSE Entr: {mse_train:.2f}, R2 Entr: {r2_train:.4f}, MSE Test: {mse_test:.2f}, R2 Test: {r2_test:.4f}")

            if target_mse is not None and mse_test <= target_mse:
                print(f"\nObjectiu MSE ({target_mse:.2f}) assolit al conjunt de prova a l'època {epoch + 1}.")
                break
            
            if mse_test < best_mse_test:
                best_mse_test = mse_test
                patience_counter = 0
            else:
                patience_counter += 1
                if patience_counter >= early_stopping_patience:
                    print(f"\nEntrenament aturat. MSE del test no ha millorat en {early_stopping_patience} èpoques a l'època {epoch + 1}.")
                    break

        return history

    def predict(self, X):
        return self.forward(X)

    def get_params(self):
        """Retorna els pesos i biaixos per ser transferits al microcontrolador."""
        return {
            'W1': self.W1.tolist(),
            'b1': self.b1.tolist(),
            'W2': self.W2.tolist(),
            'b2': self.b2.tolist()
        }

# --- 2. Procés de Neteja de Dades (integrat) ---
print(f"Iniciant el procés de neteja de dades des de '{input_filename}'...")

try:
    df = pd.read_csv(input_filename)
    print(f"Dades carregades correctament. Número de mostres inicial: {len(df)}")

    initial_rows = len(df)
    if np.isinf(df.values).any():
        print("\nADVERTÈNCIA: S'han trobat valors infinits. Reemplaçant per NaN.")
        df.replace([np.inf, -np.inf], np.nan, inplace=True)

    if df.isnull().values.any():
        print("\nADVERTÈNCIA: S'han trobat valors NaN. Eliminant files amb NaN.")
        df.dropna(inplace=True)

    rows_after_nan_inf = len(df)
    if initial_rows > rows_after_nan_inf:
        print(f"S'han eliminat {initial_rows - rows_after_nan_inf} files a causa de valors NaN/Inf.")
    else:
        print("No s'han trobat valors NaN/Inf per eliminar.")

    print(f"\nRealitzant neteja de la columna 'pwm' per limitar al rang [{PWM_MIN}, {PWM_MAX}]...")
    original_pwm_max = df['pwm'].max()
    original_pwm_min = df['pwm'].min()
    
    df['pwm'] = df['pwm'].clip(lower=PWM_MIN, upper=PWM_MAX)
    
    cleaned_pwm_max = df['pwm'].max()
    cleaned_pwm_min = df['pwm'].min()

    if original_pwm_max > PWM_MAX or original_pwm_min < PWM_MIN:
        print(f"S'han limitat valors de 'pwm' fora del rang. Max original: {original_pwm_max:.2f}, Min original: {original_pwm_min:.2f}")
        print(f"Nous valors de 'pwm': Max {cleaned_pwm_max:.2f}, Min {cleaned_pwm_min:.2f}")
    else:
        print("No s'han trobat valors de 'pwm' fora de rang per limitar.")

    print("\nEstadístiques descriptives de les dades netejades:")
    print(df.describe())
    
except FileNotFoundError:
    print(f"Error: El fitxer '{input_filename}' no s'ha trobat. Assegura't que està al mateix directori.")
    exit()
except Exception as e:
    print(f"Ha ocorregut un error durant la càrrega/neteja de dades: {e}")
    exit()

# --- 3. Preparació de les dades per a l'entrenament ---
input_features = ['T', 'error', 'integral', 'derivada']
X = df[input_features].values
y = df['pwm'].values.reshape(-1, 1)

print("\nCaracterístiques (X) i variable objectiu (y) definides.")

X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.3, random_state=42)

print(f"Mides dels conjunts: Entrenament ({len(X_train)} mostres), Prova ({len(X_test)} mostres)")
print(f"Proporció de dades de prova: {len(X_test) / len(df):.2%}")


scaler = StandardScaler()
X_train_scaled = scaler.fit_transform(X_train)
X_test_scaled = scaler.transform(X_test)

print("\nDades escalades correctament.")
print("\nRangs de les dades escalades (X_train_scaled):")
for i, col in enumerate(input_features):
    print(f"  {col}: Min={X_train_scaled[:, i].min():.2f}, Max={X_train_scaled[:, i].max():.2f}")


# --- 4. Configuració i Entrenament de la Xarxa Neuronal "a pèl" ---
input_dim = X_train_scaled.shape[1]
hidden_dim = 16
output_dim = 1

# Paràmetres d'entrenament (ajustats per a més estabilitat)
LEARNING_RATE = 0.0005 # Learning rate més petit per evitar inestabilitat
MOMENTUM = 0.9
EPOCHS = 200000
TARGET_MSE_TEST = 18.0 # Objectiu MSE inicial que ja va funcionar
EARLY_STOPPING_PATIENCE = 3000

nn_model = SimpleNeuralNetwork(input_dim, hidden_dim, output_dim,
                               learning_rate=LEARNING_RATE, momentum=MOMENTUM)

history = nn_model.train(X_train_scaled, y_train, X_test_scaled, y_test,
                         epochs=EPOCHS, target_mse=TARGET_MSE_TEST,
                         early_stopping_patience=EARLY_STOPPING_PATIENCE,
                         verbose_interval=1000)

print("\nEntrenament de la Xarxa Neuronal completat.")

# --- 5. Avaluació Final ---
y_pred_train = nn_model.predict(X_train_scaled)
y_pred_test = nn_model.predict(X_test_scaled)

mse_train = mean_squared_error(y_train, y_pred_train)
mse_test = mean_squared_error(y_test, y_pred_test)

r2_train = r2_score(y_train, y_pred_train)
r2_test = r2_score(y_test, y_pred_test)

print(f"\n--- Resultats Finals ---")
print(f"Error Quadràtic Mitjà (MSE) en entrenament: {mse_train:.2f}")
print(f"Error Quadràtic Mitjà (MSE) en prova: {mse_test:.2f}")
print(f"Coeficient de Determinació (R^2) en entrenament: {r2_train:.4f}")
print(f"Coeficient de Determinació (R^2) en prova: {r2_test:.4f}")


# --- 6. Visualització dels resultats ---
plt.figure(figsize=(14, 6))

plt.subplot(1, 2, 1)
plt.plot(history['loss'])
plt.title('Pèrdua (MSE) durant l\'entrenament')
plt.xlabel('Època')
plt.ylabel('MSE')
plt.grid(True)

plt.subplot(1, 2, 2)
plt.plot(history['mse_train'], label='MSE Entrenament')
plt.plot(history['mse_test'], label='MSE Prova')
plt.title('MSE durant l\'entrenament i prova')
plt.xlabel('Època')
plt.ylabel('MSE')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()

plt.figure(figsize=(14, 6))

plt.subplot(1, 2, 1)
plt.scatter(y_train, y_pred_train, alpha=0.5)
plt.plot([y.min(), y.max()], [y.min(), y.max()], 'r--', lw=2)
plt.xlabel("Valor Real del PWM (Entrenament)")
plt.ylabel("Valor Predicte del PWM (Entrenament)")
plt.title("Prediccions vs. Reals (Conjunt d'Entrenament)")
plt.grid(True)

plt.subplot(1, 2, 2)
plt.scatter(y_test, y_pred_test, alpha=0.5)
plt.plot([y.min(), y.max()], [y.min(), y.max()], 'r--', lw=2)
plt.xlabel("Valor Real del PWM (Prova)")
plt.ylabel("Valor Predicte del PWM (Prova)")
plt.title("Prediccions vs. Reals (Conjunt de Prova)")
plt.grid(True)
plt.tight_layout()
plt.show()


# --- 7. Impressió de Paràmetres per a la Implementació al Microcontrolador ---
print("\n--- Paràmetres del Model per a l'Implementació al Microcontrolador (C/C++) ---")
print(f"// Nombre d'entrades de la XN: {input_dim}")
print(f"// Nombre de neurones a la capa oculta: {hidden_dim}")
print(f"// Nombre de sortides de la XN: {output_dim}")


# Paràmetres de l'escalador (StandardScaler)
print("\n// Paramètres de StandardScaler:")
print(f"float scaler_mean[{input_dim}] = {{{', '.join(f'{x:.6f}f' for x in scaler.mean_)}}};")
print(f"float scaler_std[{input_dim}] = {{{', '.join(f'{x:.6f}f' for x in scaler.scale_)}}};")

# Paràmetres de la Xarxa Neuronal
nn_params = nn_model.get_params()

print("\n// Paramètres de la Xarxa Neuronal (capa oculta amb ReLU, sortida lineal):")

print("\n// Pesos de la capa 1 (input a oculta)")
print(f"float W1[{input_dim}][{hidden_dim}] = {{")
for row in nn_params['W1']:
    print(f"    {{{', '.join(f'{x:.6f}f' for x in row)}}},")
print("};")

print("\n// Biaixos de la capa 1 (capa oculta)")
print(f"float b1[{hidden_dim}] = {{{', '.join(f'{x:.6f}f' for x in nn_params['b1'][0])}}};")


print("\n// Pesos de la capa 2 (oculta a sortida)")
print(f"float W2[{hidden_dim}][{output_dim}] = {{")
for row in nn_params['W2']:
    print(f"    {{{', '.join(f'{x:.6f}f' for x in row)}}},")
print("};")

print("\n// Biaixos de la capa 2 (capa de sortida)")
print(f"float b2[{output_dim}] = {{{', '.join(f'{x:.6f}f' for x in nn_params['b2'][0])}}};")

print("\nProcés completat. Aquests són els paràmetres que necessitaràs.")