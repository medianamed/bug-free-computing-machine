// ============================================================================
// ЛАБОРАТОРНАЯ РАБОТА: СТРУКТУРЫ ДАННЫХ ДЛЯ ЗАПРОСОВ НА ОТРЕЗКАХ
// ============================================================================

// Короче, тут мы делаем разные структуры данных чтобы быстро искать сумму или минимум
// на каком-то отрезке массива. Потому что если тупо перебирать каждый раз - это дофига
// операций, а так можно заранее всё посчитать и потом за O(1) или O(log n) отвечать.

// RSQ = Range Sum Query - найти сумму на отрезке [l, r]
// RMQ = Range Minimum Query - найти минимум на отрезке [l, r]

// Мы считаем именно количество операций, а не время выполнения, потому что время
// зависит от компьютера, загрузки и всякой фигни, а операции - это честно и одинаково
// для всех.

// ============================================================================

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <functional>
#include <fstream>
#include <random>
#include <string>
#include <iomanip>
#include <climits>

using namespace std;

// ============================================================================
// СТРУКТУРА 1: ПРЕФИКСНЫЕ СУММЫ ДЛЯ RSQ (1D)
// ============================================================================

// Самая простая структура! Идея в том, что мы заранее считаем кумулятивные суммы.
// Например, если у нас массив [2, 5, 3, 1], то префиксные суммы будут:
// prefix[0] = 0 (пустая сумма)
// prefix[1] = 2 (сумма первых 1 элементов)
// prefix[2] = 2 + 5 = 7 (сумма первых 2 элементов)
// prefix[3] = 7 + 3 = 10
// prefix[4] = 10 + 1 = 11

// Теперь чтобы найти сумму от индекса l до r включительно, нам просто нужно
// вычесть prefix[l] из prefix[r+1]. Магия! Потому что prefix[r+1] содержит
// сумму всех элементов до r, а prefix[l] - сумму всех элементов до l-1.
// Когда мы вычитаем, остаётся как раз сумма от l до r.

// Построение: O(n) - просто один раз проходим по массиву
// Запрос: O(1) - одно вычитание!
// Минус: нельзя обновлять элементы (если обновить, надо всё пересчитывать)

class PrefixSum1D {
private:
    vector<long long> prefix;  // тут храним префиксные суммы
    long long ops_build;       // счётчик операций при построении
    long long ops_query;       // счётчик операций при запросе

public:
    // Конструктор - обнуляем счётчики
    PrefixSum1D() {
        ops_build = 0;
        ops_query = 0;
    }

    // Строим массив префиксных сумм
    // Сложность O(n) - проходим по массиву один раз
    void build(const vector<int>& arr) {
        ops_build = 0;  // сбрасываем счётчик
        
        int n = arr.size();
        prefix.resize(n + 1, 0);  // создаём массив на 1 больше, заполняем нулями
        // Почему n+1? Потому что prefix[0] = 0 (пустая сумма),
        // prefix[1] = сумма первого элемента, и так далее
        
        // Считаем префиксные суммы
        // prefix[i+1] = prefix[i] + arr[i]
        for (int i = 0; i < n; i++) {
            prefix[i + 1] = prefix[i] + arr[i];
            ops_build++;  // считаем эту операцию сложения
        }
    }

    // Запрос суммы от индекса l до r включительно
    // Сложность O(1) - просто одно вычитание!
    long long query(int l, int r) {
        ops_query++;  // считаем этот запрос
        return prefix[r + 1] - prefix[l];
    }

    // Геттеры для счётчиков операций
    long long getBuildOps() const { return ops_build; }
    long long getQueryOps() const { return ops_query; }
    
    // Сброс счётчика запросов (нужно для усреднения)
    void resetQueryCounter() { ops_query = 0; }
};

// ============================================================================
// СТРУКТУРА 2: ПРЕФИКСНЫЕ СУММЫ ДЛЯ RSQ (2D)
// ============================================================================

// То же самое, что и 1D, но для матрицы! Тут уже двумерные префиксные суммы.
// Используем принцип включений-исключений.

// Чтобы найти сумму в прямоугольнике от (r1,c1) до (r2,c2), мы берём:
// prefix[r2+1][c2+1] - это сумма всего прямоугольника от (0,0) до (r2,c2)
// - prefix[r1][c2+1] - вычитаем верхнюю часть (от 0 до r1-1)
// - prefix[r2+1][c1] - вычитаем левую часть (от 0 до c1-1)
// + prefix[r1][c1] - добавляем обратно верхний-левый угол, потому что мы его
//                    вычли дважды (один раз в верхней части, второй раз в левой)

// Построение: O(rows * cols)
// Запрос: O(1)

