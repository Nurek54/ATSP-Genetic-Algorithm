cmake_minimum_required(VERSION 3.30)
project(PEA_IV)

set(CMAKE_CXX_STANDARD 26)

add_executable(PEA_IV main.cpp
        SaveToCSV.cpp
        ConfigReader.cpp
        TSPInstance.cpp
        TSPUtilities.cpp
        GeneticAlgorithm.cpp
        GeneticAlgorithm.h
)
