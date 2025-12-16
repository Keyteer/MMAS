#include <random>
#include <vector>
#include <cmath>

#include "NeighList.h"
#include "PheromoneTree.h"

struct Ant{

    NeighList *nl;                  // neighborhood list of the graph
    vector<int> sol;                // current solution
    pheromoneTree *global_tree;     // reference to the global pheromone tree
    pheromoneTree tree;             // local pheromone tree for the ant
    float alpha;                    // pheromone influence exponent
    float beta;                     // heuristic influence exponent
    float *heuristic;               // precomputed heuristic values (1/(1+degree))

    Ant(NeighList *nl, pheromoneTree *tree, float alpha = 1.0f, float beta = 2.0f) : tree(*tree) {
        this->global_tree = tree;
        this->nl = nl;
        this->alpha = alpha;
        this->beta = beta;
        
        // Precompute heuristic values: η_i = 1/(1+degree_i)
        // Lower degree nodes are more attractive for MISP
        heuristic = new float[nl->n];
        for (int i = 0; i < nl->n; i++) {
            heuristic[i] = 1.0f / (1.0f + nl->degrees[i]);
        }
    }
    ~Ant(){
        delete[] heuristic;
    }
    void reset() {
        this->tree = *global_tree;
        sol.clear();
    }

    /*
        constructSolution: constructs a solution using MMAS probabilistic selection.
        Selects next node based on: p_i ∝ τ_i^α * η_i^β
        where τ = pheromone, η = heuristic (1/(1+degree))
    */
    int constructSolution() {
        // Build list of candidate nodes with their selection weights
        vector<int> candidates;
        vector<float> weights;
        
        // Initialize candidates with all valid nodes
        for (int i = 0; i < nl->n; i++) {
            float tau = tree.getPheromone(i);
            if (tau > 0.0f) {
                candidates.push_back(i);
                // MMAS selection probability: τ^α * η^β
                float weight = powf(tau, alpha) * powf(heuristic[i], beta);
                weights.push_back(weight);
            }
        }
        
        while (!candidates.empty()) {
            // Calculate total weight
            float total = 0.0f;
            for (float w : weights) total += w;
            
            if (total <= 0.0f) break;
            
            // Roulette wheel selection
            float randVal = static_cast<float>(rand()) / RAND_MAX * total;
            float cumulative = 0.0f;
            int selectedIdx = 0;
            
            for (size_t i = 0; i < candidates.size(); i++) {
                cumulative += weights[i];
                if (randVal <= cumulative) {
                    selectedIdx = i;
                    break;
                }
            }
            
            int selectedNode = candidates[selectedIdx];
            sol.push_back(selectedNode);
            
            // Mark selected node and its neighbors as invalid
            tree.invalidate(selectedNode);
            tree.invalidateVector(nl->neighborhoods[selectedNode]);
            
            // Rebuild candidates list (remove invalidated nodes)
            vector<int> newCandidates;
            vector<float> newWeights;
            
            for (size_t i = 0; i < candidates.size(); i++) {
                int node = candidates[i];
                float tau = tree.getPheromone(node);
                if (tau > 0.0f) {
                    newCandidates.push_back(node);
                    float weight = powf(tau, alpha) * powf(heuristic[node], beta);
                    newWeights.push_back(weight);
                }
            }
            
            candidates = std::move(newCandidates);
            weights = std::move(newWeights);
        }
        
        return sol.size();
    }

    /*
        localSearch: placeholder for local search improvement.
        TODO: Implement local search (e.g., 1-improvement, swap neighborhood)
    */
    void localSearch() {
        // Placeholder for local search
        // Possible improvements:
        // - Try adding nodes not in solution that don't conflict
        // - Try swapping nodes to find better configurations
    }

    /*
        depositInSolution: deposits pheromones in all nodes of the current solution.
        MMAS: deposit amount = 1/solution_quality (or proportional to quality)
    */
    void depositInSolution(float deposit_amount) {
        for (int node : sol)
            global_tree->deposit(node, deposit_amount);
    }
};