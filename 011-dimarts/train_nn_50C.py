
import numpy as np

# Arquitectura de la xarxa
input_neurons = 1
hidden1_neurons = 5
hidden2_neurons = 3
output_neurons = 1

learning_rate = 0.05  #0.1 
minimal_error = 0.01  #0.01

# Dades d'entrenament normalitzades
training_data = np.array([
    [0.0000, 1.0000],
    [0.0286, 0.9279],
    [0.1143, 0.8292],
    [0.2000, 0.7799],
    [0.2857, 0.7799],
    [0.3429, 0.7305],
    [0.4000, 0.7305],
    [0.4571, 0.6811],
    [0.5143, 0.6811],
    [0.5714, 0.6318],
    [0.6000, 0.5824],
    [0.6286, 0.5331],
    [0.6571, 0.4837],
    [0.6857, 0.4590],
    [0.7143, 0.4344],
    [0.7429, 0.4097],
    [0.7714, 0.3850],
    [0.8000, 0.3603],
    [0.8286, 0.3356],
    [0.8571, 0.3060],
    [0.8857, 0.2468],
    [0.9143, 0.1382],
    [0.9429, 0.0395],
    [1.0000, 0.0000]
])

# Normalització:
# temperature: min = 47.0, max = 50.5
# pwm: min = 10.0, max = 1023.0



# ENTRENAMENT FIX
#np.random.seed(42)

# Inicialització aleatòria dels pesos i biaixos
import time
seed = int(time.time())
np.random.seed(seed)
print(f'Llavor aleatòria usada: {seed}')
weights_input_hidden1 = np.random.rand(hidden1_neurons) - 0.5
bias_hidden1 = np.random.rand(hidden1_neurons) - 0.5

weights_hidden1_hidden2 = np.random.rand(hidden1_neurons, hidden2_neurons) - 0.5
bias_hidden2 = np.random.rand(hidden2_neurons) - 0.5

weights_hidden2_output = np.random.rand(hidden2_neurons) - 0.5
bias_output = np.random.rand() - 0.5

# Funcions d'activació
def sigmoid(x):
    return 1 / (1 + np.exp(-x))

def sigmoid_derivative(x):
    return x * (1 - x)

# Propagació endavant
def forward_propagation(input_value):
    global weights_input_hidden1, bias_hidden1
    global weights_hidden1_hidden2, bias_hidden2
    global weights_hidden2_output, bias_output

    # Capa oculta 1
    hidden1_input = input_value * weights_input_hidden1 + bias_hidden1
    hidden1_output = sigmoid(hidden1_input)

    # Capa oculta 2
    hidden2_input = np.dot(hidden1_output, weights_hidden1_hidden2) + bias_hidden2
    hidden2_output = sigmoid(hidden2_input)

    # Sortida
    output_input = np.dot(hidden2_output, weights_hidden2_output) + bias_output
    output = sigmoid(output_input)

    return output, hidden1_output, hidden2_output

# Entrenament
def train(input_value, target_output):
    global weights_input_hidden1, bias_hidden1
    global weights_hidden1_hidden2, bias_hidden2
    global weights_hidden2_output, bias_output

    output, h1_out, h2_out = forward_propagation(input_value)

    error = target_output - output
    delta_output = error * sigmoid_derivative(output)

    delta_hidden2 = delta_output * weights_hidden2_output * sigmoid_derivative(h2_out)

    delta_hidden1 = np.dot(weights_hidden1_hidden2, delta_hidden2) * sigmoid_derivative(h1_out)

    # Actualització de pesos
    weights_hidden2_output += h2_out * delta_output * learning_rate
    bias_output += delta_output * learning_rate

    for i in range(hidden1_neurons):
        for j in range(hidden2_neurons):
            weights_hidden1_hidden2[i][j] += h1_out[i] * delta_hidden2[j] * learning_rate
    bias_hidden2 += delta_hidden2 * learning_rate

    weights_input_hidden1 += input_value * delta_hidden1 * learning_rate
    bias_hidden1 += delta_hidden1 * learning_rate

    return abs(error)

# Entrenament principal
avg_error = 1.0
epoch = 0
while avg_error > minimal_error:
    total_error = 0
    for x, y in training_data:
        total_error += train(x, y)
    avg_error = total_error / len(training_data)
    epoch += 1
    if epoch % 1000 == 0:
        print(f"Època: {epoch}, Error mitjà: {avg_error:.5f}")

print("\nEntrenament completat!")
print(f"Èpoques totals: {epoch}")
print(f"Error final: {avg_error:.5f}")

# Prova
test_value = 0.6
pred, _, _ = forward_propagation(test_value)
print(f"Sortida per input {test_value}: {pred:.5f}")

print("\n// Pesos i biaixos en format per copiar al codi Arduino:")

# Pesos input → hidden1
print("float weights_input_hidden1[hidden1_neurons] = {")
print("    " + ", ".join(f"{w:.6f}" for w in weights_input_hidden1))
print("};\n")

# Pesos hidden1 → hidden2
print("float weights_hidden1_hidden2[hidden1_neurons][hidden2_neurons] = {")
for i in range(hidden1_neurons):
    row = ", ".join(f"{w:.6f}" for w in weights_hidden1_hidden2[i])
    print(f"    {{ {row} }}{',' if i < hidden1_neurons - 1 else ''}")
print("};\n")

# Pesos hidden2 → output
print("float weights_hidden2_output[hidden2_neurons] = {")
print("    " + ", ".join(f"{w:.6f}" for w in weights_hidden2_output))
print("};\n")

# Biasos hidden1
print("float bias_hidden1[hidden1_neurons] = {")
print("    " + ", ".join(f"{b:.6f}" for b in bias_hidden1))
print("};\n")

# Biasos hidden2
print("float bias_hidden2[hidden2_neurons] = {")
print("    " + ", ".join(f"{b:.6f}" for b in bias_hidden2))
print("};\n")

# Bias output
print("float bias_output = ")
print(f"    {bias_output:.6f};")


# ---------- Visualització ----------
import matplotlib.pyplot as plt

x_vals = training_data[:, 0]
y_true = training_data[:, 1]
y_pred = [forward_propagation(x)[0] for x in x_vals]

plt.plot(x_vals, y_true, 'ro', label='Dades reals')
plt.plot(x_vals, y_pred, 'b-', label='Sortida xarxa')
plt.xlabel('Input normalitzat')
plt.ylabel('Sortida')
plt.title('Resposta de la xarxa neuronal entrenada')
plt.legend()
plt.grid(True)
plt.show()
