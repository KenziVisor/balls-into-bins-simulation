CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -Iinclude
TARGET := build/balls_bins_sim
SOURCES := src/CsvWriter.cpp src/ExperimentRunner.cpp src/SimulationMetrics.cpp src/SimulationReferences.cpp src/SimulationBase.cpp src/PowerKSimulator.cpp src/StatefulRoundRobinSimulator.cpp src/HeapSizeSPowerOfKSimulator.cpp src/main.cpp
HEADERS := include/balls_bins/CsvWriter.h include/balls_bins/ExperimentRunner.h include/balls_bins/SimulationMetrics.h include/balls_bins/SimulationReferences.h include/balls_bins/SimulationBase.h include/balls_bins/PowerKSimulator.h include/balls_bins/StatefulRoundRobinSimulator.h include/balls_bins/HeapSizeSPowerOfKSimulator.h

.PHONY: all run results plots all-results clean clean-results

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

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
	rm -rf build

clean-results:
	rm -rf results plots
