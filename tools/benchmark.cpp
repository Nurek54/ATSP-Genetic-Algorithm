// Badania z projektu: rozmiar populacji (5n, 10n, 15n) x krzyzowanie (OX, PMX) przy mutacji insert,
// oraz trzy mutacje (insert, swap, reverse) przy populacji 5n i OX. Krzyzowanie 0.8, mutacja 0.01.
// Wynik: CSV na stdout, kolumny instance,n,optimum,experiment,population,crossover,mutation,run,seconds,generations,cost
// Komunikaty algorytmu (cout) sa wyciszane.
//
// Uzycie: benchmark [powtorzenia=5] [limit_ftv47_s=30] [instancja]
//         limit dla ftv170 jest 2x, dla rbg403 4x wiekszy
#include "algorithms/GeneticAlgorithm.h"
#include "core/TSPInstance.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#ifndef ATSP_DATA_DIR
#define ATSP_DATA_DIR "data"
#endif

namespace fs = std::filesystem;

struct Instance {
    string name;
    int optimum;     // znane optimum z TSPLIB
    int timeFactor;  // mnoznik limitu czasu
};

struct Setting {
    string experiment;
    int populationFactor;
    string crossover;
    string mutation;
};

int main(int argc, char** argv) {
    int runs = argc > 1 ? stoi(argv[1]) : 5;
    int baseLimit = argc > 2 ? stoi(argv[2]) : 30;
    string onlyInstance = argc > 3 ? argv[3] : "";

    const vector<Instance> instances = {{"ftv47", 1776, 1}, {"ftv170", 2755, 2}, {"rbg403", 2465, 4}};
    const vector<Setting> settings = {
            {"population", 5, "OX", "insert"}, {"population", 5, "PMX", "insert"},
            {"population", 10, "OX", "insert"}, {"population", 10, "PMX", "insert"},
            {"population", 15, "OX", "insert"}, {"population", 15, "PMX", "insert"},
            {"mutation", 5, "OX", "swap"}, {"mutation", 5, "OX", "reverse"},
    };

    fs::path work = fs::temp_directory_path() / "atsp-ga-benchmark";
    fs::create_directories(work);
    const string log = (work / "run.csv").string();

    ofstream devNull;  // nieotwarty strumien: wszystko, co algorytm wypisuje, przepada
    streambuf* console = cout.rdbuf();

    printf("instance,n,optimum,experiment,population,crossover,mutation,run,seconds,generations,cost\n");
    for (const Instance& inst : instances) {
        if (!onlyInstance.empty() && inst.name != onlyInstance) continue;
        TSPInstance tsp;
        cout.rdbuf(devNull.rdbuf());
        bool ok = tsp.loadFromFile(string(ATSP_DATA_DIR) + "/" + inst.name + ".atsp");
        cout.rdbuf(console);
        if (!ok) {
            cerr << "Brak pliku " << inst.name << ".atsp w " << ATSP_DATA_DIR << "\n";
            return 1;
        }
        int n = tsp.getCityCount();
        int limit = baseLimit * inst.timeFactor;

        for (size_t s = 0; s < settings.size(); ++s) {
            const Setting& st = settings[s];
            for (int r = 0; r < runs; ++r) {
                srand(1000u * r + 31u * s + 7u);
                int population = st.populationFactor * n;
                cout.rdbuf(devNull.rdbuf());
                GeneticAlgorithm ga(tsp.getCostMatrix(), n, population, 0.8, 0.01, st.crossover, st.mutation,
                                    "tournament", 9000000, limit, log);
                auto t0 = chrono::high_resolution_clock::now();
                ga.run();
                double sec = chrono::duration<double>(chrono::high_resolution_clock::now() - t0).count();
                cout.rdbuf(console);
                printf("%s,%d,%d,%s,%d,%s,%s,%d,%.3f,%d,%d\n", inst.name.c_str(), n, inst.optimum, st.experiment.c_str(),
                       population, st.crossover.c_str(), st.mutation.c_str(), r, sec, ga.getGenerations(), ga.getBestCost());
                fflush(stdout);
            }
            cerr << inst.name << " " << st.experiment << " " << st.populationFactor << "n " << st.crossover << " "
                 << st.mutation << " gotowe\n";
        }
    }
    return 0;
}
