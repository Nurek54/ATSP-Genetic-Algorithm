#include "TSPInstance.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>

using namespace std;

char myToUpper(char c) {
    if (c >= 'a' && c <= 'z') {
        return c - ('a' - 'A');
    }
    return c;
}

string toUpper(const string& str) {
    string upperStr = str;
    for (size_t i = 0; i < upperStr.length(); ++i) {
        upperStr[i] = myToUpper(upperStr[i]);
    }
    return upperStr;
}

TSPInstance::TSPInstance() : costMatrix(nullptr), cityCount(0) {}

TSPInstance::~TSPInstance() {
    freeMatrix();
}

void TSPInstance::freeMatrix() {
    if (costMatrix) {
        for (int i = 0; i < cityCount; ++i) {
            delete[] costMatrix[i];
        }
        delete[] costMatrix;
        costMatrix = nullptr;
    }
}

string TSPInstance::trim(const string& str) const {
    size_t start = 0;
    size_t end = str.length();

    // Usuwanie białych znaków z początku
    while (start < end && (str[start] == ' ' || str[start] == '\t' || str[start] == '\r' || str[start] == '\n')) {
        start++;
    }

    // Usuwanie białych znaków z końca
    while (end > start && (str[end - 1] == ' ' || str[end - 1] == '\t' || str[end - 1] == '\r' || str[end - 1] == '\n')) {
        end--;
    }

    return (start >= end) ? "" : str.substr(start, end - start);
}

int** TSPInstance::loadMatrix(const string& filename, int& size) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Nie można otworzyć pliku: " << filename << endl;
        return nullptr;
    }

    string line;
    bool matrixSection = false;
    size = 0;

    // Odczyt nagłówków
    while (getline(file, line)) {
        // Usunięcie komentarzy
        size_t comment_pos = line.find('#');
        if (comment_pos != string::npos) {
            line = line.substr(0, comment_pos);
        }

        line = trim(line);
        if (line.empty()) continue;

        string upperLine = toUpper(line);

        if (upperLine.find("DIMENSION") != string::npos) {
            size_t colon_pos = line.find(':');
            if (colon_pos != string::npos && colon_pos + 1 < line.length()) {
                string sizeStr = trim(line.substr(colon_pos + 1));
                size = atoi(sizeStr.c_str());
                cout << "Znaleziono rozmiar macierzy: " << size << "\n";
            } else {
                cerr << "Błąd: Nieprawidłowy format linii DIMENSION.\n";
                file.close();
                return nullptr;
            }
        } else if (upperLine.find("EDGE_WEIGHT_SECTION") != string::npos) {
            matrixSection = true;
            break;
        }
    }

    if (!matrixSection || size == 0) {
        cerr << "Nie znaleziono sekcji EDGE_WEIGHT_SECTION lub nieprawidłowy rozmiar.\n";
        file.close();
        return nullptr;
    }

    // Alokacja pamięci
    int** matrix = new int*[size];
    for (int i = 0; i < size; ++i) {
        matrix[i] = new int[size];
    }

    // Wczytywanie danych macierzy
    int value;
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (!(file >> value)) {
                cerr << "Nieprawidlowe dane w macierzy przy i=" << i << ", j=" << j << ".\n";
                for (int k = 0; k <= i; ++k) delete[] matrix[k];
                delete[] matrix;
                file.close();
                return nullptr;
            }
            matrix[i][j] = value;
        }
    }

    cout << "Macierz zostala wczytana poprawnie.\n";
    file.close();
    return matrix;
}

bool TSPInstance::loadFromFile(const string& filename) {
    freeMatrix(); // Upewniam się, że wcześniejsza pamięć jest zwolniona
    costMatrix = loadMatrix(filename, cityCount);
    return costMatrix != nullptr;
}

const int* const* TSPInstance::getCostMatrix() const {
    return costMatrix;
}

int TSPInstance::getCityCount() const {
    return cityCount;
}
