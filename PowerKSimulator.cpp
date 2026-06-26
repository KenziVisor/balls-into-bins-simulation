#include "PowerKSimulator.h"

#include <stdexcept>

namespace balls_bins {

PowerKSimulator::PowerKSimulator(int m, int n, int k)
    : SimulationBase(m, n),
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

void PowerKSimulator::run() {
    for (int ball = 0; ball < m_; ++ball) {
        int best_bin = drawRandomBin();
        double best_load = readBinLoad(best_bin);

        for (int choice = 1; choice < k_; ++choice) {
            const int candidate_bin = drawRandomBin();
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
