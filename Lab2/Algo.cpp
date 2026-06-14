/**
 * @file lab2_search.cpp
 * @brief Сравнение методов поиска по ключу "author" в массиве книг
 * @details Реализованы: линейный поиск, BST, красно-чёрное дерево,
 *          хеш-таблица (djb2 + цепочки), std::multimap.
 *          Для каждого размера выполняется ОДИН поиск случайного ключа.
 */

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <chrono>
#include <map>

using namespace std;
using namespace std::chrono;

/**
 * @brief Структура, представляющая книгу.
 */
struct Book {
    string author; ///< Автор книги
    string title;  ///< Название книги
    int year;      ///< Год издания
    int pages;     ///< Количество страниц
};

/**
 * @brief Чтение данных из CSV-файла.
 * @details Файл должен иметь заголовок в первой строке.
 *          Последующие строки: author,title,year,pages.
 * @param path Путь к CSV-файлу.
 * @return Вектор прочитанных книг.
 * @note При ошибке открытия программа завершается с кодом 1.
 */
vector<Book> readCSV(const string& path) {
    FILE* f = fopen(path.c_str(), "r");
    if (!f) {
        printf("Ошибка открытия: %s\n", path.c_str());
        exit(1);
    }
    
    vector<Book> data;
    char authorBuf[512], titleBuf[512];
    Book b;
    
    (void)fscanf(f, "%*[^\n]\n"); // пропуск заголовка
    
    while (fscanf(f, "%[^,],%[^,],%d,%d\n", 
                  authorBuf, titleBuf, &b.year, &b.pages) == 4) {
        b.author = authorBuf;
        b.title = titleBuf;
        data.push_back(b);
    }
    
    fclose(f);
    return data;
}

//ЛИНЕЙНЫЙ ПОИСК
/**
 * @brief Линейный поиск книг по автору.
 * @param data Вектор всех книг.
 * @param key Искомый автор.
 * @return Количество книг с заданным автором.
 */
int linearSearch(const vector<Book>& data, const string& key) {
    int count = 0;
    for (const auto& b : data) {
        if (b.author == key) ++count;
    }
    return count;
}

//БИНАРНОЕ ДЕРЕВО ПОИСКА (BST)
/**
 * @brief Узел бинарного дерева поиска.
 */
struct BSTNode {
    string key;          ///< Ключ (автор)
    const Book* value;   ///< Указатель на книгу
    BSTNode *left;       ///< Левый потомок
    BSTNode *right;      ///< Правый потомок
    
    /**
     * @brief Конструктор узла BST.
     * @param k Ключ.
     * @param v Указатель на книгу.
     */
    BSTNode(const string& k, const Book* v)
        : key(k), value(v), left(nullptr), right(nullptr) {}
};

/**
 * @brief Класс бинарного дерева поиска для книг.
 * @details Дубликаты ключей допускаются и могут располагаться в обоих поддеревьях.
 */
class BSTree {
private:
    BSTNode* root; ///< Корень дерева
    
    /**
     * @brief Рекурсивная очистка дерева.
     * @param node Корень поддерева для удаления.
     */
    void clearRecursive(BSTNode* node) {
        if (node) {
            clearRecursive(node->left);
            clearRecursive(node->right);
            delete node;
        }
    }
    
    /**
     * @brief Рекурсивный поиск всех книг с заданным ключом.
     * @param node Текущий узел.
     * @param key Искомый ключ.
     * @param[out] out Вектор, в который добавляются найденные указатели на книги.
     */
    void searchRecursive(BSTNode* node, const string& key,
                         vector<const Book*>& out) const {
        if (!node) return;
        if (key < node->key) {
            searchRecursive(node->left, key, out);
        } else if (key > node->key) {
            searchRecursive(node->right, key, out);
        } else {
            out.push_back(node->value);
            // Продолжаем поиск в обоих поддеревьях для сбора дубликатов
            searchRecursive(node->left, key, out);
            searchRecursive(node->right, key, out);
        }
    }
    
public:
    /// Конструктор по умолчанию – создаёт пустое дерево.
    BSTree() : root(nullptr) {}
    
    /// Запрет копирования.
    BSTree(const BSTree&) = delete;
    /// Запрет присваивания.
    BSTree& operator=(const BSTree&) = delete;
    
    /// Деструктор – рекурсивно удаляет все узлы.
    ~BSTree() { clearRecursive(root); }
    
    /**
     * @brief Вставка нового элемента в BST.
     * @param key Ключ (автор).
     * @param obj Указатель на книгу.
     */
    void insert(const string& key, const Book* obj) {
        BSTNode** pp = &root;
        while (*pp) {
            if (key < (*pp)->key)
                pp = &(*pp)->left;
            else
                pp = &(*pp)->right;
        }
        *pp = new BSTNode(key, obj);
    }
    
