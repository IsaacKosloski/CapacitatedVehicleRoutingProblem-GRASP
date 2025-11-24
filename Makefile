CXX = g++
CXXFLAGS = -Wall -std=c++20 -g -O3 -march=native -mtune=native

TARGET = bin/cvrp_solver
# All relevant .ccp files
SRC_FILES = src/main_exe.cpp src/Node.cpp src/Component.cpp src/Vehicle.cpp src/Scanner.cpp src/CVRP.cpp src/Solution.cpp  src/Functions.cpp src/Solver.cpp
# Replace the .cpp extension with .o for object files
OBJ_FILES = $(patsubst src/%.cpp,obj/%.o,$(SRC_FILES))

# The 'all' rule is the default goal. It depends on the final executable.
all: $(TARGET)

# Rule to link the object files into the final executable
$(TARGET): $(OBJ_FILES)
	@mkdir -p $(@D) # Create the bin directory if it doesn't exist
	$(CXX) $(CXXFLAGS) $^ -o $@

# Rule to compile a .cpp file from src/ into an .o file in obj/
obj/%.o: src/%.cpp
	@mkdir -p $(@D) # Create the obj directory if it doesn't exist
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to clean up object files and the executable
clean:
	rm -rf obj $(TARGET)

.PHONY: all clean
