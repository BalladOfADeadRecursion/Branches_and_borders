// Подключение необходимых библиотек
#include <iostream>
#include <vector>
#include <cmath>

// Глобальные переменные для использования в методе ветвей и границ
int a00, a10, a01, a11, b00, b11, c00, c11;

// Класс, реализующий метод симплекса и метод ветвей и границ
class Simplex {
private:
    std::vector<std::vector<double>> tableau; // Матрица для симплекс-метода

    // Инициализация матрицы для симплекс-метода
    void initializeSimplexTable(const std::vector<std::vector<double>>& constraints, const std::vector<double>& objectiveFunction) {
        int m = constraints.size(); // Количество ограничений
        int n = constraints[0].size() - 1; // Количество переменных
        tableau.resize(m + 1, std::vector<double>(n + m + 1)); // Инициализация матрицы

        // Заполнение матрицы ограничениями и коэффициентами целевой функции
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j) {
                tableau[i][j] = constraints[i][j];
            }
            tableau[i][n + i] = 1;
            tableau[i][n + m] = constraints[i][n];
        }

        // Заполнение последней строки матрицы коэффициентами целевой функции с обратными знаками
        for (int j = 0; j < n; ++j) {
            tableau[m][j] = -objectiveFunction[j];
        }
    }

    // Поиск разрешающего столбца для симплекс-метода
    int findPivotColumn() {
        int n = tableau[0].size();
        int pivotColumn = 0;
        for (int j = 1; j < n - 1; ++j) {
            if (tableau[tableau.size() - 1][j] < tableau[tableau.size() - 1][pivotColumn]) {
                pivotColumn = j;
            }
        }
        return pivotColumn;
    }

    // Поиск разрешающей строки для симплекс-метода
    int findPivotRow(int pivotColumn) {
        int m = tableau.size() - 1;
        int pivotRow = 0;
        double minRatio = -1;

        for (int i = 0; i < m; ++i) {
            if (tableau[i][pivotColumn] > 0) {
                double ratio = tableau[i][tableau[0].size() - 1] / tableau[i][pivotColumn];
                if (minRatio == -1 || ratio < minRatio) {
                    minRatio = ratio;
                    pivotRow = i;
                }
            }
        }
        return pivotRow;
    }

    // Выполнение операции пересчета при симплекс-методе
    void performPivotOperation(int pivotRow, int pivotColumn) {
        int m = tableau.size();
        int n = tableau[0].size();

        double pivot = tableau[pivotRow][pivotColumn];
        for (int j = 0; j < n; ++j) {
            tableau[pivotRow][j] /= pivot;
        }

        for (int i = 0; i < m; ++i) {
            if (i != pivotRow) {
                double ratio = tableau[i][pivotColumn];
                for (int j = 0; j < n; ++j) {
                    tableau[i][j] -= ratio * tableau[pivotRow][j];
                }
            }
        }
    }

    // Проверка, является ли текущее решение оптимальным для симплекс-метода
    bool isOptimalSolution() {
        int n = tableau[0].size();
        for (int j = 0; j < n - 1; ++j) {
            if (tableau[tableau.size() - 1][j] < 0) {
                return false;
            }
        }
        return true;
    }

    bool isIntegerSolution(const std::vector<double>& solution) {
        for (double val : solution) {
            if (std::abs(val - std::round(val)) > 1e-6) {
                return false;
            }
        }
        return true;
    }

