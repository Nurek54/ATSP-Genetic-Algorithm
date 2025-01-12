#include "SaveToCSV.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <string>

using namespace std;

// Konstruktor sprawdzający, czy plik istnieje
SaveToCSV::SaveToCSV(const string& filename) : filename(filename), isFileNew(false) {
    ifstream fileCheck(filename);
    if (!fileCheck.good()) {
        isFileNew = true; // Plik nie istnieje, ustawiamy flagę
    }
    fileCheck.close();
}

// Zapis wyników do pliku CSV
void SaveToCSV::saveResults(const string& algorithmName,
                            double algorithmParameter,
                            double finalTemperature,
                            chrono::duration<double> seconds,
                            chrono::duration<double, milli> milliseconds,
                            int cost,
                            const string& neighborhoodType,
                            const int* path,
                            int path_length) {
    ofstream csvFile(filename, ios::app);
    if (csvFile.is_open()) {
        // Nagłówek jeśli plik jest nowy
        if (isFileNew) {
            csvFile << "Algorithm,AlgorithmParameter,FinalTemperature,Seconds,Milliseconds,Cost,Neighborhood,Path\n";
            isFileNew = false;
        }

        csvFile << algorithmName << ","
                << algorithmParameter << ",";

        // Ustawienie precyzji tylko dla temperatury końcowej
        csvFile << fixed << setprecision(10) << finalTemperature << ",";
        csvFile.unsetf(ios::fixed);

        csvFile << seconds.count() << ","
                << milliseconds.count() << ","
                << cost << ","
                << neighborhoodType << ",";

        // Zapis trasy
        for (int i = 0; i < path_length; ++i) {
            csvFile << path[i];
            if (i != path_length - 1) {
                csvFile << "->";
            }
        }

        csvFile << "\n";
        csvFile.close();
    } else {
        cerr << "Nie mozna otworzyc pliku: " << filename << "\n";
    }
}

void SaveToCSV::saveGeneticAlgorithmResults(
        const string& algorithmName,
        int populationSize,
        double crossoverRate,
        double mutationRate,
        const string& crossoverMethod,
        const string& mutationMethod,
        const string& selectionMethod,
        int maxGenerations,
        chrono::duration<double> seconds,
        chrono::duration<double, milli> milliseconds,
        int bestCost,
        const int* bestSolution,
        int cityCount
) {
    ofstream csvFile(filename, ios::app);
    if (csvFile.is_open()) {
        // Nagłówek, jeśli plik jest nowy
        if (isFileNew) {
            csvFile << "Algorithm,PopulationSize,CrossoverRate,MutationRate,CrossoverMethod,MutationMethod,SelectionMethod,"
                       "MaxGenerations,Seconds,Milliseconds,BestCost,BestSolution\n";
            isFileNew = false;
        }

        // Zapis wyników i parametrów
        csvFile << algorithmName << ","
                << populationSize << ","
                << crossoverRate << ","
                << mutationRate << ","
                << crossoverMethod << ","
                << mutationMethod << ","
                << selectionMethod << ","
                << maxGenerations << ","
                << seconds.count() << ","
                << milliseconds.count() << ","
                << bestCost << ",";

        // Zapis ścieżki najlepszego rozwiązania
        for (int i = 0; i < cityCount; ++i) {
            csvFile << bestSolution[i];
            if (i != cityCount - 1) csvFile << "->";
        }

        csvFile << "\n";
        csvFile.close();
    } else {
        cerr << "Nie można otworzyć pliku: " << filename << "\n";
    }
}
