#include <chrono>
#include <cstring>
#include <algorithm>

#include "Ant.h"
#include "PheromoneTree.h"

using namespace std;

/*
    MMAS (Max-Min Ant System) for Maximum Independent Set Problem.
    
    Key features:
    - Pheromone bounds [tau_min, tau_max] to prevent stagnation
    - Only the iteration-best ant deposits pheromones
    - Pheromone initialization at tau_max
    - Pheromone trails are clamped to bounds after updates
    - Multiple ants per iteration
    - Heuristic information based on node degree
    
    Parameters:
    - m: number of ants per iteration
    - alpha: pheromone influence exponent
    - beta: heuristic influence exponent
    - rho: evaporation rate
    - tau_min, tau_max: pheromone bounds
*/
int MMAS(NeighList *nl, double time_limit, int m, float alpha, float beta, float rho,
         float tau_min = 1.0f, float tau_max = 100.0f,
         bool verbose = false, int *iterations = nullptr) {
    auto start_time = chrono::high_resolution_clock::now();

    int it = 0;
    if (iterations == nullptr) {
        iterations = &it;
    }

    // MMAS: Initialize pheromone tree with tau_min and tau_max bounds
    pheromoneTree pheromones(nl->n, rho, tau_min, tau_max);

    int global_best_size = 0;
    vector<int> global_best_solution;

    // Create colony of m ants
    vector<Ant*> colony;
    for (int i = 0; i < m; i++) {
        colony.push_back(new Ant(nl, &pheromones, alpha, beta));
    }
    
    while (chrono::duration<double>(chrono::high_resolution_clock::now() - start_time).count() < time_limit) {
        
        int iteration_best_size = 0;
        int iteration_best_ant = 0;
        
        // Each ant constructs a solution
        for (int i = 0; i < m; i++) {
            int size = colony[i]->constructSolution();
            
            // Optional: Apply local search (placeholder)
            // colony[i]->localSearch();
            // size = colony[i]->sol.size(); // Update size after local search
            
            // Track iteration best
            if (size > iteration_best_size) {
                iteration_best_size = size;
                iteration_best_ant = i;
            }
            
            // Track global best
            if (size > global_best_size) {
                global_best_size = size;
                global_best_solution = colony[i]->sol;
                
                if (verbose) printf("New best size: %d at iteration %d\n", global_best_size, *iterations);
            }
        }
        
        // MMAS: Only the iteration-best ant deposits pheromones
        // Deposit amount = 1/f(s) where f(s) is solution quality
        // For MISP, we want larger sets, so deposit amount = solution_size
        float deposit_amount = static_cast<float>(iteration_best_size);
        colony[iteration_best_ant]->depositInSolution(deposit_amount);
        
        // Reset all ants for next iteration
        for (int i = 0; i < m; i++) {
            colony[i]->reset();
        }
        
        // Evaporate pheromones
        pheromones.evaporate();

        (*iterations)++;
    }

    if (verbose) printf("Best size found: %d in %d iterations\n", global_best_size, *iterations);

    // Cleanup
    for (Ant* ant : colony) {
        delete ant;
    }

    return global_best_size;
}