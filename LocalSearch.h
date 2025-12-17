#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include "utils.h"

int try1Adds(MISP_Solution *sol) {
    int n = sol->graph->n;
    int added = 0;

    for (int node = 0; node < n; node++) {
        if (sol->MISP_IndependentDegree[node] == 0) {
            sol->addNode(node);
            added++;
        }
    }

    return added;
}

// Local Search: Try to improve solution by adding independent nodes
// budget: 0 to deactivate, 1 to only try 1-1 swaps, more to also do 2-1 swaps
void localSearch(MISP_Solution *sol, int budget) {

    int n = sol->graph->n;

    // try adding nodes
    try1Adds(sol);

    while (budget > 0) {

        // try 1-1 improving swaps
        bool improvement = false;
        for (int node_in = 0; node_in < n; node_in++) {
            if (sol->MISP_IndependentDegree[node_in] == 1) {

                // find the neighbor in the solution
                int node_out = -1;
                for (int nd : sol->solution)
                    if (sol->graph->isNeighbor(node_in, nd))
                        node_out = nd;

                if (node_out == -1) {
                    std::cerr << "Error: Inconsistent independent degree for node " << node_in << "\n";
                    continue;
                }

                // apply swap
                sol->removeNode(node_out);
                sol->addNode(node_in);

                // check if swap is improving
                if (try1Adds(sol) > 0) {
                    // improvement found, start over
                    improvement = true;
                    break;
                } else {
                    // not improving, revert swap
                    sol->removeNode(node_in);
                    sol->addNode(node_out);
                }
            }
        }

        if (improvement) {
            continue; // start over
        }

        // do any 2-1 swap if at least 2 budget
        if (budget > 1) {
            bool swapFound = false;

            for (int i = 0; i < n; i++) {
                if (sol->MISP_IndependentDegree[i] == 2) {

                    // find the two neighbors in the solution
                    int node_out1 = -1;
                    int node_out2 = -1;
                    for (int nd : sol->solution) {
                        if (sol->graph->isNeighbor(i, nd)) {
                            if (node_out1 == -1) {
                                node_out1 = nd;
                            } else {
                                node_out2 = nd;
                                break;
                            }
                        }
                    }

                    if (node_out1 == -1 || node_out2 == -1) {
                        std::cerr << "Error: Inconsistent independent degree for node " << i << "\n";
                        continue;
                    }

                    // apply swap
                    sol->removeNode(node_out1);
                    sol->removeNode(node_out2);
                    sol->addNode(i);

                    swapFound = true;

                    budget--;

                    break; // only one 2-1 swap per extra budget unit
                }
            }

            if (swapFound) {
                // start over
                try1Adds(sol);
                continue;
            } else {
                // no posible 2-1 swap found
                // localSearch finished
                break;
            }
        }
    }
}
