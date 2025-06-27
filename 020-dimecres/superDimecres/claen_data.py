import pandas as pd
import numpy as np

# Definir el nom del fitxer d'entrada i del fitxer de sortida
input_filename = 'dades_entrenament.csv'
output_filename = 'dades_entrenament_netejes.csv'

# Definir els límits per a la columna 'pwm'
PWM_MIN = 0
PWM_MAX = 1023

print(f"Iniciant el procés de neteja de dades des de '{input_filename}'...")

try:
    # 1. Carregar les dades
    df = pd.read_csv(input_filename)
    print(f"Dades carregades correctament. Número de mostres inicial: {len(df)}")
    print("Primeres 5 files de les dades originals:")
    print(df.head())
    print("\nEstadístiques descriptives de les dades originals:")
    print(df.describe())

    # 2. Gestió de valors NaN i Infinits
    # Convertir infinits a NaN i després eliminar totes les files amb NaN
    initial_rows = len(df)
    if np.isinf(df.values).any():
        print("\nADVERTÈNCIA: S'han trobat valors infinits al DataFrame. Reemplaçant per NaN.")
        df.replace([np.inf, -np.inf], np.nan, inplace=True)

    if df.isnull().values.any():
        print("\nADVERTÈNCIA: S'han trobat valors NaN al DataFrame. Eliminant files amb NaN.")
        print(df.isnull().sum()) # Mostra quants NaN hi ha per columna
        df.dropna(inplace=True)

    rows_after_nan_inf = len(df)
    if initial_rows > rows_after_nan_inf:
        print(f"S'han eliminat {initial_rows - rows_after_nan_inf} files a causa de valors NaN/Inf.")
    else:
        print("No s'han trobat valors NaN/Inf per eliminar.")


    # 3. Neteja de la columna 'pwm' (Clamping/Capping)
    print(f"\nRealitzant neteja de la columna 'pwm' per limitar al rang [{PWM_MIN}, {PWM_MAX}]...")
    original_pwm_max = df['pwm'].max()
    original_pwm_min = df['pwm'].min()
    
    # Aplica el 'clamping' (limitació)
    df['pwm'] = df['pwm'].clip(lower=PWM_MIN, upper=PWM_MAX)
    
    cleaned_pwm_max = df['pwm'].max()
    cleaned_pwm_min = df['pwm'].min()

    if original_pwm_max > PWM_MAX or original_pwm_min < PWM_MIN:
        print(f"S'han limitat valors de 'pwm' fora del rang. Max original: {original_pwm_max:.2f}, Min original: {original_pwm_min:.2f}")
        print(f"Nous valors de 'pwm': Max {cleaned_pwm_max:.2f}, Min {cleaned_pwm_min:.2f}")
    else:
        print("No s'han trobat valors de 'pwm' fora de rang per limitar.")

    # 4. Estadístiques descriptives de les dades netejades
    print("\nEstadístiques descriptives de les dades DESPRÉS de la neteja:")
    print(df.describe())

    # 5. Guardar les dades netejades en un nou fitxer CSV
    df.to_csv(output_filename, index=False) # index=False per no guardar l'índex de Pandas com una columna
    print(f"\nProcés de neteja completat. Dades netejades guardades a '{output_filename}'.")

except FileNotFoundError:
    print(f"Error: El fitxer '{input_filename}' no s'ha trobat. Assegura't que està al mateix directori.")
except Exception as e:
    print(f"Ha ocorregut un error durant la neteja de dades: {e}")