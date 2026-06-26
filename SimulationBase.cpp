#include "SimulationBase.h"

#include <algorithm>
#include <stdexcept>

namespace balls_bins {

SimulationBase::SimulationBase(int m, int n)
    : m_(m),
      n_(n),
      bins_(),
      cost_weights_{
          {"random_draw", 1.0},
          {"load_read", 5.0},
          {"compare", 1.0},
      },
      total_cost_(0.0),
      rng_(std::random_device{}()) {
    if (m_ < 0) {
        throw std::invalid_argument("Number of balls must be non-negative.");
    }

    if (n_ <= 0) {
        throw std::invalid_argument("Number of bins must be positive.");
    }

    bins_.assign(static_cast<std::size_t>(n_), 0.0);
}

int SimulationBase::getM() const {
    return m_;
}

int SimulationBase::getN() const {
    return n_;
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

void SimulationBase::setCostWeight(const std::string& key, double value) {
    cost_weights_[key] = value;
}

void SimulationBase::reset() {
    std::fill(bins_.begin(), bins_.end(), 0.0);
    total_cost_ = 0.0;
}

int SimulationBase::drawRandomBin() {
    total_cost_ += cost_weights_["random_draw"];

    std::uniform_int_distribution<int> distribution(0, n_ - 1);
    return distribution(rng_);
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

void SimulationBase::addBallToBin(int bin_index) {
    validateBinIndex(bin_index);
    bins_[static_cast<std::size_t>(bin_index)] += 1.0;
}

void SimulationBase::validateBinIndex(int bin_index) const {
    if (bin_index < 0 || bin_index >= n_) {
        throw std::out_of_range("Bin index is out of range.");
    }
}

}  // namespace balls_bins
