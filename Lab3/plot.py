import pandas as pd
import matplotlib.pyplot as plt

# Загружаем данные из CSV-файла
df = pd.read_csv('times.csv')

plt.figure(figsize=(10, 6))

# Строим графики для каждого метода
plt.plot(df['size'], df['method1'], marker='o', label='Метод 1: MidSquare (8 цифр)')
plt.plot(df['size'], df['method2'], marker='s', label='Метод 2: LCG + XOR')
plt.plot(df['size'], df['method3'], marker='^', label='Метод 3: Xorshift + умножение')
plt.plot(df['size'], df['std_mt19937'], marker='d', label='std::mt19937 (стандартный)')

# Подписи осей и заголовок
plt.xlabel('Количество сгенерированных чисел')
plt.ylabel('Время (секунды)')
plt.title('Сравнение производительности генераторов псевдослучайных чисел')

# Логарифмическая шкала для наглядности
plt.xscale('log')
plt.yscale('log')

# Легенда и сетка
plt.legend()
plt.grid(True, which='both', linestyle='--', alpha=0.7)

# Сохраняем график в файл и показываем
plt.savefig('speed_graph.png', dpi=150)
plt.show()