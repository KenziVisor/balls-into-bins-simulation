#ifndef BALLS_BINS_SIMULATION_METRICS_H
#define BALLS_BINS_SIMULATION_METRICS_H

#include <vector>

namespace balls_bins {

struct CostBreakdown {
    double random_draw = 0.0;
    double load_read = 0.0;
    double compare = 0.0;
    double state_read = 0.0;
    double state_update = 0.0;
    double state_memory = 0.0;
    double heap_update = 0.0;

    double total() const;
    double statelessCost() const;
    double statefulCost() const;
};

struct TrialMetrics {
    double average_load = 0.0;
    double min_load = 0.0;
    double max_load = 0.0;
    double max_min_gap = 0.0;
    double stddev_load = 0.0;
    double cv_load = 0.0;
    double normalized_max_load = 0.0;
    double normalized_gap = 0.0;
    double p90_load = 0.0;
    double p95_load = 0.0;
    double total_cost = 0.0;
    double cost_per_ball = 0.0;
    double random_draw_cost = 0.0;
    double load_read_cost = 0.0;
    double compare_cost = 0.0;
    double state_read_cost = 0.0;
    double state_update_cost = 0.0;
    double state_memory_cost = 0.0;
    double heap_update_cost = 0.0;
    double stateless_cost = 0.0;
    double stateful_cost = 0.0;
};

struct MetricSummary {
    double mean = 0.0;
    double stddev = 0.0;
};

struct AggregatedMetrics {
    MetricSummary average_load;
    MetricSummary min_load;
    MetricSummary max_load;
    MetricSummary max_min_gap;
    MetricSummary stddev_load;
    MetricSummary cv_load;
    MetricSummary normalized_max_load;
    MetricSummary normalized_gap;
    MetricSummary p90_load;
    MetricSummary p95_load;
    MetricSummary total_cost;
    MetricSummary cost_per_ball;
    MetricSummary random_draw_cost;
    MetricSummary load_read_cost;
    MetricSummary compare_cost;
    MetricSummary state_read_cost;
    MetricSummary state_update_cost;
    MetricSummary state_memory_cost;
    MetricSummary heap_update_cost;
    MetricSummary stateless_cost;
    MetricSummary stateful_cost;
};

TrialMetrics computeTrialMetrics(const std::vector<double>& bins,
                                 double total_cost,
                                 int balls,
                                 const CostBreakdown& cost_breakdown);

AggregatedMetrics aggregateMetrics(const std::vector<TrialMetrics>& trials);

}  // namespace balls_bins

#endif  // BALLS_BINS_SIMULATION_METRICS_H
