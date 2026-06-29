#include "balls_bins/SimulationReferences.h"

#include <cmath>
#include <stdexcept>
#include <vector>

namespace balls_bins {

namespace {

void validateReferenceInputs(int balls, int bins) {
    if (balls < 0) {
        throw std::invalid_argument("Number of balls must be non-negative.");
    }

    if (bins <= 0) {
        throw std::invalid_argument("Number of bins must be positive.");
    }
}

}  // namespace

double computeUnweightedOptimalMaxLoad(int balls, int bins) {
    validateReferenceInputs(balls, bins);

    return std::ceil(static_cast<double>(balls) / static_cast<double>(bins));
}

double computeUnweightedOptimalCvLoad(int balls, int bins) {
    validateReferenceInputs(balls, bins);

    if (balls == 0) {
        return 0.0;
    }

    const int floor_load = balls / bins;
    const int remainder = balls % bins;
    const int ceil_load = floor_load + (remainder == 0 ? 0 : 1);
    const double average_load = static_cast<double>(balls) / static_cast<double>(bins);
    double squared_difference_sum = 0.0;

    for (int bin = 0; bin < bins; ++bin) {
        const double load = bin < remainder ? static_cast<double>(ceil_load)
                                            : static_cast<double>(floor_load);
        const double difference = load - average_load;
        squared_difference_sum += difference * difference;
    }

    const double stddev =
        std::sqrt(squared_difference_sum / static_cast<double>(bins));
    return average_load == 0.0 ? 0.0 : stddev / average_load;
}

ReferenceMetrics computeReferenceMetrics(int balls,
                                         int bins,
                                         bool weighted_balls,
                                         double average_load,
                                         bool random_initialization_enabled) {
    validateReferenceInputs(balls, bins);

    if (!weighted_balls && !random_initialization_enabled) {
        return {ReferenceType::UnweightedOptimal,
                computeUnweightedOptimalMaxLoad(balls, bins),
                computeUnweightedOptimalCvLoad(balls, bins)};
    }

    // With weighted arrivals or synthetic initial loads, average load is only
    // a lower bound on max load. True optimum is intentionally not computed.
    return {weighted_balls ? ReferenceType::WeightedAverageLowerBound
                           : ReferenceType::AverageLoadLowerBound,
            average_load,
            0.0};
}

std::string referenceTypeToString(ReferenceType type) {
    switch (type) {
        case ReferenceType::None:
            return "none";
        case ReferenceType::UnweightedOptimal:
            return "unweighted_optimal";
        case ReferenceType::AverageLoadLowerBound:
            return "average_load_lower_bound";
        case ReferenceType::WeightedAverageLowerBound:
            return "weighted_average_lower_bound";
    }

    return "none";
}

}  // namespace balls_bins
