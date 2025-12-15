#pragma once

#include <vector>
using std::vector;

// Neighborhood List
struct NeighList {
    int n;
    int *degrees;
    vector<int> *neighborhoods;

    NeighList(int n) {
        this->n = n;
        degrees = new int[n];
        neighborhoods = new vector<int>[n];
        for (int i = 0; i < n; i++) {
            degrees[i] = 0;
        }
    }
    ~NeighList() {
        delete[] degrees;
        delete[] neighborhoods;
    }

    void push(int u, int v) {
        neighborhoods[u].push_back(v);
        degrees[u]++;
    }

    bool isNeighbor(int u, int v) {
        for (int neighbor : neighborhoods[u]) {
            if (neighbor == v) {
                return true;
            }
        }
        return false;
    }
};