#ifndef BALLS_BINS_EXPERIMENT_RUNNER_H
#define BALLS_BINS_EXPERIMENT_RUNNER_H

#include "balls_bins/SimulationMetrics.h"
#include "balls_bins/SimulationReferences.h"

#include <string>
#include <vector>

namespace balls_bins {

struct Scenario {
    std::string experiment_id;
    std::string experiment_title;
    std::string policy_name;
    std::string policy_family;
    std::string simulator_name;
    int balls = 0;
    int bins = 0;
    int trials = 1;
    bool weighted_balls = false;
    double max_weight = 1.0;
    unsigned int workload_seed = 42;
    unsigned int allocation_seed = 1337;
    int k = 0;
    int heap_size = 0;
    bool random_initialization_enabled = false;
    int max_initial_load = 0;
    std::string mode_label;
};

struct ScenarioResult {
    Scenario scenario;
    AggregatedMetrics metrics;
    ReferenceMetrics reference;
    std::string reference_note;
};

std::vector<Scenario> buildDefaultScenarios();
std::vector<ScenarioResult> runScenarios(const std::vector<Scenario>& scenarios);
void exportScenarioResultsCsv(const std::vector<ScenarioResult>& results,
                              const std::string& path);

}  // namespace balls_bins

#endif  // BALLS_BINS_EXPERIMENT_RUNNER_H
