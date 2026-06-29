#include "balls_bins/ExperimentRunner.h"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

enum class SingleModel {
    PowerK,
    RoundRobin,
    Heap
};

struct SingleRunConfig {
    // PowerK: k=1 random, k=2 power of two, k=bins absolute minimum.
    // RoundRobin ignores k/s. Heap uses s; full heap is s=bins.
    SingleModel model = SingleModel::PowerK;

    int balls = 10000;
    int bins = 32;
    int trials = 10;

    int k = 2;
    int s = 32;

    bool weighted_balls = true;
    double max_weight = 10.0;

    bool random_initialization = false;
    int max_initial_load = 100;

    unsigned int workload_seed = 42;
    unsigned int allocation_seed = 1337;

    balls_bins::CostWeights cost_weights{
        {"random_draw", 1.0},
        {"load_read", 3.0},
        {"compare", 1.0},
        {"state_read", 3.0},
        {"state_update", 5.0},
        {"state_memory_per_slot", 100.0},
        {"heap_update_per_level", 5.0},
    };

    std::string output_path = "results/single_simulation_results.csv";
};

const SingleRunConfig kConfig{};

std::string modelToString(SingleModel model) {
    switch (model) {
        case SingleModel::PowerK:
            return "PowerK";
        case SingleModel::RoundRobin:
            return "RoundRobin";
        case SingleModel::Heap:
            return "Heap";
    }

    return "Unknown";
}

std::string modeLabel(const SingleRunConfig& config) {
    return std::string(config.weighted_balls ? "weighted" : "unweighted") + "_" +
           (config.random_initialization ? "initialized" : "empty");
}

std::string powerKPolicyName(const SingleRunConfig& config) {
    if (config.k == 1) {
        return config.weighted_balls ? "Random weighted" : "Random";
    }

    if (config.k == config.bins) {
        return config.weighted_balls ? "Absolute minimum weighted"
                                     : "Absolute minimum";
    }

    std::string name = "Power-" + std::to_string(config.k);
    if (config.weighted_balls) {
        name += " weighted";
    }

    return name;
}

std::string heapPolicyName(const SingleRunConfig& config) {
    if (config.s == config.bins) {
        return "Full heap";
    }

    return "Heap s=" + std::to_string(config.s);
}

std::string policyName(const SingleRunConfig& config) {
    switch (config.model) {
        case SingleModel::PowerK:
            return powerKPolicyName(config);
        case SingleModel::RoundRobin:
            return "Round-robin";
        case SingleModel::Heap:
            return heapPolicyName(config);
    }

    return "Unknown";
}

void validateConfig(const SingleRunConfig& config) {
    if (config.balls < 0) {
        throw std::invalid_argument("balls must be non-negative.");
    }

    if (config.bins <= 0) {
        throw std::invalid_argument("bins must be positive.");
    }

    if (config.trials <= 0) {
        throw std::invalid_argument("trials must be positive.");
    }

    if (config.weighted_balls && config.max_weight < 1.0) {
        throw std::invalid_argument(
            "weighted_balls=true requires max_weight >= 1.0.");
    }

    if (config.random_initialization && config.max_initial_load < 0) {
        throw std::invalid_argument(
            "random_initialization=true requires max_initial_load >= 0.");
    }

    if (config.model == SingleModel::PowerK) {
        if (config.k < 1 || config.k > config.bins) {
            throw std::invalid_argument("PowerK requires 1 <= k <= bins.");
        }
    }

    if (config.model == SingleModel::Heap) {
        if (config.s < 1 || config.s > config.bins) {
            throw std::invalid_argument("Heap requires 1 <= s <= bins.");
        }

        if (config.s < config.bins &&
            (config.k < 1 || config.k > config.bins - config.s)) {
            throw std::invalid_argument(
                "Partial heap requires 1 <= k <= bins - s.");
        }
    }
}

balls_bins::Scenario buildSingleScenario(const SingleRunConfig& config) {
    balls_bins::Scenario scenario;
    scenario.experiment_id = "single_custom";
    scenario.experiment_title = "Single custom simulation";
    scenario.policy_name = policyName(config);
    scenario.balls = config.balls;
    scenario.bins = config.bins;
    scenario.trials = config.trials;
    scenario.weighted_balls = config.weighted_balls;
    scenario.max_weight = config.weighted_balls ? config.max_weight : 1.0;
    scenario.workload_seed = config.workload_seed;
    scenario.allocation_seed = config.allocation_seed;
    scenario.random_initialization_enabled = config.random_initialization;
    scenario.max_initial_load =
        config.random_initialization ? config.max_initial_load : 0;
    scenario.mode_label = modeLabel(config);
    scenario.cost_weights = config.cost_weights;

    switch (config.model) {
        case SingleModel::PowerK:
            scenario.policy_family = "stateless";
            scenario.simulator_name = "PowerKSimulator";
            scenario.k = config.k;
            scenario.heap_size = 0;
            break;

        case SingleModel::RoundRobin:
            scenario.policy_family = "round_robin";
            scenario.simulator_name = "StatefulRoundRobinSimulator";
            scenario.k = 0;
            scenario.heap_size = 0;
            break;

        case SingleModel::Heap:
            scenario.policy_family = "heap";
            scenario.simulator_name = "HeapSizeSPowerOfKSimulator";
            scenario.heap_size = config.s;
            scenario.k = config.s == config.bins ? 0 : config.k;
            break;
    }

    return scenario;
}

void printConfig(const SingleRunConfig& config,
                 const balls_bins::Scenario& scenario) {
    std::cout << "Starting single balls-into-bins simulation...\n";
    std::cout << "Model: " << modelToString(config.model) << '\n';
    std::cout << "Policy: " << scenario.policy_name << '\n';
    std::cout << "Balls: " << scenario.balls << '\n';
    std::cout << "Bins: " << scenario.bins << '\n';
    std::cout << "Trials: " << scenario.trials << '\n';
    std::cout << "k: " << scenario.k << '\n';
    std::cout << "Heap size: " << scenario.heap_size << '\n';
    std::cout << "Weighted balls: "
              << (scenario.weighted_balls ? "true" : "false") << '\n';
    std::cout << "Max weight: " << scenario.max_weight << '\n';
    std::cout << "Random initialization: "
              << (scenario.random_initialization_enabled ? "true" : "false")
              << '\n';
    std::cout << "Max initial load: " << scenario.max_initial_load << '\n';
    std::cout << "Mode label: " << scenario.mode_label << '\n';
    std::cout << "Workload seed: " << scenario.workload_seed << '\n';
    std::cout << "Allocation seed: " << scenario.allocation_seed << '\n';
    std::cout << "Cost weights:\n";
    for (const auto& [name, value] : config.cost_weights) {
        std::cout << "  " << name << ": " << value << '\n';
    }
    std::cout << "Output CSV: " << config.output_path << "\n\n";
}

}  // namespace

int main() {
    try {
        validateConfig(kConfig);
        const balls_bins::Scenario scenario = buildSingleScenario(kConfig);

        std::filesystem::create_directories(
            std::filesystem::path(kConfig.output_path).parent_path());

        printConfig(kConfig, scenario);

        const std::vector<balls_bins::ScenarioResult> results =
            balls_bins::runScenarios({scenario});
        balls_bins::exportScenarioResultsCsv(results, kConfig.output_path);

        std::cout << "\nFinished single simulation.\n";
        std::cout << "Output CSV: " << kConfig.output_path << '\n';
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
