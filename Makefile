CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic
TARGET := balls_bins_sim
SOURCES := CsvWriter.cpp ExperimentRunner.cpp SimulationMetrics.cpp SimulationReferences.cpp SimulationBase.cpp PowerKSimulator.cpp StatefulRoundRobinSimulator.cpp HeapSizeSPowerOfKSimulator.cpp main.cpp

.PHONY: all run results plots all-results clean clean-results

all: $(TARGET)

$(TARGET): $(SOURCES) CsvWriter.h ExperimentRunner.h SimulationMetrics.h SimulationReferences.h SimulationBase.h PowerKSimulator.h StatefulRoundRobinSimulator.h HeapSizeSPowerOfKSimulator.h
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

results: $(TARGET)
	./$(TARGET)

results/simulation_results.csv: $(TARGET)
	./$(TARGET)

plots: results/simulation_results.csv
	python3 plot_results.py

all-results: $(TARGET)
	./$(TARGET)
	python3 plot_results.py

clean:
	rm -f $(TARGET)

clean-results:
	rm -rf results plots
