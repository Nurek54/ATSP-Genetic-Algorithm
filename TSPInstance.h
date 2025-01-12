#ifndef TSPINSTANCE_H
#define TSPINSTANCE_H

#include <string>

using namespace std;

class TSPInstance {
public:
    TSPInstance();
    ~TSPInstance();

    bool loadFromFile(const string& filename);
    const int* const* getCostMatrix() const;
    int getCityCount() const;

private:
    int** costMatrix;
    int cityCount;

    void freeMatrix();
    int** loadMatrix(const string& filename, int& size);
    string trim(const string& str) const;
};

#endif // TSPINSTANCE_H
