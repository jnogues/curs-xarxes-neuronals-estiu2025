#ifndef NN_PARAMS_SIMPLE_H
#define NN_PARAMS_SIMPLE_H

// Parameters for the Simple Neural Network model
#define INPUT_DIM_SIMPLE 1
#define HIDDEN_DIM_SIMPLE 5
#define OUTPUT_DIM_SIMPLE 3

// Class names for reference
const char* CLASS_NAMES_SIMPLE[] = {
    "low",
    "medium",
    "high",
};

// W1 (shape: (1, 5), scale: 0.169313)
const int8_t W1_simple[5] = {
    0, 63, 127, 0, 0
};

const float W1_simple_scale = 0.169313f;

// b1 (shape: (1, 5), scale: 0.056304)
const int8_t b1_simple[5] = {
    0, -127, -107, 0, 0
};

const float b1_simple_scale = 0.056304f;

// W2 (shape: (5, 3), scale: 0.141567)
const int8_t W2_simple[15] = {
    0, 0, 0, -53, -18, 71, -127, 45, 82, 0, 0, 0, 0, 0, 0
};

const float W2_simple_scale = 0.141567f;

// b2 (shape: (1, 3), scale: 0.261817)
const int8_t b2_simple[3] = {
    77, 50, -127
};

const float b2_simple_scale = 0.261817f;

#endif // NN_PARAMS_SIMPLE_H
