import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# --- CONFIGURACIÓ ---
learning_rate = 0.01
target_mse = 0.01 #0.01
verbose_every = 1000

# --- FUNCIONS D'ACTIVACIÓ ---
def tanh(x):
    return np.tanh(x)

def tanh_derivative(x):
    return 1 - np.tanh(x) ** 2

# --- Llegeix i normalitza dataset ---
df = pd.read_csv('dataset_pid.csv')

X_raw = df[['error', 'errorPrev']].values
y_raw = df[['outputPWM']].values

X_mean = X_raw.mean(axis=0)
X_std = X_raw.std(axis=0)
y_mean = y_raw.mean(axis=0)
y_std = y_raw.std(axis=0)

X = (X_raw - X_mean) / X_std
y = (y_raw - y_mean) / y_std

# --- Inicialització de pesos (arquitectura 2-5-3-1) ---
np.random.seed(42)
W1 = np.random.randn(2, 5)
b1 = np.zeros((1, 5))
W2 = np.random.randn(5, 3)
b2 = np.zeros((1, 3))
W3 = np.random.randn(3, 1)
b3 = np.zeros((1, 1))

mse_list = []
epoch = 0
mse = float('inf')

# --- Entrenament ---
while mse > target_mse:
    Z1 = X @ W1 + b1
    A1 = tanh(Z1)

    Z2 = A1 @ W2 + b2
    A2 = tanh(Z2)

    Z3 = A2 @ W3 + b3
    y_pred = Z3

    error = y_pred - y
    mse = np.mean(error ** 2)
    mse_list.append(mse)
    epoch += 1

    dZ3 = 2 * error / len(X)
    dW3 = A2.T @ dZ3
    db3 = np.sum(dZ3, axis=0, keepdims=True)

    dA2 = dZ3 @ W3.T
    dZ2 = dA2 * tanh_derivative(Z2)
    dW2 = A1.T @ dZ2
    db2 = np.sum(dZ2, axis=0, keepdims=True)

    dA1 = dZ2 @ W2.T
    dZ1 = dA1 * tanh_derivative(Z1)
    dW1 = X.T @ dZ1
    db1 = np.sum(dZ1, axis=0, keepdims=True)

    W3 -= learning_rate * dW3
    b3 -= learning_rate * db3
    W2 -= learning_rate * dW2
    b2 -= learning_rate * db2
    W1 -= learning_rate * dW1
    b1 -= learning_rate * db1

    if epoch % verbose_every == 0:
        print(f"[{epoch}] MSE = {mse:.5f}")

print(f"\n✅ Entrenament finalitzat en {epoch} epochs. MSE = {mse:.5f}")

# --- Gràfic MSE
plt.plot(mse_list)
plt.axhline(target_mse, color='red', linestyle='--', label='Target MSE')
plt.xlabel('Epoch')
plt.ylabel('MSE')
plt.grid()
plt.title('Evolució del MSE')
plt.legend()
plt.tight_layout()
plt.show()

# --- Mostrar pesos i normalització
print("\n--- Pesos i Biaixos ---")
print("W1 =", W1)
print("b1 =", b1)
print("W2 =", W2)
print("b2 =", b2)
print("W3 =", W3)
print("b3 =", b3)

print("\n--- Normalització ---")
print("X_mean =", X_mean)
print("X_std =", X_std)
print("y_mean =", y_mean)
print("y_std =", y_std)

# --- Genera fitxer C++ per ESP8266
def format_array(arr, name, is_matrix=False):
    if is_matrix:
        lines = [f"  {{{', '.join(f'{v:.6f}f' for v in row)}}}" for row in arr]
        body = ",\n".join(lines)
        return f"const float {name}[{arr.shape[0]}][{arr.shape[1]}] = {{\n{body}\n}};\n"
    else:
        body = ", ".join(f"{v:.6f}f" for v in arr.flatten())
        return f"const float {name}[{arr.size}] = {{ {body} }};\n"

def format_scalar(value, name):
    return f"const float {name} = {value:.6f}f;\n"

header = "// Xarxa neuronal 2-5-3-1 entrenada amb NumPy\n\n"
header += format_array(W1, "W1", is_matrix=True)
header += format_array(b1, "b1")
header += "\n"
header += format_array(W2, "W2", is_matrix=True)
header += format_array(b2, "b2")
header += "\n"
header += format_array(W3.T, "W3")  # W3 era (3,1)
header += format_scalar(b3[0, 0], "b3")
header += "\n"
header += format_array(X_mean.reshape(1, -1), "X_mean")
header += format_array(X_std.reshape(1, -1), "X_std")
header += format_scalar(y_mean[0], "y_mean")
header += format_scalar(y_std[0], "y_std")

with open("xarxa_2_5_3_1.h", "w") as f:
    f.write(header)
print("✅ Fitxer xarxa_2_5_3_1.h creat amb èxit.")

