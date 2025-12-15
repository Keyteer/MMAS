#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>
#include <ctime>
#include <sys/stat.h>
#include "AntColony.h"
#include "loader.h"


int main(int argc, char *argv[]) {
    // Default parameters
    char *path = nullptr;
    double time_limit = 10.0; // default time limit seconds
    float evaporation_rate = 0.01f; // default evaporation rate
    float deposit_amount = 10.0f; // default deposit amount
    bool proportional_deposit_type = true; // default deposit type
    bool verbose = false; // verbose flag

    // Parse required arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            path = argv[++i];
        }
    }

    // Validate parameters
    if (path == nullptr) {
        fprintf(stderr, "Usage: %s -i <path> [-t <time_limit_seconds>] [-e <evaporation_rate>] [-d <deposit_amount>] [-p <proportional_deposit_type>] [-v]\n", argv[0]);
        fprintf(stderr, "\nParameters:\n");
        fprintf(stderr, "  -i <path>                : Path to the graph instance/s file/directory (required)\n");
        fprintf(stderr, "  -t <time_limit_seconds>  : Maximum execution time in seconds (default: %.2f)\n", time_limit);
        fprintf(stderr, "  -e <evaporation_rate>    : Pheromone evaporation rate (default: %f)\n", evaporation_rate);
        fprintf(stderr, "  -d <deposit_amount>      : Pheromone deposit amount (default: %.2f)\n", deposit_amount);
        fprintf(stderr, "  -p <deposit_type>        : Pheromone deposit type: 'proportional' or 'fixed' (default: proportional = %s)\n", proportional_deposit_type? "true" : "false");
        fprintf(stderr, "  -v                       : Verbose output for improvements during search (On single file case)\n");
        return 1;
    }

    // Parse optional arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            time_limit = atof(argv[++i]);
        } else if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
            evaporation_rate = atof(argv[++i]);
        } else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            deposit_amount = atof(argv[++i]);
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            if (strcmp(argv[++i], "proportional") == 0) {
                proportional_deposit_type = true;
            } else if (strcmp(argv[i], "fixed") == 0) {
                proportional_deposit_type = false;
            } else {
                fprintf(stderr, "Error: Invalid deposit type '%s'. Use 'proportional' or 'fixed'.\n", argv[i]);
                return 1;
            }
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose = true;
        }
    }

    if (time_limit <= 0) {
        fprintf(stderr, "Error: Time limit must be positive\n");
        return 1;
    }

    struct stat path_stat;
    if (stat(path, &path_stat) != 0) {
        perror("Error accessing path");
        return 1;
    }

    char **fileNames;
    int fileCount = 0;

    if (S_ISDIR(path_stat.st_mode)) {
        // directory
        fileCount = getAllFiles(path, fileNames);
        fileCount = filterFiles(fileNames, fileCount);
        if (fileCount <= 0) {
            fprintf(stderr, "Error: No files found in directory: %s\n", path);
            return 1;
        }
    } else {
        // single file case
        // Run single file with verbose parameter

        NeighList *nl = loadGraph(path);
        if (nl == nullptr) {
            fprintf(stderr, "Error: Could not load graph from file: %s\n", path);
            return 1;
        }

        int result = AntColony(nl, time_limit, evaporation_rate, deposit_amount, proportional_deposit_type, verbose);

        if (!verbose) {
            printf("%d\n", - result); // print negative for irace minimization
        }

        delete nl;

        return 0;
    }


    // print csv header
    printf("Density,Tests,Avg_MISP_Size,Avg_Time(s),Avg_Iterations\n");


    // variables
    int lastDensityDecimal = 0;
    int tests = 0;
    double avgResult = 0.0;
    double avgTime = 0.0;
    double avgIterations = 0.0;


    for (int i = 0; i < fileCount; i++) {

        // get full path
        char *file = fileNames[i];
        char *fullPath = new char[strlen(path) + strlen(file) + 2];
        sprintf(fullPath, "%s/%s", path, file);

        // check density, if new density reset counters and print newline
        int currentDensityDecimal;
        sscanf(file, "%*[^.].%d_", &currentDensityDecimal);
        if (currentDensityDecimal != lastDensityDecimal) {
            lastDensityDecimal = currentDensityDecimal;
            if (i != 0) {
                avgResult = 0.0;
                avgTime = 0.0;
                avgIterations = 0.0;
                tests = 0;
                printf("\n");
            }
        }

        // Load graph
        NeighList *nl = loadGraph(fullPath);
        if (nl == nullptr) {
            fprintf(stderr, "Error: Could not load graph from file: %s\n", fullPath);
            return 1;
        }

        int iterations;

        // Run ILS and measure time
        auto start = std::chrono::high_resolution_clock::now();
        int misp_size = AntColony(nl, time_limit, evaporation_rate, deposit_amount, proportional_deposit_type, false, &iterations);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        double execution_time = elapsed.count();

        // accumulate results pondered by number of tests
        avgResult = (avgResult * tests + misp_size) / (tests + 1);
        avgTime = (avgTime * tests + execution_time) / (tests + 1);
        avgIterations = (avgIterations * tests + iterations) / (tests + 1);
        tests++;

        // print current average results
        printf("\r0.%d,%d,%.2f,%.4f,%.0f   ", currentDensityDecimal, tests, avgResult, avgTime, avgIterations);
        fflush(stdout);

        // Cleanup
        delete nl;
    }

    printf("\n");

    return 0;
}
