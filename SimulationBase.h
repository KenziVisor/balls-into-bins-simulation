#ifndef BALLS_BINS_SIMULATION_BASE_H
#define BALLS_BINS_SIMULATION_BASE_H

#include "SimulationMetrics.h"

#include <random>
#include <string>
#include <unordered_map>
#include <vector>

namespace balls_bins {

class SimulationBase {
public:
    SimulationBase(int m,
                   int n,
                   int trials = 1,
                   bool weighted_balls = false,
                   double max_weight = 1.0,
                   unsigned int workload_seed = 42,
                   unsigned int allocation_seed = 1337);
    virtual ~SimulationBase() = default;

    int getM() const;
    int getN() const;
    int getTrials() const;
    bool isWeightedBalls() const;
    double getMaxWeight() const;
    unsigned int getWorkloadSeed() const;
    unsigned int getAllocationSeed() const;
    const std::vector<double>& getBins() const;
    double getTotalCost() const;
    const std::vector<TrialMetrics>& getTrialMetrics() const;
    const AggregatedMetrics& getAggregatedMetrics() const;
    const CostBreakdown& getCostBreakdown() const;
    double getCostWeight(const std::string& key) const;
    std::vector<double> previewBallWeights(int count, int trial_index = 0) const;
    void setCostWeight(const std::string& key, double value);
    void reset();

    void run();

protected:
    virtual void runSingleTrial() = 0;

    std::vector<int> drawRandomBins(int k);
    double drawBallWeight();
    double readBinLoad(int bin_index);
    bool lessThan(double a, double b);
    void addBallToBin(int bin_index, double weight);
    void validateBinIndex(int bin_index) const;
    void addRandomDrawCost(double units);
    void addLoadReadCost(double units);
    void addCompareCost(double units);
    void addStateReadCost(double units);
    void addStateUpdateCost(double units);
    void addStateMemoryCost(double units);
    void addHeapUpdateCost(double units);

    int m_;
    int n_;
    int trials_;
    bool weighted_balls_;
    double max_weight_;
    unsigned int workload_seed_;
    unsigned int allocation_seed_;
    std::vector<double> bins_;
    std::vector<TrialMetrics> trial_metrics_;
    AggregatedMetrics aggregated_metrics_;
    CostBreakdown cost_breakdown_;
    std::unordered_map<std::string, double> cost_weights_;
    double total_cost_;
    std::mt19937 rng_;
    std::mt19937 workload_rng_;
};

}  // namespace balls_bins

#endif  // BALLS_BINS_SIMULATION_BASE_H
