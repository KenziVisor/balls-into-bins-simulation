#include "balls_bins/ExperimentRunner.h"

#include "balls_bins/CsvWriter.h"
#include "balls_bins/HeapSizeSPowerOfKSimulator.h"
#include "balls_bins/PowerKSimulator.h"
#include "balls_bins/StatefulRoundRobinSimulator.h"

#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>

namespace balls_bins {

namespace {

constexpr int kBalls = 10000;
constexpr int kTrials = 10;
constexpr double kWeightedMaxWeight = 10.0;
constexpr int kMaxInitialLoad = 100;
constexpr unsigned int kWorkloadSeed = 42;
constexpr unsigned int kAllocationSeed = 1337;

std::string boolToString(bool value) {
    return value ? "true" : "false";
}

std::string numberToString(double value) {
    std::ostringstream output;
    output << std::setprecision(12) << value;
    return output.str();
}

std::string intToString(int value) {
    return std::to_string(value);
}

std::string uintToString(unsigned int value) {
    return std::to_string(value);
}

double perBall(const MetricSummary& summary, int balls) {
    return balls == 0 ? 0.0 : summary.mean / static_cast<double>(balls);
}

std::string modeLabel(bool weighted_balls, bool random_initialization_enabled) {
    return std::string(weighted_balls ? "weighted" : "unweighted") + "_" +
           (random_initialization_enabled ? "initialized" : "empty");
}

Scenario makePowerScenario(const std::string& experiment_id,
                           const std::string& experiment_title,
                           const std::string& policy_name,
                           int bins,
                           int k,
                           bool weighted_balls,
                           bool random_initialization_enabled = false) {
    return {experiment_id,
            experiment_title,
            policy_name,
            "stateless",
            "PowerKSimulator",
            kBalls,
            bins,
            kTrials,
            weighted_balls,
            weighted_balls ? kWeightedMaxWeight : 1.0,
            kWorkloadSeed,
            kAllocationSeed,
            k,
            0,
            random_initialization_enabled,
            random_initialization_enabled ? kMaxInitialLoad : 0,
            modeLabel(weighted_balls, random_initialization_enabled)};
}

Scenario makeRoundRobinScenario(const std::string& experiment_id,
                                const std::string& experiment_title,
                                int bins,
                                bool weighted_balls,
                                bool random_initialization_enabled = false) {
    return {experiment_id,
            experiment_title,
            "Round-robin",
            "round_robin",
            "StatefulRoundRobinSimulator",
            kBalls,
            bins,
            kTrials,
            weighted_balls,
            weighted_balls ? kWeightedMaxWeight : 1.0,
            kWorkloadSeed,
            kAllocationSeed,
            0,
            0,
            random_initialization_enabled,
            random_initialization_enabled ? kMaxInitialLoad : 0,
            modeLabel(weighted_balls, random_initialization_enabled)};
}

Scenario makeHeapScenario(const std::string& experiment_id,
                          const std::string& experiment_title,
                          const std::string& policy_name,
                          int bins,
                          int heap_size,
                          int k,
                          bool weighted_balls,
                          bool random_initialization_enabled = false) {
    return {experiment_id,
            experiment_title,
            policy_name,
            "heap",
            "HeapSizeSPowerOfKSimulator",
            kBalls,
            bins,
            kTrials,
            weighted_balls,
            weighted_balls ? kWeightedMaxWeight : 1.0,
            kWorkloadSeed,
            kAllocationSeed,
            k,
            heap_size,
            random_initialization_enabled,
            random_initialization_enabled ? kMaxInitialLoad : 0,
            modeLabel(weighted_balls, random_initialization_enabled)};
}

void addScenario(std::vector<Scenario>& scenarios, const Scenario& scenario) {
    scenarios.push_back(scenario);
}

void addPowerPolicies(std::vector<Scenario>& scenarios,
                      const std::string& experiment_id,
                      const std::string& experiment_title,
                      int bins,
                      bool weighted_balls,
                      bool random_initialization_enabled,
                      const std::vector<int>& k_values) {
    for (const int k : k_values) {
        std::string policy_name;
        if (k == 1) {
            policy_name = weighted_balls ? "Random weighted" : "Random";
        } else if (k == bins) {
            policy_name = weighted_balls ? "Absolute minimum weighted"
                                         : "Absolute minimum";
        } else {
            policy_name = "Power-" + std::to_string(k);
            if (weighted_balls) {
                policy_name += " weighted";
            }
        }

        addScenario(scenarios,
                    makePowerScenario(experiment_id,
                                      experiment_title,
                                      policy_name,
                                      bins,
                                      k,
                                      weighted_balls,
                                      random_initialization_enabled));
    }
}

std::unique_ptr<SimulationBase> createSimulator(const Scenario& scenario) {
    if (scenario.simulator_name == "PowerKSimulator") {
        return std::make_unique<PowerKSimulator>(scenario.balls,
                                                 scenario.bins,
                                                 scenario.k,
                                                 scenario.trials,
                                                 scenario.weighted_balls,
                                                 scenario.max_weight,
                                                 scenario.workload_seed,
                                                 scenario.allocation_seed,
                                                 scenario.random_initialization_enabled,
                                                 scenario.max_initial_load);
    }

    if (scenario.simulator_name == "StatefulRoundRobinSimulator") {
        return std::make_unique<StatefulRoundRobinSimulator>(scenario.balls,
                                                             scenario.bins,
                                                             scenario.trials,
                                                             scenario.weighted_balls,
                                                             scenario.max_weight,
                                                             scenario.workload_seed,
                                                             scenario.allocation_seed,
                                                             scenario.random_initialization_enabled,
                                                             scenario.max_initial_load);
    }

    if (scenario.simulator_name == "HeapSizeSPowerOfKSimulator") {
        return std::make_unique<HeapSizeSPowerOfKSimulator>(scenario.balls,
                                                            scenario.bins,
                                                            scenario.heap_size,
                                                            scenario.k,
                                                            scenario.trials,
                                                            scenario.weighted_balls,
                                                            scenario.max_weight,
                                                            scenario.workload_seed,
                                                            scenario.allocation_seed,
                                                            scenario.random_initialization_enabled,
                                                            scenario.max_initial_load);
    }

    throw std::invalid_argument("Unknown simulator: " + scenario.simulator_name);
}

std::string referenceNote(bool weighted_balls, bool random_initialization_enabled) {
    if (weighted_balls) {
        return "Average-load lower bound only; true weighted optimum requires "
               "partition/bin-packing";
    }

    if (random_initialization_enabled) {
        return "Average-load lower bound only; optimum with initialized bins is "
               "not computed";
    }

    return "Optimal unweighted allocation";
}

void printScenarioStart(const Scenario& scenario,
                        std::size_t scenario_index,
                        std::size_t scenario_count) {
    std::cout << '[' << scenario_index << '/' << scenario_count << "] "
              << scenario.experiment_id << " | " << scenario.policy_name
              << " | n=" << scenario.bins << " | " << scenario.mode_label;

    if (scenario.k > 0 || scenario.simulator_name == "PowerKSimulator" ||
        scenario.simulator_name == "HeapSizeSPowerOfKSimulator") {
        std::cout << " | k=" << scenario.k;
    }

    if (scenario.heap_size > 0) {
        std::cout << " | heap_size=" << scenario.heap_size;
    }

    if (scenario.random_initialization_enabled) {
        std::cout << " | max_initial_load=" << scenario.max_initial_load;
    }

    std::cout << '\n';
}

void printScenarioDone(const AggregatedMetrics& metrics) {
    std::cout << "      done: max_load=" << std::fixed << std::setprecision(2)
              << metrics.max_load.mean << ", cv=" << std::setprecision(4)
              << metrics.cv_load.mean << ", cost/ball=" << std::setprecision(2)
              << metrics.cost_per_ball.mean << std::defaultfloat << '\n';
}

}  // namespace

std::vector<Scenario> buildDefaultScenarios() {
    std::vector<Scenario> scenarios;
    const std::vector<int> bin_values{16, 32};

    for (const int bins : bin_values) {
        addPowerPolicies(scenarios,
                         "01_stateless_baseline",
                         "Stateless baseline",
                         bins,
                         false,
                         false,
                         {1, 2, 3, bins});
    }

    for (const int bins : bin_values) {
        const std::vector<int> k_values =
            bins == 16 ? std::vector<int>{1, 2, 3, 4, 8, 16}
                       : std::vector<int>{1, 2, 3, 4, 8, 16, 32};
        addPowerPolicies(scenarios,
                         "02_diminishing_returns",
                         "Diminishing returns of k",
                         bins,
                         false,
                         false,
                         k_values);
    }

    for (const int bins : bin_values) {
        addPowerPolicies(scenarios,
                         "03_stateful_vs_stateless_unweighted",
                         "Stateful vs stateless under equal weights",
                         bins,
                         false,
                         false,
                         {1, 2, 3});
        addScenario(scenarios,
                    makeRoundRobinScenario("03_stateful_vs_stateless_unweighted",
                                           "Stateful vs stateless under equal weights",
                                           bins,
                                           false));
        addScenario(scenarios,
                    makePowerScenario("03_stateful_vs_stateless_unweighted",
                                      "Stateful vs stateless under equal weights",
                                      "Absolute minimum",
                                      bins,
                                      bins,
                                      false));
    }

    for (const int bins : bin_values) {
        addPowerPolicies(scenarios,
                         "04_weighted_round_robin_break",
                         "Weighted balls break round-robin",
                         bins,
                         true,
                         false,
                         {1, 2, 3});
        addScenario(scenarios,
                    makeRoundRobinScenario("04_weighted_round_robin_break",
                                           "Weighted balls break round-robin",
                                           bins,
                                           true));
        addScenario(scenarios,
                    makePowerScenario("04_weighted_round_robin_break",
                                      "Weighted balls break round-robin",
                                      "Absolute minimum weighted",
                                      bins,
                                      bins,
                                      true));
    }

    for (const int bins : bin_values) {
        const std::vector<int> heap_sizes =
            bins == 16 ? std::vector<int>{2, 4, 8, 16}
                       : std::vector<int>{2, 4, 8, 16, 32};
        for (const int heap_size : heap_sizes) {
            const bool full_heap = heap_size == bins;
            addScenario(scenarios,
                        makeHeapScenario("05_heap_sweep",
                                         "Heap size sweep",
                                         full_heap ? "Full heap"
                                                   : "Heap s=" +
                                                         std::to_string(heap_size),
                                         bins,
                                         heap_size,
                                         full_heap ? 0 : 2,
                                         true));
        }
    }

    for (const int bins : bin_values) {
        for (const bool weighted_balls : {false, true}) {
            const std::string experiment_id =
                weighted_balls ? "07_final_tradeoff_weighted"
                               : "06_final_tradeoff_unweighted";
            const std::string experiment_title =
                weighted_balls ? "Final tradeoff weighted"
                               : "Final tradeoff unweighted";

            addPowerPolicies(scenarios,
                             experiment_id,
                             experiment_title,
                             bins,
                             weighted_balls,
                             false,
                             {1, 2, 3, 4});
            addScenario(scenarios,
                        makePowerScenario(experiment_id,
                                          experiment_title,
                                          weighted_balls
                                              ? "Absolute minimum weighted"
                                              : "Absolute minimum",
                                          bins,
                                          bins,
                                          weighted_balls));
            addScenario(scenarios,
                        makeRoundRobinScenario(experiment_id,
                                               experiment_title,
                                               bins,
                                               weighted_balls));

            for (const int heap_size : {4, 8, 16}) {
                if (heap_size >= bins) {
                    continue;
                }

                addScenario(scenarios,
                            makeHeapScenario(experiment_id,
                                             experiment_title,
                                             "Heap s=" +
                                                 std::to_string(heap_size),
                                             bins,
                                             heap_size,
                                             2,
                                             weighted_balls));
            }

            addScenario(scenarios,
                        makeHeapScenario(experiment_id,
                                         experiment_title,
                                         "Full heap",
                                         bins,
                                         bins,
                                         0,
                                         weighted_balls));
        }
    }

    for (const bool weighted_balls : {false, true}) {
        for (const bool initialized : {false, true}) {
            const std::string mode = modeLabel(weighted_balls, initialized);
            const std::string main_id = "08_main_comparison_" + mode;
            const std::string tradeoff_id = "09_tradeoff_" + mode;
            const std::string main_title = "Main comparison " + mode;
            const std::string tradeoff_title = "Cost-quality tradeoff " + mode;

            for (const int bins : bin_values) {
                addPowerPolicies(scenarios,
                                 main_id,
                                 main_title,
                                 bins,
                                 weighted_balls,
                                 initialized,
                                 {1, 2});
                addScenario(scenarios,
                            makePowerScenario(main_id,
                                              main_title,
                                              weighted_balls
                                                  ? "Absolute minimum weighted"
                                                  : "Absolute minimum",
                                              bins,
                                              bins,
                                              weighted_balls,
                                              initialized));
                addScenario(scenarios,
                            makeRoundRobinScenario(main_id,
                                                   main_title,
                                                   bins,
                                                   weighted_balls,
                                                   initialized));

                addPowerPolicies(scenarios,
                                 tradeoff_id,
                                 tradeoff_title,
                                 bins,
                                 weighted_balls,
                                 initialized,
                                 {1, 2, 3, 4});
                addScenario(scenarios,
                            makePowerScenario(tradeoff_id,
                                              tradeoff_title,
                                              weighted_balls
                                                  ? "Absolute minimum weighted"
                                                  : "Absolute minimum",
                                              bins,
                                              bins,
                                              weighted_balls,
                                              initialized));
                addScenario(scenarios,
                            makeRoundRobinScenario(tradeoff_id,
                                                   tradeoff_title,
                                                   bins,
                                                   weighted_balls,
                                                   initialized));
            }
        }
    }

    return scenarios;
}

std::vector<ScenarioResult> runScenarios(const std::vector<Scenario>& scenarios) {
    std::vector<ScenarioResult> results;
    results.reserve(scenarios.size());

    for (std::size_t index = 0; index < scenarios.size(); ++index) {
        const Scenario& scenario = scenarios[index];
        printScenarioStart(scenario, index + 1, scenarios.size());

        std::unique_ptr<SimulationBase> simulator = createSimulator(scenario);
        simulator->run();

        const AggregatedMetrics metrics = simulator->getAggregatedMetrics();
        printScenarioDone(metrics);

        const ReferenceMetrics reference = computeReferenceMetrics(
            scenario.balls,
            scenario.bins,
            scenario.weighted_balls,
            metrics.average_load.mean,
            scenario.random_initialization_enabled);

        results.push_back({scenario,
                           metrics,
                           reference,
                           referenceNote(scenario.weighted_balls,
                                         scenario.random_initialization_enabled)});
    }

    return results;
}

void exportScenarioResultsCsv(const std::vector<ScenarioResult>& results,
                              const std::string& path) {
    CsvWriter writer(path);
    writer.writeRow({"experiment_id",
                     "experiment_title",
                     "policy_name",
                     "policy_family",
                     "simulator_name",
                     "mode_label",
                     "num_balls",
                     "num_bins",
                     "balls",
                     "bins",
                     "trials",
                     "weighted_balls",
                     "weighted_enabled",
                     "random_initialization_enabled",
                     "max_initial_load",
                     "max_weight",
                     "workload_seed",
                     "allocation_seed",
                     "k",
                     "heap_size",
                     "average_load_mean",
                     "average_load_stddev",
                     "min_load_mean",
                     "min_load_stddev",
                     "max_load_mean",
                     "max_load_stddev",
                     "max_min_gap_mean",
                     "max_min_gap_stddev",
                     "load_stddev_mean",
                     "load_stddev_stddev",
                     "cv_load_mean",
                     "cv_load_stddev",
                     "normalized_max_load_mean",
                     "normalized_max_load_stddev",
                     "normalized_gap_mean",
                     "normalized_gap_stddev",
                     "cost_per_ball_mean",
                     "cost_per_ball_stddev",
                     "total_cost_mean",
                     "total_cost_stddev",
                     "random_draw_cost_per_ball",
                     "load_read_cost_per_ball",
                     "compare_cost_per_ball",
                     "state_read_cost_per_ball",
                     "state_update_cost_per_ball",
                     "state_memory_cost_per_ball",
                     "heap_update_cost_per_ball",
                     "stateless_cost_per_ball",
                     "stateful_cost_per_ball",
                     "reference_type",
                     "reference_max_load",
                     "reference_cv_load",
                     "reference_note"});

    for (const ScenarioResult& result : results) {
        const Scenario& scenario = result.scenario;
        const AggregatedMetrics& metrics = result.metrics;

        writer.writeRow({scenario.experiment_id,
                         scenario.experiment_title,
                         scenario.policy_name,
                         scenario.policy_family,
                         scenario.simulator_name,
                         scenario.mode_label,
                         intToString(scenario.balls),
                         intToString(scenario.bins),
                         intToString(scenario.balls),
                         intToString(scenario.bins),
                         intToString(scenario.trials),
                         boolToString(scenario.weighted_balls),
                         boolToString(scenario.weighted_balls),
                         boolToString(scenario.random_initialization_enabled),
                         intToString(scenario.max_initial_load),
                         numberToString(scenario.max_weight),
                         uintToString(scenario.workload_seed),
                         uintToString(scenario.allocation_seed),
                         intToString(scenario.k),
                         intToString(scenario.heap_size),
                         numberToString(metrics.average_load.mean),
                         numberToString(metrics.average_load.stddev),
                         numberToString(metrics.min_load.mean),
                         numberToString(metrics.min_load.stddev),
                         numberToString(metrics.max_load.mean),
                         numberToString(metrics.max_load.stddev),
                         numberToString(metrics.max_min_gap.mean),
                         numberToString(metrics.max_min_gap.stddev),
                         numberToString(metrics.stddev_load.mean),
                         numberToString(metrics.stddev_load.stddev),
                         numberToString(metrics.cv_load.mean),
                         numberToString(metrics.cv_load.stddev),
                         numberToString(metrics.normalized_max_load.mean),
                         numberToString(metrics.normalized_max_load.stddev),
                         numberToString(metrics.normalized_gap.mean),
                         numberToString(metrics.normalized_gap.stddev),
                         numberToString(metrics.cost_per_ball.mean),
                         numberToString(metrics.cost_per_ball.stddev),
                         numberToString(metrics.total_cost.mean),
                         numberToString(metrics.total_cost.stddev),
                         numberToString(perBall(metrics.random_draw_cost,
                                                scenario.balls)),
                         numberToString(perBall(metrics.load_read_cost,
                                                scenario.balls)),
                         numberToString(perBall(metrics.compare_cost,
                                                scenario.balls)),
                         numberToString(perBall(metrics.state_read_cost,
                                                scenario.balls)),
                         numberToString(perBall(metrics.state_update_cost,
                                                scenario.balls)),
                         numberToString(perBall(metrics.state_memory_cost,
                                                scenario.balls)),
                         numberToString(perBall(metrics.heap_update_cost,
                                                scenario.balls)),
                         numberToString(perBall(metrics.stateless_cost,
                                                scenario.balls)),
                         numberToString(perBall(metrics.stateful_cost,
                                                scenario.balls)),
                         referenceTypeToString(result.reference.type),
                         numberToString(result.reference.max_load),
                         numberToString(result.reference.cv_load),
                         result.reference_note});
    }
}

}  // namespace balls_bins
