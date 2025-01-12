#include <iostream>
#include "ConfigReader.h"
#include "TSPInstance.h"
#include "SimulatedAnnealing.h"
#include "TabuSearch.h"
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

        if (selectedAlgorithm == "annealing") {
            cout << "[INFO] Uruchamianie Symulowanego Wyzarzania (SA)..." << endl;

            double coolRate = config.getCoolingRate();
            double initialTemp = 0.0;
            string initialTempStr = config.getInitialTemperature();
            if (initialTempStr == "auto") {
                initialTemp = -1.0;
            } else {
                try {
                    initialTemp = stod(initialTempStr);
                } catch (const invalid_argument& e) {
                    cerr << "Nieprawidlowa wartosc initial_temperature. Uzycie domyslnej wartosci 0.0.\n";
                    initialTemp = 0.0;
                }
            }
            string moveType = config.getNeighborhoodType();
            int maxIter = config.getMaxIterations();

            SimulatedAnnealing sa(instance.getCostMatrix(),
                                  instance.getCityCount(),
                                  initialTemp,
                                  coolRate,
                                  timeLimitSec,
                                  maxIter,
                                  moveType,
                                  logFile);

            sa.run();
            cout << "Najlepszy koszt (SA): " << sa.retrieveBestCost() << endl;

        } else if (selectedAlgorithm == "tabu") {
            cout << "[INFO] Uruchamianie Tabu Search..." << endl;

            int tabuTenure = config.getTabuTenure();
            int maxIter = config.getMaxIterations();
            string moveType = config.getNeighborhoodType();

            TabuSearch ts(instance.getCostMatrix(),
                          instance.getCityCount(),
                          timeLimitSec,
                          tabuTenure,
                          maxIter,
                          moveType,
                          logFile);

            ts.run();
            cout << "Najlepszy koszt (Tabu): " << ts.retrieveBestCost() << endl;

        } else if (selectedAlgorithm == "genetic") {
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
