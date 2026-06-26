CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic
TARGET := balls_bins_sim
SOURCES := SimulationBase.cpp PowerKSimulator.cpp main.cpp

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SOURCES) SimulationBase.h PowerKSimulator.h
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
