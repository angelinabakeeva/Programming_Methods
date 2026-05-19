/**
 * @file
 * @brief Сравнение алгоритмов сортировки (пузырьковая, пирамидальная, быстрая, std::sort)
 * на наборах данных о книгах, загружаемых из CSV-файлов.
 *
 * Программа последовательно читает файлы library_books<размер>.csv,
 * выполняет для каждого набора сортировку четырьмя способами,
 * замеряет время выполнения в миллисекундах и дописывает результаты в файл results.txt.
 *
 * Формат файла результатов: размер, тип сортировки, время (мс).
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <chrono>
#include <algorithm>

using namespace std;
using namespace chrono;

/**
 * @brief Структура данных "Книга".
 *
 * Хранит автора, название, год издания и количество страниц.
 * Поддерживает сравнение по цепочке: автор -> название -> год.
 */
class Book {
public:
    string author; ///< Автор книги.
    string title;  ///< Название книги.
    int year;      ///< Год издания.
    int pages;     ///< Количество страниц.

    /**
     * @brief Сравнение "больше".
     * @param other Другая книга.
     * @return true, если текущая книга лексикографически больше (автор, название, год).
     */
    bool operator>(const Book& other) const {
        if (author != other.author) return author > other.author;
        if (title != other.title) return title > other.title;
        return year > other.year;
    }

    /**
     * @brief Сравнение "меньше".
     * @param other Другая книга.
     * @return true, если текущая книга лексикографически меньше.
     */
    bool operator<(const Book& other) const {
        if (author != other.author) return author < other.author;
        if (title != other.title) return title < other.title;
        return year < other.year;
    }

    /**
     * @brief "Больше или равно".
     * @param other Другая книга.
     * @return true, если не меньше.
     */
    bool operator>=(const Book& other) const {
        return !(*this < other);
    }

    /**
     * @brief "Меньше или равно".
     * @param other Другая книга.
     * @return true, если не больше.
     */
    bool operator<=(const Book& other) const {
        return !(*this > other);
    }
};

/**
 * @brief Пузырьковая сортировка.
 * @param arr Вектор книг (изменяется на месте).
 */
void bubbleSort(vector<Book>& arr) {
    long size = arr.size();
    for (long i = 0; i < size; i++) {
        bool swapped = false;
        for (long j = size - 1; j > i; j--) {
            if (arr[j - 1] > arr[j]) {
                swap(arr[j - 1], arr[j]);
                swapped = true;
            }
        }
        if (!swapped) 
            break;
    }
}

/**
 * @brief Восстанавливает свойство кучи в нисходящем направлении (пирамидальная сортировка).
 * @param a Вектор книг (начинается с индекса 1, a[0] — неиспользуемый, пустой элемент).
 * @param k Индекс родительского узла.
 * @param n Размер кучи (последний значимый индекс).
 */
void downHeap(vector<Book>& a, long k, long n)
{
    Book new_elem = a[k];
    long child;
    while (k <= n / 2)
    {
        child = 2 * k;
        if (child < n && a[child] < a[child + 1])
            child++;
        if (new_elem >= a[child])
            break;
        a[k] = a[child];
        k = child;
    }
    a[k] = new_elem;
}

/**
 * @brief Пирамидальная сортировка (heap sort) вектора книг.
 *
 * Временно вставляет пустой элемент в начало массива для удобства индексации с 1.
 * @param a Вектор книг (изменяется на месте).
 */
void heapSort(vector<Book>& a)
{
    a.insert(a.begin(), Book());
    long size = a.size() - 1;
    for (long i = size / 2; i >= 1; i--)
        downHeap(a, i, size);
    for (long i = size; i > 1; i--)
    {
        swap(a[1], a[i]);
        downHeap(a, 1, i - 1);
    }
    a.erase(a.begin());
}

/**
 * @brief Быстрая сортировка (рекурсивная).
 * @param a Вектор книг.
 * @param low Нижняя граница сортируемой части.
 * @param high Верхняя граница сортируемой части.
 */
