#include "ExperimentRunner.h"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <vector>

int main() {
    try {
        const std::string output_path = "results/simulation_results.csv";
        std::filesystem::create_directories("results");

        const std::vector<balls_bins::Scenario> scenarios =
            balls_bins::buildDefaultScenarios();
        const std::vector<balls_bins::ScenarioResult> results =
            balls_bins::runScenarios(scenarios);

        balls_bins::exportScenarioResultsCsv(results, output_path);

        const int trials = scenarios.empty() ? 0 : scenarios.front().trials;
        const unsigned int workload_seed =
            scenarios.empty() ? 0U : scenarios.front().workload_seed;
        const unsigned int allocation_seed =
            scenarios.empty() ? 0U : scenarios.front().allocation_seed;

        std::cout << "Scenarios run: " << results.size() << '\n';
        std::cout << "Output CSV: " << output_path << '\n';
        std::cout << "Trials per scenario: " << trials << '\n';
        std::cout << "Workload seed: " << workload_seed << '\n';
        std::cout << "Allocation seed: " << allocation_seed << '\n';
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
