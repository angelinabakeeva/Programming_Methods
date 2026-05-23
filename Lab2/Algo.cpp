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

struct Book {
    string author;
    string title;
    int year;
    int pages;
};

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
int linearSearch(const vector<Book>& data, const string& key) {
    int count = 0;
    for (const auto& b : data) {
        if (b.author == key) ++count;
    }
    return count;
}

//БИНАРНОЕ ДЕРЕВО ПОИСКА
struct BSTNode {
    string key;
    const Book* value;
    BSTNode *left, *right;
    
    BSTNode(const string& k, const Book* v)
        : key(k), value(v), left(nullptr), right(nullptr) {}
};

class BSTree {
private:
    BSTNode* root;
    
    void clearRecursive(BSTNode* node) {
        if (node) {
            clearRecursive(node->left);
            clearRecursive(node->right);
            delete node;
        }
    }
    
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
    BSTree() : root(nullptr) {}
    BSTree(const BSTree&) = delete;
    BSTree& operator=(const BSTree&) = delete;
    ~BSTree() { clearRecursive(root); }
    
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
    
    void search(const string& key, vector<const Book*>& out) const {
        out.clear();
        searchRecursive(root, key, out);
    }
};

//КРАСНО-ЧЁРНОЕ ДЕРЕВО
enum class Color { RED, BLACK };

struct RBNode {
    string key;
    const Book* value;
    Color color;
    RBNode *left, *right, *parent;
    
    RBNode(const string& k, const Book* v)
        : key(k), value(v), color(Color::RED),
          left(nullptr), right(nullptr), parent(nullptr) {}
};

class RBTree {
private:
    RBNode* root;
    
    void clearRecursive(RBNode* node) {
        if (node) {
            clearRecursive(node->left);
            clearRecursive(node->right);
            delete node;
        }
    }
    
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
    RBTree() : root(nullptr) {}
    RBTree(const RBTree&) = delete;
    RBTree& operator=(const RBTree&) = delete;
    ~RBTree() { clearRecursive(root); }
    
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
    
    void search(const string& key, vector<const Book*>& out) const {
        out.clear();
        searchRecursive(root, key, out);
    }
};

// ==================== 4. ХЕШ-ТАБЛИЦА ====================
struct HashNode {
    string key;
    const Book* value;
    HashNode* next;
    
    HashNode(const string& k, const Book* v)
        : key(k), value(v), next(nullptr) {}
};

class HashMap {
private:
    vector<HashNode*> buckets;
    size_t num_elements;
    
    size_t hash(const string& s) const {
        size_t h = 5381;
        for (char c : s) h = ((h << 5) + h) + c;
        return h;
    }
    
public:
    HashMap(size_t bucketCount)
        : buckets(bucketCount, nullptr), num_elements(0) {}
    
    HashMap(const HashMap&) = delete;
    HashMap& operator=(const HashMap&) = delete;
    
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
    
    void insert(const string& key, const Book* obj) {
        size_t idx = hash(key) % buckets.size();
        HashNode* node = new HashNode(key, obj);
        node->next = buckets[idx];
        buckets[idx] = node;
        ++num_elements;
    }
    
    void search(const string& key, vector<const Book*>& out) const {
        out.clear();
        size_t idx = hash(key) % buckets.size();
        for (HashNode* cur = buckets[idx]; cur; cur = cur->next) {
            if (cur->key == key) out.push_back(cur->value);
        }
    }
    
    size_t countRealCollisions() const {
        size_t collisions = 0;
        for (const auto& bucket : buckets) {
            int len = 0;
            for (HashNode* cur = bucket; cur; cur = cur->next) ++len;
            if (len > 1) collisions += (len - 1);
        }
        return collisions;
    }
    
    size_t size() const { return num_elements; }
};

// ==================== ЗАМЕРЫ ВРЕМЕНИ ====================
using ms = chrono::duration<double, milli>;

double measureLinear(const vector<Book>& data, const string& key) {
    volatile int sink = 0;
    auto t0 = high_resolution_clock::now();
    sink = linearSearch(data, key);
    return ms(high_resolution_clock::now() - t0).count();
}

double measureBST(const BSTree& tree, const string& key) {
    volatile int sink = 0;
    vector<const Book*> out;
    auto t0 = high_resolution_clock::now();
    tree.search(key, out);
    sink = out.size();
    return ms(high_resolution_clock::now() - t0).count();
}

double measureRBT(const RBTree& tree, const string& key) {
    volatile int sink = 0;
    vector<const Book*> out;
    auto t0 = high_resolution_clock::now();
    tree.search(key, out);
    sink = out.size();
    return ms(high_resolution_clock::now() - t0).count();
}

double measureHash(const HashMap& hm, const string& key) {
    volatile int sink = 0;
    vector<const Book*> out;
    auto t0 = high_resolution_clock::now();
    hm.search(key, out);
    sink = out.size();
    return ms(high_resolution_clock::now() - t0).count();
}

double measureMultimap(const multimap<string, const Book*>& mm,
                       const string& key) {
    volatile int sink = 0;
    auto t0 = high_resolution_clock::now();
    auto range = mm.equal_range(key);
    for (auto it = range.first; it != range.second; ++it)
        ++sink;
    return ms(high_resolution_clock::now() - t0).count();
}

// ==================== MAIN ====================
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