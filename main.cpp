#include <iostream>
#include "ConfigReader.h"
#include "TSPInstance.h"
#include "GeneticAlgorithm.h"

using namespace std;

int main(int argc, char** argv) {
    // 1. Wczytujemy plik konfiguracyjny
    ConfigReader config("config.txt");
    if (!config.parseConfig()) {
        cerr << "Blad podczas odczytu pliku konfiguracyjnego.\n";
        return 1;
    }

    // 2. Odczyt typu algorytmu ("annealing", "tabu", "genetic")
    string selectedAlgorithm = config.getAlgorithm();

    // 3. Wczytywanie macierzy TSP
    TSPInstance instance;
    if (!instance.loadFromFile(config.getMatrixFile())) {
        cerr << "Nie mozna wczytac pliku z macierza kosztow.\n";
        return 1;
    }

    // 4. Pobranie wspólnych parametrów
    int timeLimitSec = config.getTimeLimit();       // Maksymalny czas działania
    string logFile = config.getLogFile();           // Plik logów
    int algorithmRuns = config.getAlgorithmRuns();  // Liczba powtórzeń

    // 5. Wybór i uruchomienie algorytmu
    for (int run = 0; run < algorithmRuns; ++run) {
        cout << "[INFO] Uruchomienie algorytmu, iteracja: " << (run + 1) << "/" << algorithmRuns << endl;

        if (selectedAlgorithm == "genetic") {
            cout << "[INFO] Uruchamianie Algorytmu Genetycznego (GA)..." << endl;

            // Parametry algorytmu genetycznego
            int populationSize = config.getPopulationSize();
            double crossoverRate = config.getCrossoverRate();
            double mutationRate = config.getMutationRate();
            string crossoverMethod = config.getCrossoverMethod();
            string mutationMethod = config.getMutationMethod();
            string selectionMethod = config.getSelectionMethod();
            int maxGenerations = config.getMaxGenerations();

            GeneticAlgorithm ga(instance.getCostMatrix(),
                                instance.getCityCount(),
                                populationSize,
                                crossoverRate,
                                mutationRate,
                                crossoverMethod,
                                mutationMethod,
                                selectionMethod,
                                maxGenerations,
                                timeLimitSec,
                                logFile);

            ga.run();
            cout << "Najlepszy koszt (GA): " << ga.getBestCost() << endl;

        } else {
            cerr << "Nieznany algorytm w configu: " << selectedAlgorithm << endl;
            return 1;
        }

        cout << "[INFO] Iteracja " << (run + 1) << " zakonczona.\n";
    }

    cout << "[INFO] Wszystkie uruchomienia algorytmu zakonczone pomyslnie.\n";
    return 0;
}
