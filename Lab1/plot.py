import matplotlib.pyplot as plt
import sys
import os

MIN_TIME = 0.1

def read_data(filepath):
    data = {'bubble': [], 'heap': [], 'quick': [], 'std_sort': []}
    
    with open(filepath, 'r') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            parts = [p.strip() for p in line.split(',')]
            if len(parts) != 3:
                continue
            size_str, algo_str, time_str = parts
            try:
                size = int(size_str)
                time_val = int(time_str)
            except ValueError:
                continue
            
            if time_val == 0:
                time_val = MIN_TIME
            
            if algo_str in data:
                data[algo_str].append((size, time_val))
    
    result = {}
    for algo, pairs in data.items():
        if pairs:
            sizes, times = zip(*sorted(pairs))
            result[algo] = (list(sizes), list(times))
    return result

def plot_combined(data):
    plt.figure(figsize=(10, 6))
    
    styles = {
        'bubble': {'color': 'red', 'marker': 'o', 'label': 'Bubble sort'},
        'heap': {'color': 'blue', 'marker': 's', 'label': 'Heap sort'},
        'quick': {'color': 'green', 'marker': '^', 'label': 'Quick sort'},
        'std_sort': {'color': 'purple', 'marker': 'd', 'label': 'std::sort'}
    }
    
    for algo, (sizes, times) in data.items():
        if algo in styles:
            style = styles[algo]
            plt.plot(sizes, times,
                     color=style['color'],
                     marker=style['marker'],
                     linestyle='-',
                     linewidth=1.5,
                     markersize=5,
                     label=style['label'])
    
    plt.xscale('log')
    plt.yscale('log')
    plt.xlabel('Размер массива (N)', fontsize=12)
    plt.ylabel('Время (усл. ед.)', fontsize=12)
    plt.title('Сравнение всех алгоритмов сортировки (логарифмический масштаб)', fontsize=14)
    plt.grid(True, which='both', linestyle='--', alpha=0.7)
    plt.legend(loc='upper left')
    plt.tight_layout()
    plt.show()

def plot_individual(data):
    names = {
        'bubble': 'Bubble sort',
        'heap': 'Heap sort',
        'quick': 'Quick sort',
        'std_sort': 'std::sort'
    }
    
    colors = {
        'bubble': 'red',
        'heap': 'blue',
        'quick': 'green',
        'std_sort': 'purple'
    }
    
    for algo, (sizes, times) in data.items():
        plt.figure(figsize=(8, 5))
        plt.plot(sizes, times,
                 color=colors.get(algo, 'black'),
                 marker='o',
                 linestyle='-',
                 linewidth=2,
                 markersize=6)
        
        plt.xscale('log')
        plt.yscale('log')
        plt.xlabel('Размер массива (N)', fontsize=12)
        plt.ylabel('Время (усл. ед.)', fontsize=12)
        plt.title(f'{names.get(algo, algo)} (логарифмический масштаб)', fontsize=14)
        plt.grid(True, which='both', linestyle='--', alpha=0.7)
        plt.tight_layout()
        plt.show()

def save_individual(data, output_dir="."):
    names = {
        'bubble': 'bubble_sort',
        'heap': 'heap_sort',
        'quick': 'quick_sort',
        'std_sort': 'std_sort'
    }
    
    colors = {
        'bubble': 'red',
        'heap': 'blue',
        'quick': 'green',
        'std_sort': 'purple'
    }
    
    for algo, (sizes, times) in data.items():
        plt.figure(figsize=(8, 5))
        plt.plot(sizes, times,
                 color=colors.get(algo, 'black'),
                 marker='o',
                 linestyle='-',
                 linewidth=2,
                 markersize=6)
        
        plt.xscale('log')
        plt.yscale('log')
        plt.xlabel('Размер массива (N)', fontsize=12)
        plt.ylabel('Время (усл. ед.)', fontsize=12)
        plt.title(f'{names.get(algo, algo)} (логарифмический масштаб)', fontsize=14)
        plt.grid(True, which='both', linestyle='--', alpha=0.7)
        
        filename = os.path.join(output_dir, f"{names.get(algo, algo)}_loglog.png")
        plt.savefig(filename, dpi=150, bbox_inches='tight')
        print(f"Сохранён график: {filename}")
        plt.close()

if __name__ == "__main__":

    if len(sys.argv) > 1:
        filepath = sys.argv[1]
    else:
        script_dir = os.path.dirname(os.path.abspath(__file__))
        filepath = os.path.join(script_dir, "results.txt")
    
    if not os.path.exists(filepath):
        print(f"Ошибка: файл не найден - {filepath}")
        print(f"Использование: python {sys.argv[0]} [путь_к_results.txt]")
        sys.exit(1)
    
    data = read_data(filepath)
    
    print("Построение общего графика...")
    plot_combined(data)

    print("Построение отдельных графиков для каждой сортировки...")
    plot_individual(data)
    