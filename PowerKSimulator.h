#ifndef BALLS_BINS_POWER_K_SIMULATOR_H
#define BALLS_BINS_POWER_K_SIMULATOR_H

#include "SimulationBase.h"

namespace balls_bins {

class PowerKSimulator : public SimulationBase {
public:
    PowerKSimulator(int m,
                    int n,
                    int k,
                    int trials = 1,
                    bool weighted_balls = false,
                    double max_weight = 1.0,
                    unsigned int workload_seed = 42);

    int getK() const;

private:
    void runSingleTrial() override;

    int k_;
};

}  // namespace balls_bins

#endif  // BALLS_BINS_POWER_K_SIMULATOR_H
