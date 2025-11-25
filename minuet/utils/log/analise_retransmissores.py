import matplotlib.pyplot as plt
import re
import os
import pandas as pd

def parse_retransmitter_logs(filepath, bs_id, event_id):
    """
    Lê um arquivo de log, encontra as mensagens para uma BS e evento específicos,
    e conta quantas vezes cada 'From' ID entregou uma mensagem.
    Retorna um dicionário: {retransmitter_id: count, ...}
    """
    print(f"Analisando retransmissores em: {filepath}...")
    
    retransmitter_counts = {}
    
    log_pattern = re.compile(
        r'Node #(?P<bs_id>\d+): '
        r'.*?From = (?P<from_id>\d+)'
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
                        from_id = int(data['from_id'])
                        retransmitter_counts[from_id] = retransmitter_counts.get(from_id, 0) + 1
    except FileNotFoundError:
        print(f"ERRO: Arquivo não encontrado: {filepath}")
        
    print(f"Contagens encontradas: {retransmitter_counts}")
    return retransmitter_counts

def plot_stacked_bar_chart(all_data, methods_in_order):
    """
    Cria um gráfico de barras empilhadas mostrando a contribuição de cada método
    para as entregas de cada veículo retransmissor.
    """
    # Usando pandas para facilitar a manipulação e plotagem dos dados
    df = pd.DataFrame(all_data).fillna(0).astype(int)
    
    # Garante que as colunas (métodos) estejam na ordem desejada
    df = df.reindex(columns=methods_in_order, fill_value=0)
    
    # Soma as contribuições para ordenar os retransmissores pela contagem total
    df['total'] = df.sum(axis=1)
    df = df.sort_values('total', ascending=False)
    df = df.drop(columns='total')

    if df.empty:
        print("Nenhum dado de retransmissor para plotar.")
        return

    fig, ax = plt.subplots(figsize=(14, 8))
    
    # Cores correspondentes aos métodos
    colors = {'AHP': '#1f77b4', 'PROMETHEE': '#ff7f0e', 'TOPSIS': '#2ca02c', 'BORDA': '#d62728'}
    
    # Plotando as barras empilhadas
    df.plot(kind='bar', stacked=True, ax=ax, color=[colors.get(c, '#808080') for c in df.columns])

    # Adicionando rótulos de total no topo de cada barra empilhada
    for i, total in enumerate(df.sum(axis=1)):
        ax.text(i, total + 1, str(total), ha='center', weight='bold')

    ax.set_ylabel('Nº de Datagramas Entregues à RSU', fontsize=14)
    ax.set_xlabel('ID do Veículo Retransmissor', fontsize=14)
    
    ax.tick_params(axis='x', rotation=45, labelsize=11)
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)
    ax.yaxis.grid(True, linestyle='--', which='major', color='grey', alpha=0.6)
    
    ax.legend(title='Método de Decisão', fontsize=11)
    
    if not df.empty:
        ax.set_ylim(0, df.sum(axis=1).max() * 1.15)

    fig.tight_layout()

    output_filename = "grafico_retransmissores.png"
    plt.savefig(output_filename, dpi=300)
    print(f"\nGráfico de retransmissores salvo como '{output_filename}'")
    
    plt.show()


# --- BLOCO PRINCIPAL DE EXECUÇÃO ---
if __name__ == "__main__":
    
    # =======================  CONFIGURAÇÃO  =========================
    BASE_STATION_ID = 300 
    EVENT_ID_TO_ANALYZE = 0 
    BASE_LOG_PATH = "." 
    METHODS = ["AHP", "PROMETHEE", "TOPSIS", "BORDA"]
    # =================================================================

    # Dicionário para armazenar todos os dados: { 'AHP': {from_id: count}, 'TOPSIS': {from_id: count}, ... }
    all_retransmitter_data = {}
    
    for method in METHODS:
        log_path = os.path.join(BASE_LOG_PATH, method, "logFileBaseStation.log")
        
        if not os.path.exists(log_path):
            print(f"\nAviso: O arquivo '{log_path}' não foi encontrado. Pulando o método {method}.")
            continue
        
        retransmitter_counts = parse_retransmitter_logs(log_path, BASE_STATION_ID, EVENT_ID_TO_ANALYZE)
        all_retransmitter_data[method] = retransmitter_counts
        
    if all_retransmitter_data:
        plot_stacked_bar_chart(all_retransmitter_data, METHODS)
    else:
        print("\nNenhum dado para plotar. Verifique os caminhos e IDs na seção de CONFIGURAÇÃO.")