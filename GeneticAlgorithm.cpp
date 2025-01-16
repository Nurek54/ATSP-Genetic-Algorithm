#include "GeneticAlgorithm.h"
#include "TSPUtilities.h"
#include "SaveToCSV.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <cstdint>
#include <chrono>
#include <climits>

using namespace std;

// Konstruktor
GeneticAlgorithm::GeneticAlgorithm(const int* const* distanceMatrix,
                                   int cityCount,
                                   int populationSize,
                                   double crossoverRate,
                                   double mutationRate,
                                   const string& crossoverMethod,
                                   const string& mutationMethod,
                                   const string& selectionMethod,
                                   int maxGenerations,
                                   int timeLimit,
                                   const string& /*logFile*/)
        : distanceMatrix(distanceMatrix),
          cityCount(cityCount),
          populationSize(populationSize),
          crossoverRate(crossoverRate),
          mutationRate(mutationRate),
          crossoverMethod(crossoverMethod),
          mutationMethod(mutationMethod),
          selectionMethod(selectionMethod),
          maxGenerations(maxGenerations),
          timeLimit(timeLimit) {

    // Alokacja pamięci na populację
    population = new int*[populationSize];
    for (int i = 0; i < populationSize; ++i) {
        population[i] = new int[cityCount];
    }

    // Alokacja pamięci na fitness i bestSolution
    fitness = new int[populationSize];
    bestSolution = new int[cityCount];

    bestCost = INT32_MAX; // Na start przyjmujemy duży koszt
}

// Destruktor
GeneticAlgorithm::~GeneticAlgorithm() {
    freePopulation();    // Zwalnianie tablicy population
    delete[] fitness;    // Zwalnianie tablicy fitness
    delete[] bestSolution;  // Zwalnianie tablicy bestSolution
}

// Funkcja zwalniająca pamięć populacji
void GeneticAlgorithm::freePopulation() {
    if (population) {
        for (int i = 0; i < populationSize; ++i) {
            delete[] population[i];
        }
        delete[] population;
        population = nullptr;
    }
}

//--------------------------------------------------------------------------------------------
// Metoda: createRandomNearestNeighborSolution
// Tworzy ścieżkę metodą "losowy NN", tzn.:
//  1) losowo wybieramy miasto startowe
//  2) w każdej iteracji idziemy do najbliższego jeszcze nieodwiedzonego miasta
//--------------------------------------------------------------------------------------------
void GeneticAlgorithm::createRandomNearestNeighborSolution(int* solution) {
    // Dynamiczna tablica bool odwiedzonych miast
    bool* visited = new bool[cityCount];
    for(int i = 0; i < cityCount; i++) {
        visited[i] = false;
    }

    // Wybór losowego miasta startowego
    int startCity = rand() % cityCount;
    solution[0] = startCity;
    visited[startCity] = true;

    int currentCity = startCity;

    // Iterujemy, zawsze wybierając najbliższe nieodwiedzone miasto
    for (int i = 1; i < cityCount; ++i) {
        int nextCity = -1;
        int bestDist = INT_MAX;
        for (int c = 0; c < cityCount; ++c) {
            if (!visited[c]) {
                int dist = distanceMatrix[currentCity][c];
                if (dist < bestDist) {
                    bestDist = dist;
                    nextCity = c;
                }
            }
        }
        solution[i] = nextCity;
        visited[nextCity] = true;
        currentCity = nextCity;
    }

    delete[] visited; // Zwolnij tablicę visited
}

