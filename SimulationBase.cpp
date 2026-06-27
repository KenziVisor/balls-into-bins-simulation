#include "SimulationBase.h"

#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <utility>

namespace balls_bins {

SimulationBase::SimulationBase(int m,
                               int n,
                               int trials,
                               bool weighted_balls,
                               double max_weight,
                               unsigned int workload_seed)
    : m_(m),
      n_(n),
      trials_(trials),
      weighted_balls_(weighted_balls),
      max_weight_(max_weight),
      workload_seed_(workload_seed),
      bins_(),
      cost_weights_{
          {"random_draw", 1.0},
          {"load_read", 5.0},
          {"compare", 1.0},
          {"state_read", 1.0},
          {"state_update", 1.0},
          {"state_memory_per_slot", 1.0},
      },
      total_cost_(0.0),
      rng_(std::random_device{}()),
      workload_rng_(workload_seed_) {
    if (m_ < 0) {
        throw std::invalid_argument("Number of balls must be non-negative.");
    }

    if (n_ <= 0) {
        throw std::invalid_argument("Number of bins must be positive.");
    }

    if (trials_ <= 0) {
        throw std::invalid_argument("Number of trials must be positive.");
    }

    if (max_weight_ < 1.0) {
        throw std::invalid_argument("Maximum ball weight must be at least 1.0.");
    }

    bins_.assign(static_cast<std::size_t>(n_), 0.0);
}

int SimulationBase::getM() const {
    return m_;
}

int SimulationBase::getN() const {
    return n_;
}

int SimulationBase::getTrials() const {
    return trials_;
}

bool SimulationBase::isWeightedBalls() const {
    return weighted_balls_;
}

double SimulationBase::getMaxWeight() const {
    return max_weight_;
}

unsigned int SimulationBase::getWorkloadSeed() const {
    return workload_seed_;
}

const std::vector<double>& SimulationBase::getBins() const {
    return bins_;
}

double SimulationBase::getTotalCost() const {
    return total_cost_;
}

double SimulationBase::getCostWeight(const std::string& key) const {
    const auto it = cost_weights_.find(key);
    if (it == cost_weights_.end()) {
        throw std::out_of_range("Unknown cost weight key: " + key);
    }

    return it->second;
}

std::vector<double> SimulationBase::previewBallWeights(int count, int trial_index) const {
    if (count < 0) {
        throw std::invalid_argument("Preview weight count must be non-negative.");
    }

    std::vector<double> weights(static_cast<std::size_t>(count), 1.0);

    if (!weighted_balls_) {
        return weights;
    }

    std::mt19937 preview_rng(workload_seed_ + static_cast<unsigned int>(trial_index));
    std::uniform_real_distribution<double> distribution(1.0, max_weight_);

    for (double& weight : weights) {
        weight = distribution(preview_rng);
    }

    return weights;
}

void SimulationBase::setCostWeight(const std::string& key, double value) {
    cost_weights_[key] = value;
}

void SimulationBase::reset() {
    std::fill(bins_.begin(), bins_.end(), 0.0);
    total_cost_ = 0.0;
}

void SimulationBase::run() {
    if (trials_ <= 0) {
        throw std::invalid_argument("Number of trials must be positive.");
    }

    std::vector<double> accumulated_bins(static_cast<std::size_t>(n_), 0.0);
    double accumulated_cost = 0.0;

    for (int trial = 0; trial < trials_; ++trial) {
        reset();
        workload_rng_.seed(workload_seed_ + static_cast<unsigned int>(trial));
        runSingleTrial();

        for (int bin = 0; bin < n_; ++bin) {
            accumulated_bins[static_cast<std::size_t>(bin)] +=
                bins_[static_cast<std::size_t>(bin)];
        }

        accumulated_cost += total_cost_;
    }

    const double trial_count = static_cast<double>(trials_);
    for (double& load : accumulated_bins) {
        load /= trial_count;
    }

    bins_ = accumulated_bins;
    total_cost_ = accumulated_cost / trial_count;
}

std::vector<int> SimulationBase::drawRandomBins(int k) {
    if (k <= 0) {
        throw std::invalid_argument("Number of random bins must be positive.");
    }

    if (k > n_) {
        throw std::invalid_argument("Number of random bins must be less than or equal to n.");
    }

    std::vector<int> candidates(static_cast<std::size_t>(n_));
    std::iota(candidates.begin(), candidates.end(), 0);

    for (int i = 0; i < k; ++i) {
        std::uniform_int_distribution<int> distribution(i, n_ - 1);
        const int swap_index = distribution(rng_);
        std::swap(candidates[static_cast<std::size_t>(i)],
                  candidates[static_cast<std::size_t>(swap_index)]);
    }

    total_cost_ += static_cast<double>(k) * cost_weights_["random_draw"];
    candidates.resize(static_cast<std::size_t>(k));

    return candidates;
}

double SimulationBase::drawBallWeight() {
    if (!weighted_balls_) {
        return 1.0;
    }

    std::uniform_real_distribution<double> distribution(1.0, max_weight_);
    return distribution(workload_rng_);
}

double SimulationBase::readBinLoad(int bin_index) {
    validateBinIndex(bin_index);
    total_cost_ += cost_weights_["load_read"];

    return bins_[static_cast<std::size_t>(bin_index)];
}

bool SimulationBase::lessThan(double a, double b) {
    total_cost_ += cost_weights_["compare"];
    return a < b;
}

void SimulationBase::addBallToBin(int bin_index, double weight) {
    validateBinIndex(bin_index);
    bins_[static_cast<std::size_t>(bin_index)] += weight;
}

void SimulationBase::validateBinIndex(int bin_index) const {
    if (bin_index < 0 || bin_index >= n_) {
        throw std::out_of_range("Bin index is out of range.");
    }
}

}  // namespace balls_bins
