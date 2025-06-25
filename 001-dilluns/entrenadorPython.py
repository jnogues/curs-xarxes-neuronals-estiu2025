import numpy as np
import math
import random

# Xarxa neuronal 4-8-2 sense frameworks, amb sigmoid i backpropagation

# Configuració
InputNodes = 4
HiddenNodes = 8
OutputNodes = 2
PatternCount = 16
LearningRate = 0.1
Momentum = 0.8
InitialWeightMax = 0.5
Success = 0.01

# Dades d'entrenament
Input = np.array([
    [0,0,0,0], [0,0,0,1], [0,0,1,0], [0,0,1,1],
    [0,1,0,0], [0,1,0,1], [0,1,1,0], [0,1,1,1],
    [1,0,0,0], [1,0,0,1], [1,0,1,0], [1,0,1,1],
    [1,1,0,0], [1,1,0,1], [1,1,1,0], [1,1,1,1]
])

Target = np.array([
    [0,0], [1,1], [1,0], [0,1],
    [1,1], [0,1], [0,1], [1,1],
    [1,0], [0,1], [0,0], [1,1],
    [0,1], [1,1], [1,1], [0,1]
])

# Inicialització
HiddenWeights = np.random.uniform(-InitialWeightMax, InitialWeightMax, (InputNodes+1, HiddenNodes))
OutputWeights = np.random.uniform(-InitialWeightMax, InitialWeightMax, (HiddenNodes+1, OutputNodes))
ChangeHiddenWeights = np.zeros_like(HiddenWeights)
ChangeOutputWeights = np.zeros_like(OutputWeights)

# Sigmoide
sigmoid = lambda x: 1.0 / (1.0 + math.exp(-x))
dsigmoid = lambda y: y * (1.0 - y)  # derivada respecte a sortida

# Entrenament
TrainingCycle = 0
while True:
    TrainingCycle += 1
    Error = 0.0
    order = list(range(PatternCount))
    random.shuffle(order)

    for p in order:
        input_pattern = Input[p]
        target = Target[p]

        # Forward pass - Hidden
        Hidden = np.zeros(HiddenNodes)
        for i in range(HiddenNodes):
            acc = HiddenWeights[InputNodes][i]  # bias
            for j in range(InputNodes):
                acc += input_pattern[j] * HiddenWeights[j][i]
            Hidden[i] = sigmoid(acc)

        # Forward pass - Output
        Output = np.zeros(OutputNodes)
        for i in range(OutputNodes):
            acc = OutputWeights[HiddenNodes][i]  # bias
            for j in range(HiddenNodes):
                acc += Hidden[j] * OutputWeights[j][i]
            Output[i] = sigmoid(acc)

        # Error
        OutputDelta = (target - Output) * dsigmoid(Output)
        Error += 0.5 * np.sum((target - Output)**2)

        # Backpropagation - Hidden
        HiddenDelta = np.zeros(HiddenNodes)
        for i in range(HiddenNodes):
            acc = 0.0
            for j in range(OutputNodes):
                acc += OutputWeights[i][j] * OutputDelta[j]
            HiddenDelta[i] = acc * dsigmoid(Hidden[i])

        # Update Output Weights
        for i in range(OutputNodes):
            ChangeOutputWeights[HiddenNodes][i] = LearningRate * OutputDelta[i] + Momentum * ChangeOutputWeights[HiddenNodes][i]
            OutputWeights[HiddenNodes][i] += ChangeOutputWeights[HiddenNodes][i]
            for j in range(HiddenNodes):
                ChangeOutputWeights[j][i] = LearningRate * Hidden[j] * OutputDelta[i] + Momentum * ChangeOutputWeights[j][i]
                OutputWeights[j][i] += ChangeOutputWeights[j][i]

        # Update Hidden Weights
        for i in range(HiddenNodes):
            ChangeHiddenWeights[InputNodes][i] = LearningRate * HiddenDelta[i] + Momentum * ChangeHiddenWeights[InputNodes][i]
            HiddenWeights[InputNodes][i] += ChangeHiddenWeights[InputNodes][i]
            for j in range(InputNodes):
                ChangeHiddenWeights[j][i] = LearningRate * input_pattern[j] * HiddenDelta[i] + Momentum * ChangeHiddenWeights[j][i]
                HiddenWeights[j][i] += ChangeHiddenWeights[j][i]

    if TrainingCycle % 100 == 0:
        print(f"Cicle {TrainingCycle}, Error = {Error:.6f}")
    if Error < Success:
        break

# Exporta pesos en format amb noms de dimensions
print("\nconst float HiddenWeights[][HiddenNodes] = {")
for i in range(InputNodes):
    print("  {" + ", ".join(f"{v:.6f}" for v in HiddenWeights[i]) + "},")
print("  {" + ", ".join(f"{v:.6f}" for v in HiddenWeights[InputNodes]) + "}  // Biaixos")
print("};")

print("\nconst float OutputWeights[][OutputNodes] = {")
for i in range(HiddenNodes):
    print("  {" + ", ".join(f"{v:.6f}" for v in OutputWeights[i]) + "},")
print("  {" + ", ".join(f"{v:.6f}" for v in OutputWeights[HiddenNodes]) + "}  // Biaixos")
print("};")
