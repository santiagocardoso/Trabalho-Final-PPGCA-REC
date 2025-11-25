import os
import collections
from itertools import combinations

def process_log_file(filepath):
    """
    Lê um arquivo de log, agrupa por timestamp e encontra o NodeID com o maior FinalScore para cada timestamp.
    Retorna um dicionário: {timestamp: best_node_id}
    """
    print(f"Processando arquivo: {filepath}...")
    
    decisions_at_time = collections.defaultdict(list)
    
    try:
        with open(filepath, 'r') as f:
            next(f)
            
            for line in f:
                try:
                    parts = line.strip().split(',')
                    if len(parts) < 3: # Precisa de pelo menos ns, ID, ICR
                        continue
                        
                    timestamp = int(parts[0])
                    node_id = int(parts[1])
                    final_score = float(parts[-1])
                    
                    decisions_at_time[timestamp].append({'node_id': node_id, 'score': final_score})
                except (ValueError, IndexError):
                    continue

    except FileNotFoundError:
        print(f"  - ERRO: Arquivo não encontrado: {filepath}")
        return {}

    best_choices = {}
    for timestamp, candidates in decisions_at_time.items():
        if candidates:
            best_candidate = max(candidates, key=lambda x: x['score'])
            best_choices[timestamp] = best_candidate['node_id']
            
    print(f"  - Análise concluída. {len(best_choices)} pontos de decisão encontrados.")
    return best_choices

# --- BLOCO PRINCIPAL DE EXECUÇÃO ---
if __name__ == "__main__":
    
    # =======================  CONFIGURAÇÃO  =========================
    LOG_FILES = {
        "AHP": "score_history_AHP.csv",
        "PROMETHEE": "score_history_PROMETHEE.csv",
        "TOPSIS": "score_history_TOPSIS.csv",
        "BORDA": "score_history_BORDA.csv" # <-- ADICIONADO AQUI
    }
    # =================================================================

    all_best_choices = {}
    all_timestamps = set()

    method_names = list(LOG_FILES.keys())
    for method in method_names:
        filename = LOG_FILES[method]
        if os.path.exists(filename):
            best_choices = process_log_file(filename)
            all_best_choices[method] = best_choices
            all_timestamps.update(best_choices.keys())
        else:
            print(f"Aviso: O arquivo '{filename}' não foi encontrado. O método {method} será ignorado.")

    if not all_best_choices:
        print("\nNenhum arquivo de log válido foi encontrado. Encerrando.")
    else:
        agreement_counts = collections.defaultdict(int)
        comparison_points = collections.defaultdict(int)
        
        # Gera todas as combinações de pares possíveis entre os métodos
        pairs_to_compare = list(combinations(all_best_choices.keys(), 2))
        
        for timestamp in sorted(list(all_timestamps)):
            methods_present = [m for m in all_best_choices if timestamp in all_best_choices[m]]
            
            # --- LÓGICA PARA CONCORDÂNCIA PAR A PAR ---
            if len(methods_present) >= 2:
                for method1, method2 in pairs_to_compare:
                    if method1 in methods_present and method2 in methods_present:
                        comparison_points[f"{method1}_vs_{method2}"] += 1
                        
                        choice1 = all_best_choices[method1][timestamp]
                        choice2 = all_best_choices[method2][timestamp]
                        
                        if choice1 == choice2:
                            agreement_counts[f"{method1}_vs_{method2}"] += 1
            
            # --- LÓGICA PARA CONCORDÂNCIA TOTAL (TODOS OS 4) ---
            if len(methods_present) == len(all_best_choices):
                comparison_points["Todos"] += 1
                
                choices = [all_best_choices[m][timestamp] for m in methods_present]
                # Verifica se todos os elementos da lista são iguais
                if all(c == choices[0] for c in choices):
                    agreement_counts["Todos"] += 1

        print("\n--- Relatório de Concordância na Seleção de Relay ---")
        
        # --- TABELA DE CONCORDÂNCIA PAR A PAR ---
        print("\n--- Concordância Par a Par ---")
        print("---------------------------------------------------")
        print("| Comparação          | Concordância  | Porcentagem |")
        print("---------------------------------------------------")
        
        for method1, method2 in pairs_to_compare:
            key = f"{method1}_vs_{method2}"
            points = comparison_points.get(key, 0)
            count = agreement_counts.get(key, 0)
            percentage = (count / points) * 100 if points > 0 else 0
            
            print(f"| {method1:<9} vs. {method2:<9} | {count:<13} | {percentage:10.2f}% |")
        
        print("---------------------------------------------------")

        # --- RESULTADO DA CONCORDÂNCIA TOTAL ---
        total_points = comparison_points.get("Todos", 0)
        if total_points > 0:
            print(f"\nTotal de pontos de decisão onde todos os {len(all_best_choices)} métodos estavam presentes: {total_points}\n")
            print("--- Concordância Total ---")
            print("-----------------------------------------------------------------")
            print("| Comparação                  | Concordância  | Porcentagem     |")
            print("-----------------------------------------------------------------")
            
            count = agreement_counts.get("Todos", 0)
            percentage = (count / total_points) * 100 if total_points > 0 else 0
            
            comparison_label = "Todos os Métodos"
            print(f"| {comparison_label:<27} | {count:<13} | {percentage:10.2f}%       |")
            print("-----------------------------------------------------------------")
        else:
            print(f"\nNenhum ponto de decisão encontrado onde todos os {len(all_best_choices)} métodos estivessem presentes.")