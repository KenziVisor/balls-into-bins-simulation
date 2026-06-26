#ifndef BALLS_BINS_RANDOM_SIMULATOR_H
#define BALLS_BINS_RANDOM_SIMULATOR_H

#include "SimulationBase.h"

namespace balls_bins {

class RandomSimulator : public SimulationBase {
public:
    RandomSimulator(int m, int n);

    void run() override;
};

}  // namespace balls_bins

#endif  // BALLS_BINS_RANDOM_SIMULATOR_H
