#include <iostream>
#include <vector>
#include <random>
#include <thread>
#include <chrono>
#include <ctime>

using namespace std;
using namespace chrono;

class MatrixWorker 
{
private:
    int threadIndex, totalThreads;
    vector<vector<int>>& matrix;

public:
    MatrixWorker(int threadIndex, int totalThreads, vector<vector<int>>& matrix)
        : threadIndex(threadIndex), totalThreads(totalThreads), matrix(matrix) {}

    void operator()() 
    {
        random_device rd;
        mt19937 generator(rd());
        uniform_int_distribution<int> distribution(INT_MIN, INT_MAX);

        for (size_t column = threadIndex; column < matrix.size(); column += totalThreads) 
        {
            int minValueInColumn = INT_MAX;

            // Перший прохід: заповнюємо матрицю випадковими числами та шукаємо мінімум у стовпці
            for (size_t row = 0; row < matrix.size(); row++) 
            {
                matrix[row][column] = distribution(generator);
                minValueInColumn = min(minValueInColumn, matrix[row][column]);
            }

            // Другий прохід: замінюємо елемент побічної діагоналі на мінімум
            matrix[matrix.size() - column - 1][column] = minValueInColumn;
        }
    }
};

void computeSequentially(vector<vector<int>>& matrix) 
{
    auto startTime = high_resolution_clock::now();
    random_device rd;
    mt19937 generator(rd());
    uniform_int_distribution<int> distribution(INT_MIN, INT_MAX);

    for (size_t column = 0; column < matrix.size(); column++) 
    {
        int minValueInColumn = INT_MAX;

        // Перший прохід: заповнюємо матрицю випадковими числами та шукаємо мінімум у стовпці
        for (size_t row = 0; row < matrix.size(); row++) 
        {
            matrix[row][column] = distribution(generator);
            minValueInColumn = min(minValueInColumn, matrix[row][column]);
        }

        // Другий прохід: замінюємо елемент побічної діагоналі на мінімум
        matrix[matrix.size() - column - 1][column] = minValueInColumn;
    }

    auto endTime = high_resolution_clock::now();
    cout << "Sequential execution time: "
        << duration_cast<milliseconds>(endTime - startTime).count() << " ms\n";
}

void computeInParallel(vector<vector<int>>& matrix, int numberOfThreads) 
{
    auto startTime = high_resolution_clock::now();
    vector<thread> threads;

    for (int i = 0; i < numberOfThreads; i++) 
    {
        threads.emplace_back(MatrixWorker(i, numberOfThreads, ref(matrix)));
    }

    for (auto& thread : threads) 
    {
        thread.join();
    }

    auto endTime = high_resolution_clock::now();
    cout << "Parallel execution time (" << numberOfThreads << " threads): "
        << duration_cast<milliseconds>(endTime - startTime).count() << " ms\n";
}

// Функція для перевірки мінімуму в стовпці
void checkColumnMinimum(const vector<vector<int>>& matrix, size_t column) 
{
    int minValue = INT_MAX;
    for (size_t row = 0; row < matrix.size(); row++) {
        minValue = min(minValue, matrix[row][column]);
    }

    // Перевірка мінімуму на побічній діагоналі
    if (matrix[matrix.size() - column - 1][column] == minValue) {
        cout << "Column " << column << " is correct.\n";
    }
    else 
    {
        cout << "Column " << column << " is incorrect.\n";
    }
}

int main() 
{
    vector<int> matrixSizes = { 100, 1000, 10000, 20000 }; // Різні розміри матриці
    int numberOfPhysicalCores = thread::hardware_concurrency() / 2;
    int numberOfLogicalCores = thread::hardware_concurrency();
    vector<int> threadConfigurations = {
        numberOfPhysicalCores / 2, numberOfPhysicalCores, numberOfLogicalCores,
        numberOfLogicalCores * 2, numberOfLogicalCores * 4, numberOfLogicalCores * 8, numberOfLogicalCores * 16
    };

    for (int size : matrixSizes) 
    {
        vector<vector<int>> matrix(size, vector<int>(size));
        cout << "Matrix size: " << size << "\n";

        computeSequentially(matrix);

        for (int threadCount : threadConfigurations) 
        {
            if (threadCount < 1) continue; // Мінімум 1 потік
            matrix.assign(size, vector<int>(size));
            // Очищення перед запуском
            computeInParallel(matrix, threadCount);
        }

        // Вибір трьох випадкових стовпців для перевірки
        random_device rd;
        mt19937 generator(rd());
        uniform_int_distribution<int> distribution(0, size - 1);

        for (int i = 0; i < 3; i++) 
        {
            size_t randomColumn = distribution(generator);
            checkColumnMinimum(matrix, randomColumn);
        }

        cout << "------------------------------------------------------\n";
    }

    return 0;
}