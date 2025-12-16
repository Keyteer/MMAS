#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>
#include <ctime>
#include <sys/stat.h>
#include "AntColony.h"
#include "loader.h"


int main(int argc, char *argv[]) {
    // Default MMAS parameters
    char *path = nullptr;
    double time_limit = 10.0;       // default time limit seconds
    int m = 10;                     // number of ants per iteration
    float alpha = 1.0f;             // pheromone influence exponent
    float beta = 2.0f;              // heuristic influence exponent
    float rho = 0.02f;              // evaporation rate
    float tau_min = 1.0f;           // MMAS: minimum pheromone level
    float tau_max = 100.0f;         // MMAS: maximum pheromone level
    bool verbose = false;           // verbose flag

    // Parse required arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            path = argv[++i];
        }
    }

    // Validate parameters
    if (path == nullptr) {
        fprintf(stderr, "Usage: %s -i <path> [-t <time>] [-m <ants>] [-a <alpha>] [-b <beta>] [-r <rho>] [-min <tau_min>] [-max <tau_max>] [-v]\n", argv[0]);
        fprintf(stderr, "\nMandatory:\n");
        fprintf(stderr, "  -i <path>      : Path to graph instance file/directory (required)\n");
        fprintf(stderr, "\nMAS Parameters:\n");
        fprintf(stderr, "  -t <time>      : Time limit in seconds (default: %.2f)\n", time_limit);
        fprintf(stderr, "  -m <ants>      : Number of ants per iteration (default: %d)\n", m);
        fprintf(stderr, "  -a <alpha>     : Pheromone influence exponent (default: %.2f)\n", alpha);
        fprintf(stderr, "  -b <beta>      : Heuristic influence exponent (default: %.2f)\n", beta);
        fprintf(stderr, "  -r <rho>       : Evaporation rate (default: %.2f)\n", rho);
        fprintf(stderr, "  -min <tau_min> : Minimum pheromone level (default: %.2f)\n", tau_min);
        fprintf(stderr, "  -max <tau_max> : Maximum pheromone level (default: %.2f)\n", tau_max);
        fprintf(stderr, "  -v             : Verbose output\n");
        return 1;
    }

    // Parse optional arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            time_limit = atof(argv[++i]);
        } else if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            m = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-a") == 0 && i + 1 < argc) {
            alpha = atof(argv[++i]);
        } else if (strcmp(argv[i], "-b") == 0 && i + 1 < argc) {
            beta = atof(argv[++i]);
        } else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
            rho = atof(argv[++i]);
        } else if (strcmp(argv[i], "-min") == 0 && i + 1 < argc) {
            tau_min = atof(argv[++i]);
        } else if (strcmp(argv[i], "-max") == 0 && i + 1 < argc) {
            tau_max = atof(argv[++i]);
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose = true;
        }
    }

    if (time_limit <= 0) {
        fprintf(stderr, "Error: Time limit must be positive\n");
        return 1;
    }

    if (m <= 0) {
        fprintf(stderr, "Error: Number of ants must be positive\n");
        return 1;
    }

    if (alpha < 0 || beta < 0) {
        fprintf(stderr, "Error: alpha and beta must be non-negative\n");
        return 1;
    }

    if (rho <= 0 || rho > 1) {
        fprintf(stderr, "Error: rho must be in (0, 1]\n");
        return 1;
    }

    if (tau_min >= tau_max) {
        fprintf(stderr, "Error: tau_min must be less than tau_max\n");
        return 1;
    }

    if (tau_min <= 0) {
        fprintf(stderr, "Error: tau_min must be positive\n");
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

        int result = MMAS(nl, time_limit, m, alpha, beta, rho, tau_min, tau_max, verbose);

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

        // Run MMAS and measure time
        auto start = std::chrono::high_resolution_clock::now();
        int misp_size = MMAS(nl, time_limit, m, alpha, beta, rho, tau_min, tau_max, false, &iterations);
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
