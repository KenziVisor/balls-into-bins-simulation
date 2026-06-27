#include "HeapSizeSPowerOfKSimulator.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace balls_bins {

HeapSizeSPowerOfKSimulator::HeapSizeSPowerOfKSimulator(int m,
                                                       int n,
                                                       int s,
                                                       int k,
                                                       int trials,
                                                       bool weighted_balls,
                                                       double max_weight,
                                                       unsigned int workload_seed)
    : SimulationBase(m, n, trials, weighted_balls, max_weight, workload_seed),
      heap_(),
      is_tracked_(),
      s_(s),
      k_(k) {
    if (s_ < 1 || s_ > n_) {
        throw std::invalid_argument("HeapSizeSPowerOfKSimulator requires 1 <= s <= n.");
    }

    if (s_ == n_) {
        if (k_ < 0) {
            throw std::invalid_argument("HeapSizeSPowerOfKSimulator requires k to be non-negative.");
        }
    } else if (k_ < 1 || k_ > n_ - s_) {
        throw std::invalid_argument(
            "HeapSizeSPowerOfKSimulator requires 1 <= k <= n - s when s < n.");
    }
}

int HeapSizeSPowerOfKSimulator::getHeapSize() const {
    return s_;
}

int HeapSizeSPowerOfKSimulator::getK() const {
    return k_;
}

void HeapSizeSPowerOfKSimulator::runSingleTrial() {
    initializeTrialState();

    for (int ball = 0; ball < m_; ++ball) {
        const double ball_weight = drawBallWeight();

        if (s_ < n_) {
            maybeAdmitUntrackedBin();
        }

        total_cost_ += cost_weights_["state_read"];
        const int min_bin = heap_.front().bin_index;
        addBallToBin(min_bin, ball_weight);

        heap_.front().load += ball_weight;
        total_cost_ += cost_weights_["state_update"];
        reorderHeap();
    }
}

void HeapSizeSPowerOfKSimulator::initializeTrialState() {
    heap_.clear();
    is_tracked_.assign(static_cast<std::size_t>(n_), false);

    for (int bin = 0; bin < s_; ++bin) {
        heap_.push_back({0.0, bin});
        is_tracked_[static_cast<std::size_t>(bin)] = true;
    }

    std::make_heap(heap_.begin(), heap_.end(), heapEntryIsWorse);
    total_cost_ += static_cast<double>(s_) * cost_weights_["state_memory_per_slot"];
}

void HeapSizeSPowerOfKSimulator::maybeAdmitUntrackedBin() {
    std::vector<int> candidates;

    if (k_ == n_ - s_) {
        for (int bin = 0; bin < n_; ++bin) {
            if (!is_tracked_[static_cast<std::size_t>(bin)]) {
                candidates.push_back(bin);
            }
        }
    } else {
        candidates = sampleUntrackedBins(k_);
    }

    int best_bin = candidates[0];
    double best_load = readBinLoad(best_bin);

    for (std::size_t i = 1; i < candidates.size(); ++i) {
        const int candidate_bin = candidates[i];
        const double candidate_load = readBinLoad(candidate_bin);

        if (isBetterMinCandidate(candidate_load, candidate_bin, best_load, best_bin)) {
            best_bin = candidate_bin;
            best_load = candidate_load;
        }
    }

    const int worst_index = findHeaviestTrackedEntryIndex();
    const HeapEntry& worst_entry = heap_[static_cast<std::size_t>(worst_index)];

    total_cost_ += cost_weights_["compare"];
    const bool should_admit = best_load <= worst_entry.load;

    if (!should_admit) {
        return;
    }

    is_tracked_[static_cast<std::size_t>(worst_entry.bin_index)] = false;
    is_tracked_[static_cast<std::size_t>(best_bin)] = true;
    heap_[static_cast<std::size_t>(worst_index)] = {best_load, best_bin};

    total_cost_ += cost_weights_["state_update"];
    reorderHeap();
}

std::vector<int> HeapSizeSPowerOfKSimulator::sampleUntrackedBins(int count) {
    std::vector<int> untracked_bins;

    for (int bin = 0; bin < n_; ++bin) {
        if (!is_tracked_[static_cast<std::size_t>(bin)]) {
            untracked_bins.push_back(bin);
        }
    }

    for (int i = 0; i < count; ++i) {
        std::uniform_int_distribution<int> distribution(
            i, static_cast<int>(untracked_bins.size()) - 1);
        const int swap_index = distribution(rng_);
        std::swap(untracked_bins[static_cast<std::size_t>(i)],
                  untracked_bins[static_cast<std::size_t>(swap_index)]);
    }

    total_cost_ += static_cast<double>(count) * cost_weights_["random_draw"];
    untracked_bins.resize(static_cast<std::size_t>(count));

    return untracked_bins;
}

bool HeapSizeSPowerOfKSimulator::isBetterMinCandidate(double candidate_load,
                                                      int candidate_bin,
                                                      double best_load,
                                                      int best_bin) {
    total_cost_ += cost_weights_["compare"];

    if (candidate_load != best_load) {
        return candidate_load < best_load;
    }

    return candidate_bin < best_bin;
}

bool HeapSizeSPowerOfKSimulator::isWorseTrackedEntry(const HeapEntry& candidate,
                                                     const HeapEntry& worst) {
    total_cost_ += cost_weights_["compare"];

    if (candidate.load != worst.load) {
        return candidate.load > worst.load;
    }

    return candidate.bin_index > worst.bin_index;
}

int HeapSizeSPowerOfKSimulator::findHeaviestTrackedEntryIndex() {
    int worst_index = 0;

    for (std::size_t i = 0; i < heap_.size(); ++i) {
        total_cost_ += cost_weights_["state_read"];

        if (i == 0) {
            continue;
        }

        if (isWorseTrackedEntry(heap_[i], heap_[static_cast<std::size_t>(worst_index)])) {
            worst_index = static_cast<int>(i);
        }
    }

    return worst_index;
}

void HeapSizeSPowerOfKSimulator::reorderHeap() {
    std::make_heap(heap_.begin(), heap_.end(), heapEntryIsWorse);
    total_cost_ += static_cast<double>(heapUpdateLevels()) *
                   cost_weights_["heap_update_per_level"];
}

int HeapSizeSPowerOfKSimulator::heapUpdateLevels() const {
    if (s_ <= 1) {
        return 1;
    }

    return static_cast<int>(std::ceil(std::log2(static_cast<double>(s_ + 1))));
}

bool HeapSizeSPowerOfKSimulator::heapEntryIsWorse(const HeapEntry& left,
                                                  const HeapEntry& right) {
    if (left.load != right.load) {
        return left.load > right.load;
    }

    return left.bin_index > right.bin_index;
}

}  // namespace balls_bins
