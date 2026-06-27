#include "HeapSizeSPowerOfKSimulator.h"
#include "PowerKSimulator.h"
#include "StatefulRoundRobinSimulator.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

double computeAverage(const std::vector<double>& bins) {
    if (bins.empty()) {
        return 0.0;
    }

    double total = 0.0;
    for (const double load : bins) {
        total += load;
    }

    return total / static_cast<double>(bins.size());
}

double computeMin(const std::vector<double>& bins) {
    if (bins.empty()) {
        return 0.0;
    }

    return *std::min_element(bins.begin(), bins.end());
}

double computeMax(const std::vector<double>& bins) {
    if (bins.empty()) {
        return 0.0;
    }

    return *std::max_element(bins.begin(), bins.end());
}

double computeStdDev(const std::vector<double>& bins) {
    if (bins.empty()) {
        return 0.0;
    }

    const double average = computeAverage(bins);
    double squared_difference_sum = 0.0;

    for (const double load : bins) {
        const double difference = load - average;
        squared_difference_sum += difference * difference;
    }

    return std::sqrt(squared_difference_sum / static_cast<double>(bins.size()));
}

void printSimulationSummary(const balls_bins::SimulationBase& sim,
                            const std::string& policy_name) {
    const std::vector<double>& bins = sim.getBins();
    const double average = computeAverage(bins);
    const double min_load = computeMin(bins);
    const double max_load = computeMax(bins);
    const double total_cost = sim.getTotalCost();
    const double cost_per_ball =
        sim.getM() == 0 ? 0.0 : total_cost / static_cast<double>(sim.getM());

    std::cout << "Policy: " << policy_name << '\n';
    std::cout << "Balls: " << sim.getM() << '\n';
    std::cout << "Bins: " << sim.getN() << '\n';
    std::cout << "Trials: " << sim.getTrials() << '\n';
    std::cout << "Average load: " << average << '\n';
    std::cout << "Minimum averaged bin load: " << min_load << '\n';
    std::cout << "Maximum averaged bin load: " << max_load << '\n';
    std::cout << "Max-min gap over averaged loads: " << max_load - min_load << '\n';
    std::cout << "Standard deviation: " << computeStdDev(bins) << '\n';
    std::cout << "Average total logical cost per trial: " << total_cost << '\n';
    std::cout << "Average cost per ball: " << cost_per_ball << '\n';
    std::cout << "Averaged final bin loads: ";

    for (std::size_t i = 0; i < bins.size(); ++i) {
        if (i > 0) {
            std::cout << ", ";
        }

        std::cout << bins[i];
    }

    std::cout << "\n\n";
}

void printWorkloadPreviewCheck() {
    const int balls = 10000;
    const int bins = 16;
    const int trials = 1;
    const double max_weight = 10.0;
    const unsigned int workload_seed = 42;
    const int preview_count = 10;

    balls_bins::PowerKSimulator power_2(
        balls, bins, 2, trials, true, max_weight, workload_seed);
    balls_bins::StatefulRoundRobinSimulator round_robin(
        balls, bins, trials, true, max_weight, workload_seed);

    const std::vector<double> power_weights = power_2.previewBallWeights(preview_count);
    const std::vector<double> round_robin_weights =
        round_robin.previewBallWeights(preview_count);
    const bool match = (power_weights == round_robin_weights);

    std::cout << "Workload preview match: " << (match ? "yes" : "no") << '\n';
    std::cout << "First " << preview_count << " workload weights: ";

    for (int i = 0; i < preview_count; ++i) {
        if (i > 0) {
            std::cout << ", ";
        }

        std::cout << power_weights[static_cast<std::size_t>(i)];
    }

    std::cout << "\n\n";
}

