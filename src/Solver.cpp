/* Created by Isaac on 03/02/2025. */

#include "Solver.h"
#include <limits>

Solver::Solver()
{

}

void Solver::initialSolution_Greedy(CVRP *instance , Solution *initialSolution)
{
    int currentCapacity = 0;
    int counterVehicles = 0;
    double minimumCost;
    int minimumCostumer = -1;

    vector<bool> visited(instance->nodesDimension, false);

    initialSolution->routes.clear();
    initialSolution->routes.push_back(vector<int>()); // Start with one vehicle
    initialSolution->routes[counterVehicles].push_back(instance->depotID); // Start at depot
    initialSolution->totalCost = 0;

    for (int j = 0; j < instance->nodesDimension - 1; j++) // Exclude depot
    {
        minimumCost = numeric_limits<double>::max();
        minimumCostumer = -1;

        int lastCustomer = initialSolution->routes[counterVehicles].back();

        for (int i = 1; i < instance->nodesDimension; i++) // Start from 1 to exclude depot
        {
            if (visited[i]) continue; // Skip if already assigned

            double distance = instance->distanceMatrix[(lastCustomer * instance->nodesDimension) + i];

            if (currentCapacity + instance->nodes[i].demand <= instance->capacityOfVehicle && distance < minimumCost)
            {
                minimumCostumer = i;
                minimumCost = distance;
            }
        }

        if (minimumCostumer == -1)
        {
            // Close current route by returning to depot
            initialSolution->routes[counterVehicles].push_back(instance->depotID);
            counterVehicles++;
            initialSolution->routes.push_back(vector<int>());
            initialSolution->routes[counterVehicles].push_back(instance->depotID);
            currentCapacity = 0;
            j--; // Retry this iteration with a new vehicle
            continue;
        }

        initialSolution->routes[counterVehicles].push_back(minimumCostumer);
        currentCapacity += instance->nodes[minimumCostumer].demand;
        visited[minimumCostumer] = true;
        initialSolution->totalCost += minimumCost;
    }

    // Close last vehicle's route
    initialSolution->routes[counterVehicles].push_back(instance->depotID);
    initialSolution->fleetSize = counterVehicles + 1;
}

void Solver::GRASP_Construct(CVRP *instance, Solution *initialSolution, double alpha)
{
    initialSolution->routes.clear();
    initialSolution->totalCost = 0.0;

    vector<bool> visited(instance->nodesDimension, false);
    int numVehicles = 0;
    int depot = instance->depotID;

    // Start with an empty solution
    while (true)
    {
        vector<int> route;
        int currentCapacity = 0;
        int currentNode = depot;
        route.push_back(currentNode);

        while (true)
        {
            vector<pair<int, double>> candidates;

            // Find all unvisited nodes and their cost from the current node
            for (int i = 1; i < instance->nodesDimension; i++)
            {
                if (!visited[i] && currentCapacity + instance->nodes[i].demand <= instance->capacityOfVehicle)
                {
                    double cost = instance->distanceMatrix[(currentNode * instance->nodesDimension) + i];
                    candidates.emplace_back(i, cost);
                }
            }

            // If no candidates left, close the route
            if (candidates.empty())
                break;

            // Sort candidates by cost (ascending)
            sort(candidates.begin(), candidates.end(), [](const auto &a, const auto &b) {
                return a.second < b.second;
            });

            // Restricted Candidate List (RCL)
            double minCost = candidates.front().second;
            double maxCost = candidates.back().second;
            double threshold = minCost + alpha * (maxCost - minCost);
            vector<int> RCL;

            for (const auto &candidate : candidates)
            {
                if (candidate.second <= threshold)
                    RCL.push_back(candidate.first);
            }

            // Randomly select a node from the RCL
            int chosenNode = RCL[rand() % RCL.size()];
            route.push_back(chosenNode);
            visited[chosenNode] = true;
            currentCapacity += instance->nodes[chosenNode].demand;
            initialSolution->totalCost += instance->distanceMatrix[(currentNode * instance->nodesDimension) + chosenNode];
            currentNode = chosenNode;
        }

        // Return to depot
        route.push_back(depot);
        initialSolution->routes.push_back(route);
        initialSolution->totalCost += instance->distanceMatrix[(currentNode * instance->nodesDimension) + depot];

        numVehicles++;

        // Check if all nodes are visited
        if (all_of(visited.begin() + 1, visited.end(), [](bool v) { return v; }))
            break;
    }

    initialSolution->fleetSize = numVehicles;
}


