#ifndef GENETICALGORITHM_H
#define GENETICALGORITHM_H

#include <string>

class GeneticAlgorithm {
public:
    GeneticAlgorithm(const int* const* distanceMatrix, int cityCount, int populationSize,
                     double crossoverRate, double mutationRate, const std::string& crossoverMethod,
                     const std::string& mutationMethod, const std::string& selectionMethod,
                     int maxGenerations, int timeLimit, const std::string& logFile);
    ~GeneticAlgorithm();

    void run();
    int getBestCost() const;
    const int* getBestSolution() const;

private:
    // Parametry algorytmu
    const int* const* distanceMatrix;  // Macierz odległości
    int cityCount;                     // Liczba miast
    int populationSize;                // Rozmiar populacji
    double crossoverRate;              // Współczynnik krzyżowania
    double mutationRate;               // Współczynnik mutacji
    std::string crossoverMethod;       // Wybrana metoda krzyżowania (PMX, OX)
    std::string mutationMethod;        // Wybrana metoda mutacji (swap, reverse)
    std::string selectionMethod;       // Wybrana metoda selekcji (tournament, roulette)
    int maxGenerations;                // Maksymalna liczba generacji
    int timeLimit;                     // Limit czasu wykonania algorytmu

    // Dane populacji
    int** population;                  // Populacja (tablica dynamiczna)
    int* fitness;                      // Koszty rozwiązań w populacji
    int* bestSolution;                 // Najlepsze znalezione rozwiązanie
    int bestCost;                      // Koszt najlepszego rozwiązania

    // Inicjalizacja i ewaluacja populacji
    void initializePopulation();
    void evaluatePopulation();

    // Metody związane z algorytmem genetycznym
    void selectParents(int& parent1Idx, int& parent2Idx); // Selekcja rodziców
    void crossover(const int* parent1, const int* parent2, int* offspring1, int* offspring2); // Krzyżowanie
    void mutate(int* individual); // Mutacja

    // Obsługa różnych metod krzyżowania
    void performPMXCrossover(const int* parent1, const int* parent2, int* offspring1, int* offspring2);
    void performOrderCrossover(const int* parent1, const int* parent2, int* offspring1, int* offspring2);

    // Obsługa różnych metod mutacji
    void performSwapMutation(int* individual);
    void performReverseMutation(int* individual);
    void performInsertMutation(int* individual);

    int selectIndexBasedOnProbability(const double* probabilities, int size);

    // Metody pomocnicze
    int calculateCost(const int* solution) const; // Obliczanie kosztu rozwiązania
    void copyIndividual(const int* source, int* destination); // Kopiowanie rozwiązania
    void freePopulation(); // Zwalnianie pamięci populacji
};

#endif // GENETICALGORITHM_H
