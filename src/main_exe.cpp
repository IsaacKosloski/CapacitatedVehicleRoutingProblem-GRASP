/*Created by Isaac on 30/01/2025.*/

#include "Functions.h"
#include "Solver.h"
#include <chrono>
#include <format>

#define MAX_ITERATIONS 10'000
#define EXECUTIONS 30

int main(int argc, char **argv)
{
    // Validate input parameters
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <input_file> <output_file>" << endl;
        return 1;
    }

    // Setting the alpha parameter
    double alpha = 0.3;

    // Initialize problem components
    auto *cvrp = new CVRP(argv[1]);
    for (int executions = 0; executions < EXECUTIONS; executions++)
    {
        auto *solver = new Solver();
        auto *solution = new Solution();
        auto *bestSolution = new Solution();

        // Get the current time before execution
        auto start = chrono::high_resolution_clock::now();

        // Generate an initial greedy solution
        solver->initialSolution_Greedy(cvrp, solution);
        *bestSolution = *solution;  // Initialize the best solution

        for(int i = 0; i < MAX_ITERATIONS; i++)
        {
            // Reset the solution before each iteration
            solution->routes.clear();
            solution->routeLoads.clear();
            solution->totalCost = 0.0;

            solver->GRASP_Construct(cvrp, solution, alpha);
            solver->localSearch_SwapStar(cvrp, solution, 2);
            solver->acceptanceCriterion_BestSolution(bestSolution, solution);
        }

        // Get the current time after execution
        auto finish = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = finish - start;

        // Generating output file name
        string baseOutputFile(argv[2]);
        string outputFile = baseOutputFile.substr(0, baseOutputFile.find_last_of('.')) + "-" + std::format("{:02}", executions) + ".sol";
        char const *outputFileC = outputFile.c_str();

        // Printing the result on an output file
        bestSolution->printSolution(outputFileC, elapsed.count(), MAX_ITERATIONS, cvrp->nodesDimension);

        // Memory cleanup
        delete bestSolution;
        delete solution;
        delete solver;
    }
    delete cvrp;

    return 0;
}
