#pragma once

#include <cstring>
#include <iostream>
#include <algorithm>
#include <vector>
#include "NeighList.h"


struct MISP_Solution {
    NeighList *graph;
    std::vector<int> solution;
    int *MISP_IndependentDegree;

    MISP_Solution(NeighList *nl) {
        graph = nl;
        MISP_IndependentDegree = new int[nl->n];
        memset(MISP_IndependentDegree, 0, nl->n * sizeof(int));
    }
    MISP_Solution(NeighList *nl, int *nodes, int sz) {
        graph = nl;
        MISP_IndependentDegree = new int[nl->n];
        memset(MISP_IndependentDegree, 0, nl->n * sizeof(int));

        // Add nodes to solution
        for (int i = 0; i < sz; i++) {
            addNode(nodes[i]);
        }
    }
    ~MISP_Solution() {
        delete[] MISP_IndependentDegree;
    }

    int size() const {
        return solution.size();
    }

    void addNode(int node) {
        if (MISP_IndependentDegree[node] > 0) {
            std::cerr << "Error: Trying to add a non-independent node" << node << "to MISP_Solution";
            return;
        }

        solution.push_back(node);
        MISP_IndependentDegree[node] = -1;
        for (int neighbor : graph->neighborhoods[node]) {
            MISP_IndependentDegree[neighbor] += 1;
        }
    }
    void removeNode(int node) {
        // Find and remove node from solution
        auto found = std::find(solution.begin(), solution.end(), node);
        if (found != solution.end()) {
            // Node found, swap with last and pop
            *found = solution.back();
            solution.pop_back();
        } else {
            std::cerr << "Error: Trying to remove a non-existing node" << node << "from MISP_Solution";
            return;
        }

        MISP_IndependentDegree[node] = 0;
        // Re-evaluate independence of neighbors
        for (int neighbor : graph->neighborhoods[node]) {
            MISP_IndependentDegree[neighbor] -= 1;
        }
    }
};