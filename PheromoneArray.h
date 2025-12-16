#pragma once

#include <vector>

struct pheromoneArray {
    /*
            MMAS: Uses tau_min and tau_max bounds to prevent stagnation.
    */

    int n;                      // number of leaves (nodes in the graph)
    float *pheromones;          // array storing pheromone levels
    float evaporation_rate;     // rate at which pheromones evaporate
    float tau_min;              // minimum pheromone level (MMAS)
    float tau_max;              // maximum pheromone level (MMAS)


    pheromoneArray(int n, float evaporation_rate, float tau_min = 1.0f, float tau_max = 100.0f) {
        
        this->n = n;
        this->evaporation_rate = evaporation_rate;
        this->tau_min = tau_min;
        this->tau_max = tau_max;

        pheromones = new float[n];
        for (int i = 0; i <= n; i++) {
            pheromones[i] = tau_max;
        }
    }

    ~pheromoneArray(){
        delete[] pheromones;
    }

    void evaporate() {
        for (int i = 0; i <= n; i++) {
            pheromones[i] = pheromones[i] * (1.0f - evaporation_rate);
            // MMAS: clamp to tau_min
            if (pheromones[i] < tau_min) {
                pheromones[i] = tau_min; 
            }
        }
    }

    /*
        Deposit: add pheromones to a node and propagate the changes up the tree.
        MMAS: Clamps the pheromone level to tau_max.
    */
    void deposit(int node, float amount) {
        pheromones[node] += amount;
        // MMAS: clamp to tau_max
        if (pheromones[node] > tau_max) {
            pheromones[node] = tau_max;
        }
    }
    /*
        Invalidate: set the pheromone level of a node to 0 and propagate the changes up the tree.
    */
    void invalidate(int node) {
        if (pheromones[node] == 0.0f) {
            return;
        }
        pheromones[node] = 0.0f;
    }
    /*
        InvalidateVector: set the pheromone level of a group of nodes in a vector to 0 and propagate the changes up the tree.
    */
    void invalidateVector(const std::vector<int>& nodes) {
        for (int node : nodes) {
            pheromones[node] = 0.0f;
        }
    }
    /*
        SetPheromone: set the pheromone level of a node to a specific value and propagate the changes up the tree.
        MMAS: Clamps the pheromone level to [tau_min, tau_max].
    */
    void setPheromone(int node, float value) {
        // MMAS: clamp to bounds
        if (value < tau_min) value = tau_min;
        if (value > tau_max) value = tau_max;
        pheromones[node] = value;
    }

    /*
        GetPheromone: returns the pheromone level of a node (for use in ant selection).
    */
    float getPheromone(int node) {
        return pheromones[node];
    }

    int gradSearch(){
        /*
            
        */
    }

    int minHubSearch(){
        /*
            
        */
    }

    int dynamicSearch(){
        /*

        */
    }
};