    /**
     * @brief Поиск всех книг по ключу.
     * @param key Ключ (автор).
     * @param[out] out Вектор, заполняемый указателями на найденные книги.
     */
    void search(const string& key, vector<const Book*>& out) const {
        out.clear();
        searchRecursive(root, key, out);
    }
};

//КРАСНО-ЧЁРНОЕ ДЕРЕВО
/// Цвет узла красно-чёрного дерева.
enum class Color { RED, BLACK };

/**
 * @brief Узел красно-чёрного дерева.
 */
struct RBNode {
    string key;          ///< Ключ (автор)
    const Book* value;   ///< Указатель на книгу
    Color color;         ///< Цвет узла
    RBNode *left;        ///< Левый потомок
    RBNode *right;       ///< Правый потомок
    RBNode *parent;      ///< Родительский узел
    
    /**
     * @brief Конструктор узла RBT.
     * @param k Ключ.
     * @param v Указатель на книгу.
     */
    RBNode(const string& k, const Book* v)
        : key(k), value(v), color(Color::RED),
          left(nullptr), right(nullptr), parent(nullptr) {}
};

/**
 * @brief Класс красно-чёрного дерева для книг.
 * @details Поддерживает стандартные свойства: корень чёрный,
 *          красный узел имеет чёрных потомков, одинаковая чёрная высота.
 *          Дубликаты ключей разрешены.
 */
class RBTree {
private:
    RBNode* root; ///< Корень дерева
    
    /**
     * @brief Рекурсивная очистка дерева.
     * @param node Корень поддерева.
     */
    void clearRecursive(RBNode* node) {
        if (node) {
            clearRecursive(node->left);
            clearRecursive(node->right);
            delete node;
        }
    }
    
    /**
     * @brief Левый поворот вокруг узла x.
     * @param x Узел, вокруг которого выполняется поворот.
     */
    void rotateLeft(RBNode* x) {
        RBNode* y = x->right;
        x->right = y->left;
        if (y->left) y->left->parent = x;
        y->parent = x->parent;
        if (!x->parent) root = y;
        else if (x == x->parent->left) x->parent->left = y;
        else x->parent->right = y;
        y->left = x;
        x->parent = y;
    }
    
    /**
     * @brief Правый поворот вокруг узла x.
     * @param x Узел, вокруг которого выполняется поворот.
     */
    void rotateRight(RBNode* x) {
        RBNode* y = x->left;
        x->left = y->right;
        if (y->right) y->right->parent = x;
        y->parent = x->parent;
        if (!x->parent) root = y;
        else if (x == x->parent->right) x->parent->right = y;
        else x->parent->left = y;
        y->right = x;
        x->parent = y;
    }
    
    /**
     * @brief Восстановление свойств красно-чёрного дерева после вставки.
     * @param z Вставленный узел (красный).
     */
    void fixInsert(RBNode* z) {
        while (z->parent && z->parent->color == Color::RED) {
            if (z->parent == z->parent->parent->left) {
                RBNode* y = z->parent->parent->right;
                if (y && y->color == Color::RED) {
                    z->parent->color = Color::BLACK;
                    y->color = Color::BLACK;
                    z->parent->parent->color = Color::RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->right) {
                        z = z->parent;
                        rotateLeft(z);
                    }
                    z->parent->color = Color::BLACK;
                    z->parent->parent->color = Color::RED;
                    rotateRight(z->parent->parent);
                }
            } else {
                RBNode* y = z->parent->parent->left;
                if (y && y->color == Color::RED) {
                    z->parent->color = Color::BLACK;
                    y->color = Color::BLACK;
                    z->parent->parent->color = Color::RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        rotateRight(z);
                    }
                    z->parent->color = Color::BLACK;
                    z->parent->parent->color = Color::RED;
                    rotateLeft(z->parent->parent);
                }
            }
        }
        root->color = Color::BLACK;
    }
    
    /**
     * @brief Рекурсивный поиск всех книг с заданным ключом.
     * @param node Текущий узел.
     * @param key Искомый ключ.
     * @param[out] out Вектор для сбора результатов.
     */
    void searchRecursive(RBNode* node, const string& key,
                         vector<const Book*>& out) const {
        if (!node) return;
        if (key < node->key) {
            searchRecursive(node->left, key, out);
        } else if (key > node->key) {
            searchRecursive(node->right, key, out);
        } else {
            out.push_back(node->value);
            // Продолжаем поиск в обоих поддеревьях для сбора дубликатов
            searchRecursive(node->left, key, out);
            searchRecursive(node->right, key, out);
        }
    }
    