class PrefixSum2D {
private:
    vector<vector<long long>> prefix;  // двумерный массив префиксных сумм
    int rows, cols;                    // размеры матрицы
    long long ops_build;               // счётчик операций построения
    long long ops_query;               // счётчик операций запроса

public:
    PrefixSum2D() {
        ops_build = 0;
        ops_query = 0;
    }

    // Строим двумерные префиксные суммы
    // Сложность O(rows * cols)
    void build(const vector<vector<int>>& matrix) {
        ops_build = 0;
        
        rows = matrix.size();
        cols = matrix[0].size();
        
        // Создаём массив префиксов с дополнительной строкой и столбцом нулей
        // Это чтобы не было проблем с границами
        prefix.assign(rows + 1, vector<long long>(cols + 1, 0));
        
        // Заполняем по формуле:
        // prefix[i][j] = matrix[i-1][j-1] (текущий элемент)
        //              + prefix[i-1][j] (сверху)
        //              + prefix[i][j-1] (слева)
        //              - prefix[i-1][j-1] (сверху-слева, вычли дважды поэтому добавляем)
        for (int i = 1; i <= rows; i++) {
            for (int j = 1; j <= cols; j++) {
                prefix[i][j] = matrix[i-1][j-1]
                             + prefix[i-1][j]
                             + prefix[i][j-1]
                             - prefix[i-1][j-1];
                ops_build++;
            }
        }
    }

    // Запрос суммы в прямоугольнике от (r1,c1) до (r2,c2)
    // Сложность O(1)
    long long query(int r1, int c1, int r2, int c2) {
        ops_query++;
        
        // Принцип включений-исключений
        return prefix[r2+1][c2+1]
             - prefix[r1][c2+1]    // вычитаем верхнюю часть
             - prefix[r2+1][c1]    // вычитаем левую часть
             + prefix[r1][c1];     // добавляем обратно верхний-левый угол
    }

    long long getBuildOps() const { return ops_build; }
    long long getQueryOps() const { return ops_query; }
    void resetQueryCounter() { ops_query = 0; }
};

// ============================================================================
// СТРУКТУРА 3: RMQ ЧЕРЕЗ ПРЕДПРОСЧЕТ ВСЕХ ОТРЕЗКОВ
// ============================================================================

// Идея - предпосчитать минимум для ВСЕХ возможных отрезков [i,j]
// Храним в таблице table[i][j] = минимум на отрезке [i,j]
// Очень просто, но жрёт O(n^2) памяти и времени на построение!
// Зато запрос за O(1) - просто смотрим в таблицу.

// Как строим:
// table[i][i] = arr[i] (отрезок длины 1)
// table[i][j] = min(table[i][j-1], arr[j]) - минимум на [i,j] это минимум из
//               минимума на [i,j-1] и нового элемента arr[j]

class RMQAllSegments {
private:
    vector<vector<int>> table;  // table[i][j] = минимум на отрезке [i,j]
    long long ops_build;        // счётчик операций построения
    long long ops_query;        // счётчик операций запроса

public:
    RMQAllSegments() {
        ops_build = 0;
        ops_query = 0;
    }

    // Предпосчитываем минимумы для всех отрезков
    // Сложность O(n^2)
    void build(const vector<int>& arr) {
        ops_build = 0;
        
        int n = arr.size();
        table.assign(n, vector<int>(n));
        
        // Для каждой начальной позиции i
        for (int i = 0; i < n; i++) {
            table[i][i] = arr[i];  // отрезок длины 1
            ops_build++;
            
            // Расширяем отрезок вправо
            for (int j = i + 1; j < n; j++) {
                // Минимум [i,j] = min(минимум [i,j-1], arr[j])
                table[i][j] = min(table[i][j-1], arr[j]);
                ops_build++;
            }
        }
    }

    // Запрос - просто смотрим в таблицу!
    // Сложность O(1)
    int query(int l, int r) {
        ops_query++;
        return table[l][r];
    }

    long long getBuildOps() const { return ops_build; }
    long long getQueryOps() const { return ops_query; }
    void resetQueryCounter() { ops_query = 0; }
};

// ============================================================================
// СТРУКТУРА 4: КОРНЕВАЯ ДЕКОМПОЗИЦИЯ (УНИВЕРСАЛЬНАЯ)
// ============================================================================

// Идея - делим массив на блоки размером sqrt(n) (корень из n)
// Для каждого блока предпосчитываем комбинированное значение (сумма или минимум)
// При запросе: используем предпосчитанные значения для полных блоков + перебираем
// элементы в неполных блоках на границах.

// Почему sqrt(n)? Потому что это оптимальный баланс между количеством блоков
// и размером блока. Если блоков слишком много - долго перебирать, если блоки
// слишком большие - долго перебирать внутри них.

