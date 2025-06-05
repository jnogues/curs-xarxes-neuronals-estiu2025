// Xarxa neuronal 2-5-3-1 entrenada amb NumPy

const float W1[2][5] = {
  {-0.708814f, -0.564292f, 2.676348f, 3.721413f, -0.475335f},
  {1.697868f, 4.315132f, 3.891678f, -1.612409f, 2.305671f}
};
const float b1[5] = { -0.656386f, 0.308432f, 0.379782f, 0.964367f, 1.492287f };

const float W2[5][3] = {
  {-1.307182f, -0.590371f, -0.267415f},
  {-4.399492f, -2.716198f, -1.913359f},
  {-3.232391f, 1.348142f, -2.837018f},
  {-1.005698f, 3.687562f, -0.277330f},
  {-0.018852f, -3.379418f, -0.325920f}
};
const float b2[3] = { 0.915471f, 1.739337f, 0.854907f };

const float W3[3] = { -1.360327f, -2.043678f, 0.775924f };
const float b3 = 1.887695f;

const float X_mean[2] = { 0.448598f, 0.450350f };
const float X_std[2] = { 6.522366f, 6.522144f };
const float y_mean = 270.990654f;
const float y_std = 335.397184f;

/*

float tanh_approx(float x) {
  // Aproximació ràpida de tanh (millor si evites math.h en micro)
  if (x < -3.0f) return -1.0f;
  if (x >  3.0f) return  1.0f;
  return x * (27 + x * x) / (27 + 9 * x * x);
}

float predict(float error, float errorPrev) {
  // --- Normalitza entrada ---
  float x0 = (error     - X_mean[0]) / X_std[0];
  float x1 = (errorPrev - X_mean[1]) / X_std[1];

  // --- Capa 1 (2 -> 5) ---
  float a1[5];
  for (int i = 0; i < 5; ++i) {
    float z = W1[0][i] * x0 + W1[1][i] * x1 + b1[i];
    a1[i] = tanh_approx(z);
  }

  // --- Capa 2 (5 -> 3) ---
  float a2[3];
  for (int i = 0; i < 3; ++i) {
    float z = 0;
    for (int j = 0; j < 5; ++j)
      z += W2[j][i] * a1[j];
    a2[i] = tanh_approx(z + b2[i]);
  }

  // --- Capa 3 (3 -> 1) ---
  float z3 = 0;
  for (int i = 0; i < 3; ++i)
    z3 += W3[i] * a2[i];
  z3 += b3;

  // --- Desnormalitza sortida ---
  float y = z3 * y_std + y_mean;
  return y;
}


*/
