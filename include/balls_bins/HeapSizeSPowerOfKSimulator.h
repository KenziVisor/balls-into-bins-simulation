#ifndef BALLS_BINS_HEAP_SIZE_S_POWER_OF_K_SIMULATOR_H
#define BALLS_BINS_HEAP_SIZE_S_POWER_OF_K_SIMULATOR_H

#include "balls_bins/SimulationBase.h"

#include <vector>

namespace balls_bins {

class HeapSizeSPowerOfKSimulator : public SimulationBase {
public:
    HeapSizeSPowerOfKSimulator(int m,
                               int n,
                               int s,
                               int k,
                               int trials = 1,
                               bool weighted_balls = false,
                               double max_weight = 1.0,
                               unsigned int workload_seed = 42,
                               unsigned int allocation_seed = 1337);

    int getHeapSize() const;
    int getK() const;

private:
    struct HeapEntry {
        double load;
        int bin_index;
    };

    void runSingleTrial() override;

    void initializeTrialState();
    void maybeAdmitUntrackedBin();
    std::vector<int> sampleUntrackedBins(int count);
    bool isBetterMinCandidate(double candidate_load,
                              int candidate_bin,
                              double best_load,
                              int best_bin);
    bool isWorseTrackedEntry(const HeapEntry& candidate, const HeapEntry& worst);
    int findHeaviestTrackedEntryIndex();
    void removeFromUntracked(int bin);
    void addToUntracked(int bin);
    void updateHeapPositions();
    void reorderHeap();
    int heapUpdateLevels() const;
    static bool heapEntryIsWorse(const HeapEntry& left, const HeapEntry& right);

    std::vector<HeapEntry> heap_;
    std::vector<int> untracked_bins_;
    std::vector<int> heap_positions_;
    std::vector<int> untracked_positions_;
    int s_;
    int k_;
};

}  // namespace balls_bins

#endif  // BALLS_BINS_HEAP_SIZE_S_POWER_OF_K_SIMULATOR_H
