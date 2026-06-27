#ifndef BALLS_BINS_STATEFUL_ROUND_ROBIN_SIMULATOR_H
#define BALLS_BINS_STATEFUL_ROUND_ROBIN_SIMULATOR_H

#include "SimulationBase.h"

namespace balls_bins {

class StatefulRoundRobinSimulator : public SimulationBase {
public:
    StatefulRoundRobinSimulator(int m,
                                int n,
                                int trials = 1,
                                bool weighted_balls = false,
                                double max_weight = 1.0,
                                unsigned int workload_seed = 42);

    int getCurrentBin() const;

private:
    void runSingleTrial() override;

    int current_bin_;
};

}  // namespace balls_bins

#endif  // BALLS_BINS_STATEFUL_ROUND_ROBIN_SIMULATOR_H