void quickSort(vector<Book>& a, long low, long high)
{
    if (low >= high) return;
    
    long i = low, j = high;
    Book pivot = a[(low + high) / 2];
    
    do
    {
        while (a[i] < pivot) i++;
        while (a[j] > pivot) j--;
        
        if (i <= j)
        {
            swap(a[i], a[j]);
            i++;
            j--;
        }
    } while (i <= j);
    
    if (low < j) quickSort(a, low, j);
    if (i < high) quickSort(a, i, high);
}

/**
 * @brief Чтение книг из CSV-файла.
 *
 * Формат CSV: заголовок (первая строка) пропускается,
 * далее строки: Автор,Название,Год,Страницы.
 * @param filename Имя файла.
 * @return Вектор книг. Если файл не открыт, возвращается пустой вектор.
 */
vector<Book> readCSV(const string& filename) {
    vector<Book> books;
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Не удалось открыть файл: " << filename << "\n";
        return books;
    }
    
    string line;
    getline(file, line);
    
    while (getline(file, line)) {
        stringstream ss(line);
        Book book;
        string temp;
        getline(ss, book.author, ',');
        getline(ss, book.title, ',');
        getline(ss, temp, ',');
        book.year = stoi(temp);
        getline(ss, temp, ',');
        book.pages = stoi(temp);
        books.push_back(book);
    }
    return books;
}

/**
 * @brief Дозаписывает результат теста в файл.
 *
 * Строка формата: размер , тип_сортировки , время_мс.
 * @param filename Имя файла.
 * @param size Размер набора данных.
 * @param sortType Название сортировки (например, "bubble").
 * @param timeMs Время выполнения в миллисекундах.
 */
void writeResults(const string& filename, int size, const string& sortType, long long timeMs) {
    ofstream file(filename, ios::app);
    file << size << " , " << sortType << " , " << timeMs << "\n";
    file.close();
}

/**
 * @brief Точка входа.
 *
 * Перебирает предопределённые размеры наборов данных,
 * загружает книги из CSV-файлов, выполняет последовательно
 * пузырьковую, пирамидальную, быструю и std::sort,
 * записывает время выполнения в results.txt.
 *
 * @return 0 при успешном завершении.
 */
int main() {
    string resultsFile = "results.txt";
    
    int sizes[] = {100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000, 100000};
    int numSizes = 10;
    
    ofstream clearFile(resultsFile);
    clearFile.close();
    
    for (int i = 0; i < numSizes; i++) {
        int size = sizes[i];
        string filename = "library_books" + to_string(size) + ".csv";
        
        cout << "Файл: " << filename << "\n";
        
        vector<Book> books = readCSV(filename);
        
        if (books.empty()) {
            cout << "Файл пуст или не найден\n\n";
            continue;
        }
        
        cout << "  Загружено " << books.size() << " книг\n";
        
        vector<Book> temp = books;
        auto start = high_resolution_clock::now();
        bubbleSort(temp);
        auto end = high_resolution_clock::now();
        long long bubbleTime = duration_cast<milliseconds>(end - start).count();
        writeResults(resultsFile, size, "bubble", bubbleTime);
        cout << "  bubble: " << bubbleTime << " мс\n";
        
        temp = books;
        start = high_resolution_clock::now();
        heapSort(temp);
        end = high_resolution_clock::now();
        long long heapTime = duration_cast<milliseconds>(end - start).count();
        writeResults(resultsFile, size, "heap", heapTime);
        cout << "  heap: " << heapTime << " мс\n";
        
        temp = books;
        start = high_resolution_clock::now();
        quickSort(temp, 0, temp.size() - 1);
        end = high_resolution_clock::now();
        long long quickTime = duration_cast<milliseconds>(end - start).count();
        writeResults(resultsFile, size, "quick", quickTime);
        cout << "  quick: " << quickTime << " мс\n";
        
        temp = books;
        start = high_resolution_clock::now();
        sort(temp.begin(), temp.end());
        end = high_resolution_clock::now();
        long long stdTime = duration_cast<milliseconds>(end - start).count();
        writeResults(resultsFile, size, "std_sort", stdTime);
        cout << "  std_sort: " << stdTime << " мс\n\n";
    }
    
    cout << "Результаты сохранены в " << resultsFile << "\n";
    
    return 0;
}