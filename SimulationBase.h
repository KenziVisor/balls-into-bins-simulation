#ifndef BALLS_BINS_SIMULATION_BASE_H
#define BALLS_BINS_SIMULATION_BASE_H

#include <random>
#include <string>
#include <unordered_map>
#include <vector>

namespace balls_bins {

class SimulationBase {
public:
    SimulationBase(int m, int n);
    virtual ~SimulationBase() = default;

    int getM() const;
    int getN() const;
    const std::vector<double>& getBins() const;
    double getTotalCost() const;
    double getCostWeight(const std::string& key) const;
    void setCostWeight(const std::string& key, double value);
    void reset();

    virtual void run() = 0;

protected:
    int drawRandomBin();
    double readBinLoad(int bin_index);
    bool lessThan(double a, double b);
    void addBallToBin(int bin_index);
    void validateBinIndex(int bin_index) const;

    int m_;
    int n_;
    std::vector<double> bins_;
    std::unordered_map<std::string, double> cost_weights_;
    double total_cost_;
    std::mt19937 rng_;
};

}  // namespace balls_bins

#endif  // BALLS_BINS_SIMULATION_BASE_H