public:
    /// Создаёт пустое красно-чёрное дерево.
    RBTree() : root(nullptr) {}
    
    /// Запрет копирования.
    RBTree(const RBTree&) = delete;
    /// Запрет присваивания.
    RBTree& operator=(const RBTree&) = delete;
    
    /// Деструктор.
    ~RBTree() { clearRecursive(root); }
    
    /**
     * @brief Вставка элемента в красно-чёрное дерево.
     * @param key Ключ (автор).
     * @param obj Указатель на книгу.
     */
    void insert(const string& key, const Book* obj) {
        RBNode* z = new RBNode(key, obj);
        RBNode* y = nullptr;
        RBNode* x = root;
        
        while (x) {
            y = x;
            if (key < x->key) x = x->left;
            else x = x->right; // равные ключи направо
        }
        
        z->parent = y;
        if (!y) root = z;
        else if (key < y->key) y->left = z;
        else y->right = z;
        
        fixInsert(z);
    }
    
    /**
     * @brief Поиск всех книг по ключу.
     * @param key Ключ.
     * @param[out] out Вектор с найденными указателями на книги.
     */
    void search(const string& key, vector<const Book*>& out) const {
        out.clear();
        searchRecursive(root, key, out);
    }
};

//ХЕШ-ТАБЛИЦА (djb2 + цепочки)
/**
 * @brief Узел цепочки хеш-таблицы.
 */
struct HashNode {
    string key;        ///< Ключ (автор)
    const Book* value; ///< Указатель на книгу
    HashNode* next;    ///< Следующий узел в цепочке
    
    /**
     * @brief Конструктор узла.
     * @param k Ключ.
     * @param v Указатель на книгу.
     */
    HashNode(const string& k, const Book* v)
        : key(k), value(v), next(nullptr) {}
};

/**
 * @brief Хеш-таблица с цепочками для разрешения коллизий.
 * @details Используется хеш-функция djb2.
 */
class HashMap {
private:
    vector<HashNode*> buckets; ///< Массив корзин (указателей на цепочки)
    size_t num_elements;       ///< Количество хранимых элементов
    
    /**
     * @brief Хеш-функция djb2.
     * @param s Строка для хеширования.
     * @return Хеш-значение.
     */
    size_t hash(const string& s) const {
        size_t h = 5381;
        for (char c : s) h = ((h << 5) + h) + c;
        return h;
    }
    
public:
    /**
     * @brief Конструктор хеш-таблицы.
     * @param bucketCount Количество корзин (размер вектора корзин).
     */
    HashMap(size_t bucketCount)
        : buckets(bucketCount, nullptr), num_elements(0) {}
    
    /// Запрет копирования.
    HashMap(const HashMap&) = delete;
    /// Запрет присваивания.
    HashMap& operator=(const HashMap&) = delete;
    
    /**
     * @brief Деструктор. Освобождает все цепочки.
     */
    ~HashMap() {
        for (auto head : buckets) {
            HashNode* cur = head;
            while (cur) {
                HashNode* next = cur->next;
                delete cur;
                cur = next;
            }
        }
    }
    
    /**
     * @brief Вставка элемента в хеш-таблицу.
     * @param key Ключ (автор).
     * @param obj Указатель на книгу.
     */
    void insert(const string& key, const Book* obj) {
        size_t idx = hash(key) % buckets.size();
        HashNode* node = new HashNode(key, obj);
        node->next = buckets[idx];
        buckets[idx] = node;
        ++num_elements;
    }
    
    /**
     * @brief Поиск всех книг с заданным ключом.
     * @param key Ключ.
     * @param[out] out Вектор указателей на найденные книги.
     */
    void search(const string& key, vector<const Book*>& out) const {
        out.clear();
        size_t idx = hash(key) % buckets.size();
        for (HashNode* cur = buckets[idx]; cur; cur = cur->next) {
            if (cur->key == key) out.push_back(cur->value);
        }
    }
    
    /**
     * @brief Подсчёт количества "лишних" элементов в цепочках (коллизий).
     * @details Учитывает все корзины с длиной > 1.
     * @return Сумма (длина - 1) для каждой такой корзины.
     */
    size_t countRealCollisions() const {
        size_t collisions = 0;
        for (const auto& bucket : buckets) {
            int len = 0;
            for (HashNode* cur = bucket; cur; cur = cur->next) ++len;
            if (len > 1) collisions += (len - 1);
        }
        return collisions;
    }
    
    /**
     * @brief Текущий размер таблицы (количество элементов).
     * @return Число элементов.
     */
    size_t size() const { return num_elements; }
};

//ЗАМЕРЫ ВРЕМЕНИ
/// Тип для представления времени в миллисекундах с плавающей точкой.
using ms = chrono::duration<double, milli>;

/**
 * @brief Измерение времени линейного поиска.
 * @param data Вектор книг.
 * @param key Искомый автор.
 * @return Время в миллисекундах.
 */
