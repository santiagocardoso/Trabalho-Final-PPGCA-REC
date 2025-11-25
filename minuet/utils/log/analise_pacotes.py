import matplotlib.pyplot as plt
import re
import os
import numpy as np

def parse_log_file(filepath, bs_id, event_id):
    print(f"Analisando o arquivo: {filepath}...")
    
    message_count = 0
    
    log_pattern = re.compile(
        r'Node #(?P<bs_id>\d+): '
        r'.*?EventId = (?P<event_id>\d+)'
    )

    try:
        with open(filepath, 'r') as f:
            for line in f:
                match = log_pattern.search(line)
                if match:
                    data = match.groupdict()
                    current_bs_id = int(data['bs_id'])
                    current_event_id = int(data['event_id'])
                    
                    if current_bs_id == bs_id and current_event_id == event_id:
                        message_count += 1

    except FileNotFoundError:
        print(f"ERRO: Arquivo não encontrado: {filepath}")
        return 0
        
    print(f"Análise concluída. Total de mensagens recebidas: {message_count}")
    return message_count

def plot_bar_chart(results):
    methods_in_order = ["AHP", "PROMETHEE", "TOPSIS", "BORDA"]
    
    methods_to_plot = [m for m in methods_in_order if m in results]
    counts = [results.get(m, 0) for m in methods_to_plot]

    fig, ax = plt.subplots(figsize=(10, 7))
    
    colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728']
    bars = ax.bar(methods_to_plot, counts, color=colors[:len(methods_to_plot)])

    ax.bar_label(bars, padding=3, weight='bold', fontsize=12)

    ax.set_ylabel('Nº de Datagramas Recebidos', fontsize=14)
    ax.set_xlabel('Método de Decisão', fontsize=14)
    
    ax.tick_params(axis='x', labelsize=12)
    ax.tick_params(axis='y', labelsize=11)
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)
    ax.yaxis.grid(True, linestyle='--', which='major', color='grey', alpha=0.6)
    
    if counts:
        ax.set_ylim(0, max(counts) * 1.20)
    else:
        ax.set_ylim(0, 10)

    fig.tight_layout()

    output_filename = "grafico_pacotes.png"
    plt.savefig(output_filename, dpi=300)
    print(f"\nGráfico salvo como '{output_filename}'")
    
    plt.show()

if __name__ == "__main__":
    
    BASE_STATION_ID = 300
    EVENT_ID_TO_ANALYZE = 0 
    BASE_LOG_PATH = "." 
    METHODS = ["AHP", "PROMETHEE", "TOPSIS", "BORDA"]

    all_results = {}
    
    for method in METHODS:
        log_path = os.path.join(BASE_LOG_PATH, method, "logFileBaseStation.log")
        
        if not os.path.exists(log_path):
            print(f"\nAviso: O arquivo '{log_path}' não foi encontrado. Pulando o método {method}.")
            continue
        
        count = parse_log_file(log_path, BASE_STATION_ID, EVENT_ID_TO_ANALYZE)
        all_results[method] = count
        
    if all_results:
        plot_bar_chart(all_results)
    else:
        print("\nNenhum dado para plotar. Verifique os caminhos e IDs na seção de CONFIGURAÇÃO.")