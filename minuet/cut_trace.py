import re
import os
from collections import defaultdict

# --- CONFIGURAÇÕES ---
START_TIME = 72000.0
END_TIME = 72900.0
ACTIVITY_INPUT_FILE = 'TraceActivity.tcl'
MOBILITY_INPUT_FILE = 'TraceMobility.tcl'
OUTPUT_SUFFIX = '_cut'
# --- FIM DAS CONFIGURAÇÕES ---

def find_fully_contained_nodes(activity_file, start_t, end_t):
    """
    Encontra IDs de nós cujo 'start' E 'stop' ocorrem dentro da janela de tempo.
    Esta é a etapa crucial para garantir que apenas os veículos corretos sejam selecionados.
    """
    print(f"--- PASSO 1: Encontrando nós com START e STOP entre {start_t}s e {end_t}s ---")
    if not os.path.exists(activity_file):
        print(f"ERRO: Arquivo de atividade '{activity_file}' não encontrado. Abortando.")
        return None

    # Dicionário para armazenar os tempos de start e stop de cada nó
    # ex: {1248: {'start': 71374.0, 'stop': 71926.0}}
    node_events = defaultdict(dict)
    
    # Regex para extrair tempo, ID e tipo de evento (start/stop)
    activity_regex = re.compile(r'^\$ns_ at (\d+\.?\d*).*?\$g\((\d+)\) (start|stop)')

    with open(activity_file, 'r') as f:
        for line in f:
            match = activity_regex.search(line)
            if match:
                time, node_id_str, event_type = match.groups()
                node_id = int(node_id_str)
                node_events[node_id][event_type] = float(time)

    # Agora, filtre os nós que atendem à condição
    contained_ids = set()
    for node_id, events in node_events.items():
        if 'start' in events and 'stop' in events:
            if start_t <= events['start'] <= end_t and start_t <= events['stop'] <= end_t:
                contained_ids.add(node_id)
    
    print(f"Concluído. {len(contained_ids)} nós encontrados com ciclo de vida completo na janela de tempo.")
    return contained_ids

def process_and_filter_file(input_path, output_path, valid_ids, id_map, start_t):
    """
    Filtra as linhas para manter apenas nós válidos, remapeia seus IDs e normaliza o tempo.
    """
    print(f"\n--- PASSO 3: Processando '{input_path}' -> '{output_path}' ---")
    if not os.path.exists(input_path):
        print(f"AVISO: Arquivo de entrada '{input_path}' não encontrado. Pulando.")
        return

    # Regex para encontrar qualquer ID no formato $g(ID) ou $node_(ID)
    id_regex = re.compile(r'(\$(?:g|node_)\()(\d+)(\))')
    # Regex para encontrar o ID de um nó em qualquer linha
    node_id_finder_regex = re.compile(r'\$(?:g|node_)\((\d+)\)')

    def remap_id_in_line(match_obj):
        """Função auxiliar para re.sub que substitui um ID antigo por um novo."""
        prefix, old_id_str, suffix = match_obj.groups()
        old_id = int(old_id_str)
        # O ID deve estar no mapa, pois já filtramos
        new_id = id_map.get(old_id, old_id) 
        return f"{prefix}{new_id}{suffix}"

    lines_written = 0
    with open(input_path, 'r') as f_in, open(output_path, 'w') as f_out:
        for line in f_in:
            # 1. Verificar se a linha pertence a um nó válido
            found_ids_str = node_id_finder_regex.findall(line)
            if not found_ids_str:
                continue # Linha sem ID de nó, pular

            # Converte para int e verifica se algum dos IDs encontrados é um nó que queremos manter
            found_ids = {int(i) for i in found_ids_str}
            if not found_ids.intersection(valid_ids):
                continue # Nenhum dos nós nesta linha é válido, pular

            # 2. Se a linha é válida, remapeie todos os IDs nela
            processed_line = re.sub(id_regex, remap_id_in_line, line)

            # 3. Verifique se a linha tem um timestamp para normalizar
            time_match = re.search(r'(\$ns_ at )(\d+\.?\d*)', processed_line)
            if time_match:
                prefix = time_match.group(1)
                current_time = float(time_match.group(2))
                new_time = current_time - start_t
                # Substitui o tempo antigo pelo novo tempo normalizado
                processed_line = re.sub(r'(\$ns_ at )(\d+\.?\d*)', r'{}{:.4f}'.format(prefix, new_time), processed_line, 1)

            f_out.write(processed_line)
            lines_written += 1
            
    print(f"Concluído. {lines_written} linhas relevantes escritas.")

# --- BLOCO PRINCIPAL ---
if __name__ == "__main__":
    # PASSO 1: Encontrar os nós cujo ciclo de vida COMPLETO está na janela
    valid_node_ids = find_fully_contained_nodes(ACTIVITY_INPUT_FILE, START_TIME, END_TIME)
    
    if valid_node_ids:
        # PASSO 2: Criar o mapa de remapeamento de ID (antigo -> novo)
        sorted_ids = sorted(list(valid_node_ids))
        id_map = {old_id: new_id for new_id, old_id in enumerate(sorted_ids)}
        print(f"\n--- PASSO 2: Mapeando {len(valid_node_ids)} IDs antigos para novos IDs (0 a {len(valid_node_ids)-1}) ---")

        # Define os nomes dos arquivos de saída
        activity_output = ACTIVITY_INPUT_FILE.replace('.tcl', f'{OUTPUT_SUFFIX}.tcl')
        mobility_output = MOBILITY_INPUT_FILE.replace('.tcl', f'{OUTPUT_SUFFIX}.tcl')
        
        # PASSO 3: Processar ambos os arquivos com base nos IDs válidos e no mapa
        process_and_filter_file(ACTIVITY_INPUT_FILE, activity_output, valid_node_ids, id_map, START_TIME)
        process_and_filter_file(MOBILITY_INPUT_FILE, mobility_output, valid_node_ids, id_map, START_TIME)
        
        print(f"\n\nSucesso! Novos arquivos criados:")
        print(f"- {activity_output}")
        print(f"- {mobility_output}")
        print(f"\nLEMBRE-SE: O número total de nós para sua simulação agora é {len(valid_node_ids)}.")
    else:
        print("\nNenhum nó com ciclo de vida completo foi encontrado na janela de tempo especificada.")