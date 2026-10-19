cmake_minimum_required(VERSION 3.20)
project(ATSP_GeneticAlgorithm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GA_CORE
        src/core/TSPInstance.cpp
        src/core/TSPUtilities.cpp
        src/algorithms/GeneticAlgorithm.cpp
        src/io/SaveToCSV.cpp)

add_executable(atsp_ga
        src/main.cpp
        src/app/ConfigReader.cpp
        ${GA_CORE})
target_include_directories(atsp_ga PRIVATE src)
# Domyslny config i folder z macierzami, gdy w katalogu roboczym ich nie ma
target_compile_definitions(atsp_ga PRIVATE
        ATSP_CONFIG_FILE="${CMAKE_SOURCE_DIR}/config/config.txt"
        ATSP_DATA_DIR="${CMAKE_SOURCE_DIR}/data")

# Pomiary do README: zawsze z optymalizacja, niezaleznie od typu builda
add_executable(benchmark tools/benchmark.cpp ${GA_CORE})
target_include_directories(benchmark PRIVATE src)
target_compile_definitions(benchmark PRIVATE ATSP_DATA_DIR="${CMAKE_SOURCE_DIR}/data")
if (MSVC)
    target_compile_options(benchmark PRIVATE /O2)
else ()
    target_compile_options(benchmark PRIVATE -O2)
endif ()

# MinGW: libstdc++ i libgcc wbudowane w exe, a libwinpthread-1.dll kopiowany z kompilatora
# obok exe, zeby Windows nie wzial starszej wersji z innego MinGW w PATH.
# (Pelne -static nie dziala z MinGW dolaczonym do CLion: brak __intrinsic_setjmpex)
if (MINGW)
    get_filename_component(MINGW_BIN "${CMAKE_CXX_COMPILER}" DIRECTORY)
    foreach (target atsp_ga benchmark)
        target_link_options(${target} PRIVATE -static-libgcc -static-libstdc++)
        if (EXISTS "${MINGW_BIN}/libwinpthread-1.dll")
            add_custom_command(TARGET ${target} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${MINGW_BIN}/libwinpthread-1.dll" $<TARGET_FILE_DIR:${target}>)
        endif ()
    endforeach ()
endif ()
