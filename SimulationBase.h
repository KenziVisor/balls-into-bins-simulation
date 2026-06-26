#ifndef BALLS_BINS_SIMULATION_BASE_H
#define BALLS_BINS_SIMULATION_BASE_H

#include <random>
#include <string>
#include <unordered_map>
#include <vector>

namespace balls_bins {

class SimulationBase {
public:
    SimulationBase(int m, int n, int trials = 1);
    virtual ~SimulationBase() = default;

    int getM() const;
    int getN() const;
    int getTrials() const;
    const std::vector<double>& getBins() const;
    double getTotalCost() const;
    double getCostWeight(const std::string& key) const;
    void setCostWeight(const std::string& key, double value);
    void reset();

    void run();

protected:
    virtual void runSingleTrial() = 0;

    std::vector<int> drawRandomBins(int k);
    double readBinLoad(int bin_index);
    bool lessThan(double a, double b);
    void addBallToBin(int bin_index);
    void validateBinIndex(int bin_index) const;

    int m_;
    int n_;
    int trials_;
    std::vector<double> bins_;
    std::unordered_map<std::string, double> cost_weights_;
    double total_cost_;
    std::mt19937 rng_;
};

}  // namespace balls_bins

#endif  // BALLS_BINS_SIMULATION_BASE_H
