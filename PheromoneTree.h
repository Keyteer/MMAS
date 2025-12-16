#pragma once

#include <random>
#include <vector>
#include <stdexcept>

// fix seeded for reproducibility
#define RANDOM_SEED 42

struct pheromoneTree {
    /*
        pheromoneTree is a complete binary tree stored in an array.
        The leaves represent the pheromone levels of the nodes in the graph.
        Internal nodes store the sum of pheromone levels of their children.
        
        MMAS: Uses tau_min and tau_max bounds to prevent stagnation.
    */

    int n;                      // number of leaves (nodes in the graph)
    int tree_size;              // total size of the tree array  
    float *pheromones;          // array storing pheromone levels
    float evaporation_rate;     // rate at which pheromones evaporate
    float tau_min;              // minimum pheromone level (MMAS)
    float tau_max;              // maximum pheromone level (MMAS)

    /*
        Constructor: initializes the pheromone tree as a deep copy of another tree.
    */
    pheromoneTree(pheromoneTree const &other) {
        n = other.n;
        tree_size = other.tree_size;
        srand(RANDOM_SEED);
        evaporation_rate = other.evaporation_rate;
        tau_min = other.tau_min;
        tau_max = other.tau_max;
        pheromones = new float[tree_size];
        memcpy(pheromones, other.pheromones, tree_size * sizeof(float));
    }
    /* 
        Constructor: initializes the pheromone tree with n leaves and sets
        the evaporation rate. MMAS: All leaves start at tau_max.
    */
    pheromoneTree(int n, float evaporation_rate, float tau_min = 1.0f, float tau_max = 100.0f) {
        // calculate tree size 
        int i = 1;
        while( i < n) {
            i = i << 1;    // search smallest power of 2 greater than n
        }
        tree_size = i*2 - 1;

        this->n = n;
        this->evaporation_rate = evaporation_rate;
        this->tau_min = tau_min;
        this->tau_max = tau_max;
        
        // initialize pheromone levels at tau_max (MMAS)
        pheromones = new float[tree_size];
        memset(pheromones, 0, tree_size * sizeof(float));
        for (int i = getLeaf(0); i <= getLeaf(n - 1); i++) {
            pheromones[i] = tau_max;
        }
        propagateAll();
    }
    ~pheromoneTree() {
        delete[] pheromones;
    }

    /*
        operator: deep copy of another pheromone tree.
    */
    pheromoneTree& operator=(const pheromoneTree& other) {
        if (this != &other) {
            delete[] pheromones;
            n = other.n;
            tree_size = other.tree_size;
            evaporation_rate = other.evaporation_rate;
            tau_min = other.tau_min;
            tau_max = other.tau_max;
            pheromones = new float[tree_size];
            memcpy(pheromones, other.pheromones, tree_size * sizeof(float));
        }
        return *this;
    }

    /*
        Evaporate: implement the disipation of pheromones in all nodes.
        This is done by multiplying the pheromone level of each node by (1 - evaporation_rate).
        MMAS: Clamp pheromones to [tau_min, tau_max] bounds.
        Then propagate the changes up the tree.
    */
    void evaporate() {
        for (int i = getLeaf(0); i <= getLeaf(n - 1); i++) {
            pheromones[i] = pheromones[i] * (1.0f - evaporation_rate);
            // MMAS: clamp to tau_min
            if (pheromones[i] < tau_min) {
                pheromones[i] = tau_min; 
            }
        }
        propagateAll();
    }
    /*
        Deposit: add pheromones to a node and propagate the changes up the tree.
        MMAS: Clamps the pheromone level to tau_max.
    */
    void deposit(int node, float amount) {
        node = getLeaf(node);
        pheromones[node] += amount;
        // MMAS: clamp to tau_max
        if (pheromones[node] > tau_max) {
            pheromones[node] = tau_max;
        }
        propagate(node);
    }
    /*
        Invalidate: set the pheromone level of a node to 0 and propagate the changes up the tree.
    */
    void invalidate(int node) {
        node = getLeaf(node);
        if (pheromones[node] == 0.0f) {
            return;
        }
        pheromones[node] = 0.0f;
        propagate(node);
    }
    /*
        InvalidateVector: set the pheromone level of a group of nodes in a vector to 0 and propagate the changes up the tree.
    */
    void invalidateVector(const std::vector<int>& nodes) {
        for (int node : nodes) {
            pheromones[getLeaf(node)] = 0.0f;
        }
        propagateAll();
    }
    /*
        SetPheromone: set the pheromone level of a node to a specific value and propagate the changes up the tree.
        MMAS: Clamps the pheromone level to [tau_min, tau_max].
    */
    void setPheromone(int node, float value) {
        node = getLeaf(node);
        // MMAS: clamp to bounds
        if (value < tau_min) value = tau_min;
        if (value > tau_max) value = tau_max;
        pheromones[node] = value;
        propagate(node);
    }

    /*
        GetPheromone: returns the pheromone level of a node (for use in ant selection).
    */
    float getPheromone(int node) {
        return pheromones[getLeaf(node)];
    }

    private:

    /*
        Propagate: updates the pheromone levels up the tree from a given node.
    */
    void propagate(int node) {
        while(node > 0) {
            pheromones[getFather(node)] = pheromones[node] + pheromones[getBrother(node)];
            node = getFather(node);
        }
    }
    /*
        PropagateAll: updates the pheromone levels of all internal nodes.
    */
    void propagateAll() {
        for (int i = tree_size / 2 - 1; i >= 0; i--) {
            pheromones[i] = pheromones[getLeftChild(i)] + pheromones[getRightChild(i)];
        }
    }

    /*
        Getters for tree navigation.
    */
    inline int getLeaf(int node) {
        if (node < n) {
            return node + tree_size / 2;
        } else if (node > tree_size / 2 && node < tree_size) {
            return node;
        }else{
            throw std::out_of_range("Node index is not a valid leaf");
        }
        return -1;
    }
    inline int getNodeFromLeaf(int leaf) {
        return leaf - tree_size / 2;
    }

    inline int getFather(int node) {
        return (node - 1) / 2;
    }
    inline int getLeftChild(int node) {
        return (node << 1) + 1;
    }
    inline int getRightChild(int node) {
        return (node << 1) + 2;
    }
    inline int getBrother(int node) {
        // returns -1 if root
        if (node % 2 == 0) {
            return node - 1;
        } else {
            return node + 1;
        }
    }
    inline bool isLeaf(int node) {
        return node >= tree_size / 2;
    }

    public:

    /*
        PondRandSearch: selects a random way down the tree but biased by pheromone levels.
    */
    int pondRandSearch(int father=0){
        // if both children are 0 pheromone, return -1
        if (pheromones[getLeftChild(father)] == 0.0f && pheromones[getRightChild(father)] == 0.0f) {
            return -1;
        }
        // biased random walk down the tree
        while (!isLeaf(father)) {
            // choose child based on pheromone levels
            float total = pheromones[getLeftChild(father)] + pheromones[getRightChild(father)];
            float randVal = static_cast<float>(rand()) / RAND_MAX * total;
            if (randVal <= pheromones[getLeftChild(father)] ){
                father = getLeftChild(father) > 0.0f? getLeftChild(father) : getRightChild(father);
            } else {
                father = getRightChild(father) > 0.0f? getRightChild(father) : getLeftChild(father);
            }
        }

        return getNodeFromLeaf(father);
    }

};