int main() {
    std::cout << std::fixed << std::setprecision(2);

    const int trials = 1;
    const double weighted_max_weight = 10.0;
    const unsigned int workload_seed = 42;

    printWorkloadPreviewCheck();

    balls_bins::PowerKSimulator random_16_bins(10000, 16, 1, trials);
    random_16_bins.run();
    printSimulationSummary(random_16_bins, "Unweighted random bin per ball");

    balls_bins::PowerKSimulator power_2_16_bins(10000, 16, 2, trials);
    power_2_16_bins.run();
    printSimulationSummary(power_2_16_bins, "Unweighted power of 2 random choices");

    balls_bins::PowerKSimulator power_3_16_bins(10000, 16, 3, trials);
    power_3_16_bins.run();
    printSimulationSummary(power_3_16_bins, "Unweighted power of 3 random choices");

    balls_bins::PowerKSimulator power_16_16_bins(10000, 16, 16, trials);
    power_16_16_bins.run();
    printSimulationSummary(power_16_16_bins, "Unweighted minimal load (power of n=16)");

    balls_bins::HeapSizeSPowerOfKSimulator full_heap_16_bins(10000, 16, 16, 0, trials);
    full_heap_16_bins.run();
    printSimulationSummary(full_heap_16_bins, "Unweighted full heap (s=16)");

    balls_bins::HeapSizeSPowerOfKSimulator partial_heap_16_bins(10000, 16, 4, 2, trials);
    partial_heap_16_bins.run();
    printSimulationSummary(partial_heap_16_bins, "Unweighted partial heap (s=4, k=2)");

    balls_bins::HeapSizeSPowerOfKSimulator partial_heap_full_scan_16_bins(
        10000, 16, 4, 12, trials);
    partial_heap_full_scan_16_bins.run();
    printSimulationSummary(partial_heap_full_scan_16_bins,
                           "Unweighted partial heap full outside scan (s=4, k=12)");

    balls_bins::StatefulRoundRobinSimulator round_robin_16_bins(10000, 16, trials);
    round_robin_16_bins.run();
    printSimulationSummary(round_robin_16_bins, "Unweighted stateful round-robin");

    balls_bins::StatefulRoundRobinSimulator weighted_round_robin_16_bins(
        10000, 16, trials, true, weighted_max_weight, workload_seed);
    weighted_round_robin_16_bins.run();
    printSimulationSummary(weighted_round_robin_16_bins,
                           "Weighted stateful round-robin (max weight 10)");

    balls_bins::PowerKSimulator weighted_power_2_16_bins(
        10000, 16, 2, trials, true, weighted_max_weight, workload_seed);
    weighted_power_2_16_bins.run();
    printSimulationSummary(weighted_power_2_16_bins,
                           "Weighted power of 2 random choices (max weight 10)");

    balls_bins::HeapSizeSPowerOfKSimulator weighted_full_heap_16_bins(
        10000, 16, 16, 0, trials, true, weighted_max_weight, workload_seed);
    weighted_full_heap_16_bins.run();
    printSimulationSummary(weighted_full_heap_16_bins,
                           "Weighted full heap (s=16, max weight 10)");

    balls_bins::HeapSizeSPowerOfKSimulator weighted_partial_heap_16_bins(
        10000, 16, 4, 2, trials, true, weighted_max_weight, workload_seed);
    weighted_partial_heap_16_bins.run();
    printSimulationSummary(weighted_partial_heap_16_bins,
                           "Weighted partial heap (s=4, k=2, max weight 10)");

    balls_bins::PowerKSimulator random_32_bins(10000, 32, 1, trials);
    random_32_bins.run();
    printSimulationSummary(random_32_bins, "Unweighted random bin per ball");

    balls_bins::PowerKSimulator power_2_32_bins(10000, 32, 2, trials);
    power_2_32_bins.run();
    printSimulationSummary(power_2_32_bins, "Unweighted power of 2 random choices");

    balls_bins::PowerKSimulator power_3_32_bins(10000, 32, 3, trials);
    power_3_32_bins.run();
    printSimulationSummary(power_3_32_bins, "Unweighted power of 3 random choices");

    balls_bins::PowerKSimulator power_32_32_bins(10000, 32, 32, trials);
    power_32_32_bins.run();
    printSimulationSummary(power_32_32_bins, "Unweighted minimal load (power of n=32)");

    balls_bins::StatefulRoundRobinSimulator round_robin_32_bins(10000, 32, trials);
    round_robin_32_bins.run();
    printSimulationSummary(round_robin_32_bins, "Unweighted stateful round-robin");

    balls_bins::StatefulRoundRobinSimulator weighted_round_robin_32_bins(
        10000, 32, trials, true, weighted_max_weight, workload_seed);
    weighted_round_robin_32_bins.run();
    printSimulationSummary(weighted_round_robin_32_bins,
                           "Weighted stateful round-robin (max weight 10)");

    balls_bins::PowerKSimulator weighted_power_2_32_bins(
        10000, 32, 2, trials, true, weighted_max_weight, workload_seed);
    weighted_power_2_32_bins.run();
    printSimulationSummary(weighted_power_2_32_bins,
                           "Weighted power of 2 random choices (max weight 10)");

    return 0;
}
