#include <random>
#include <vector>

#include "NeighList.h"
#include "PheromoneTree.h"

struct Ant{

    NeighList *nl;                  // neighborhood list of the graph
    vector<int> sol;                // current solution
    pheromoneTree *global_tree;     // reference to the global pheromone tree
    pheromoneTree tree;             // local pheromone tree for the ant

    Ant(NeighList *nl, pheromoneTree *tree) : tree(*tree) {
        this->global_tree = tree;
        this->nl = nl;
    }
    ~Ant(){}
    void reset() {
        this->tree = *global_tree;
        sol.clear();
    }

    /*
        constructPondSolution: constructs a solution using the pond-randomized
        search method to select nodes based on pheromone levels.
    */
    int constructPondSolution() {
        // assumed empty solution
        for (int node = tree.pondRandSearch(); node != -1; node = tree.pondRandSearch()) {
            sol.push_back(node);
            // invalidate neighbors
            tree.invalidateVector(nl->neighborhoods[node]);
            // invalidate node itself
            tree.invalidate(node);
        }
        return sol.size();
    }

    /*
        depositInSolution: deposits pheromones in all nodes of the current solution.
    */
    void depositInSolution(float deposit_amount) {
        for (int node : sol)
            global_tree->deposit(node, deposit_amount);
    }

    /*
        depositToSetAmountInSolution: sets pheromone amount in all nodes of the current solution.
    */
    void depositToSetAmountInSolution(float deposit_amount) {
        for (int node : sol)
            global_tree->setPheromone(node, deposit_amount);
    }
};