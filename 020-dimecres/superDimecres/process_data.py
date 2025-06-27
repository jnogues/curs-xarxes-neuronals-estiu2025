# process_data.py

import pandas as pd

def process_teleplot_data(input_filename, output_filename):
    """
    Processa un fitxer de text amb format Teleplot i el converteix a CSV.
    Cada grup de dades (T, Setpoint, error, integral, derivada, pwm) es converteix en una fila.
    """
    
    # Ordre dels paràmetres que esperem a cada 'frame' de Teleplot
    # (assegura't que coincideix amb l'ordre del teu codi Arduino)
    expected_params = ['T', 'Setpoint', 'error', 'integral', 'derivada', 'pwm']
    
    # Diccionari temporal per emmagatzemar les dades d'un cicle de lectura
    current_data_point = {}
    
    # Llista per emmagatzemar tots els punts de dades processats
    all_data_points = []

    print(f"Processant el fitxer: {input_filename}")

    try:
        with open(input_filename, 'r') as f:
            for line_num, line in enumerate(f):
                line = line.strip() # Elimina espais en blanc i salts de línia

                if line.startswith('>'):
                    # Separa el nom del paràmetre del valor
                    parts = line[1:].split(':') # Elimina el '>' i separa per ':'
                    if len(parts) == 2:
                        param_name = parts[0].strip()
                        try:
                            param_value = float(parts[1].strip())
                        except ValueError:
                            print(f"Advertència: No s'ha pogut convertir el valor a flotant a la línia {line_num + 1}: '{line}'")
                            continue

                        # Si és un dels nostres paràmetres esperats, l'afegim al punt de dades actual
                        if param_name in expected_params:
                            current_data_point[param_name] = param_value
                        
                        # Quan tenim totes les dades d'un cicle, l'afegim a la llista
                        # Assumim que el 'pwm' és l'últim paràmetre del cicle de dades que ens interessa
                        if param_name == 'pwm' and len(current_data_point) == len(expected_params):
                            # Construïm la fila en l'ordre correcte
                            ordered_row = [current_data_point.get(p_name) for p_name in expected_params]
                            all_data_points.append(ordered_row)
                            current_data_point = {} # Reinicia per al següent punt de dades
    except FileNotFoundError:
        print(f"Error: El fitxer '{input_filename}' no s'ha trobat.")
        return
    except Exception as e:
        print(f"S'ha produït un error inesperat: {e}")
        return

    if not all_data_points:
        print("No s'han trobat dades vàlides per processar.")
        return

    # Converteix la llista de dades a un DataFrame de Pandas
    df = pd.DataFrame(all_data_points, columns=expected_params)
    
    # Guarda el DataFrame en un fitxer CSV
    df.to_csv(output_filename, index=False)
    
    print(f"Procés completat. S'han processat {len(df)} files i s'ha guardat a '{output_filename}'")

# --- Ús de la funció ---
input_file = 'dades_entrenament.txt'
output_csv_file = 'dades_entrenament.csv'

process_teleplot_data(input_file, output_csv_file)