#ifndef TSPUTILITIES_H
#define TSPUTILITIES_H

using namespace std;

namespace Utilities {
    // Zamiana wartości dwóch zmiennych
    void customSwap(int& a, int& b);

    // Odwrócenie segmentu trasy
    void reverseSegment(int* route, int start, int end);

    // Przeniesienie miasta na inną pozycję
    void insertPosition(int* route, int i, int j, int numberOfCities);

    // Funkcja minimalna
    int customMin(int a, int b);

    // Funkcja maksymalna
    int customMax(int a, int b);

    // Obliczanie kosztu trasy
    int calculateCost(const int* route, int numberOfCities, const int* const* distancesMatrix);

    // Funkcja weryfikująca poprawność trasy
    bool validateRoute(const int* route, int numberOfCities);

    // Funkcja drukująca trasę (dla celów debugowania)
    void printRoute(const int* route, int numberOfCities);

    // Sprawdzenie, czy miasto jest w segmencie
    bool isInSegment(const int* segment, int start, int end, int city);
}

#endif // TSPUTILITIES_H
