#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
analise_comparativa.py
Script único que:
- lê logs (estrutura fornecida pelo usuário)
- calcula métricas por cenário/algoritmo
- gera 5 gráficos comparativos (estilo IEEE, matplotlib puro)
- exporta 5 tabelas LaTeX (uma por métrica)
Salva saídas em 'resultados_analise/'.
"""

import os
import re
import pandas as pd
import matplotlib.pyplot as plt

# ---------------- CONFIGURAÇÃO ----------------
# --- RENOMEADO PARA INGLÊS ---
LOG_FILES_BY_SCENARIO = {
    "150": {
        "RTT-B (Baseline)": "./RTT/V150/RTTV0/logFileClusteringAlgorithm.log",
        "RTT-H (Hesitation)": "./RTT/V150/RTTV1/logFileClusteringAlgorithm.log",
        "RTT-G (Grace Period)": "./RTT/V150/RTTV2/logFileClusteringAlgorithm.log",
        "RTT-HG (Combined)": "./RTT/V150/RTTV3/logFileClusteringAlgorithm.log",
    },
    "300": {
        "RTT-B (Baseline)": "./RTT/V300/RTTV0/logFileClusteringAlgorithm.log",
        "RTT-H (Hesitation)": "./RTT/V300/RTTV1/logFileClusteringAlgorithm.log",
        "RTT-G (Grace Period)": "./RTT/V300/RTTV2/logFileClusteringAlgorithm.log",
        "RTT-HG (Combined)": "./RTT/V300/RTTV3/logFileClusteringAlgorithm.log",
    },
    "450": {
        "RTT-B (Baseline)": "./RTT/V450/RTTV0/logFileClusteringAlgorithm.log",
        "RTT-H (Hesitation)": "./RTT/V450/RTTV1/logFileClusteringAlgorithm.log",
        "RTT-G (Grace Period)": "./RTT/V450/RTTV2/logFileClusteringAlgorithm.log",
        "RTT-HG (Combined)": "./RTT/V450/RTTV3/logFileClusteringAlgorithm.log",
    },
    "600": {
        "RTT-B (Baseline)": "./RTT/V600/RTTV0/logFileClusteringAlgorithm.log",
        "RTT-H (Hesitation)": "./RTT/V600/RTTV1/logFileClusteringAlgorithm.log",
        "RTT-G (Grace Period)": "./RTT/V600/RTTV2/logFileClusteringAlgorithm.log",
        "RTT-HG (Combined)": "./RTT/V600/RTTV3/logFileClusteringAlgorithm.log",
    }
}

SIMULATION_DURATION = 600.0
OUTPUT_DIR = "resultados_analise"
os.makedirs(OUTPUT_DIR, exist_ok=True)

# ---------------- FUNÇÕES DE PARSING ----------------
def parse_event_string(event_str):
    data = {}
    for part in event_str.strip().split(';'):
        if '=' in part:
            key, value = part.split('=', 1)
            key = key.lower().strip()
            value = value.strip()
            try:
                data[key] = float(value)
            except ValueError:
                data[key] = value
    return data

def parse_log_file(filepath, algorithm_name):
    """Retorna DataFrame com colunas: timestamp, node_id, algorithm, event, ...outros campos"""
    records = []
    line_regex = re.compile(r"([\d\.]+)s - .*? - Node #(\d+) : (.*)")
    try:
        with open(filepath, 'r') as f:
            for line in f:
                match = line_regex.match(line.strip())
                if match:
                    timestamp, node_id, event_str = match.groups()
                    record = {'timestamp': float(timestamp), 'node_id': int(node_id), 'algorithm': algorithm_name}
                    event_data = parse_event_string(event_str)
                    if 'event' in event_data:
                        event = str(event_data.pop('event')).strip().upper()
                        record['event'] = event
                        record.update(event_data)
                        records.append(record)
    except FileNotFoundError:
        print(f"[WARNING] File not found: {filepath}  (Algorithm: {algorithm_name})")
        return None

    if not records:
        print(f"[WARNING] No valid events in: {filepath}  (Algorithm: {algorithm_name})")
        return None

    return pd.DataFrame(records)

# ---------------- FUNÇÃO DE ANÁLISE ----------------
def analyze_metrics(df):
    """Recebe df (parseado) e retorna dicionário com métricas padronizadas."""
    if df is None or df.empty:
        return {
            'overhead_total': 0,
            'avg_cluster_lifetime': 0,
            'total_ch_elections': 0,
            'total_ch_renounces': 0,
            'avg_cluster_size': 0,
            'avg_rtt_ms': 0.0,
            'std_rtt_ms': 0.0
        }

    metrics = {}
    sim_duration = SIMULATION_DURATION

    packet_df = df[df['event'] == 'PACKET_SENT']
    metrics['overhead_total'] = int(len(packet_df))

    ch_events = df[df['event'].isin(['CH_ELECTED', 'CH_RENOUNCED'])].sort_values('timestamp')
    ch_lifetimes = []
    active_chs = {}
    if 'ch_id' in ch_events.columns:
        for _, row in ch_events.iterrows():
            try:
                ch_id = int(row['ch_id'])
            except Exception:
                continue
            if row['event'] == 'CH_ELECTED' and ch_id not in active_chs:
                active_chs[ch_id] = row['timestamp']
            elif row['event'] == 'CH_RENOUNCED' and ch_id in active_chs:
                duration = row['timestamp'] - active_chs.pop(ch_id)
                if duration >= 0:
                    ch_lifetimes.append(duration)
        for ch_id, start_time in active_chs.items():
            ch_lifetimes.append(max(0.0, sim_duration - start_time))

    metrics['avg_cluster_lifetime'] = float(sum(ch_lifetimes) / len(ch_lifetimes)) if ch_lifetimes else 0.0
    metrics['total_ch_elections'] = int(len(df[df['event'] == 'CH_ELECTED']))
    metrics['total_ch_renounces'] = int(len(df[df['event'] == 'CH_RENOUNCED']))

    if 'size' in df.columns:
        cluster_sizes = pd.to_numeric(df[df['event'] == 'CLUSTER_SIZE']['size'], errors='coerce').dropna()
        non_zero_cluster_sizes = cluster_sizes[cluster_sizes > 0]
        metrics['avg_cluster_size'] = float(non_zero_cluster_sizes.mean()) if not non_zero_cluster_sizes.empty else 0.0
    else:
        metrics['avg_cluster_size'] = 0.0

    if 'rtt' in df.columns:
        rtt_values = pd.to_numeric(df[df['event'] == 'RTT_MEASUREMENT']['rtt'], errors='coerce').dropna()
        if not rtt_values.empty:
            metrics['avg_rtt_ms'] = float(rtt_values.mean() * 1000.0)
            metrics['std_rtt_ms'] = float(rtt_values.std() * 1000.0)
        else:
            metrics['avg_rtt_ms'] = 0.0
            metrics['std_rtt_ms'] = 0.0
    else:
        metrics['avg_rtt_ms'] = 0.0
        metrics['std_rtt_ms'] = 0.0

    return metrics

# ---------------- PLOTAGEM (estilo IEEE, matplotlib puro) ----------------
def plot_comparative_lines(all_metrics_by_scenario, output_dir):
    """
    Gera gráficos de linha comparativos:
    - x = número de veículos (150,300,450,600)
    - y = métrica
    - uma linha por algoritmo
    """
    scenario_keys = sorted(all_metrics_by_scenario.keys(), key=lambda s: int(s))
    vehicle_counts = [int(k) for k in scenario_keys]

    first_scenario = all_metrics_by_scenario[scenario_keys[0]]
    algorithms = list(first_scenario.keys())

    # --- RENOMEADO PARA INGLÊS ---
    metrics_to_plot = {
        'avg_rtt_ms': ("Average RTT", "RTT (ms)"),
        'overhead_total': ("Total Overhead", "Number of Packets"),
        'total_ch_elections': ("Total CH Elections", "Number of Events"),
        'avg_cluster_lifetime': ("Average Cluster Duration", "Time (s)"),
        'avg_cluster_size': ("Average Cluster Size", "Number of Members"),
    }

    # Paleta para 4 algoritmos
    colors = ['#1b9e77', '#d95f02', '#7570b3', '#e7298a']
    markers = ['o', 's', '^', 'D']
    linestyles = ['-', '--', '-.', ':']

    for idx, (metric_key, (_, ylabel)) in enumerate(metrics_to_plot.items()):
        plt.figure(figsize=(8.0, 5.0))
        for i, algo in enumerate(algorithms):
            y = []
            for scen in scenario_keys:
                metrics = all_metrics_by_scenario[scen].get(algo, {})
                y.append(metrics.get(metric_key, 0.0))
            plt.plot(vehicle_counts, y,
                     label=algo,
                     marker=markers[i % len(markers)],
                     linestyle=linestyles[i % len(linestyles)],
                     linewidth=1.8,
                     markersize=6,
                     color=colors[i % len(colors)])

        # --- RENOMEADO PARA INGLÊS ---
        plt.xlabel("Number of Vehicles", fontsize=11)
        plt.ylabel(ylabel, fontsize=11)
        # --- TÍTULO REMOVIDO CONFORME SOLICITADO ---
        # plt.title(f"{title} — Comparative analysis between algorithms", fontsize=12)
        plt.xticks(vehicle_counts)
        plt.grid(False)
        plt.legend(fontsize=9, frameon=False)
        plt.tight_layout()
        filename = os.path.join(output_dir, f"comparative_{metric_key}.png")
        plt.savefig(filename, dpi=300)
        plt.close()
        print(f"[INFO] Saved: {filename}")

# ---------------- EXPORTA TABELAS LaTeX (uma por métrica) ----------------
def export_latex_tables(all_metrics_by_scenario, output_dir):
    """
    Gera uma tabela LaTeX por métrica.
    Cada tabela: linhas = cenários (150,300,450,600), colunas = algoritmos.
    """
    scenario_keys = sorted(all_metrics_by_scenario.keys(), key=lambda s: int(s))
    algorithms = list(next(iter(all_metrics_by_scenario.values())).keys())

    # --- RENOMEADO PARA INGLÊS ---
    metrics_to_export = {
        'avg_rtt_ms': ("Average RTT (ms)", "{:.2f}"),
        'overhead_total': ("Total Overhead (packets)", "{:.0f}"),
        'total_ch_elections': ("Total CH Elections", "{:.0f}"),
        'avg_cluster_lifetime': ("Average Cluster Duration (s)", "{:.2f}"),
        'avg_cluster_size': ("Average Cluster Size (members)", "{:.2f}")
    }

    for metric_key, (caption_title, fmt) in metrics_to_export.items():
        rows = []
        for scen in scenario_keys:
            # --- RENOMEADO PARA INGLÊS ---
            row = {'Scenario': scen}
            for algo in algorithms:
                val = all_metrics_by_scenario[scen].get(algo, {}).get(metric_key, 0.0)
                row[algo] = val
            rows.append(row)
        # --- RENOMEADO PARA INGLÊS ---
        df_table = pd.DataFrame(rows).set_index('Scenario')

        tex = df_table.to_latex(float_format=lambda x: fmt.format(x),
                                index=True, caption=caption_title,
                                label=f"tab:{metric_key}", column_format='l' + 'c'*len(algorithms),
                                escape=False)

        tex_full = ("% \\begin{table*}[t]\n"
                    "\\centering\n"
                    "\\begingroup\n"
                    "\\footnotesize\n"
                    f"{tex}\n"
                    "\\endgroup\n"
                    "% \\end{table*}\n")
        filename = os.path.join(output_dir, f"tabela_{metric_key}.tex")
        with open(filename, 'w') as f:
            f.write(tex_full)
        # --- RENOMEADO PARA INGLÊS ---
        print(f"[INFO] LaTeX table saved: {filename}")

# ---------------- MAIN ----------------
if __name__ == "__main__":
    # --- MENSAGENS TRADUZIDAS ---
    print("Starting comparative analysis...")

    all_metrics_by_scenario = {}

    for scenario_key, logs in sorted(LOG_FILES_BY_SCENARIO.items(), key=lambda kv: int(kv[0])):
        print(f"\n[PROCESSING] Scenario: {scenario_key} vehicles")
        scenario_metrics = {}
        for algo_name, filepath in logs.items():
            print(f"  - Reading: {algo_name}  -> {filepath}")
            df = parse_log_file(filepath, algo_name)
            metrics = analyze_metrics(df)
            scenario_metrics[algo_name] = metrics
            print(f"    -> Metrics: Average RTT = {metrics.get('avg_rtt_ms',0):.2f} ms, Overhead = {metrics.get('overhead_total',0)}")
        all_metrics_by_scenario[scenario_key] = scenario_metrics

    print("\nGenerating comparative graphs...")
    plot_comparative_lines(all_metrics_by_scenario, OUTPUT_DIR)

    print("\nExporting LaTeX tables...")
    export_latex_tables(all_metrics_by_scenario, OUTPUT_DIR)

    print("\nAnalysis complete. Check the directory:", OUTPUT_DIR)