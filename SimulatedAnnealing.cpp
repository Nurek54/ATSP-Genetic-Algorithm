#include "SimulatedAnnealing.h"
#include "TSPUtilities.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <iostream>
#include <cstring>
#include <chrono>

using namespace std;

// Konstruktor
SimulatedAnnealing::SimulatedAnnealing(const int* const* distancesMatrix,
                                       int numberOfCities,
                                       double startingTemperature,
                                       double temperatureDecreaseFactor,
                                       int timeLimitSec,
                                       int maximumSteps,
                                       const string& neighborhoodType,
                                       const string& resultFile)
        : distancesMatrix(distancesMatrix),
          numberOfCities(numberOfCities),
          startingTemperature(startingTemperature),
          temperatureDecreaseFactor(temperatureDecreaseFactor),
          timeLimitSec(timeLimitSec),
          maximumSteps(maximumSteps),
          neighborhoodType(neighborhoodType),
          resultFile(resultFile),
          saveToCSV(resultFile),
          globalBestCost(INT32_MAX),
          timesWithoutBetterSolution(0),
          noImprovementThreshold(500) {

    activeRoute = new int[numberOfCities];
    globalBestRoute = new int[numberOfCities];
    srand(static_cast<unsigned int>(time(nullptr)));

    buildGreedyRoute();
    globalBestCost = currentRouteCost = computeRouteCost(activeRoute);

    if (startingTemperature <= 0) {
        this->currentTemperature = computeInitialTemperature();
    } else {
        this->currentTemperature = startingTemperature;
    }
}


// Destruktor
SimulatedAnnealing::~SimulatedAnnealing() {
    delete[] activeRoute;
    delete[] globalBestRoute;
}

// Obliczanie temperatury początkowej
double SimulatedAnnealing::computeInitialTemperature() {
    int maxDelta = 0;
    int sampleSize = numberOfCities * 100;

    // Szukanie maksymalnej różnicy kosztów
    for (int i = 0; i < sampleSize; ++i) {
        int a = rand() % numberOfCities;
        int b = rand() % numberOfCities;

        while (a == b) {
            b = rand() % numberOfCities;
        }

        swapCities(activeRoute, a, b);
        int newCost = computeRouteCost(activeRoute);
        int delta = abs(newCost - currentRouteCost);

        if (delta > maxDelta) {
            maxDelta = delta;
        }

        // Cofnięcie ruchu
        swapCities(activeRoute, a, b);
    }

    // Ustalanie temperatury początkowej według wzoru
    double initialTemp = -static_cast<double>(maxDelta) / log(0.9);
    cout << "Obliczona temperatura poczatkowa (poprawiona): " << initialTemp << endl;

    return initialTemp;
}


// Generowanie początkowego rozwiązania za pomocą metody zachłannej
void SimulatedAnnealing::buildGreedyRoute() {
    bool* visited = new bool[numberOfCities] {false};
    activeRoute[0] = 0;
    visited[0] = true;

    for (int i = 1; i < numberOfCities; ++i) {
        int last = activeRoute[i - 1];
        int nearest = -1;
        int minCost = INT32_MAX;

        for (int j = 0; j < numberOfCities; ++j) {
            if (!visited[j] && distancesMatrix[last][j] < minCost) {
                minCost = distancesMatrix[last][j];
                nearest = j;
            }
        }

        if (nearest == -1) {
            for (int j = 0; j < numberOfCities; ++j) {
                if (!visited[j]) {
                    nearest = j;
                    break;
                }
            }
        }

        activeRoute[i] = nearest;
        visited[nearest] = true;
    }

    delete[] visited;
    memcpy(globalBestRoute, activeRoute, numberOfCities * sizeof(int));
    globalBestCost = computeRouteCost(activeRoute);
}

// Obliczanie kosztu trasy
int SimulatedAnnealing::computeRouteCost(const int* route) const {
    int cost = 0;
    for (int i = 0; i < numberOfCities - 1; ++i) {
        cost += distancesMatrix[route[i]][route[i + 1]];
    }
    cost += distancesMatrix[route[numberOfCities - 1]][route[0]];
    return cost;
}

void SimulatedAnnealing::swapCities(int* solution, int i, int j) {
    Utilities::customSwap(solution[i], solution[j]);
}

void SimulatedAnnealing::insertCity(int* solution, int from, int to) {
    if (from == to) {
        return;
    }

    int city = solution[from];

    if (from < to) {
        for (int i = from; i < to; ++i) {
            solution[i] = solution[i + 1];
        }
    } else {
        for (int i = from; i > to; --i) {
            solution[i] = solution[i - 1];
        }
    }

    solution[to] = city;
}

void SimulatedAnnealing::reverseSegment(int* solution, int start, int end) {
    while (start < end) {
        Utilities::customSwap(solution[start], solution[end]);
        ++start;
        --end;
    }
}

