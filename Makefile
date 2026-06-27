CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic
TARGET := balls_bins_sim
SOURCES := SimulationMetrics.cpp SimulationReferences.cpp SimulationBase.cpp PowerKSimulator.cpp StatefulRoundRobinSimulator.cpp HeapSizeSPowerOfKSimulator.cpp main.cpp

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SOURCES) SimulationMetrics.h SimulationReferences.h SimulationBase.h PowerKSimulator.h StatefulRoundRobinSimulator.h HeapSizeSPowerOfKSimulator.h
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
