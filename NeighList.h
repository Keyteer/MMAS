#pragma once

#include <vector>
#include <algorithm>
using std::vector;

// Neighborhood List
struct NeighList {
    int n;
    int *degrees;
    vector<int> *neighborhoods;
    int *degeneracy;      // degeneracy of each node (computed on demand)
    int maxDegeneracy;    // graph degeneracy (max node degeneracy)

    NeighList(int n) {
        this->n = n;
        degrees = new int[n];
        neighborhoods = new vector<int>[n];
        degeneracy = nullptr;
        maxDegeneracy = 0;
        for (int i = 0; i < n; i++) {
            degrees[i] = 0;
        }
    }
    ~NeighList() {
        delete[] degrees;
        delete[] neighborhoods;
        if (degeneracy) delete[] degeneracy;
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

    // Compute degeneracy for all nodes using the peeling algorithm O(n + m)
    void buildDegeneracy() {
        if (degeneracy) delete[] degeneracy;
        degeneracy = new int[n];
        maxDegeneracy = 0;

        // Working copy of degrees
        int *d = new int[n];
        for (int i = 0; i < n; i++) d[i] = degrees[i];

        // Find max degree for bucket allocation
        int maxDeg = 0;
        for (int i = 0; i < n; i++) {
            if (d[i] > maxDeg) maxDeg = d[i];
        }

        // Bucket structure: bucket[k] = list of nodes with current degree k
        vector<int> *bucket = new vector<int>[maxDeg + 1];
        int *nodePos = new int[n];  // position of node in its bucket
        
        for (int i = 0; i < n; i++) {
            nodePos[i] = bucket[d[i]].size();
            bucket[d[i]].push_back(i);
        }

        bool *removed = new bool[n];
        for (int i = 0; i < n; i++) removed[i] = false;

        int currentDeg = 0;
        for (int count = 0; count < n; count++) {
            // Find minimum non-empty bucket
            while (currentDeg <= maxDeg && bucket[currentDeg].empty()) {
                currentDeg++;
            }

            // Remove a node from the minimum bucket
            int v = bucket[currentDeg].back();
            bucket[currentDeg].pop_back();
            removed[v] = true;
            degeneracy[v] = currentDeg;
            if (currentDeg > maxDegeneracy) maxDegeneracy = currentDeg;

            // Update neighbors
            for (int u : neighborhoods[v]) {
                if (!removed[u] && d[u] > 0) {
                    int oldDeg = d[u];
                    int pos = nodePos[u];
                    
                    // Swap with last element in old bucket
                    if (pos < (int)bucket[oldDeg].size() - 1) {
                        int last = bucket[oldDeg].back();
                        bucket[oldDeg][pos] = last;
                        nodePos[last] = pos;
                    }
                    bucket[oldDeg].pop_back();

                    // Move to new bucket
                    d[u]--;
                    nodePos[u] = bucket[d[u]].size();
                    bucket[d[u]].push_back(u);

                    // Can decrease currentDeg
                    if (d[u] < currentDeg) currentDeg = d[u];
                }
            }
        }

        delete[] d;
        delete[] bucket;
        delete[] nodePos;
        delete[] removed;
    }
};