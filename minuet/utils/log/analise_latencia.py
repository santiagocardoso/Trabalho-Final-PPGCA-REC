import matplotlib.pyplot as plt
import re
import os
import pandas as pd

def parse_creation_times(filepath, event_id_to_analyze):
    """
    Lê o log do DetectionLayer para encontrar o timestamp da PRIMEIRA detecção
    de um evento por cada nó.
    Retorna um dicionário: {(monitor_id, event_id): creation_time_ns}
    """
    print(f"Analisando tempos de criação em: {filepath}...")
    
    creation_times = {}
    
    # Padrão para a primeira detecção
    pattern = re.compile(
        r'^(?P<time_ns>\d+)ns - DetectionLayer - Node #(?P<node_id>\d+).*?: '
        r'Event \((?P<event_id>\d+)\) Detected'
    )

    try:
        with open(filepath, 'r') as f:
            for line in f:
                match = pattern.search(line)
                if match:
                    data = match.groupdict()
                    event_id = int(data['event_id'])
                    
                    if event_id == event_id_to_analyze:
                        node_id = int(data['node_id'])
                        time_ns = int(data['time_ns'])
                        
                        # Chave única para o evento (originador, id_evento)
                        event_key = (node_id, event_id)
                        
                        # Armazena apenas a PRIMEIRA vez que este evento foi detectado por este nó
                        if event_key not in creation_times:
                            creation_times[event_key] = time_ns
    except FileNotFoundError:
        print(f"ERRO: Arquivo não encontrado: {filepath}")
    
    print(f"Tempos de criação encontrados: {creation_times}")
    return creation_times

def calculate_latencies(filepath, bs_id, creation_times):
    """
    Lê o log da BaseStation, compara com os tempos de criação e calcula as latências
    para cada mensagem única.
    Retorna uma lista de latências em milissegundos.
    """
    print(f"Calculando latências em: {filepath}...")
    latencies = []
    processed_messages = set() # Para evitar calcular latência para duplicatas
    
    pattern = re.compile(
        r'^(?P<time_ns>\d+)ns - BASE STATION - Node #(?P<bs_id>\d+): '
        r'.*?MonitorId = (?P<monitor_id>\d+)'
        r'.*?Seq = (?P<seq>\d+)'
        r'.*?EventId = (?P<event_id>\d+)'
    )

    try:
        with open(filepath, 'r') as f:
            for line in f:
                match = pattern.search(line)
                if match:
                    data = match.groupdict()
                    current_bs_id = int(data['bs_id'])
                    
                    if current_bs_id == bs_id:
                        monitor_id = int(data['monitor_id'])
                        event_id = int(data['event_id'])
                        seq = int(data['seq'])
                        delivery_time_ns = int(data['time_ns'])
                        
                        event_key = (monitor_id, event_id)
                        message_key = (monitor_id, event_id, seq)

                        # Se temos o tempo de criação para este evento e ainda não processamos esta mensagem
                        if event_key in creation_times and message_key not in processed_messages:
                            creation_time_ns = creation_times[event_key]
                            
                            # Calcula a latência e converte para milissegundos
                            latency_ns = delivery_time_ns - creation_time_ns
                            latency_ms = latency_ns / 1_000_000.0
                            
                            if latency_ms >= 0: # Ignora casos anômalos
                                latencies.append(latency_ms)
                                processed_messages.add(message_key)
    except FileNotFoundError:
        print(f"ERRO: Arquivo não encontrado: {filepath}")

    print(f"Latências calculadas: {len(latencies)} mensagens.")
    return latencies

def plot_latency_boxplot(all_data, methods_in_order):
    """
    Cria um gráfico de boxplot para comparar a distribuição de latências entre os métodos.
    """
    # Filtra e ordena os dados para a plotagem
    methods_to_plot = [m for m in methods_in_order if m in all_data and all_data[m]]
    latency_data = [all_data[m] for m in methods_to_plot]

    if not latency_data:
        print("Nenhum dado de latência para plotar.")
        return

    fig, ax = plt.subplots(figsize=(12, 8))
    
    box = ax.boxplot(latency_data, patch_artist=True, showfliers=True) # showfliers=True para mostrar outliers

    colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728']
    for patch, color in zip(box['boxes'], colors):
        patch.set_facecolor(color)

    ax.set_ylabel('(milissegundos)', fontsize=14)
    ax.set_xlabel('Método de Decisão', fontsize=14)
    
    ax.set_xticklabels(methods_to_plot)
    ax.tick_params(axis='x', labelsize=12)
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)
    ax.yaxis.grid(True, linestyle='--', which='major', color='grey', alpha=0.6)

    fig.tight_layout()

    output_filename = "grafico_latencia.png"
    plt.savefig(output_filename, dpi=300)
    print(f"\nGráfico de latência salvo como '{output_filename}'")
    
    plt.show()


# --- BLOCO PRINCIPAL DE EXECUÇÃO ---
if __name__ == "__main__":
    
    # =======================  CONFIGURAÇÃO  =========================
    BASE_STATION_ID = 300
    EVENT_ID_TO_ANALYZE = 0 
    BASE_LOG_PATH = "." 
    METHODS = ["AHP", "PROMETHEE", "TOPSIS", "BORDA"]
    # =================================================================

    all_latency_data = {}
    
    for method in METHODS:
        print(f"\n--- Processando Método: {method} ---")
        detection_log_path = os.path.join(BASE_LOG_PATH, method, "logFileDetectionLayer.log")
        bs_log_path = os.path.join(BASE_LOG_PATH, method, "logFileBaseStation.log")

        if not os.path.exists(detection_log_path) or not os.path.exists(bs_log_path):
            print(f"Aviso: Um ou mais arquivos de log para o método '{method}' não foram encontrados. Pulando.")
            continue
        
        # 1. Parsear os tempos de criação
        creation_times = parse_creation_times(detection_log_path, EVENT_ID_TO_ANALYZE)
        
        # 2. Calcular as latências
        latencies = calculate_latencies(bs_log_path, BASE_STATION_ID, creation_times)
        
        all_latency_data[method] = latencies
        
    if all_latency_data:
        plot_latency_boxplot(all_latency_data, METHODS)
    else:
        print("\nNenhum dado de latência para plotar. Verifique os caminhos e IDs.")