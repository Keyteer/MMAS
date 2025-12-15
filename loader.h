#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include "NeighList.h"


// Function to filter and sort files based on naming convention
// Sorted by density from 0.1 to 0.9
// fileNames [in/out]: array of file names
// fileCount [in]: number of files in fileNames
// returns: number of files that match the naming convention
int filterFiles(char **fileNames, int fileCount) {
    int matchCount = 0;
    for (int i = 1; i <= 9; i++) {
        for (int j = 0; j < fileCount; j++) {

            // selecting files with correct nomenclature and density [0.1, 0.2, ... , 0.9]
            char densStr[10];
            sprintf(densStr, "p0c0.%d_", i);
            if (strstr(fileNames[j], densStr) != nullptr) {

                // Swap to front
                char *temp = fileNames[matchCount];
                fileNames[matchCount] = fileNames[j];
                fileNames[j] = temp;

                matchCount++;
            }
        }
    }
    return matchCount;
}

// returns an neighborhood list (NeighList) from a file
NeighList *loadGraph(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error opening file");
        return nullptr;
    }

    int n;
    if (fscanf(fp, "%d", &n) != 1) {
        perror("Error reading number of nodes");
        fclose(fp);
        return nullptr;
    }

    NeighList *nl = new NeighList(n);

    while(!feof(fp)) {
        int u, v;
        if (fscanf(fp, "%d %d", &u, &v) == 2) {
            nl->push(u, v);
            nl->push(v, u);
        }
    }

    fclose(fp);
    return nl;
}

// Get all file paths from a directory
// dirPath [in]: path to the directory
// fileNames [out]: pointer to an array of strings to store file names (allocated inside the function)
// returns the number of files read, or -1 on error
int getAllFiles(const char *dirPath, char **&fileNames) {
    int count = 0;
    DIR *dir = opendir(dirPath);
    if (dir == NULL) {
        perror("Error opening directory");
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // if regular file
            count++;
        }
    }

    fileNames = (char **)malloc(count * sizeof(char *));

    dir = opendir(dirPath); // reset directory stream to beginning
    for(int i = 0; (entry = readdir(dir)) != NULL;) {
        if (entry->d_type == DT_REG) { // if regular file
            fileNames[i] = (char *)malloc((strlen(entry->d_name) + 1) * sizeof(char));
            sprintf(fileNames[i], "%s", entry->d_name);
            i++;
        }
    }

    return count;
}