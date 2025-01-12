#ifndef TABUSEARCH_H
#define TABUSEARCH_H

#include "SaveToCSV.h"
#include <string>
#include <chrono>

using namespace std;

class TabuSearch {
public:
    // Konstruktor
    TabuSearch(const int* const* distancesMatrix,
               int numberOfCities,
               int timeLimitSec,
               int tabuTenure,
               int maximumSteps,
               const string& neighborhoodType,
               const string& resultFile);

    // Destruktor
    ~TabuSearch();

    // Uruchomienie algorytmu Tabu Search
    void run();

    // Zwracanie najlepszego kosztu
    int retrieveBestCost() const;

    // Zwracanie najlepszej trasy
    const int* retrieveBestRoute() const;

private:
    // Macierz odległości między miastami
    const int* const* distancesMatrix;

    // Liczba miast
    int numberOfCities;

    // Maksymalny czas działania algorytmu w sekundach
    int timeLimitSec;

    // Okres tabu
    int tabuTenure;

    // Maksymalna liczba kroków
    int maximumSteps;

    // Typ sąsiedztwa (swap, insert, reverse)
    string neighborhoodType;

    // Nazwa pliku wynikowego
    string resultFile;

    // Najlepsza znaleziona trasa
    int* globalBestRoute;

    // Aktualna trasa
    int* activeRoute;

    // Koszt najlepszej trasy
    int globalBestCost;

    // Koszt bieżącej trasy
    int currentRouteCost;

    // Lista tabu przechowująca ostatnie tabuTenure ruchów
    int** tabuList;

    // Indeks bieżący w liście tabu
    int tabuIndex;

    // Obiekt do zapisywania wyników do pliku CSV
    SaveToCSV saveToCSV;

    // Dywersyfikacja
    int iterationsSinceLastImprovement;    // Licznik iteracji bez poprawy
    int diversificationThreshold;         // Próg iteracji bez poprawy do dywersyfikacji
    double diversificationFactor;         // Procent trasy do przetasowania podczas dywersyfikacji

    // Budowanie rozwiązania początkowego metodą zachłanną
    void buildGreedyRoute();

    // Obliczanie kosztu trasy
    int computeRouteCost(const int* route) const;

    // Sprawdzanie, czy ruch jest tabu
    bool isTabu(int cityA, int cityB) const;

    // Dodawanie ruchu do listy tabu
    void addToTabuList(int cityA, int cityB);

    // Wykonanie jednego ruchu w algorytmie Tabu Search
    void performMove(int* route, int& cost);

    // Logowanie wyników
    void recordResults(chrono::duration<double> seconds,
                       chrono::duration<double, milli> milliseconds);

    // Ręczne przetasowanie trasy (implementacja Fisher-Yates)
    void shuffleRoute(int* route, int start, int end);

    // Metoda Dywersyfikacji
    void performDiversification();
};

#endif // TABUSEARCH_H