// Сложность:
// Построение: O(n)
// Запрос: O(sqrt(n)) - в худшем случае перебираем 2 * sqrt(n) элементов на границах
//        + sqrt(n) полных блоков
// Обновление: O(sqrt(n)) - нужно пересчитать один блок

// ВАЖНО: используем шаблоны и лямбды для гибкости!
// Это позволяет использовать один и тот же код и для RSQ (сумма), и для RMQ (минимум)
// Просто передаём разную функцию комбинирования и разный нейтральный элемент.

template<typename T, typename CombineFunc>
class SqrtDecomposition {
private:
    vector<T> arr;           // исходный массив
    vector<T> blocks;        // значения для каждого блока
    int block_size;          // размер блока (корень из n)
    int n;                   // размер массива
    CombineFunc combine;     // функция комбинирования (лямбда)
    T neutral;               // нейтральный элемент (0 для суммы, INT_MAX для минимума)
    long long ops_build;     // счётчик операций построения
    long long ops_query;     // счётчик операций запроса
    long long ops_update;    // счётчик операций обновления

public:
    // Конструктор принимает лямбду и нейтральный элемент
    SqrtDecomposition(CombineFunc func, T neut) : combine(func), neutral(neut) {
        ops_build = 0;
        ops_query = 0;
        ops_update = 0;
    }

    // Строим структуру
    // Сложность O(n)
    void build(const vector<T>& input_arr) {
        ops_build = 0;
        arr = input_arr;
        n = arr.size();
        
        // Размер блока = корень из n (но не меньше 1)
        block_size = max(1, (int)sqrt(n));
        int num_blocks = (n + block_size - 1) / block_size;  // количество блоков
        blocks.assign(num_blocks, neutral);  // заполняем нейтральными элементами
        
        // Для каждого элемента добавляем его в соответствующий блок
        for (int i = 0; i < n; i++) {
            blocks[i / block_size] = combine(blocks[i / block_size], arr[i]);
            ops_build++;
        }
    }

    // Запрос на отрезке [l, r]
    // Сложность O(sqrt(n))
    T query(int l, int r) {
        ops_query = 0;
        T result = neutral;
        
        int bl = l / block_size;  // блок левого конца
        int br = r / block_size;  // блок правого конца
        
        // Если оба конца в одном блоке - просто перебираем
        if (bl == br) {
            for (int i = l; i <= r; i++) {
                result = combine(result, arr[i]);
                ops_query++;
            }
        } else {
            // Перебираем элементы от l до конца его блока
            for (int i = l; i < (bl + 1) * block_size; i++) {
                result = combine(result, arr[i]);
                ops_query++;
            }
            
            // Перебираем полные блоки между bl и br
            for (int b = bl + 1; b < br; b++) {
                result = combine(result, blocks[b]);
                ops_query++;
            }
            
            // Перебираем элементы от начала блока br до r
            for (int i = br * block_size; i <= r; i++) {
                result = combine(result, arr[i]);
                ops_query++;
            }
        }
        
        return result;
    }

    // Обновление элемента в позиции pos на значение val
    // Сложность O(sqrt(n)) - нужно пересчитать блок
    void update(int pos, T val) {
        ops_update = 0;
        int b = pos / block_size;  // номер блока
        arr[pos] = val;
        ops_update++;
        
        // Пересчитываем весь блок
        blocks[b] = neutral;
        int start = b * block_size;
        int end = min(start + block_size, n);
        for (int i = start; i < end; i++) {
            blocks[b] = combine(blocks[b], arr[i]);
            ops_update++;
        }
    }

    // Геттеры
    long long getBuildOps() const { return ops_build; }
    long long getQueryOps() const { return ops_query; }
    long long getUpdateOps() const { return ops_update; }
    void resetQueryCounter() { ops_query = 0; }
};

// ============================================================================
// СТРУКТУРА 5: ДЕРЕВО ОТРЕЗКОВ ДЛЯ RSQ
// ============================================================================

// Идея - строим дерево, где каждый узел хранит сумму своего отрезка.
// Корень хранит сумму всего массива.
// Листья хранят отдельные элементы.
// Внутренние узлы хранят сумму своих детей.

// Почему размер дерева 4*n? Потому что дерево не всегда полное, и нам нужен
// запас, чтобы не выйти за границы массива. На самом деле достаточно 2*n, если
// n - степень двойки, но 4*n - безопаснее.

// Построение: O(n) - проходим по всем узлам дерева один раз
// Запрос: O(log n) - спускаемся от корня к листьям, высота дерева log(n)
// Обновление: O(log n) - обновляем лист и пересчитываем всех предков

