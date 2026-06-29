#include "balls_bins/HeapSizeSPowerOfKSimulator.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace balls_bins {

HeapSizeSPowerOfKSimulator::HeapSizeSPowerOfKSimulator(int m,
                                                       int n,
                                                       int s,
                                                       int k,
                                                       int trials,
                                                       bool weighted_balls,
                                                       double max_weight,
                                                       unsigned int workload_seed,
                                                       unsigned int allocation_seed,
                                                       bool random_initialization_enabled,
                                                       int max_initial_load)
    : SimulationBase(m,
                     n,
                     trials,
                     weighted_balls,
                     max_weight,
                     workload_seed,
                     allocation_seed,
                     random_initialization_enabled,
                     max_initial_load),
      heap_(),
      untracked_bins_(),
      heap_positions_(),
      untracked_positions_(),
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

        addStateReadCost(1.0);
        const int min_bin = heap_.front().bin_index;
        addBallToBin(min_bin, ball_weight);

        heap_.front().load += ball_weight;
        addStateUpdateCost(1.0);
        repairHeapFromRoot();
    }
}

void HeapSizeSPowerOfKSimulator::initializeTrialState() {
    heap_.clear();
    untracked_bins_.clear();
    heap_positions_.assign(static_cast<std::size_t>(n_), -1);
    untracked_positions_.assign(static_cast<std::size_t>(n_), -1);

    for (int bin = 0; bin < s_; ++bin) {
        heap_.push_back({bins_[static_cast<std::size_t>(bin)], bin});
        heap_positions_[static_cast<std::size_t>(bin)] = bin;
    }

    for (int bin = s_; bin < n_; ++bin) {
        untracked_positions_[static_cast<std::size_t>(bin)] =
            static_cast<int>(untracked_bins_.size());
        untracked_bins_.push_back(bin);
    }

    std::make_heap(heap_.begin(), heap_.end(), heapEntryIsWorse);
    updateHeapPositions();
    addStateMemoryCost(static_cast<double>(s_));
}

void HeapSizeSPowerOfKSimulator::maybeAdmitUntrackedBin() {
    std::vector<int> candidates;

    if (k_ == n_ - s_) {
        candidates = untracked_bins_;
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

    const std::size_t insertion_index = heap_.size();
    heap_.push_back({best_load, best_bin});
    std::push_heap(heap_.begin(), heap_.end(), heapEntryIsWorse);

    const HeapEntry discarded_entry = heap_.back();
    heap_.pop_back();
    updateHeapPositionsAlongInsertionPath(insertion_index);

    addHeapUpdateCost(static_cast<double>(heapUpdateLevels()));

    if (discarded_entry.bin_index == best_bin) {
        heap_positions_[static_cast<std::size_t>(best_bin)] = -1;
        return;
    }

    removeFromUntracked(best_bin);
    addToUntracked(discarded_entry.bin_index);

    heap_positions_[static_cast<std::size_t>(discarded_entry.bin_index)] = -1;
    addStateUpdateCost(1.0);
}

std::vector<int> HeapSizeSPowerOfKSimulator::sampleUntrackedBins(int count) {
    std::vector<int> sampled_bins;
    std::unordered_set<int> sampled_positions;

    while (static_cast<int>(sampled_bins.size()) < count) {
        std::uniform_int_distribution<int> distribution(
            0, static_cast<int>(untracked_bins_.size()) - 1);
        const int position = distribution(rng_);

        if (sampled_positions.insert(position).second) {
            sampled_bins.push_back(untracked_bins_[static_cast<std::size_t>(position)]);
        }
    }

    addRandomDrawCost(static_cast<double>(count));
    return sampled_bins;
}

bool HeapSizeSPowerOfKSimulator::isBetterMinCandidate(double candidate_load,
                                                      int candidate_bin,
                                                      double best_load,
                                                      int best_bin) {
    addCompareCost(1.0);

    if (candidate_load != best_load) {
        return candidate_load < best_load;
    }

    return candidate_bin < best_bin;
}

void HeapSizeSPowerOfKSimulator::removeFromUntracked(int bin) {
    const int position = untracked_positions_[static_cast<std::size_t>(bin)];
    const int last_bin = untracked_bins_.back();

    untracked_bins_[static_cast<std::size_t>(position)] = last_bin;
    untracked_positions_[static_cast<std::size_t>(last_bin)] = position;
    untracked_bins_.pop_back();
    untracked_positions_[static_cast<std::size_t>(bin)] = -1;
}

void HeapSizeSPowerOfKSimulator::addToUntracked(int bin) {
    untracked_positions_[static_cast<std::size_t>(bin)] =
        static_cast<int>(untracked_bins_.size());
    untracked_bins_.push_back(bin);
}

void HeapSizeSPowerOfKSimulator::updateHeapPositions() {
    std::fill(heap_positions_.begin(), heap_positions_.end(), -1);

    for (std::size_t i = 0; i < heap_.size(); ++i) {
        heap_positions_[static_cast<std::size_t>(heap_[i].bin_index)] =
            static_cast<int>(i);
    }
}

void HeapSizeSPowerOfKSimulator::updateHeapPositionsAlongInsertionPath(
    std::size_t insertion_index) {
    while (true) {
        if (insertion_index < heap_.size()) {
            heap_positions_[static_cast<std::size_t>(heap_[insertion_index].bin_index)] =
                static_cast<int>(insertion_index);
        }

        if (insertion_index == 0) {
            break;
        }

        insertion_index = (insertion_index - 1) / 2;
    }
}

void HeapSizeSPowerOfKSimulator::repairHeapFromRoot() {
    std::size_t index = 0;

    while (true) {
        const std::size_t left_child = 2 * index + 1;
        const std::size_t right_child = left_child + 1;

        if (left_child >= heap_.size()) {
            break;
        }

        std::size_t best_child = left_child;
        if (right_child < heap_.size() &&
            heapEntryIsWorse(heap_[best_child], heap_[right_child])) {
            best_child = right_child;
        }

        if (!heapEntryIsWorse(heap_[index], heap_[best_child])) {
            break;
        }

        std::swap(heap_[index], heap_[best_child]);
        heap_positions_[static_cast<std::size_t>(heap_[index].bin_index)] =
            static_cast<int>(index);
        heap_positions_[static_cast<std::size_t>(heap_[best_child].bin_index)] =
            static_cast<int>(best_child);

        index = best_child;
    }

    addHeapUpdateCost(static_cast<double>(heapUpdateLevels()));
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
