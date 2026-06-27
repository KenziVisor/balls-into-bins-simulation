#include "SimulationMetrics.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace balls_bins {

double CostBreakdown::total() const {
    return random_draw + load_read + compare + state_read + state_update +
           state_memory + heap_update;
}

double CostBreakdown::statelessCost() const {
    return random_draw + load_read + compare;
}

double CostBreakdown::statefulCost() const {
    return state_read + state_update + state_memory + heap_update;
}

namespace {

double mean(const std::vector<double>& values) {
    if (values.empty()) {
        return 0.0;
    }

    double sum = 0.0;
    for (const double value : values) {
        sum += value;
    }

    return sum / static_cast<double>(values.size());
}

double populationStddev(const std::vector<double>& values, double average) {
    if (values.empty()) {
        return 0.0;
    }

    double squared_difference_sum = 0.0;
    for (const double value : values) {
        const double difference = value - average;
        squared_difference_sum += difference * difference;
    }

    return std::sqrt(squared_difference_sum / static_cast<double>(values.size()));
}

double percentileNearestRank(std::vector<double> sorted_values, double percentile) {
    if (sorted_values.empty()) {
        return 0.0;
    }

    std::sort(sorted_values.begin(), sorted_values.end());

    // Nearest-rank percentile: ceil(p / 100 * n), clamped to [1, n].
    const double rank = std::ceil((percentile / 100.0) *
                                  static_cast<double>(sorted_values.size()));
    const std::size_t index =
        static_cast<std::size_t>(std::max(1.0, rank)) - static_cast<std::size_t>(1);

    return sorted_values[std::min(index, sorted_values.size() - 1)];
}

MetricSummary summarize(const std::vector<TrialMetrics>& trials,
                        double TrialMetrics::*member) {
    if (trials.empty()) {
        return {};
    }

    std::vector<double> values;
    values.reserve(trials.size());

    for (const TrialMetrics& trial : trials) {
        values.push_back(trial.*member);
    }

    const double average = mean(values);

    if (values.size() < 2) {
        return {average, 0.0};
    }

    double squared_difference_sum = 0.0;
    for (const double value : values) {
        const double difference = value - average;
        squared_difference_sum += difference * difference;
    }

    return {average, std::sqrt(squared_difference_sum /
                               static_cast<double>(values.size() - 1))};
}

}  // namespace

TrialMetrics computeTrialMetrics(const std::vector<double>& bins,
                                 double total_cost,
                                 int balls,
                                 const CostBreakdown& cost_breakdown) {
    TrialMetrics metrics;
    metrics.total_cost = total_cost;
    metrics.cost_per_ball = balls == 0 ? 0.0 : total_cost / static_cast<double>(balls);

    metrics.random_draw_cost = cost_breakdown.random_draw;
    metrics.load_read_cost = cost_breakdown.load_read;
    metrics.compare_cost = cost_breakdown.compare;
    metrics.state_read_cost = cost_breakdown.state_read;
    metrics.state_update_cost = cost_breakdown.state_update;
    metrics.state_memory_cost = cost_breakdown.state_memory;
    metrics.heap_update_cost = cost_breakdown.heap_update;
    metrics.stateless_cost = cost_breakdown.statelessCost();
    metrics.stateful_cost = cost_breakdown.statefulCost();

    if (bins.empty()) {
        return metrics;
    }

    metrics.average_load = mean(bins);
    metrics.min_load = *std::min_element(bins.begin(), bins.end());
    metrics.max_load = *std::max_element(bins.begin(), bins.end());
    metrics.max_min_gap = metrics.max_load - metrics.min_load;
    metrics.stddev_load = populationStddev(bins, metrics.average_load);
    metrics.cv_load = metrics.average_load == 0.0
                          ? 0.0
                          : metrics.stddev_load / metrics.average_load;
    metrics.normalized_max_load = metrics.average_load == 0.0
                                      ? 0.0
                                      : metrics.max_load / metrics.average_load;
    metrics.normalized_gap = metrics.average_load == 0.0
                                 ? 0.0
                                 : metrics.max_min_gap / metrics.average_load;
    metrics.p90_load = percentileNearestRank(bins, 90.0);
    metrics.p95_load = percentileNearestRank(bins, 95.0);

    return metrics;
}

AggregatedMetrics aggregateMetrics(const std::vector<TrialMetrics>& trials) {
    AggregatedMetrics metrics;
    metrics.average_load = summarize(trials, &TrialMetrics::average_load);
    metrics.min_load = summarize(trials, &TrialMetrics::min_load);
    metrics.max_load = summarize(trials, &TrialMetrics::max_load);
    metrics.max_min_gap = summarize(trials, &TrialMetrics::max_min_gap);
    metrics.stddev_load = summarize(trials, &TrialMetrics::stddev_load);
    metrics.cv_load = summarize(trials, &TrialMetrics::cv_load);
    metrics.normalized_max_load = summarize(trials, &TrialMetrics::normalized_max_load);
    metrics.normalized_gap = summarize(trials, &TrialMetrics::normalized_gap);
    metrics.p90_load = summarize(trials, &TrialMetrics::p90_load);
    metrics.p95_load = summarize(trials, &TrialMetrics::p95_load);
    metrics.total_cost = summarize(trials, &TrialMetrics::total_cost);
    metrics.cost_per_ball = summarize(trials, &TrialMetrics::cost_per_ball);
    metrics.random_draw_cost = summarize(trials, &TrialMetrics::random_draw_cost);
    metrics.load_read_cost = summarize(trials, &TrialMetrics::load_read_cost);
    metrics.compare_cost = summarize(trials, &TrialMetrics::compare_cost);
    metrics.state_read_cost = summarize(trials, &TrialMetrics::state_read_cost);
    metrics.state_update_cost = summarize(trials, &TrialMetrics::state_update_cost);
    metrics.state_memory_cost = summarize(trials, &TrialMetrics::state_memory_cost);
    metrics.heap_update_cost = summarize(trials, &TrialMetrics::heap_update_cost);
    metrics.stateless_cost = summarize(trials, &TrialMetrics::stateless_cost);
    metrics.stateful_cost = summarize(trials, &TrialMetrics::stateful_cost);

    return metrics;
}

}  // namespace balls_bins