class SegmentTreeRSQ {
private:
    vector<long long> tree;  // массив дерева (размер 4*n)
    int n;                   // размер исходного массива
    long long ops_build;     // счётчик операций построения
    long long ops_query;     // счётчик операций запроса
    long long ops_update;    // счётчик операций обновления

    // Рекурсивное построение дерева
    void buildRec(const vector<int>& arr, int node, int start, int end) {
        if (start == end) {
            // Лист дерева - один элемент
            tree[node] = arr[start];
            ops_build++;
            return;
        }
        
        int mid = (start + end) / 2;
        
        // Строим левое и правое поддерево
        // Левый ребёнок имеет индекс 2*node, правый - 2*node+1
        buildRec(arr, 2*node, start, mid);
        buildRec(arr, 2*node+1, mid+1, end);
        
        // Сумма текущего узла = сумма левого + сумма правого
        tree[node] = tree[2*node] + tree[2*node+1];
        ops_build++;
    }

    // Рекурсивный запрос суммы на отрезке [l, r]
    long long queryRec(int node, int start, int end, int l, int r) {
        ops_query++;
        
        // Если отрезок запроса не пересекается с текущим - возвращаем 0
        if (r < start || end < l) return 0;
        
        // Если текущий отрезок полностью внутри запроса - возвращаем его сумму
        if (l <= start && end <= r) return tree[node];
        
        // Иначе рекурсивно идём в детей
        int mid = (start + end) / 2;
        return queryRec(2*node, start, mid, l, r)
             + queryRec(2*node+1, mid+1, end, l, r);
    }

    // Рекурсивное обновление элемента
    void updateRec(int node, int start, int end, int idx, int val) {
        ops_update++;
        
        if (start == end) {
            // Нашли лист - обновляем
            tree[node] = val;
            return;
        }
        
        int mid = (start + end) / 2;
        
        // Идём в нужного ребёнка
        if (idx <= mid) updateRec(2*node, start, mid, idx, val);
        else updateRec(2*node+1, mid+1, end, idx, val);
        
        // Пересчитываем сумму текущего узла
        tree[node] = tree[2*node] + tree[2*node+1];
    }

public:
    SegmentTreeRSQ() {
        ops_build = 0;
        ops_query = 0;
        ops_update = 0;
    }

    // Публичный метод построения
    void build(const vector<int>& arr) {
        ops_build = 0;
        n = arr.size();
        tree.assign(4 * n, 0);  // размер дерева 4*n (с запасом)
        buildRec(arr, 1, 0, n - 1);  // начинаем с корня (узел 1)
    }

    // Публичный метод запроса
    long long query(int l, int r) {
        return queryRec(1, 0, n - 1, l, r);
    }

    // Публичный метод обновления
    void update(int idx, int val) {
        updateRec(1, 0, n - 1, idx, val);
    }

    // Геттеры
    long long getBuildOps() const { return ops_build; }
    long long getQueryOps() const { return ops_query; }
    long long getUpdateOps() const { return ops_update; }
    void resetQueryCounter() { ops_query = 0; }
};

// ============================================================================
// СТРУКТУРА 6: ДЕРЕВО ФЕНВИКА (БИНАРНОЕ ИНДЕКСНОЕ ДЕРЕВО) ДЛЯ RSQ
// ============================================================================

// Идея - хитрая структура для быстрых запросов суммы и обновлений.
// Каждый элемент хранит сумму некоторого диапазона, причём диапазон определяется
// двоичным представлением индекса.

// Работает за O(log n) на запрос и обновление.
// Строится за O(n).

// Главное отличие от дерева отрезков - здесь используется 1-индексация!
// И формула для перехода к следующему узлу: idx += idx & (-idx)
// А для перехода к родительскому узлу: idx -= idx & (-idx)

// Это работает потому что idx & (-idx) выделяет младший установленный бит.

class FenwickTree {
private:
    vector<long long> bit;  // массив дерева Фенвика (1-индексация!)
    int n;                  // размер массива
    long long ops_build;    // счётчик операций построения
    long long ops_query;    // счётчик операций запроса
    long long ops_update;   // счётчик операций обновления

public:
    FenwickTree() {
        ops_build = 0;
        ops_query = 0;
        ops_update = 0;
    }

    // Построение дерева Фенвика
    // Сложность O(n)
    void build(const vector<int>& arr) {
        ops_build = 0;
        n = arr.size();
        bit.assign(n + 1, 0);  // 1-индексация поэтому размер n+1
        
        // Добавляем каждый элемент в дерево
        for (int i = 0; i < n; i++) {
            int idx = i + 1;  // переходим к 1-индексации
            while (idx <= n) {
                bit[idx] += arr[i];
                ops_build++;
                idx += idx & (-idx);  // переходим к следующему ответственному узлу
            }
        }
    }

