# -*- coding: utf-8 -*-
"""
Построение графиков сравнения методов поиска
по ключу "author" в массиве книг библиотеки
"""

import matplotlib.pyplot as plt
import csv
import os

# Автоматически определяем путь к папке со скриптом
script_dir = os.path.dirname(os.path.abspath(__file__))
csv_path = os.path.join(script_dir, 'search_results.csv')

# Списки для данных
sizes = []
linear, bst, rbt, hsh, mmap = [], [], [], [], []
collisions = []

# Чтение CSV-файла
with open(csv_path, 'r', encoding='utf-8') as f:
    reader = csv.DictReader(f)
    for row in reader:
        sizes.append(int(row['size']))
        linear.append(float(row['linear_ms']))
        bst.append(float(row['bst_ms']))
        rbt.append(float(row['rbt_ms']))
        hsh.append(float(row['hash_ms']))
        mmap.append(float(row['multimap_ms']))
        collisions.append(int(row['collisions']))

print(f"Загружено {len(sizes)} точек данных")

# Настройка стиля
plt.rcParams['font.size'] = 12

# ==================== ГРАФИК 1: Сравнение времени поиска (log-log) ====================
plt.figure(figsize=(12, 8))
plt.loglog(sizes, linear, 'o-', color='red', label='Линейный поиск O(n)', 
           linewidth=2.5, markersize=8)
plt.loglog(sizes, bst, 's-', color='orange', label='Бинарное дерево (BST)', 
           linewidth=2.5, markersize=8)
plt.loglog(sizes, rbt, '^-', color='green', label='Красно-чёрное дерево (RBT)', 
           linewidth=2.5, markersize=8)
plt.loglog(sizes, hsh, 'D-', color='blue', label='Хеш-таблица', 
           linewidth=2.5, markersize=8)
plt.loglog(sizes, mmap, 'v-', color='purple', label='std::multimap', 
           linewidth=2.5, markersize=8)

plt.xlabel('Размер массива (количество книг)', fontsize=14, fontweight='bold')
plt.ylabel('Время поиска (мс)', fontsize=14, fontweight='bold')
plt.title('Сравнение времени поиска по ключу "author"', fontsize=16, fontweight='bold')
plt.grid(True, which='both', linestyle='--', alpha=0.5)
plt.legend(fontsize=12, loc='upper left')
plt.tight_layout()
plt.savefig(os.path.join(script_dir, 'search_time.png'), dpi=300, bbox_inches='tight')
print("Сохранён: search_time.png")

# ==================== ГРАФИК 2: Коллизии хеш-таблицы ====================
plt.figure(figsize=(12, 8))
plt.plot(sizes, collisions, 'o-', color='darkred', linewidth=2.5, 
         markersize=8, markerfacecolor='red')

plt.xlabel('Размер массива (количество книг)', fontsize=14, fontweight='bold')
plt.ylabel('Количество коллизий', fontsize=14, fontweight='bold')
plt.title('Зависимость числа коллизий хеш-таблицы от размера массива\n(хеш-функция djb2, метод цепочек)', 
          fontsize=16, fontweight='bold')
plt.grid(True, linestyle='--', alpha=0.5)

# Подписи значений
for i, (size, col) in enumerate(zip(sizes, collisions)):
    if i % 2 == 0:
        plt.annotate(f'{col:,}'.replace(',', ' '), 
                    xy=(size, col), xytext=(5, 10), 
                    textcoords='offset points', fontsize=9, alpha=0.8)

plt.tight_layout()
plt.savefig(os.path.join(script_dir, 'collisions.png'), dpi=300, bbox_inches='tight')
print("Сохранён: collisions.png")

# Статистика
print(f"\nРазмеры: от {min(sizes):,} до {max(sizes):,}")
print(f"Коллизий на {max(sizes):,}: {collisions[-1]:,} ({(collisions[-1]/max(sizes))*100:.1f}%)")

plt.show()