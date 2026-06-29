CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -Iinclude
TARGET := build/balls_bins_sim
SINGLE_TARGET := build/single_runner
COMMON_SOURCES := src/CsvWriter.cpp src/ExperimentRunner.cpp src/SimulationMetrics.cpp src/SimulationReferences.cpp src/SimulationBase.cpp src/PowerKSimulator.cpp src/StatefulRoundRobinSimulator.cpp src/HeapSizeSPowerOfKSimulator.cpp
SOURCES := $(COMMON_SOURCES) src/main.cpp
SINGLE_SOURCES := $(COMMON_SOURCES) src/SingleRunner.cpp
HEADERS := include/balls_bins/CsvWriter.h include/balls_bins/ExperimentRunner.h include/balls_bins/SimulationMetrics.h include/balls_bins/SimulationReferences.h include/balls_bins/SimulationBase.h include/balls_bins/PowerKSimulator.h include/balls_bins/StatefulRoundRobinSimulator.h include/balls_bins/HeapSizeSPowerOfKSimulator.h

.PHONY: all run single results plots all-results clean clean-results

all: $(TARGET) $(SINGLE_TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

$(SINGLE_TARGET): $(SINGLE_SOURCES) $(HEADERS)
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(SINGLE_SOURCES) -o $(SINGLE_TARGET)

run: $(TARGET)
	./$(TARGET)

single: $(SINGLE_TARGET)
	./$(SINGLE_TARGET)

results: $(TARGET)
	./$(TARGET)

results/simulation_results.csv: $(TARGET)
	./$(TARGET)

plots: results/simulation_results.csv
	python3 scripts/plot_results.py

all-results: $(TARGET)
	./$(TARGET)
	python3 scripts/plot_results.py

clean:
	rm -f $(TARGET) $(SINGLE_TARGET)
	rm -rf build

clean-results:
	rm -rf results plots