// Decyzja o akceptacji ruchu
bool SimulatedAnnealing::shouldAcceptMove(int delta) {
    if (delta < 0) {
        return true;
    }
    double probability = exp(-delta / currentTemperature);
    if ((static_cast<double>(rand()) / RAND_MAX) < probability) {
        return true;
    } else {
        return false;
    }
}

void SimulatedAnnealing::recordResults(chrono::duration<double> seconds,
                                       chrono::duration<double, milli> milliseconds,
                                       double finalTemperature) {
    cout << "=== Wyniki Symulowanego Wyzarzania ===" << endl;
    cout << "Najlepszy koszt: " << globalBestCost << endl;
    cout << "Czas wykonania: " << seconds.count() << " sekund (" << milliseconds.count() << " ms)" << endl;
    cout << "Temperatura koncowa: " << finalTemperature << endl;
    cout << "Typ sasiedztwa: " << neighborhoodType << endl;
    cout << "Parametr algorytmu (Cooling Rate): " << temperatureDecreaseFactor << endl;

    saveToCSV.saveResults(
            "Simulated Annealing",   // Nazwa algorytmu
            temperatureDecreaseFactor, // Parametr algorytmu
            finalTemperature,        // Końcowa temperatura
            seconds,                 // Sekundy
            milliseconds,            // Milisekundy
            globalBestCost,          // Koszt
            neighborhoodType,        // Typ sąsiedztwa
            globalBestRoute,         // Ścieżka
            numberOfCities           // Liczba miast
    );
}

// Uruchomienie algorytmu SA
void SimulatedAnnealing::run() {
    cout << "Rozpoczecie Symulowanego Wyzarzania z typem sasiedztwa: " << neighborhoodType << "...\n";

    auto startTime = chrono::high_resolution_clock::now();
    int iterationsPerTemperature = 100 * numberOfCities; // Liczba ruchów na poziomie temperatury

    while ((chrono::duration_cast<chrono::seconds>(
            chrono::high_resolution_clock::now() - startTime).count() < timeLimitSec)
           && (currentTemperature > 1e-6)) {

        bool improvement = false;

        for (int iter = 0; iter < iterationsPerTemperature; ++iter) {
            int i = rand() % numberOfCities;
            int j = rand() % numberOfCities;
            while (i == j) j = rand() % numberOfCities;

            // Używamy typu sąsiedztwa ustawionego przez użytkownika
            if (neighborhoodType == "swap") {
                swapCities(activeRoute, i, j);
            }
            else if (neighborhoodType == "insert") {
                insertCity(activeRoute, i, j);
            }
            else if (neighborhoodType == "reverse") {
                reverseSegment(activeRoute, Utilities::customMin(i, j), Utilities::customMax(i, j));
            }
            else {
                int moveTypeRandom = rand() % 3;
                switch (moveTypeRandom) {
                    case 0: swapCities(activeRoute, i, j); break;
                    case 1: insertCity(activeRoute, i, j); break;
                    case 2: reverseSegment(activeRoute, Utilities::customMin(i, j), Utilities::customMax(i, j)); break;
                }
            }

            int newCost = computeRouteCost(activeRoute);
            int delta = newCost - currentRouteCost;

            if (shouldAcceptMove(delta)) {
                currentRouteCost = newCost;
                improvement = true;
                if (currentRouteCost < globalBestCost) {
                    globalBestCost = currentRouteCost;
                    memcpy(globalBestRoute, activeRoute, numberOfCities * sizeof(int));
                }
            } else {
                // Cofnięcie ruchu w zależności od typu sąsiedztwa
                if (neighborhoodType == "swap") {
                    swapCities(activeRoute, i, j);
                }
                else if (neighborhoodType == "insert") {
                    insertCity(activeRoute, j, i);
                }
                else if (neighborhoodType == "reverse") {
                    reverseSegment(activeRoute, Utilities::customMin(i, j), Utilities::customMax(i, j));
                }
            }
        }

        // Spadek temperatury po wykonaniu wewnętrznej pętli
        currentTemperature *= temperatureDecreaseFactor;

        // Warunek braku poprawy
        if (!improvement) {
            timesWithoutBetterSolution++;
            if (timesWithoutBetterSolution >= noImprovementThreshold) {
                cout << "Restartowanie z rozwiazania zachlannego...\n";
                buildGreedyRoute();
                currentRouteCost = computeRouteCost(activeRoute);
                timesWithoutBetterSolution = 0;
            }
        }
        else {
            timesWithoutBetterSolution = 0;
        }
    }

    auto endTime = chrono::high_resolution_clock::now();
    auto seconds = chrono::duration_cast<chrono::duration<double>>(endTime - startTime);
    auto milliseconds = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);

    recordResults(seconds, milliseconds, currentTemperature);
}

// Zwracanie najlepszego kosztu
int SimulatedAnnealing::retrieveBestCost() const {
    return globalBestCost;
}

// Zwracanie najlepszej trasy
const int* SimulatedAnnealing::retrieveBestRoute() const {
    return globalBestRoute;
}
