#include "balls_bins/StatefulRoundRobinSimulator.h"

namespace balls_bins {

StatefulRoundRobinSimulator::StatefulRoundRobinSimulator(int m,
                                                         int n,
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
      current_bin_(0) {
}

int StatefulRoundRobinSimulator::getCurrentBin() const {
    return current_bin_;
}

void StatefulRoundRobinSimulator::runSingleTrial() {
    current_bin_ = 0;
    addStateMemoryCost(1.0);

    for (int ball = 0; ball < m_; ++ball) {
        const double ball_weight = drawBallWeight();

        addStateReadCost(1.0);
        addBallToBin(current_bin_, ball_weight);

        current_bin_ = (current_bin_ + 1) % n_;
        addStateUpdateCost(1.0);
    }
}

}  // namespace balls_bins