    // Запрос суммы на отрезке [l, r]
    // Сложность O(log n)
    long long query(int l, int r) {
        ops_query = 0;
        long long sum = 0;
        
        // Сумма [l,r] = сумма [1,r] - сумма [1,l-1]
        int idx = r + 1;  // 1-индексация
        while (idx > 0) {
            sum += bit[idx];
            ops_query++;
            idx -= idx & (-idx);  // идём к родительскому узлу
        }
        
        idx = l;  // l уже в правильной индексации для вычитания
        while (idx > 0) {
            sum -= bit[idx];
            ops_query++;
            idx -= idx & (-idx);
        }
        
        return sum;
    }

    // Обновление: добавить val к элементу на позиции idx
    // Сложность O(log n)
    void update(int idx, int val) {
        ops_update = 0;
        idx++;  // переходим к 1-индексации
        while (idx <= n) {
            bit[idx] += val;
            ops_update++;
            idx += idx & (-idx);  // обновляем все ответственные узлы
        }
    }

    // Геттеры
    long long getBuildOps() const { return ops_build; }
    long long getQueryOps() const { return ops_query; }
    long long getUpdateOps() const { return ops_update; }
    void resetQueryCounter() { ops_query = 0; }
};

// ============================================================================
// СТРУКТУРА 7: РАЗРЕЖЕННАЯ ТАБЛИЦА (SPARSE TABLE) ДЛЯ RMQ
// ============================================================================

// Идея - предпосчитываем минимумы для отрезков длины 1, 2, 4, 8, ...
// Любой отрезок можно покрыть двумя отрезками степени двойки (они могут пересекаться).
// Построение за O(n log n), запрос за O(1)!
// Но нельзя обновлять элементы (статическая структура).

// Как это работает:
// st[j][i] = минимум на отрезке [i, i + 2^j - 1]
// База: st[0][i] = arr[i] (отрезки длины 1)
// Рекуррентность: st[j][i] = min(st[j-1][i], st[j-1][i + 2^(j-1)])
// То есть минимум на отрезке длины 2^j это минимум из двух половин длиной 2^(j-1)

// При запросе [l, r]:
// Находим наибольшую степень двойки k такую что 2^k <= r - l + 1
// Тогда ответ = min(st[k][l], st[k][r - 2^k + 1])
// Эти два отрезка могут пересекаться, но для минимума это не проблема!

class SparseTable {
private:
    vector<vector<int>> st;      // st[j][i] = минимум на отрезке [i, i+2^j-1]
    vector<int> log_table;       // предпосчитанные логарифмы
    int n;                       // размер массива
    long long ops_build;         // счётчик операций построения
    long long ops_query;         // счётчик операций запроса

public:
    SparseTable() {
        ops_build = 0;
        ops_query = 0;
    }

    // Построение разреженной таблицы
    // Сложность O(n log n)
    void build(const vector<int>& arr) {
        ops_build = 0;
        n = arr.size();
        int LOG = (int)log2(n) + 1;  // максимальная степень двойки
        
        st.assign(LOG, vector<int>(n));
        log_table.resize(n + 1);
        
        // Предпосчитываем логарифмы (округлённые вниз)
        // Это чтобы не считать log2 каждый раз при запросе
        log_table[1] = 0;
        for (int i = 2; i <= n; i++) {
            log_table[i] = log_table[i / 2] + 1;
            ops_build++;
        }
        
        // База: отрезки длины 1 (2^0)
        for (int i = 0; i < n; i++) {
            st[0][i] = arr[i];
            ops_build++;
        }
        
        // Заполняем для больших степеней двойки
        for (int j = 1; j < LOG; j++) {
            for (int i = 0; i + (1 << j) <= n; i++) {
                // Минимум [i, i+2^j-1] = min(минимум [i, i+2^(j-1)-1],
                //                             минимум [i+2^(j-1), i+2^j-1])
                st[j][i] = min(st[j-1][i], st[j-1][i + (1 << (j-1))]);
                ops_build++;
            }
        }
    }

    // Запрос минимума на отрезке [l, r]
    // Сложность O(1) - просто два обращения к таблице!
    int query(int l, int r) {
        ops_query++;
        int j = log_table[r - l + 1];  // наибольшая степень двойки <= длины отрезка
        // Покрываем отрезок двумя отрезками длины 2^j (они могут пересекаться)
        return min(st[j][l], st[j][r - (1 << j) + 1]);
    }

    // Геттеры
    long long getBuildOps() const { return ops_build; }
    long long getQueryOps() const { return ops_query; }
    void resetQueryCounter() { ops_query = 0; }
};

