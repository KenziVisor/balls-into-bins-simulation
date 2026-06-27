#ifndef BALLS_BINS_SIMULATION_REFERENCES_H
#define BALLS_BINS_SIMULATION_REFERENCES_H

#include <string>

namespace balls_bins {

enum class ReferenceType {
    None,
    UnweightedOptimal,
    WeightedAverageLowerBound
};

struct ReferenceMetrics {
    ReferenceType type = ReferenceType::None;
    double max_load = 0.0;
    double cv_load = 0.0;
};

double computeUnweightedOptimalMaxLoad(int balls, int bins);
double computeUnweightedOptimalCvLoad(int balls, int bins);

ReferenceMetrics computeReferenceMetrics(int balls,
                                         int bins,
                                         bool weighted_balls,
                                         double average_load);

std::string referenceTypeToString(ReferenceType type);

}  // namespace balls_bins

#endif  // BALLS_BINS_SIMULATION_REFERENCES_H
