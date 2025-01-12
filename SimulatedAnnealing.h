#ifndef SIMULATEDANNEALING_H
#define SIMULATEDANNEALING_H

#include <string>
#include "SaveToCSV.h"

using namespace std;

class SimulatedAnnealing {
public:
    // Konstruktor
    SimulatedAnnealing(const int* const* distancesMatrix, int numberOfCities, double startingTemperature,
                       double temperatureDecreaseFactor, int timeLimitSec, int maximumSteps,
                       const string& neighborhoodType, const string& resultFile);

    // Destruktor
    ~SimulatedAnnealing();

    // Metoda uruchamiająca algorytm
    void run();

    // Getter dla najlepszego kosztu
    int retrieveBestCost() const;

    // Getter dla najlepszej trasy
    const int* retrieveBestRoute() const;

private:
    // Stałe i parametry konfiguracyjne
    const int* const* distancesMatrix; // Macierz odległości
    int numberOfCities;                // Liczba miast
    double startingTemperature;        // Temperatura początkowa
    double temperatureDecreaseFactor;  // Współczynnik chłodzenia
    int timeLimitSec;                  // Limit czasu w sekundach
    int maximumSteps;                  // Maksymalna liczba iteracji
    string neighborhoodType;      // Typ sąsiedztwa
    string resultFile;            // Plik wynikowy

    // Zmienna do przechowywania bieżącej trasy i kosztów
    int* activeRoute;                  // Aktualna trasa
    int* globalBestRoute;              // Najlepsza trasa
    int globalBestCost;                // Najlepszy koszt
    int currentRouteCost;              // Aktualny koszt

    SaveToCSV saveToCSV;

    double currentTemperature;         // Aktualna temperatura

    int timesWithoutBetterSolution;    // Licznik braku poprawy
    int noImprovementThreshold;        // Próg braku poprawy

    // Metody pomocnicze
    void buildGreedyRoute();           // Generowanie początkowej trasy zachłannej
    double computeInitialTemperature(); // Obliczanie automatycznej temperatury początkowej
    int computeRouteCost(const int* route) const; // Obliczanie kosztu trasy
    void performMove(int* route);       // Wykonanie ruchu w trasie
    bool shouldAcceptMove(int delta);   // Decyzja o akceptacji ruchu
    void recordResults(chrono::duration<double> seconds,
                       chrono::duration<double, milli> milliseconds,
                       double finalTemperature);

    // Operacje na trasach
    void swapCities(int* solution, int i, int j); // Zamiana dwóch miast
    void insertCity(int* solution, int from, int to); // Wstawienie miasta w inne miejsce
    void reverseSegment(int* solution, int start, int end); // Odwrócenie segmentu trasy
};

#endif // SIMULATEDANNEALING_H
