#include "RandomSimulator.h"

namespace balls_bins {

RandomSimulator::RandomSimulator(int m, int n)
    : SimulationBase(m, n) {}

void RandomSimulator::run() {
    for (int ball = 0; ball < m_; ++ball) {
        const int bin_index = drawRandomBin();
        addBallToBin(bin_index);
    }
}

}  // namespace balls_bins
