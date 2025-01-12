#include "TabuSearch.h"
#include "TSPUtilities.h"
#include <iostream>
#include <cstdlib>
#include <limits>
#include <chrono>
#include <cstring>

using namespace std;

// Konstruktor
TabuSearch::TabuSearch(const int* const* distancesMatrix,
                       int numberOfCities,
                       int timeLimitSec,
                       int tabuTenure,
                       int maximumSteps,
                       const string& neighborhoodType,
                       const string& resultFile)
        : distancesMatrix(distancesMatrix),
          numberOfCities(numberOfCities),
          timeLimitSec(timeLimitSec),
          tabuTenure(tabuTenure),
          maximumSteps(maximumSteps),
          neighborhoodType(neighborhoodType),
          resultFile(resultFile),
          tabuIndex(0),
          saveToCSV(resultFile),
          iterationsSinceLastImprovement(0),
          diversificationThreshold(1000), // Próg iteracji bez poprawy
          diversificationFactor(0.1)
{
    if (neighborhoodType != "swap" && neighborhoodType != "reverse" && neighborhoodType != "insert") {
        throw invalid_argument("Zly typ sasiedztwa: " + neighborhoodType);
    }

    globalBestRoute = new int[numberOfCities];
    activeRoute = new int[numberOfCities];

    tabuList = new int*[tabuTenure];
    for (int i = 0; i < tabuTenure; ++i) {
        tabuList[i] = new int[2]{-1, -1};
    }

    srand(static_cast<unsigned int>(time(nullptr)));

    // Informacja o używanym typie sąsiedztwa
    cout << "[INFO] Uzywany typ sasiedztwa: " << neighborhoodType << "\n";

    buildGreedyRoute();
    globalBestCost = computeRouteCost(activeRoute);
    currentRouteCost = globalBestCost;

}

// Destruktor
TabuSearch::~TabuSearch() {
    delete[] globalBestRoute;
    delete[] activeRoute;

    for (int i = 0; i < tabuTenure; i++) {
        delete[] tabuList[i];
    }
    delete[] tabuList;

}

// Zwracanie najlepszego kosztu
int TabuSearch::retrieveBestCost() const {
    return globalBestCost;
}

// Zwracanie najlepszej trasy
const int* TabuSearch::retrieveBestRoute() const {
    return globalBestRoute;
}

// Ręczne przetasowanie trasy (implementacja Fisher-Yates)
void TabuSearch::shuffleRoute(int* route, int start, int end) {
    for (int i = end - 1; i > start; --i) {
        int j = start + rand() % (i - start + 1);
        // Swap route[i] and route[j]
        int temp = route[i];
        route[i] = route[j];
        route[j] = temp;
    }
}

// Budowanie rozwiązania początkowego metodą zachłanną
void TabuSearch::buildGreedyRoute() {
    bool* visited = new bool[numberOfCities];
    for (int i = 0; i < numberOfCities; ++i) {
        visited[i] = false;
    }

    activeRoute[0] = 0;
    visited[0] = true;

    for (int i = 1; i < numberOfCities; i++) {
        int lastCity = activeRoute[i - 1];
        int nearestCity = -1;
        int minCost = numeric_limits<int>::max();

        for (int j = 0; j < numberOfCities; j++) {
            if (!visited[j]) {
                int cost = distancesMatrix[lastCity][j];
                if (cost < minCost) {
                    minCost = cost;
                    nearestCity = j;
                }
            }
        }
        if (nearestCity == -1) {
            // Jeśli nie znaleziono nieodwiedzonego miasta, wybierz losowe
            nearestCity = rand() % numberOfCities;
            while (visited[nearestCity]) {
                nearestCity = rand() % numberOfCities;
            }
        }
        activeRoute[i] = nearestCity;
        visited[nearestCity] = true;
    }

    // Opcjonalne: Przetasowanie trasy początkowej
    /*
    if (numberOfCities > 2) {
        shuffleRoute(activeRoute, 1, numberOfCities);
    }
    */

    memcpy(globalBestRoute, activeRoute, numberOfCities * sizeof(int));
    globalBestCost = computeRouteCost(activeRoute);

    delete[] visited;
}

// Obliczanie kosztu trasy
int TabuSearch::computeRouteCost(const int* route) const {
    int total = Utilities::calculateCost(route, numberOfCities, distancesMatrix);
    return total;
}

// Sprawdzanie, czy ruch jest tabu
bool TabuSearch::isTabu(int cityA, int cityB) const {
    for (int i = 0; i < tabuTenure; i++) {
        int a = tabuList[i][0];
        int b = tabuList[i][1];
        if ((a == cityA && b == cityB) || (a == cityB && b == cityA)) {
            return true;
        }
    }
    return false;
}

// Dodawanie ruchu do listy tabu
void TabuSearch::addToTabuList(int cityA, int cityB) {
    tabuList[tabuIndex][0] = cityA;
    tabuList[tabuIndex][1] = cityB;
    tabuIndex = (tabuIndex + 1) % tabuTenure;
}

