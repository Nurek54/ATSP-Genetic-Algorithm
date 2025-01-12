#ifndef CONFIGREADER_H
#define CONFIGREADER_H

#include <string>

using namespace std;

class ConfigReader {
public:
    ConfigReader(const string& filename);
    ~ConfigReader();

    bool parseConfig();

    // Ogólne ustawienia
    string getAlgorithm() const;
    int getAlgorithmRuns() const;
    string getMatrixFile() const;
    int getTimeLimit() const;
    string getLogFile() const;

    // Parametry dla Symulowanego Wyżarzania (SA)
    double getCoolingRate() const;
    string getInitialTemperature() const;
    string getNeighborhoodType() const;

    // Parametry dla Tabu Search
    int getTabuTenure() const;
    int getMaxIterations() const;

    // Parametry dla Algorytmu Genetycznego (GA)
    int getPopulationSize() const;
    double getCrossoverRate() const;
    double getMutationRate() const;
    string getCrossoverMethod() const;
    string getMutationMethod() const;
    string getSelectionMethod() const;
    int getMaxGenerations() const;

private:
    struct ConfigEntry {
        char* key;
        char* value;
    };

    string filename;
    ConfigEntry* configEntries;
    int numEntries;
    int capacityEntries;

    string trim(const string& str) const;
    const char* getValue(const string& key) const;
};

#endif // CONFIGREADER_H