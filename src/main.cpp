#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include "app/ConfigReader.h"
#include "core/TSPInstance.h"
#include "algorithms/GeneticAlgorithm.h"

#ifndef ATSP_CONFIG_FILE
#define ATSP_CONFIG_FILE "config/config.txt"
#endif
#ifndef ATSP_DATA_DIR
#define ATSP_DATA_DIR "data"
#endif

using namespace std;

static bool fileExists(const string& path) {
    ifstream f(path);
    return f.good();
}

// Plik z macierza: najpierw wzgledem katalogu roboczego, potem w folderze data/ projektu
static string resolveMatrixFile(const string& name) {
    if (fileExists(name)) {
        return name;
    }
    string inData = string(ATSP_DATA_DIR) + "/" + name;
    return fileExists(inData) ? inData : name;
}

int main(int argc, char** argv) {
    // Ziarno raz na caly program, kolejne uruchomienia kontynuuja ten sam ciag rand()
    srand(static_cast<unsigned int>(time(nullptr)));

    // 1. Konfiguracja: sciezka z argumentu, config.txt w katalogu roboczym albo config/config.txt projektu
    string configFile = argc > 1 ? argv[1] : (fileExists("config.txt") ? "config.txt" : ATSP_CONFIG_FILE);
    cout << "[INFO] Konfiguracja: " << configFile << endl;
    ConfigReader config(configFile);
    if (!config.parseConfig()) {
        cerr << "Blad podczas odczytu pliku konfiguracyjnego.\n";
        return 1;
    }

    // 2. Odczyt typu algorytmu ("genetic")
    string selectedAlgorithm = config.getAlgorithm();

    // 3. Wczytywanie macierzy TSP
    TSPInstance instance;
    if (!instance.loadFromFile(resolveMatrixFile(config.getMatrixFile()))) {
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