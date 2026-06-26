#include "RandomSimulator.h"

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
    std::cout << "Average load: " << average << '\n';
    std::cout << "Minimum bin load: " << min_load << '\n';
    std::cout << "Maximum bin load: " << max_load << '\n';
    std::cout << "Max-min gap: " << max_load - min_load << '\n';
    std::cout << "Standard deviation: " << computeStdDev(bins) << '\n';
    std::cout << "Total logical cost: " << total_cost << '\n';
    std::cout << "Cost per ball: " << cost_per_ball << '\n';
    std::cout << "Final bin loads: ";

    for (std::size_t i = 0; i < bins.size(); ++i) {
        if (i > 0) {
            std::cout << ", ";
        }

        std::cout << bins[i];
    }

    std::cout << "\n\n";
}

int main() {
    std::cout << std::fixed << std::setprecision(2);

    balls_bins::RandomSimulator random_16_bins(10000, 16);
    random_16_bins.run();
    printSimulationSummary(random_16_bins, "Random bin per ball");

    balls_bins::RandomSimulator random_32_bins(10000, 32);
    random_32_bins.run();
    printSimulationSummary(random_32_bins, "Random bin per ball");

    return 0;
}