public:
    // Метод симплекс-метода для решения задачи линейного программирования
    std::vector<double> solve(std::vector<std::vector<double>> constraints, std::vector<double> objectiveFunction) {
        initializeSimplexTable(constraints, objectiveFunction);

        while (!isOptimalSolution()) {
            int pivotColumn = findPivotColumn();
            int pivotRow = findPivotRow(pivotColumn);
            performPivotOperation(pivotRow, pivotColumn);
        }

        // Формирование вектора решения из таблицы симплекс-метода
        std::vector<double> solution;
        int m = constraints.size();
        int n = constraints[0].size() - 1;

        for (int i = 0; i < m; ++i) {
            solution.push_back(tableau[i][n + m]);
        }

        return solution;
    }

    bool hasIntegerSolution(std::vector<std::vector<double>> constraints, std::vector<double> objectiveFunction) {
        std::vector<double> solution = solve(constraints, objectiveFunction);
        return isIntegerSolution(solution);
    }

    // Структура для представления узла в методе ветвей и границ
    struct Node {
        int x1;
        int x3;
        double value;

        // Конструктор для инициализации узла
        Node(int x1, int x3, double value) : x1(x1), x3(x3), value(value) {}
    };

    // Функция для вычисления значения целевой функции
    double calculateObjectiveValue(int x1, int x3) {
        // Можно изменить на другую целевую функцию по необходимости
        //return 3 * x1 + 4 * x3;
        return c00 * x1 + c11 * x3;
    }

    // Функция для проверки ограничений задачи линейного программирования
    bool checkConstraints(int x1, int x3) {
        // Можно изменить на другие ограничения по необходимости
        //return (7 * x1 + 44 * x3 <= 132) && (13 * x1 + 11 * x3 <= 250);
        return (a00 * x1 + a01 * x3 <= b00) && (a10 * x1 + a01 * x3 <= b11);
    }

    // Основная функция, реализующая метод ветвей и границ
    void branchAndBoundWithOutput() {
        // Максимальные значения переменных
        int max_x1 = 100;
        int max_x3 = 100;

        // Вектор для хранения узлов дерева
        std::vector<Node> nodes;
        // Начальный узел с нулевыми значениями
        nodes.push_back(Node(0, 0, 0));

        // Переменные для хранения оптимального решения
        double max_value = 0;
        int optimal_x1 = 0;
        int optimal_x3 = 0;

        // Цикл, пока есть узлы для обработки
        while (!nodes.empty()) {
            // Извлечение последнего узла из вектора
            Node current = nodes.back();
            nodes.pop_back();

            // Если значение целевой функции текущего узла больше максимального значения
            if (current.value > max_value) {
                // Обновление оптимальных значений
                max_value = current.value;
                optimal_x1 = current.x1; // Сохраняем оптимальное x1
                optimal_x3 = current.x3; // Сохраняем оптимальное x3
            }

            // Перебор вариантов ветвления (две ветви: +1 к x1 или +1 к x3)
            for (int i = 0; i < 2; ++i) {
                int new_x1 = current.x1 + i;
                int new_x3 = current.x3 + (1 - i);

                // Проверка ограничений для новых значений переменных
                if (new_x1 <= max_x1 && new_x3 <= max_x3 && checkConstraints(new_x1, new_x3)) {
                    // Вычисление нового значения целевой функции
                    double new_value = calculateObjectiveValue(new_x1, new_x3);
                    // Если новое значение целевой функции больше текущего максимального значения
                    if (new_value > max_value) {
                        // Добавление нового узла в вектор для дальнейшего рассмотрения
                        nodes.push_back(Node(new_x1, new_x3, new_value));
                    }
                }
            }
        }

        // Вывод оптимальных значений
        std::cout << "Оптимальное значение целевой функции: " << max_value << std::endl;
        std::cout << "Используемые значения x1 и x3 для оптимального решения:" << std::endl;
        std::cout << "x1 = " << optimal_x1 << std::endl;
        std::cout << "x2 = " << optimal_x3 << std::endl;
    }
};

// Главная функция программы
int main() {
    setlocale(LC_ALL, "Rus"); // Установка локали для корректного отображения русского текста
    Simplex simplexSolver; // Создание экземпляра класса Simplex

    int numConstraints, numVariables;
    std::cout << "Введите количество ограничений: ";
    std::cin >> numConstraints;

    std::cout << "Введите количество переменных: ";
    std::cin >> numVariables;

    // Создание вектора для хранения коэффициентов ограничений
    std::vector<std::vector<double>> constraints(numConstraints, std::vector<double>(numVariables + 1));

    std::cout << "Введите коэффициенты для каждого ограничения:" << std::endl;
    // Ввод коэффициентов ограничений
    for (int i = 0; i < numConstraints; ++i) {
        std::cout << "Ограничение " << i + 1 << ": ";
        for (int j = 0; j <= numVariables; ++j) {
            std::cout << "Коэффициент " << j + 1 << ": ";
            std::cin >> constraints[i][j];
        }
    }

    // Создание вектора для хранения коэффициентов целевой функции
    std::vector<double> objectiveFunction(numVariables);
    std::cout << "Введите коэффициенты для целевой функции:" << std::endl;
    // Ввод коэффициентов целевой функции
    for (int i = 0; i < numVariables; ++i) {
        std::cout << "Коэффициент " << i + 1 << ": ";
        std::cin >> objectiveFunction[i];
    }

    // Решение задачи линейного программирования методом симплекса
    std::vector<double> solution = simplexSolver.solve(constraints, objectiveFunction);

    // Установка параметров для метода ветвей и границ
    a00 = constraints[0][0]; a01 = constraints[0][1];
    a10 = constraints[1][0]; a11 = constraints[1][1];
    b00 = constraints[0][2]; b11 = constraints[1][2];
    c00 = objectiveFunction[0]; c11 = objectiveFunction[1];

    if (simplexSolver.hasIntegerSolution(constraints, objectiveFunction)) {
        std::cout << "оптимизация не требуется" << std::endl;
    }
    else {
        // Запуск метода ветвей и границ
        simplexSolver.branchAndBoundWithOutput();
    }

    return 0;
}