//--------------------------------------------------------------------------------------------
// Inicjalizacja populacji: wypełniamy większość populacji losowo,
// a jednego osobnika (indeks 0) metodą "random NN"
//--------------------------------------------------------------------------------------------
void GeneticAlgorithm::initializePopulation() {
    srand(static_cast<unsigned>(time(nullptr)));

    // Najpierw generujemy losowe permutacje
    for (int i = 0; i < populationSize; ++i) {
        // Wypełnij tablicę wartościami 0..(cityCount-1)
        for (int j = 0; j < cityCount; ++j) {
            population[i][j] = j;
        }
        // Tasowanie (Fisher-Yates)
        for (int j = cityCount - 1; j > 0; --j) {
            int k = rand() % (j + 1);
            Utilities::customSwap(population[i][j], population[i][k]);
        }
    }

    // Nadpisz jednego osobnika (np. population[0]) ścieżką z random NN
    createRandomNearestNeighborSolution(population[0]);
}

// Kopiowanie rozwiązania
void GeneticAlgorithm::copyIndividual(const int* source, int* destination) {
    for (int i = 0; i < cityCount; ++i) {
        destination[i] = source[i];
    }
}

// Ocena populacji
void GeneticAlgorithm::evaluatePopulation() {
    for (int i = 0; i < populationSize; ++i) {
        fitness[i] = calculateCost(population[i]);
        // Sprawdzenie, czy mamy nowe optimum globalne
        if (fitness[i] < bestCost) {
            bestCost = fitness[i];
            copyIndividual(population[i], bestSolution);
        }
    }
}

// Obliczanie kosztu trasy
int GeneticAlgorithm::calculateCost(const int* solution) const {
    int cost = 0;
    for (int i = 0; i < cityCount - 1; ++i) {
        cost += distanceMatrix[solution[i]][solution[i + 1]];
    }
    // Powrót do miasta startowego
    cost += distanceMatrix[solution[cityCount - 1]][solution[0]];
    return cost;
}

// Selekcja rodziców (Turniejowa)
void GeneticAlgorithm::selectParents(int& parent1Idx, int& parent2Idx) {
    int tournamentSize = 5;
    auto selectOne = [&]() {
        int bestIdx = rand() % populationSize;
        for (int i = 1; i < tournamentSize; ++i) {
            int idx = rand() % populationSize;
            if (fitness[idx] < fitness[bestIdx]) {
                bestIdx = idx;
            }
        }
        return bestIdx;
    };

    parent1Idx = selectOne();
    parent2Idx = selectOne();
}

// Krzyżowanie
void GeneticAlgorithm::crossover(const int* parent1,
                                 const int* parent2,
                                 int* offspring1,
                                 int* offspring2) {
    if (crossoverMethod == "PMX") {
        performPMXCrossover(parent1, parent2, offspring1, offspring2);
    } else if (crossoverMethod == "OX") {
        performOrderCrossover(parent1, parent2, offspring1, offspring2);
    }
}

// Mutacja
void GeneticAlgorithm::mutate(int* individual) {
    if (mutationMethod == "swap") {
        performSwapMutation(individual);
    } else if (mutationMethod == "insert") {
        performInsertMutation(individual);
    } else if (mutationMethod == "reverse") {
        performReverseMutation(individual);
    }
}

// PMX Krzyżowanie
void GeneticAlgorithm::performPMXCrossover(const int* parent1,
                                           const int* parent2,
                                           int* offspring1,
                                           int* offspring2) {
    int start = rand() % cityCount;
    int end = start + rand() % (cityCount - start);

    for (int i = 0; i < cityCount; ++i) {
        offspring1[i] = -1;
        offspring2[i] = -1;
    }

    // Skopiuj segment
    for (int i = start; i <= end; ++i) {
        offspring1[i] = parent1[i];
        offspring2[i] = parent2[i];
    }

    // Funkcja lambda do wypełnienia pozostałych elementów
    auto fillOffspring = [&](const int* parent, int* offspring) {
        for (int i = 0; i < cityCount; ++i) {
            // Sprawdź, czy już znajduje się w skopiowanym fragmencie
            if (!Utilities::isInSegment(offspring, start, end, parent[i])) {
                // Znajdź miejsce puste (-1)
                for (int j = 0; j < cityCount; ++j) {
                    if (offspring[j] == -1) {
                        offspring[j] = parent[i];
                        break;
                    }
                }
            }
        }
    };

    fillOffspring(parent2, offspring1);
    fillOffspring(parent1, offspring2);
}

