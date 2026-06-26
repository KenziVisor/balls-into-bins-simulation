#ifndef BALLS_BINS_POWER_K_SIMULATOR_H
#define BALLS_BINS_POWER_K_SIMULATOR_H

#include "SimulationBase.h"

namespace balls_bins {

class PowerKSimulator : public SimulationBase {
public:
    PowerKSimulator(int m, int n, int k);

    int getK() const;
    void run() override;

private:
    int k_;
};

}  // namespace balls_bins

#endif  // BALLS_BINS_POWER_K_SIMULATOR_H
