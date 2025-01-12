#include "ConfigReader.h"
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <iostream>

using namespace std;

// Usuwanie białych znaków z początku i końca
string ConfigReader::trim(const string& str) const {
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    return (start == string::npos) ? "" : str.substr(start, end - start + 1);
}

// Konstruktor
ConfigReader::ConfigReader(const string& filename)
        : filename(filename), numEntries(0), capacityEntries(10) {
    configEntries = new ConfigEntry[capacityEntries];
}

// Destruktor
ConfigReader::~ConfigReader() {
    for (int i = 0; i < numEntries; ++i) {
        delete[] configEntries[i].key;
        delete[] configEntries[i].value;
    }
    delete[] configEntries;
}

// Parsowanie pliku konfiguracyjnego
bool ConfigReader::parseConfig() {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Nie mozna otworzyc pliku konfiguracyjnego: " << filename << endl;
        return false;
    }

    string line;
    while (getline(file, line)) {
        size_t comment_pos = line.find('#');
        if (comment_pos != string::npos) {
            line = line.substr(0, comment_pos);
        }

        line = trim(line);
        if (line.empty()) {
            continue;
        }

        size_t pos = line.find('=');
        if (pos == string::npos) {
            cerr << "Pominieto nieprawidlowa linie: " << line << endl;
            continue;
        }

        string key = trim(line.substr(0, pos));
        string value = trim(line.substr(pos + 1));

        if (key.empty() || value.empty()) {
            cerr << "Pominieto pusty klucz lub wartosc: " << line << endl;
            continue;
        }

        if (numEntries >= capacityEntries) {
            capacityEntries *= 2;
            ConfigEntry* newEntries = new ConfigEntry[capacityEntries];
            for (int i = 0; i < numEntries; ++i) {
                newEntries[i] = configEntries[i];
            }
            delete[] configEntries;
            configEntries = newEntries;
        }

        configEntries[numEntries].key = new char[key.length() + 1];
        strcpy(configEntries[numEntries].key, key.c_str());

        configEntries[numEntries].value = new char[value.length() + 1];
        strcpy(configEntries[numEntries].value, value.c_str());

        numEntries++;
    }

    file.close();
    return true;
}

// Pobieranie wartości dla klucza
const char* ConfigReader::getValue(const string& key) const {
    for (int i = 0; i < numEntries; ++i) {
        if (strcmp(key.c_str(), configEntries[i].key) == 0) {
            return configEntries[i].value;
        }
    }
    return nullptr;
}

// Metody ogólne
string ConfigReader::getAlgorithm() const {
    const char* value = getValue("algorithm");
    return value != nullptr ? string(value) : "annealing";
}

int ConfigReader::getAlgorithmRuns() const {
    const char* value = getValue("algorithm_runs");
    return value != nullptr ? atoi(value) : 1;
}

string ConfigReader::getMatrixFile() const {
    const char* value = getValue("matrix_file");
    return value != nullptr ? string(value) : "";
}

int ConfigReader::getTimeLimit() const {
    const char* value = getValue("time_limit");
    return value != nullptr ? atoi(value) : 60;
}

string ConfigReader::getLogFile() const {
    const char* value = getValue("log_file");
    return value != nullptr ? string(value) : "results.csv";
}

// Parametry dla Symulowanego Wyżarzania
double ConfigReader::getCoolingRate() const {
    const char* value = getValue("cooling_rate");
    return value != nullptr ? atof(value) : 0.97;
}

string ConfigReader::getInitialTemperature() const {
    const char* value = getValue("initial_temperature");
    return value != nullptr ? string(value) : "auto";
}

string ConfigReader::getNeighborhoodType() const {
    const char* value = getValue("neighborhood_type");
    return value != nullptr ? string(value) : "swap";
}

// Parametry dla Tabu Search
int ConfigReader::getTabuTenure() const {
    const char* value = getValue("tabu_tenure");
    return value != nullptr ? atoi(value) : 7;
}

int ConfigReader::getMaxIterations() const {
    const char* value = getValue("max_iterations");
    return value != nullptr ? atoi(value) : 1000000;
}

// Parametry dla Algorytmu Genetycznego
int ConfigReader::getPopulationSize() const {
    const char* value = getValue("population_size");
    return value != nullptr ? atoi(value) : 100;
}

double ConfigReader::getCrossoverRate() const {
    const char* value = getValue("crossover_rate");
    return value != nullptr ? atof(value) : 0.8;
}

double ConfigReader::getMutationRate() const {
    const char* value = getValue("mutation_rate");
    return value != nullptr ? atof(value) : 0.01;
}

string ConfigReader::getCrossoverMethod() const {
    const char* value = getValue("crossover_method");
    return value != nullptr ? string(value) : "PMX";
}

string ConfigReader::getMutationMethod() const {
    const char* value = getValue("mutation_method");
    return value != nullptr ? string(value) : "swap";
}

string ConfigReader::getSelectionMethod() const {
    const char* value = getValue("selection_method");
    return value != nullptr ? string(value) : "tournament";
}

int ConfigReader::getMaxGenerations() const {
    const char* value = getValue("max_generations");
    return value != nullptr ? atoi(value) : 500;
}
