import pandas as pd

# --- CONFIGURAÇÃO ---
# 1. Coloque os IDs dos veículos que você quer encontrar
VEHICLE_IDS_TO_FIND = [46, 92, 86, 103, 134, 96, 111, 67, 112, 130]

# 2. Especifique o caminho para o seu arquivo random.txt
RANDOM_TXT_PATH = 'random.txt' 
# --- FIM DA CONFIGURAÇÃO ---

def analyze_vehicle_data():
    """
    Lê o arquivo random.txt, extrai os dados para os IDs especificados
    e os imprime em um formato de tabela.
    """
    try:
        df = pd.read_csv(RANDOM_TXT_PATH, sep=r'\s+', header=None, index_col=0)
    except FileNotFoundError:
        print(f"ERRO: O arquivo '{RANDOM_TXT_PATH}' não foi encontrado.")
        return
    except Exception as e:
        print(f"Ocorreu um erro ao ler o arquivo: {e}")
        return
        
    results = pd.DataFrame(index=df.index)
    found_ids = []
    
    for vehicle_id in VEHICLE_IDS_TO_FIND:
        # CORREÇÃO: O dado para o veículo com ID 'X' está na coluna de índice 'X + 1'
        column_to_access = vehicle_id + 1
        
        if column_to_access in df.columns:
            results[f'Vehicle_{vehicle_id}'] = df[column_to_access]
            found_ids.append(vehicle_id)
        else:
            print(f"Aviso: Veículo com ID {vehicle_id} (coluna {column_to_access}) não encontrado no arquivo.")
            
    if not found_ids:
        print("\nNenhum dos veículos especificados foi encontrado.")
        return

    print("\n--- Dados dos Veículos Encontrados (CORRIGIDO) ---")
    print(results.transpose())


if __name__ == "__main__":
    analyze_vehicle_data()