// ============================================================================
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
// ============================================================================

// Генерация случайного массива
vector<int> generateArray(int n, int max_val = 1000) {
    mt19937 rng(42);  // фиксированный сид для воспроизводимости
    vector<int> arr(n);
    for (int i = 0; i < n; i++) arr[i] = rng() % max_val;
    return arr;
}

// Генерация случайной матрицы
vector<vector<int>> generate2DArray(int rows, int cols, int max_val = 1000) {
    mt19937 rng(42);
    vector<vector<int>> matrix(rows, vector<int>(cols));
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            matrix[i][j] = rng() % max_val;
    return matrix;
}

// ============================================================================
// ГЛАВНАЯ ФУНКЦИЯ: ЗАМЕРЫ И ВЫВОД РЕЗУЛЬТАТОВ
// ============================================================================

void runBenchmarks() {
    cout << "=== LABORATORY WORK: Range Query Data Structures ===\n\n";
    vector<int> sizes = {10, 50, 100, 200, 500, 1000, 2000, 5000};

    // --- 1. 1D RSQ префиксные суммы ---
    cout << "1. 1D RSQ - Prefix Sums\n";
    cout << "   Theory: build O(n), query O(1)\n";
    cout << setw(8) << "n" << setw(15) << "build_ops" << setw(15) << "query_ops(avg)" << "\n";
    for (int n : sizes) {
        auto arr = generateArray(n);
        PrefixSum1D ps;
        ps.build(arr);
        long long total_query = 0;
        int queries = 100;
        for (int q = 0; q < queries; q++) {
            int l = rand() % n;
            int r = l + rand() % (n - l);
            if (r >= n) r = n - 1;
            ps.query(l, r);
            total_query += ps.getQueryOps();
            ps.resetQueryCounter();
        }
        cout << setw(8) << n
             << setw(15) << ps.getBuildOps()
             << setw(15) << total_query / queries << "\n";
    }
    cout << "\n";

    // --- 2. 2D RSQ префиксные суммы ---
    cout << "2. 2D RSQ - Prefix Sums\n";
    cout << "   Theory: build O(n*m), query O(1)\n";
    cout << setw(8) << "n" << setw(15) << "build_ops" << setw(15) << "query_ops(avg)" << "\n";
    vector<int> sizes2d = {5, 10, 20, 30, 50, 70, 100};
    for (int n : sizes2d) {
        auto matrix = generate2DArray(n, n);
        PrefixSum2D ps;
        ps.build(matrix);
        long long total_query = 0;
        int queries = 100;
        for (int q = 0; q < queries; q++) {
            int r1 = rand() % n, c1 = rand() % n;
            int r2 = r1 + rand() % (n - r1);
            int c2 = c1 + rand() % (n - c1);
            if (r2 >= n) r2 = n-1;
            if (c2 >= n) c2 = n-1;
            ps.query(r1, c1, r2, c2);
            total_query += ps.getQueryOps();
            ps.resetQueryCounter();
        }
        cout << setw(8) << n
             << setw(15) << ps.getBuildOps()
             << setw(15) << total_query / queries << "\n";
    }
    cout << "\n";

    // --- 3. 1D RMQ предпросчет всех отрезков ---
    cout << "3. 1D RMQ - All Segments Precomputation\n";
    cout << "   Theory: build O(n^2), query O(1)\n";
    cout << setw(8) << "n" << setw(15) << "build_ops" << setw(15) << "query_ops(avg)" << "\n";
    vector<int> sizes_rmq_all = {10, 20, 30, 50, 70, 100, 150, 200};
    for (int n : sizes_rmq_all) {
        auto arr = generateArray(n);
        RMQAllSegments rmq;
        rmq.build(arr);
        long long total_query = 0;
        int queries = 100;
        for (int q = 0; q < queries; q++) {
            int l = rand() % n;
            int r = l + rand() % (n - l);
            if (r >= n) r = n - 1;
            rmq.query(l, r);
            total_query += rmq.getQueryOps();
            rmq.resetQueryCounter();
        }
        cout << setw(8) << n
             << setw(15) << rmq.getBuildOps()
             << setw(15) << total_query / queries << "\n";
    }
    cout << "\n";

    // --- 4. Корневая декомпозиция RSQ ---
    cout << "4. RSQ - Square Root Decomposition\n";
    cout << "   Theory: build O(n), query O(sqrt(n))\n";
    cout << setw(8) << "n" << setw(15) << "build_ops" << setw(15) << "query_ops(avg)" << "\n";
    for (int n : sizes) {
        auto arr = generateArray(n);
        SqrtDecomposition<long long, function<long long(long long,long long)>>
            sqrt_rsq([](long long a, long long b){ return a + b; }, 0LL);
        sqrt_rsq.build(arr);
        long long total_query = 0;
        int queries = 100;
        for (int q = 0; q < queries; q++) {
            int l = rand() % n;
            int r = l + rand() % (n - l);
            if (r >= n) r = n - 1;
            sqrt_rsq.query(l, r);
            total_query += sqrt_rsq.getQueryOps();
            sqrt_rsq.resetQueryCounter();
        }
        cout << setw(8) << n
             << setw(15) << sqrt_rsq.getBuildOps()
             << setw(15) << total_query / queries << "\n";
    }
    cout << "\n";

    // --- 4б. Корневая декомпозиция RMQ ---
    cout << "4b. RMQ - Square Root Decomposition\n";
    cout << "   Theory: build O(n), query O(sqrt(n))\n";
    cout << setw(8) << "n" << setw(15) << "build_ops" << setw(15) << "query_ops(avg)" << "\n";
    for (int n : sizes) {
        auto arr = generateArray(n);
        SqrtDecomposition<int, function<int(int,int)>>
            sqrt_rmq([](int a, int b){ return min(a,b); }, INT32_MAX);
        sqrt_rmq.build(arr);
        long long total_query = 0;
        int queries = 100;
        for (int q = 0; q < queries; q++) {
            int l = rand() % n;
            int r = l + rand() % (n - l);
            if (r >= n) r = n - 1;
            sqrt_rmq.query(l, r);
            total_query += sqrt_rmq.getQueryOps();
            sqrt_rmq.resetQueryCounter();
        }
        cout << setw(8) << n
             << setw(15) << sqrt_rmq.getBuildOps()
             << setw(15) << total_query / queries << "\n";
    }
    cout << "\n";

    // --- 5. Дерево отрезков RSQ ---
    cout << "5. RSQ - Segment Tree\n";
    cout << "   Theory: build O(n), query O(log n), update O(log n)\n";
    cout << setw(8) << "n" << setw(15) << "build_ops"
         << setw(15) << "query_ops(avg)" << setw(15) << "update_ops(avg)" << "\n";
    for (int n : sizes) {
        auto arr = generateArray(n);
        SegmentTreeRSQ st;
        st.build(arr);
        long long total_query = 0, total_update = 0;
        int queries = 100;
        for (int q = 0; q < queries; q++) {
            int l = rand() % n;
            int r = l + rand() % (n - l);
            if (r >= n) r = n - 1;
            st.query(l, r);
            total_query += st.getQueryOps();
            st.resetQueryCounter();
            st.update(rand() % n, rand() % 1000);
            total_update += st.getUpdateOps();
        }
        cout << setw(8) << n
             << setw(15) << st.getBuildOps()
             << setw(15) << total_query / queries
             << setw(15) << total_update / queries << "\n";
    }
    cout << "\n";

    // --- 6. Дерево Фенвика RSQ ---
    cout << "6. RSQ - Fenwick Tree (Binary Indexed Tree)\n";
    cout << "   Theory: build O(n), query O(log n), update O(log n)\n";
    cout << setw(8) << "n" << setw(15) << "build_ops"
         << setw(15) << "query_ops(avg)" << setw(15) << "update_ops(avg)" << "\n";
    for (int n : sizes) {
        auto arr = generateArray(n);
        FenwickTree ft;
        ft.build(arr);
        long long total_query = 0, total_update = 0;
        int queries = 100;
        for (int q = 0; q < queries; q++) {
            int l = rand() % n;
            int r = l + rand() % (n - l);
            if (r >= n) r = n - 1;
            ft.query(l, r);
            total_query += ft.getQueryOps();
            ft.resetQueryCounter();
            ft.update(rand() % n, rand() % 1000);
            total_update += ft.getUpdateOps();
        }
        cout << setw(8) << n
             << setw(15) << ft.getBuildOps()
             << setw(15) << total_query / queries
             << setw(15) << total_update / queries << "\n";
    }
    cout << "\n";

    // --- 7. Разреженная таблица RMQ ---
    cout << "7. RMQ - Sparse Table\n";
    cout << "   Theory: build O(n log n), query O(1)\n";
    cout << setw(8) << "n" << setw(15) << "build_ops" << setw(15) << "query_ops(avg)" << "\n";
    for (int n : sizes) {
        auto arr = generateArray(n);
        SparseTable st;
        st.build(arr);
        long long total_query = 0;
        int queries = 100;
        for (int q = 0; q < queries; q++) {
            int l = rand() % n;
            int r = l + rand() % (n - l);
            if (r >= n) r = n - 1;
            st.query(l, r);
            total_query += st.getQueryOps();
            st.resetQueryCounter();
        }
        cout << setw(8) << n
             << setw(15) << st.getBuildOps()
             << setw(15) << total_query / queries << "\n";
    }
    cout << "\n";
}

