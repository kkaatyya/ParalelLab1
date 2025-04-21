#include <iostream>
#include <vector>
#include <random>
#include <thread>
#include <chrono>
#include <ctime>

using namespace std;
using namespace chrono;

void fillMatrix(vector<vector<int>>& matrix)
{
    random_device rd;
    mt19937 generator(rd());
    uniform_int_distribution<int> distribution(INT_MIN, INT_MAX);

    for (size_t row = 0; row < matrix.size(); row++)
    {
        for (size_t column = 0; column < matrix[row].size(); column++)
        {
            matrix[row][column] = distribution(generator);
        }
    }
}

int findColumnMin(const vector<vector<int>>& matrix, size_t column)
{
    int minValue = INT_MAX;
    for (size_t row = 0; row < matrix.size(); row++)
    {
        minValue = min(minValue, matrix[row][column]);
    }
    return minValue;
}

void computeSequentially(vector<vector<int>>& matrix)
{
    fillMatrix(matrix);

    auto startTime = high_resolution_clock::now();

    for (size_t column = 0; column < matrix.size(); column++)
    {
        matrix[matrix.size() - column - 1][column] = findColumnMin(matrix, column);
    }

    auto endTime = high_resolution_clock::now();
    cout << "Sequential execution time: "
        << duration_cast<milliseconds>(endTime - startTime).count() << " ms\n";
}

void computeInParallel(vector<vector<int>>& matrix, int numberOfThreads)
{
    fillMatrix(matrix);
    auto startTime = high_resolution_clock::now();
    vector<thread> threads;

    auto worker = [&](int threadIndex) {
        for (size_t column = threadIndex; column < matrix.size(); column += numberOfThreads) {
            matrix[matrix.size() - column - 1][column] = findColumnMin(matrix, column);
        }
        };

    for (int i = 0; i < numberOfThreads; i++) {
        threads.emplace_back(worker, i);
    }
    for (auto& thread : threads) {
        thread.join();
    }

    auto endTime = high_resolution_clock::now();
    cout << "Parallel execution time (" << numberOfThreads << " threads): "
        << duration_cast<milliseconds>(endTime - startTime).count() << " ms\n";
}

int main()
{
    vector<int> matrixSizes = { 100, 1000, 10000, 20000, 50000 };
    int numberOfPhysicalCores = thread::hardware_concurrency() / 2;
    int numberOfLogicalCores = thread::hardware_concurrency();
    vector<int> threadConfigurations = {
        numberOfPhysicalCores / 2, numberOfPhysicalCores, numberOfLogicalCores,
        numberOfLogicalCores * 2, numberOfLogicalCores * 4, numberOfLogicalCores * 8, numberOfLogicalCores * 16
    };

    for (int size : matrixSizes) {
        vector<vector<int>> matrix(size, vector<int>(size));
        cout << "Matrix size: " << size << "\n";

        computeSequentially(matrix);

        for (int threadCount : threadConfigurations) {
            if (threadCount < 1) continue;
            matrix.assign(size, vector<int>(size));
            computeInParallel(matrix, threadCount);
        }

        cout << "------------------------------------------------------\n";
    }

    return 0;
}