// OX Krzyżowanie
void GeneticAlgorithm::performOrderCrossover(const int* parent1,
                                             const int* parent2,
                                             int* offspring1,
                                             int* offspring2) {
    int start = rand() % cityCount;
    int end = start + rand() % (cityCount - start);

    for (int i = start; i <= end; ++i) {
        offspring1[i] = parent1[i];
        offspring2[i] = parent2[i];
    }

    auto fillOffspring = [&](const int* parent, int* offspring) {
        int idx = (end + 1) % cityCount;
        for (int i = 0; i < cityCount; ++i) {
            int gene = parent[(end + 1 + i) % cityCount];
            if (!Utilities::isInSegment(offspring, start, end, gene)) {
                offspring[idx] = gene;
                idx = (idx + 1) % cityCount;
            }
        }
    };

    fillOffspring(parent2, offspring1);
    fillOffspring(parent1, offspring2);
}

// Reverse Mutacja
void GeneticAlgorithm::performReverseMutation(int* individual) {
    // Oblicz koszty krawędzi
    double* edgeCosts = new double[cityCount];
    double totalCost = 0.0;
    for (int i = 0; i < cityCount; ++i) {
        int next = (i + 1) % cityCount;
        edgeCosts[i] = distanceMatrix[individual[i]][individual[next]];
        totalCost += edgeCosts[i];
    }

    // Oblicz prawdopodobieństwa
    double* probabilities = new double[cityCount];
    for (int i = 0; i < cityCount; ++i) {
        probabilities[i] = edgeCosts[i] / totalCost;
    }

    int start = selectIndexBasedOnProbability(probabilities, cityCount);
    int end = selectIndexBasedOnProbability(probabilities, cityCount);

    if (start > end) {
        int temp = start;
        start = end;
        end = temp;
    }

    // Odwróć fragment
    Utilities::reverseSegment(individual, start, end);

    delete[] edgeCosts;
    delete[] probabilities;
}

// Swap Mutacja
void GeneticAlgorithm::performSwapMutation(int* individual) {
    double* edgeCosts = new double[cityCount];
    double totalCost = 0.0;
    for (int i = 0; i < cityCount; ++i) {
        int next = (i + 1) % cityCount;
        edgeCosts[i] = distanceMatrix[individual[i]][individual[next]];
        totalCost += edgeCosts[i];
    }

    double* probabilities = new double[cityCount];
    for (int i = 0; i < cityCount; ++i) {
        probabilities[i] = edgeCosts[i] / totalCost;
    }

    int i = selectIndexBasedOnProbability(probabilities, cityCount);
    int j = selectIndexBasedOnProbability(probabilities, cityCount);

    if (i != j) {
        Utilities::customSwap(individual[i], individual[j]);
    }

    delete[] edgeCosts;
    delete[] probabilities;
}

// Insert Mutacja
void GeneticAlgorithm::performInsertMutation(int* individual) {
    double* edgeCosts = new double[cityCount];
    double totalCost = 0.0;
    for (int i = 0; i < cityCount; ++i) {
        int next = (i + 1) % cityCount;
        edgeCosts[i] = distanceMatrix[individual[i]][individual[next]];
        totalCost += edgeCosts[i];
    }

    double* probabilities = new double[cityCount];
    for (int i = 0; i < cityCount; ++i) {
        probabilities[i] = edgeCosts[i] / totalCost;
    }

    int i = selectIndexBasedOnProbability(probabilities, cityCount);
    int j = rand() % cityCount;

    if (i != j) {
        int value = individual[i];
        if (i < j) {
            for (int k = i; k < j; ++k) {
                individual[k] = individual[k + 1];
            }
        } else {
            for (int k = i; k > j; --k) {
                individual[k] = individual[k - 1];
            }
        }
        individual[j] = value;
    }

    delete[] edgeCosts;
    delete[] probabilities;
}