// ============================================================================
// ЭКСПОРТ ДАННЫХ В CSV ДЛЯ ПОСТРОЕНИЯ ГРАФИКОВ
// ============================================================================

void exportCSV() {
    ofstream csv("results.csv");
    csv << "n,1D_RSQ_build,1D_RSQ_query,"
        << "2D_RSQ_build,2D_RSQ_query,"
        << "RMQ_all_build,RMQ_all_query,"
        << "Sqrt_RSQ_build,Sqrt_RSQ_query,"
        << "Sqrt_RMQ_build,Sqrt_RMQ_query,"
        << "SegTree_build,SegTree_query,SegTree_update,"
        << "Fenwick_build,Fenwick_query,Fenwick_update,"
        << "Sparse_build,Sparse_query\n";

    vector<int> sizes = {10, 50, 100, 200, 500, 1000, 2000, 5000};

    for (int n : sizes) {
        auto arr = generateArray(n);

        // 1D RSQ prefix
        PrefixSum1D ps; ps.build(arr);
        long long q1 = 0;
        for (int i = 0; i < 100; i++) {
            int l = rand()%n, r = l + rand()%(n-l);
            if (r >= n) r = n-1;
            ps.query(l,r);
            q1 += ps.getQueryOps();
            ps.resetQueryCounter();
        }

        // Sqrt RSQ
        SqrtDecomposition<long long, function<long long(long long,long long)>>
            sqrt_rsq([](long long a, long long b){ return a+b; }, 0LL);
        sqrt_rsq.build(arr);
        long long q2 = 0;
        for (int i = 0; i < 100; i++) {
            int l = rand()%n, r = l + rand()%(n-l);
            if (r >= n) r = n-1;
            sqrt_rsq.query(l,r);
            q2 += sqrt_rsq.getQueryOps();
            sqrt_rsq.resetQueryCounter();
        }

        // Sqrt RMQ
        SqrtDecomposition<int, function<int(int,int)>>
            sqrt_rmq([](int a, int b){ return min(a,b); }, INT32_MAX);
        sqrt_rmq.build(arr);
        long long q3 = 0;
        for (int i = 0; i < 100; i++) {
            int l = rand()%n, r = l + rand()%(n-l);
            if (r >= n) r = n-1;
            sqrt_rmq.query(l,r);
            q3 += sqrt_rmq.getQueryOps();
            sqrt_rmq.resetQueryCounter();
        }

        // Segment Tree
        SegmentTreeRSQ st; st.build(arr);
        long long q4 = 0, u1 = 0;
        for (int i = 0; i < 100; i++) {
            int l = rand()%n, r = l + rand()%(n-l);
            if (r >= n) r = n-1;
            st.query(l,r);
            q4 += st.getQueryOps();
            st.resetQueryCounter();
            st.update(rand()%n, rand()%1000);
            u1 += st.getUpdateOps();
        }

        // Fenwick
        FenwickTree ft; ft.build(arr);
        long long q5 = 0, u2 = 0;
        for (int i = 0; i < 100; i++) {
            int l = rand()%n, r = l + rand()%(n-l);
            if (r >= n) r = n-1;
            ft.query(l,r);
            q5 += ft.getQueryOps();
            ft.resetQueryCounter();
            ft.update(rand()%n, rand()%1000);
            u2 += ft.getUpdateOps();
        }

        // Sparse Table
        SparseTable sp; sp.build(arr);
        long long q6 = 0;
        for (int i = 0; i < 100; i++) {
            int l = rand()%n, r = l + rand()%(n-l);
            if (r >= n) r = n-1;
            sp.query(l,r);
            q6 += sp.getQueryOps();
            sp.resetQueryCounter();
        }

        csv << n << ","
            << ps.getBuildOps() << "," << q1/100 << ","
            << 0 << "," << 0 << ","
            << 0 << "," << 0 << ","
            << sqrt_rsq.getBuildOps() << "," << q2/100 << ","
            << sqrt_rmq.getBuildOps() << "," << q3/100 << ","
            << st.getBuildOps() << "," << q4/100 << "," << u1/100 << ","
            << ft.getBuildOps() << "," << q5/100 << "," << u2/100 << ","
            << sp.getBuildOps() << "," << q6/100 << "\n";
    }
    csv.close();
    cout << "Data exported to results.csv\n";
}

// ============================================================================
// ТОЧКА ВХОДА
// ============================================================================

int main() {
    runBenchmarks();
    exportCSV();
    return 0;
}
