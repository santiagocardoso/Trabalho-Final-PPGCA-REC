import re
import os
import pandas as pd
import plotly.graph_objects as go

def parse_flow_data(filepath, bs_id, event_id):
    """
    Lê o log da BaseStation e extrai as tuplas de fluxo: (MonitorId, From_Id).
    Conta a frequência de cada fluxo.
    Retorna um dicionário: {(monitor_id, from_id): count, ...}
    """
    print(f"Analisando fluxos em: {filepath}...")
    
    flows = {}
    
    log_pattern = re.compile(
        r'Node #(?P<bs_id>\d+): '
        r'.*?From = (?P<from_id>\d+)'
        r'.*?MonitorId = (?P<monitor_id>\d+)'
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
                        monitor_id = int(data['monitor_id'])
                        from_id = int(data['from_id'])
                        
                        # A chave é a tupla (origem, entregador_final)
                        flow_key = (monitor_id, from_id)
                        flows[flow_key] = flows.get(flow_key, 0) + 1
                        
    except FileNotFoundError:
        print(f"ERRO: Arquivo não encontrado: {filepath}")
        
    print(f"Fluxos encontrados: {flows}")
    return flows

def plot_sankey_diagram(all_data, methods_in_order, bs_id):
    """
    Cria um Diagrama de Sankey mostrando o fluxo de mensagens
    do nó detector para o nó entregador final.
    """
    if not all_data:
        print("Nenhum dado de fluxo para plotar.")
        return

    # Processar todos os dados para criar a estrutura do Sankey
    sources = []
    targets = []
    values = []
    labels = []
    node_map = {} # Mapeia ID do nó para um índice numérico

    def get_node_index(node_id, node_label_prefix=""):
        label = f"{node_label_prefix}{node_id}"
        if label not in node_map:
            node_map[label] = len(labels)
            labels.append(label)
        return node_map[label]

    # Adicionar a RSU como o destino final
    bs_index = get_node_index(bs_id, "RSU ")

    # Consolidar os fluxos de todos os métodos
    consolidated_flows = {}
    for method, flows in all_data.items():
        for (monitor_id, from_id), count in flows.items():
            key = (monitor_id, from_id)
            consolidated_flows[key] = consolidated_flows.get(key, 0) + count

    # Construir as listas para o Sankey
    for (monitor_id, from_id), count in consolidated_flows.items():
        # Fluxo do Detector -> Entregador
        detector_index = get_node_index(monitor_id, "Detector ")
        retransmitter_index = get_node_index(from_id, "Retransmissor ")
        
        sources.append(detector_index)
        targets.append(retransmitter_index)
        values.append(count)
        
        # Fluxo do Entregador -> RSU
        sources.append(retransmitter_index)
        targets.append(bs_index)
        values.append(count)


    # Criar a figura do Plotly
    fig = go.Figure(data=[go.Sankey(
        node=dict(
            pad=15,
            thickness=20,
            line=dict(color="black", width=0.5),
            label=labels,
            color="blue"
        ),
        link=dict(
            source=sources,
            target=targets,
            value=values
        ))])

    fig.update_layout(title_text="Fluxo de Retransmissão de Mensagens (Detector -> Entregador -> RSU)", font_size=12)
    
    output_filename = "grafico_fluxo_sankey.html"
    fig.write_html(output_filename)
    print(f"\nGráfico de Sankey salvo como '{output_filename}'. Abra este arquivo em um navegador.")
    # fig.show() # Descomente se quiser que o gráfico abra automaticamente

# --- BLOCO PRINCIPAL DE EXECUÇÃO ---
if __name__ == "__main__":
    
    # =======================  CONFIGURAÇÃO  =========================
    BASE_STATION_ID = 300
    EVENT_ID_TO_ANALYZE = 0 
    BASE_LOG_PATH = "." 
    METHODS = ["AHP", "PROMETHEE", "TOPSIS", "BORDA"]
    # =================================================================

    all_flow_data = {}
    
    for method in METHODS:
        log_path = os.path.join(BASE_LOG_PATH, method, "logFileBaseStation.log")
        
        if not os.path.exists(log_path):
            print(f"\nAviso: O arquivo '{log_path}' não foi encontrado. Pulando o método {method}.")
            continue
        
        flow_counts = parse_flow_data(log_path, BASE_STATION_ID, EVENT_ID_TO_ANALYZE)
        all_flow_data[method] = flow_counts
        
    if all_flow_data:
        # Para este gráfico, vamos consolidar os fluxos de todos os métodos
        plot_sankey_diagram(all_flow_data, METHODS, BASE_STATION_ID)
    else:
        print("\nNenhum dado para plotar.")