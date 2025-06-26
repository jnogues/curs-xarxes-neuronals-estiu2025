
import pandas as pd
import numpy as np

# Llegeix el fitxer CSV (ha de tenir columnes 'temperature' i 'pwm')
df = pd.read_csv('dataset50C.csv')

# Assegura tipus float
df['temperature'] = df['temperature'].astype(float)
df['pwm'] = df['pwm'].astype(float)

# Normalitza min-max
t_min, t_max = df['temperature'].min(), df['temperature'].max()
pwm_min, pwm_max = df['pwm'].min(), df['pwm'].max()

df['temperature'] = (df['temperature'] - t_min) / (t_max - t_min)
df['pwm'] = (df['pwm'] - pwm_min) / (pwm_max - pwm_min)

# Impressió en format np.array([...])
print("# Dades d'entrenament normalitzades")
print("training_data = np.array([")
for i, row in df.iterrows():
    comma = "," if i < len(df) - 1 else ""
    print(f"    [{row['temperature']:.4f}, {row['pwm']:.4f}]{comma}")
print("])")

# Mostra els valors de normalització
print("\n# Normalització:")
print(f"# temperature: min = {t_min}, max = {t_max}")
print(f"# pwm: min = {pwm_min}, max = {pwm_max}")
