#include "core/TSPUtilities.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

// Zamiana wartości dwóch zmiennych
void Utilities::customSwap(int& a, int& b) {
    int temp = a;
    a = b;
    b = temp;
}

// Odwrócenie segmentu trasy
void Utilities::reverseSegment(int* route, int start, int end) {
    while (start < end) {
        customSwap(route[start], route[end]);
        start++;
        end--;
    }
}

// Przeniesienie miasta na inną pozycję
void Utilities::insertPosition(int* route, int i, int j, int numberOfCities) {
    if (i == j) return;
    int city = route[i];
    if (i < j) {
        for (int k = i; k < j; k++) {
            route[k] = route[k + 1];
        }
    }
    else {
        for (int k = i; k > j; k--) {
            route[k] = route[k - 1];
        }
    }
    route[j] = city;
}

// Funkcja minimalna
int Utilities::customMin(int a, int b) {
    return (a < b) ? a : b;
}

// Funkcja maksymalna
int Utilities::customMax(int a, int b) {
    return (a > b) ? a : b;
}

// Obliczanie kosztu trasy
int Utilities::calculateCost(const int* route, int numberOfCities, const int* const* distancesMatrix) {
    int total = 0;
    for (int i = 0; i < numberOfCities - 1; ++i) {
        total += distancesMatrix[route[i]][route[i + 1]];
    }
    total += distancesMatrix[route[numberOfCities - 1]][route[0]]; // Powrót do miasta początkowego
    return total;
}

// Funkcja weryfikująca poprawność trasy
bool Utilities::validateRoute(const int* route, int numberOfCities) {
    bool* visited = new bool[numberOfCities];
    for (int i = 0; i < numberOfCities; ++i) {
        visited[i] = false;
    }

    for (int i = 0; i < numberOfCities; ++i) {
        if (route[i] < 0 || route[i] >= numberOfCities || visited[route[i]]) {
            delete[] visited;
            return false; // Nieprawidłowa trasa
        }
        visited[route[i]] = true;
    }

    delete[] visited;
    return true; // Trasa jest poprawna
}

// Funkcja drukująca trasę (dla celów debugowania)
void Utilities::printRoute(const int* route, int numberOfCities) {
    for (int i = 0; i < numberOfCities; ++i) {
        cout << route[i] << " ";
    }
    cout << "\n";
}

// Sprawdzenie, czy miasto jest w segmencie
bool Utilities::isInSegment(const int* segment, int start, int end, int city) {
    for (int i = start; i <= end; ++i) {
        if (segment[i] == city) {
            return true;
        }
    }
    return false;
}