// Wybór indeksu na podstawie prawdopodobieństwa
int GeneticAlgorithm::selectIndexBasedOnProbability(const double* probabilities, int size) {
    double randomValue = (double)rand() / RAND_MAX;
    double cumulativeProbability = 0.0;
    for (int i = 0; i < size; ++i) {
        cumulativeProbability += probabilities[i];
        if (randomValue <= cumulativeProbability) {
            return i;
        }
    }
    return size - 1; // W razie czego ostatni indeks
}

// Uruchomienie algorytmu
void GeneticAlgorithm::run() {
    // Start pomiaru czasu
    auto startTime = chrono::high_resolution_clock::now();

    // 1) Inicjalizacja populacji
    initializePopulation();
    // 2) Ocena populacji
    evaluatePopulation();

    // 3) Główna pętla
    for (int generation = 0; generation < maxGenerations; ++generation) {
        // Sprawdzenie limitu czasu
        auto currentTime = chrono::high_resolution_clock::now();
        auto elapsedSeconds = chrono::duration_cast<chrono::seconds>(currentTime - startTime).count();
        if (elapsedSeconds >= timeLimit) {
            cout << "[TIME LIMIT REACHED] Przekroczono limit czasu "
                 << timeLimit << " s." << endl;
            break;
        }

        // Przygotowanie nowej populacji
        int** newPopulation = new int*[populationSize];
        for (int i = 0; i < populationSize; ++i) {
            newPopulation[i] = new int[cityCount];
        }

        // Tworzenie nowej populacji
        for (int i = 0; i < populationSize; i += 2) {
            int parent1Idx, parent2Idx;
            selectParents(parent1Idx, parent2Idx);

            // Krzyżowanie z prawdopodobieństwem crossoverRate
            if (((double)rand() / RAND_MAX) < crossoverRate) {
                crossover(population[parent1Idx], population[parent2Idx],
                          newPopulation[i], newPopulation[i + 1]);
            } else {
                // Bez krzyżowania: przepisanie rodziców
                copyIndividual(population[parent1Idx], newPopulation[i]);
                if ((i + 1) < populationSize) {
                    copyIndividual(population[parent2Idx], newPopulation[i + 1]);
                }
            }

            // Mutacja z prawdopodobieństwem mutationRate
            if (((double)rand() / RAND_MAX) < mutationRate) {
                mutate(newPopulation[i]);
            }
            if ((i + 1) < populationSize && ((double)rand() / RAND_MAX) < mutationRate) {
                mutate(newPopulation[i + 1]);
            }
        }

        // Usunięcie starej populacji
        freePopulation();
        // Nowa populacja staje się aktualną
        population = newPopulation;

        // Ocena nowej populacji
        evaluatePopulation();

        cout << "Generacja " << (generation + 1)
             << ", Najlepszy koszt: " << bestCost
             << ", czas [s]: " << elapsedSeconds
             << endl;
    }

    // Zakończenie pomiaru czasu
    auto endTime = chrono::high_resolution_clock::now();
    auto durationInSeconds = chrono::duration<double>(endTime - startTime);
    auto durationInMilliseconds = chrono::duration<double, milli>(endTime - startTime);

    // Zapis wyników (jeśli potrzebne)
    SaveToCSV logger("GA_test.csv");
    logger.saveGeneticAlgorithmResults(
            "GeneticAlgorithm",
            populationSize,
            crossoverRate,
            mutationRate,
            crossoverMethod,
            mutationMethod,
            selectionMethod,
            maxGenerations,
            durationInSeconds,
            durationInMilliseconds,
            bestCost,
            bestSolution,
            cityCount
    );
}

// Zwraca najlepszy koszt
int GeneticAlgorithm::getBestCost() const {
    return bestCost;
}

// Zwraca najlepsze rozwiązanie
const int* GeneticAlgorithm::getBestSolution() const {
    return bestSolution;
}
