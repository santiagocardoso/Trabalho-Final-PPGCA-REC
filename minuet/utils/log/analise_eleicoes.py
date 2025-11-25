import pandas as pd
import matplotlib.pyplot as plt
import os
import numpy as np

def analyze_election_history(filepath):
    """
    Lê um arquivo de histórico de scores, identifica o vencedor em cada
    momento de decisão e conta o número de vitórias de cada veículo.
    """
    print(f"Analisando o arquivo: {filepath}...")
    try:
        # Lê o arquivo CSV
        df = pd.read_csv(filepath)

        # O nome da coluna de score é a última coluna do dataframe
        score_column = df.columns[-1]
        print(f"Usando a coluna '{score_column}' para determinar o vencedor.")

        # Para cada timestamp ('ns'), encontra o índice da linha com o maior score
        idx_winners = df.groupby('ns')[score_column].idxmax()

        # Seleciona as linhas dos vencedores
        winners_df = df.loc[idx_winners]

        # Conta quantas vezes cada ID de veículo aparece como vencedor
        election_counts = winners_df['ID'].value_counts()
        
        return election_counts

    except FileNotFoundError:
        print(f"ERRO: Arquivo não encontrado: {filepath}")
        return None
    except Exception as e:
        print(f"Ocorreu um erro ao processar o arquivo {filepath}: {e}")
        return None


def plot_election_chart(results, top_n=10):
    """
    Plota um gráfico de barras agrupadas mostrando os N veículos mais eleitos.
    """
    # Combina os resultados de todos os métodos em um único DataFrame
    # preenchendo com 0 os veículos não eleitos por um método.
    results_df = pd.DataFrame(results).fillna(0).astype(int)

    # Soma as vitórias de todos os métodos para encontrar os veículos mais relevantes
    results_df['Total'] = results_df.sum(axis=1)
    
    # Ordena pelo total e pega os top N
    top_vehicles_df = results_df.sort_values('Total', ascending=False).head(top_n)
    
    # Remove a coluna 'Total' antes de plotar
    top_vehicles_df = top_vehicles_df.drop(columns=['Total'])

    # --- Plotagem ---
    methods = top_vehicles_df.columns
    vehicle_ids = top_vehicles_df.index.astype(str)
    
    x = np.arange(len(vehicle_ids))  # Posições dos grupos de barras
    width = 0.20  # Largura de cada barra individual
    
    fig, ax = plt.subplots(figsize=(14, 8))

    # Cria as barras para cada método, deslocando a posição x
    for i, method in enumerate(methods):
        offset = width * (i - (len(methods) - 1) / 2)
        bars = ax.bar(x + offset, top_vehicles_df[method], width, label=method)
        ax.bar_label(bars, padding=3, fontsize=9)

    # Configurações do Gráfico
    ax.set_ylabel('Nº de Vezes Selecionado', fontsize=14)
    ax.set_xlabel('ID do Veículo', fontsize=14)
    ax.set_xticks(x)
    ax.set_xticklabels(vehicle_ids)
    ax.legend(title="Método de Decisão")
    
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)
    ax.yaxis.grid(True, linestyle='--', which='major', color='grey', alpha=0.6)

    fig.tight_layout()

    output_filename = "grafico_eleicoes.png"
    plt.savefig(output_filename, dpi=300)
    print(f"\nGráfico salvo como '{output_filename}'")
    
    plt.show()


if __name__ == "__main__":
    # CONFIGURAÇÃO
    # Adicione ou remova métodos conforme necessário.
    # BORDA é excluído pois não possui um arquivo de score próprio.
    METHODS_TO_ANALYZE = ["AHP", "PROMETHEE", "TOPSIS", "BORDA"]
    BASE_LOG_PATH = "." 

    all_election_counts = {}

    for method in METHODS_TO_ANALYZE:
        # Monta o caminho para o arquivo de log de cada método
        log_path = os.path.join(BASE_LOG_PATH, f"score_history_{method}.csv")
        
        counts = analyze_election_history(log_path)
        if counts is not None:
            all_election_counts[method] = counts
            
    if all_election_counts:
        plot_election_chart(all_election_counts, top_n=10)
    else:
        print("\nNenhum dado para plotar. Verifique os caminhos dos arquivos.")