double measureLinear(const vector<Book>& data, const string& key) {
    volatile int sink = 0;
    auto t0 = high_resolution_clock::now();
    sink = linearSearch(data, key);
    return ms(high_resolution_clock::now() - t0).count();
}

/**
 * @brief Измерение времени поиска в BST.
 * @param tree Дерево.
 * @param key Ключ.
 * @return Время в миллисекундах.
 */
double measureBST(const BSTree& tree, const string& key) {
    volatile int sink = 0;
    vector<const Book*> out;
    auto t0 = high_resolution_clock::now();
    tree.search(key, out);
    sink = out.size();
    return ms(high_resolution_clock::now() - t0).count();
}

/**
 * @brief Измерение времени поиска в красно-чёрном дереве.
 * @param tree Дерево.
 * @param key Ключ.
 * @return Время в миллисекундах.
 */
double measureRBT(const RBTree& tree, const string& key) {
    volatile int sink = 0;
    vector<const Book*> out;
    auto t0 = high_resolution_clock::now();
    tree.search(key, out);
    sink = out.size();
    return ms(high_resolution_clock::now() - t0).count();
}

/**
 * @brief Измерение времени поиска в хеш-таблице.
 * @param hm Хеш-таблица.
 * @param key Ключ.
 * @return Время в миллисекундах.
 */
double measureHash(const HashMap& hm, const string& key) {
    volatile int sink = 0;
    vector<const Book*> out;
    auto t0 = high_resolution_clock::now();
    hm.search(key, out);
    sink = out.size();
    return ms(high_resolution_clock::now() - t0).count();
}

/**
 * @brief Измерение времени поиска в std::multimap.
 * @param mm Мультиотображение.
 * @param key Ключ.
 * @return Время в миллисекундах.
 */
double measureMultimap(const multimap<string, const Book*>& mm,
                       const string& key) {
    volatile int sink = 0;
    auto t0 = high_resolution_clock::now();
    auto range = mm.equal_range(key);
    for (auto it = range.first; it != range.second; ++it)
        ++sink;
    return ms(high_resolution_clock::now() - t0).count();
}

//MAIN
/**
 * @brief Точка входа в программу.
 * @details Для каждого размера из массива SIZES читает соответствующий CSV-файл,
 *          строит все структуры данных, выполняет поиск случайного автора
 *          и замеряет время. Результаты записываются в search_results.csv.
 * @return 0 при успешном завершении, 1 при ошибке создания файла результатов.
 */
int main() {
    const int SIZES[] = {
        100, 500, 1000, 5000, 10000,
        25000, 50000, 100000, 250000, 500000, 1000000
    };
    const int NUM_SIZES = sizeof(SIZES) / sizeof(SIZES[0]);
    
    FILE* log = fopen("search_results.csv", "w");
    if (!log) {
        printf("Не удалось создать search_results.csv\n");
        return 1;
    }
    
    fprintf(log, "size,linear_ms,bst_ms,rbt_ms,hash_ms,multimap_ms,collisions\n");
    
    printf("%10s%12s%12s%12s%12s%12s%12s\n",
           "Size", "Linear", "BST", "RBT", "Hash", "Multimap", "Collisions");
    
    srand(2024);
    
    for (int i = 0; i < NUM_SIZES; ++i) {
        int sz = SIZES[i];
        string path = "books_" + to_string(sz) + ".csv";
        
        auto data = readCSV(path);
        
        // Выбираем случайный ключ для поиска
        int idx = rand() % data.size();
        string searchKey = data[idx].author;
        
        // Построение структур
        BSTree bst;
        RBTree rbt;
        HashMap hashmap(data.size() * 2);
        multimap<string, const Book*> multimapData;
        
        for (const auto& b : data) {
            const Book* ptr = &b;
            bst.insert(b.author, ptr);
            rbt.insert(b.author, ptr);
            hashmap.insert(b.author, ptr);
            multimapData.insert({b.author, ptr});
        }
        
        // Замеры времени
        double tLinear   = measureLinear(data, searchKey);
        double tBST      = measureBST(bst, searchKey);
        double tRBT      = measureRBT(rbt, searchKey);
        double tHash     = measureHash(hashmap, searchKey);
        double tMultimap = measureMultimap(multimapData, searchKey);
        size_t collisions = hashmap.countRealCollisions();
        
        // Вывод с 3 знаками после запятой
        printf("%10d%12.3f%12.3f%12.3f%12.3f%12.3f%12zu\n",
               sz, tLinear, tBST, tRBT, tHash, tMultimap, collisions);
        fprintf(log, "%d,%.3f,%.3f,%.3f,%.3f,%.3f,%zu\n",
                sz, tLinear, tBST, tRBT, tHash, tMultimap, collisions);
    }
    
    fclose(log);
    printf("\nРезультаты сохранены в search_results.csv\n");
    
    return 0;
}