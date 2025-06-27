import pandas as pd

# Definir el nom del fitxer d'entrada i del fitxer de sortida
input_csv_file = 'dades_entrenament.csv'
output_csv_file = 'dades_entrenament_sense_setpoint.csv'

print(f"Carregant dades des de '{input_csv_file}'...")

try:
    # Carregar el fitxer CSV original
    df = pd.read_csv(input_csv_file)
    print(f"Dades carregades correctament. Columnes originals: {df.columns.tolist()}")

    # Verificar si la columna 'Setpoint' existeix
    if 'Setpoint' in df.columns:
        # Eliminar la columna 'Setpoint'
        df_cleaned = df.drop(columns=['Setpoint'])
        print("Columna 'Setpoint' eliminada.")
        print(f"Noves columnes: {df_cleaned.columns.tolist()}")

        # Guardar el nou DataFrame en un fitxer CSV
        df_cleaned.to_csv(output_csv_file, index=False)
        print(f"Dades amb 'Setpoint' eliminat guardades a '{output_csv_file}'.")
    else:
        print("La columna 'Setpoint' no es troba al fitxer CSV. No s'ha fet cap canvi.")
        # Opcionalment, pots copiar l'original igualment si no es va trobar la columna
        # df.to_csv(output_csv_file, index=False)
        # print(f"El fitxer original s'ha copiat a '{output_csv_file}' sense canvis.")

except FileNotFoundError:
    print(f"Error: El fitxer '{input_csv_file}' no s'ha trobat. Assegura't que està al mateix directori.")
except Exception as e:
    print(f"Ha ocorregut un error: {e}")

print("Procés completat.")