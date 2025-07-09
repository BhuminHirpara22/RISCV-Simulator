# Compiler and flags
compiler = g++
FLAGS = -std=c++17

# Target and source files
TARGET = riscv_sim
SRCS = main.cpp simulator.cpp

# Default target
all: $(TARGET)

# Compile and link
$(TARGET): $(SRCS)
	$(compiler) $(FLAGS) -o $@ $^

# Clean up build files
clean:
	rm -f $(TARGET)

