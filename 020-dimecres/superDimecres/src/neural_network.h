#ifndef NEURAL_NETWORK_H
#define NEURAL_NETWORK_H

#include <Arduino.h> // Per a funcions bàsiques com Serial.print, si cal

// Defineix les dimensions de la xarxa
const int NN_INPUT_DIM = 4;
const int NN_HIDDEN_DIM = 16;
const int NN_OUTPUT_DIM = 1;

// --- Paràmetres de StandardScaler ---
// Aquests valors han de ser exactament els que t'ha donat el teu script Python.
// Utilitza 'f' al final de cada nombre per indicar que és un float.
float scaler_mean[NN_INPUT_DIM] = {49.706916f, 0.292951f, 92.885388f, -0.007061f};
float scaler_std[NN_INPUT_DIM] = {2.687167f, 2.686751f, 39.153511f, 0.145594f};

// --- Paràmetres de la Xarxa Neuronal (capa oculta amb Sigmoide, sortida lineal) ---

// Pesos de la capa 1 (input a oculta) [NN_INPUT_DIM][NN_HIDDEN_DIM]
float W1[NN_INPUT_DIM][NN_HIDDEN_DIM] = {
    {-24.396309f, -9.176207f, -8.753249f, -8.594545f, -8.324125f, -8.215728f, -10.330769f, -11.908634f, -9.970103f, -16.590910f, -19.286430f, -16.541848f, -16.063155f, -10.757898f, -10.746653f, -11.880066f},
    {24.400906f, 9.176195f, 8.755202f, 8.596968f, 8.325470f, 8.216835f, 10.332349f, 11.909443f, 9.969301f, 16.593564f, 19.289814f, 16.545818f, 16.067021f, 10.758740f, 10.748308f, 11.881746f},
    {6.824132f, 2.647379f, 1.686142f, 3.425470f, 3.236703f, 1.718208f, 3.907851f, 3.493980f, 3.577345f, 5.090572f, 5.534254f, 5.717570f, 2.421799f, 1.996650f, 1.946639f, 4.450541f},
    {1.014105f, 0.223966f, 0.236507f, 0.591128f, 0.514003f, 0.167469f, 0.618319f, 0.305513f, 0.490111f, 0.757431f, 0.795696f, 0.765997f, 0.286523f, 0.248281f, 0.239331f, 0.732195f}
};

// Biaixos de la capa 1 (capa oculta) [NN_HIDDEN_DIM]
float b1[NN_HIDDEN_DIM] = {30.391171f, 1.716021f, 2.655418f, 1.138473f, 3.576155f, 4.340716f, 8.732328f, -2.421601f, 6.232664f, -6.747334f, -8.964647f, 16.246005f, 18.227381f, -0.382128f, 8.235673f, -1.809057f};

// Pesos de la capa 2 (oculta a sortida) [NN_HIDDEN_DIM][NN_OUTPUT_DIM]
float W2[NN_HIDDEN_DIM][NN_OUTPUT_DIM] = {
    {72.157350f}, {62.868938f}, {62.959426f}, {62.844611f}, {62.762655f}, {62.851644f}, {62.516666f}, {63.100708f}, {62.514959f}, {63.683521f}, {64.182664f}, {63.634234f}, {64.534315f}, {62.814993f}, {63.385363f}, {63.239527f}
};

// Biaixos de la capa 2 (capa de sortida) [NN_OUTPUT_DIM]
float b2[NN_OUTPUT_DIM] = {-0.911061f};


// Funció d'activació Sigmoide
float sigmoid(float x) {
    return 1.0f / (1.0f + expf(-x)); // expf per a float
}

// Funció per realitzar la predicció de la Xarxa Neuronal
// input_features: Un array de NN_INPUT_DIM floats (T, error, integral, derivada)
// Retorna el valor predit del PWM
float predict_nn(float input_features[NN_INPUT_DIM]) {
    // 1. Escalat de les entrades (StandardScaler)
    float scaled_input[NN_INPUT_DIM];
    for (int i = 0; i < NN_INPUT_DIM; i++) {
        scaled_input[i] = (input_features[i] - scaler_mean[i]) / scaler_std[i];
    }

    // 2. Forward Pass - Capa Oculta (Input -> Hidden)
    float hidden_layer_output[NN_HIDDEN_DIM];
    for (int j = 0; j < NN_HIDDEN_DIM; j++) {
        float sum_hidden = 0.0f;
        for (int i = 0; i < NN_INPUT_DIM; i++) {
            sum_hidden += scaled_input[i] * W1[i][j];
        }
        sum_hidden += b1[j];
        hidden_layer_output[j] = sigmoid(sum_hidden); // Aplicar Sigmoide
    }

    // 3. Forward Pass - Capa de Sortida (Hidden -> Output)
    float output_layer_output[NN_OUTPUT_DIM];
    for (int k = 0; k < NN_OUTPUT_DIM; k++) {
        float sum_output = 0.0f;
        for (int j = 0; j < NN_HIDDEN_DIM; j++) {
            sum_output += hidden_layer_output[j] * W2[j][k];
        }
        sum_output += b2[k];
        output_layer_output[k] = sum_output; // Activació lineal per regressió
    }

    // Retorna el valor predit del PWM
    return output_layer_output[0];
}

#endif // NEURAL_NETWORK_H