void Solver::localSearch_TwoOpt(CVRP *instance, Solution *initialSolution, Solution *bestSolution)
{
    *bestSolution = *initialSolution; // Start with the initial solution
    bool improvement = true;

    while (improvement)
    {
        improvement = false;

        // Iterate through all routes in the solution
        for (auto &route : bestSolution->routes)
        {
            int routeSize = route.size();
            if (routeSize < 4) continue; // Ignore small routes (no swaps possible)

            for (int i = 1; i < routeSize - 2; i++)
            {
                for (int j = i + 1; j < routeSize - 1; j++)
                {
                    // Nodes before and after swap section
                    int A = route[i - 1], B = route[i];
                    int C = route[j], D = route[j + 1];

                    // Current cost of the two edges
                    double currentCost = instance->distanceMatrix[A * instance->nodesDimension + B] +
                                         instance->distanceMatrix[C * instance->nodesDimension + D];

                    // New cost if we swap (reverse the segment)
                    double newCost = instance->distanceMatrix[A * instance->nodesDimension + C] +
                                     instance->distanceMatrix[B * instance->nodesDimension + D];

                    if (newCost < currentCost)
                    {
                        // Perform the swap by reversing the segment
                        reverse(route.begin() + i, route.begin() + j + 1);

                        // Update solution cost
                        bestSolution->computeCost(instance->nodesDimension, instance->distanceMatrix);

                        improvement = true;
                    }
                }
            }
        }
    }
}


void Solver::localSearch_ThreeOpt(CVRP *instance, Solution *initialSolution, Solution *bestSolution)
{
    *bestSolution = *initialSolution; // Copy the initial solution as the starting best
    double bestCost = bestSolution->totalCost;
    bool improvement = true;

    while (improvement)
    {
        improvement = false;

        // Iterate through all possible three-cut combinations
        for (size_t i = 1; i < bestSolution->routes[0].size() - 3; i++)
        {
            for (size_t j = i + 1; j < bestSolution->routes[0].size() - 2; j++)
            {
                for (size_t k = j + 1; k < bestSolution->routes[0].size() - 1; k++)
                {
                    Solution newSolution = *bestSolution; // Copy current solution
                    vector<int> &route = newSolution.routes[0];

                    // Generate different 3-opt swaps
                    vector<vector<int>> possibleRoutes = {
                            route, // Keep the original route
                            route, // Reverse (i, j)
                            route, // Reverse (j, k)
                            route, // Reverse (i, j) and (j, k)
                            route, // Reverse entire segment (i, k)
                    };

                    reverse(possibleRoutes[1].begin() + i, possibleRoutes[1].begin() + j);
                    reverse(possibleRoutes[2].begin() + j, possibleRoutes[2].begin() + k);
                    reverse(possibleRoutes[3].begin() + i, possibleRoutes[3].begin() + j);
                    reverse(possibleRoutes[3].begin() + j, possibleRoutes[3].begin() + k);
                    reverse(possibleRoutes[4].begin() + i, possibleRoutes[4].begin() + k);

                    for (auto &newRoute : possibleRoutes)
                    {
                        newSolution.routes[0] = newRoute;
                        newSolution.computeCost(instance->nodesDimension, instance->distanceMatrix);

                        if (newSolution.totalCost < bestCost)
                        {
                            *bestSolution = newSolution;
                            bestCost = newSolution.totalCost;
                            improvement = true;
                        }
                    }
                }
            }
        }
    }
}


void Solver::acceptanceCriterion_BestSolution(Solution *bestSolution, Solution *newSolution)
{
    // Compare total costs
    if (newSolution->totalCost < bestSolution->totalCost)
        *bestSolution = *newSolution; // Update the best solution

}
