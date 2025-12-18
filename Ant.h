#pragma once

#include <random>
#include <vector>
#include <cmath>

#include "NeighList.h"
#include "PheromoneArray.h"
#include "utils.h"

struct Ant{

    NeighList *nl;                      // neighborhood list of the graph
    MISP_Solution *sol;                 // current solution
    pheromoneArray *global_pheromones;  // reference to the global pheromone array
    pheromoneArray pheromones;          // local pheromone array for the ant
    float alpha;                        // pheromone influence exponent
    float beta;                         // degree heuristic influence exponent
    float gamma;                        // degeneracy heuristic influence exponent
    float delta;                        // conflict heuristic influence exponent

    // Precomputed heuristics (static during construction)
    float *degreeH;                     // precomputed degree heuristic (if beta != 0)
    float *degeneracyH;                 // precomputed degeneracy heuristic (if gamma != 0)

    Ant(NeighList *nl, pheromoneArray *pheromones, float alpha, float beta, float gamma, float delta)
     : pheromones(*pheromones) {
        this->global_pheromones = pheromones;
        this->nl = nl;
        this->sol = new MISP_Solution(nl);
        this->alpha = alpha;
        this->beta = beta;
        this->gamma = gamma;
        this->delta = delta;

        // Precompute degree heuristic if beta != 0
        if (beta != 0.0f) {
            degreeH = new float[nl->n];
            for (int i = 0; i < nl->n; i++) {
                degreeH[i] = 1.0f / powf(1.0f + nl->degrees[i], beta);
            }
        } else {
            degreeH = nullptr;
        }

        // Precompute degeneracy heuristic if gamma != 0
        if (gamma != 0.0f && nl->degeneracy != nullptr) {
            degeneracyH = new float[nl->n];
            for (int i = 0; i < nl->n; i++) {
                degeneracyH[i] = 1.0f / powf(1.0f + nl->degeneracy[i], gamma);
            }
        } else {
            degeneracyH = nullptr;
        }
    }
    ~Ant(){
        delete sol;
        if (degreeH) delete[] degreeH;
        if (degeneracyH) delete[] degeneracyH;
    }
    void reset() {
        this->pheromones = *global_pheromones;
        delete sol;
        sol = new MISP_Solution(nl);
    }

    // Degree heuristic (precomputed)
    float degreeHeuristic(int node) {
        return degreeH ? degreeH[node] : 1.0f;
    }

    // Degeneracy heuristic (precomputed)
    float degeneracyHeuristic(int node) {
        return degeneracyH ? degeneracyH[node] : 1.0f;
    }

    // Conflict heuristic (IndependentDegree) - dynamic, depends on current solution
    float conflictHeuristic(int node) {
        return delta != 0.0f ? 1.0f / powf(1.0f + sol->MISP_IndependentDegree[node], delta) : 1.0f;
    }

    float combinedHeuristic(int node) {
        return degreeHeuristic(node) * degeneracyHeuristic(node) * conflictHeuristic(node);
    }

    // Build candidate list and weights from valid nodes in source
    void buildWeights(vector<int>& candidates, vector<float>& weights, const vector<int>& source) {
        candidates.clear();
        weights.clear();
        for (int node : source) {
            float tau = pheromones.getPheromone(node);
            if (tau > 0.0f) {
                candidates.push_back(node);
                float weight = powf(tau, alpha) * combinedHeuristic(node);
                weights.push_back(weight);
            }
        }
    }

    /*
        constructSolution: constructs a solution using MMAS probabilistic selection.
        weighting pheromone levels, node degree, node degeneracy and solution conflict.
        the weight of a node is given by: 
        pheromones^(alpha) * degreeHeuristic^(beta) * degeneracyHeuristic^(gamma) * conflictHeuristic^(delta)
    */
    int constructSolution() {
        // Initial source: all nodes
        vector<int> allNodes(nl->n);
        for (int i = 0; i < nl->n; i++) allNodes[i] = i;
        
        vector<int> candidates;
        vector<float> weights;
        buildWeights(candidates, weights, allNodes);
        
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
            sol->addNode(selectedNode);
            
            // Mark selected node and its neighbors as invalid
            pheromones.invalidate(selectedNode);
            pheromones.invalidateVector(nl->neighborhoods[selectedNode]);
            
            // Rebuild candidates - must copy source first since buildWeights clears it
            vector<int> oldCandidates = candidates;
            buildWeights(candidates, weights, oldCandidates);
        }
        
        return sol->size();
    }

    /*
        depositInSolution: deposits pheromones in all nodes of the current solution.
        MMAS: deposit amount = 1/solution_quality (or proportional to quality)
    */
    void depositInSolution(float deposit_amount) {
        for (int node : sol->solution)
            global_pheromones->deposit(node, deposit_amount);
    }
};