// Wykonanie jednego ruchu w algorytmie Tabu Search
void TabuSearch::performMove(int* route, int& cost) {
    // Liczba losowych ruchów do sprawdzenia w tej iteracji
    int numberOfRandomMoves = 100;
    int bestNeighborCost = numeric_limits<int>::max();
    int bestI = -1, bestJ = -1;

    for (int move = 0; move < numberOfRandomMoves; ++move) {
        int i = rand() % numberOfCities;
        int j = rand() % numberOfCities;
        while (j == i) {
            j = rand() % numberOfCities;
        }

        // Aktualny typ sąsiedztwa
        string currentNeighborhood = neighborhoodType;

        // Kopiowanie aktualnej trasy do tymczasowej trasy
        int* tempRoute = new int[numberOfCities];
        memcpy(tempRoute, route, numberOfCities * sizeof(int));

        // Wykonanie wybranego ruchu przy użyciu funkcji z Utilities
        if (currentNeighborhood == "swap") {
            Utilities::customSwap(tempRoute[i], tempRoute[j]);
        }
        else if (currentNeighborhood == "insert") {
            Utilities::insertPosition(tempRoute, i, j, numberOfCities);
        }
        else if (currentNeighborhood == "reverse") {
            int start = Utilities::customMin(i, j);
            int end = Utilities::customMax(i, j);
            Utilities::reverseSegment(tempRoute, start, end);
        }

        // Obliczanie kosztu tymczasowej trasy
        int tempCost = computeRouteCost(tempRoute);

        // Sprawdzenie warunków: czy ruch nie jest tabu i czy jest lepszy
        if (((tempCost < bestNeighborCost) && !isTabu(route[i], route[j])) || (tempCost < globalBestCost)) {
            bestNeighborCost = tempCost;
            bestI = i;
            bestJ = j;
        }

        delete[] tempRoute;
    }

    // Wykonanie najlepszego znalezionego ruchu
    if (bestI != -1 && bestJ != -1) {
        if (neighborhoodType == "swap") {
            Utilities::customSwap(route[bestI], route[bestJ]);
        }
        else if (neighborhoodType == "insert") {
            Utilities::insertPosition(route, bestI, bestJ, numberOfCities);
        }
        else if (neighborhoodType == "reverse") {
            int start = Utilities::customMin(bestI, bestJ);
            int end = Utilities::customMax(bestI, bestJ);
            Utilities::reverseSegment(route, start, end);
        }

        // Aktualizacja kosztu bieżącej trasy
        cost = computeRouteCost(route);

        // Aktualizacja najlepszego kosztu i trasy, jeśli to konieczne
        if (cost < globalBestCost) {
            globalBestCost = cost;
            memcpy(globalBestRoute, route, numberOfCities * sizeof(int));
            iterationsSinceLastImprovement = 0; // Resetowanie licznika
        }
        else {
            iterationsSinceLastImprovement++;
        }

        // Dodanie ruchu do listy tabu
        addToTabuList(bestI, bestJ);
    }
}

// Metoda Dywersyfikacji
void TabuSearch::performDiversification() {
    /*cout << "Wprowadzanie dywersyfikacji po "
              << iterationsSinceLastImprovement << " iteracjach bez poprawy.\n";*/

    // Obliczenie liczby miast do przetasowania
    int numberOfShuffles = static_cast<int>(numberOfCities * diversificationFactor);
    if (numberOfShuffles < 2) numberOfShuffles = 2; // Minimalna liczba przetasowań

    for (int shuffle = 0; shuffle < numberOfShuffles; ++shuffle) {
        // Losowanie dwóch pozycji do zamiany
        int i = rand() % numberOfCities;
        int j = rand() % numberOfCities;
        while (j == i) {
            j = rand() % numberOfCities;
        }

        // Wykonanie zamiany
        Utilities::customSwap(activeRoute[i], activeRoute[j]);

        // Dodanie ruchu do listy tabu, aby uniknąć natychmiastowego powrotu
        addToTabuList(i, j);
    }

    // Aktualizacja kosztu bieżącej trasy po dywersyfikacji
    currentRouteCost = computeRouteCost(activeRoute);

    // Resetowanie licznika iteracji bez poprawy
    iterationsSinceLastImprovement = 0;

    /*cout << "Dywersyfikacja zakończona. Nowy koszt trasy: "
              << globalBestCost << "\n";*/
}

// Uruchomienie algorytmu
void TabuSearch::run() {
    auto startTime = chrono::high_resolution_clock::now();
    int step = 0;
    maximumSteps = numberOfCities * 400;

    while (step < maximumSteps &&
           chrono::duration_cast<chrono::seconds>(
                   chrono::high_resolution_clock::now() - startTime).count() < timeLimitSec) {

        performMove(activeRoute, currentRouteCost);
        step++;

        // Sprawdzenie, czy należy przeprowadzić dywersyfikację
        if (iterationsSinceLastImprovement >= diversificationThreshold) {
            performDiversification();
        }
    }

    auto endTime = chrono::high_resolution_clock::now();
    auto seconds = chrono::duration_cast<chrono::duration<double>>(endTime - startTime);
    auto milliseconds = chrono::duration_cast<chrono::duration<double, milli>>(endTime - startTime);

    recordResults(seconds, milliseconds);
}

// Logowanie wyników
void TabuSearch::recordResults(chrono::duration<double> seconds,
                               chrono::duration<double, milli> milliseconds) {
    cout << "Najlepszy koszt: " << globalBestCost << "\n";
    cout << "Czas dzialania: " << seconds.count() << " sekund ("
              << milliseconds.count() << " ms)\n";
    cout << "Typ sqsiedztwa: " << neighborhoodType << "\n";
    cout << "Parametr tabuTenure: " << tabuTenure << "\n";

    saveToCSV.saveResults(
            "Tabu Search",                       // Nazwa algorytmu
            static_cast<double>(tabuTenure),     // Parametr algorytmu (tabuTenure)
            0.0,                                 // Brak temperatury końcowej
            seconds,                             // Sekundy
            milliseconds,                        // Milisekundy
            globalBestCost,                      // Koszt
            neighborhoodType,                    // Typ sąsiedztwa
            globalBestRoute,                     // Ścieżka
            numberOfCities                       // Liczba miast
    );
}
