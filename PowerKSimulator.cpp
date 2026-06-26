#include "PowerKSimulator.h"

#include <stdexcept>
#include <vector>

namespace balls_bins {

PowerKSimulator::PowerKSimulator(int m, int n, int k, int trials)
    : SimulationBase(m, n, trials),
      k_(k) {
    if (k_ <= 0) {
        throw std::invalid_argument("PowerKSimulator requires k to be positive.");
    }

    if (k_ > n_) {
        throw std::invalid_argument("PowerKSimulator requires k to be less than or equal to n.");
    }
}

int PowerKSimulator::getK() const {
    return k_;
}

void PowerKSimulator::runSingleTrial() {
    for (int ball = 0; ball < m_; ++ball) {
        const std::vector<int> candidates = drawRandomBins(k_);

        if (k_ == 1) {
            addBallToBin(candidates[0]);
            continue;
        }

        int best_bin = candidates[0];
        double best_load = readBinLoad(best_bin);

        for (int choice = 1; choice < k_; ++choice) {
            const int candidate_bin = candidates[static_cast<std::size_t>(choice)];
            const double candidate_load = readBinLoad(candidate_bin);

            if (lessThan(candidate_load, best_load)) {
                best_bin = candidate_bin;
                best_load = candidate_load;
            }
        }

        addBallToBin(best_bin);
    }
}

}  // namespace balls_bins
