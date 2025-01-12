#ifndef SAVETOCSV_H
#define SAVETOCSV_H

#include <string>
#include <chrono>

using namespace std;

class SaveToCSV {
public:
    SaveToCSV(const string& filename);

    void saveResults(const string& algorithmName,
                     double algorithmParameter,
                     double finalTemperature,
                     chrono::duration<double> seconds,
                     chrono::duration<double, milli> milliseconds,
                     int cost,
                     const string& neighborhoodType,
                     const int* path,
                     int path_length);

    void saveGeneticAlgorithmResults(
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
    );

private:
    string filename;
    bool isFileNew;

    string replaceDotWithCommaDouble(double value) const;
};

#endif // SAVETOCSV_H
