# Makefile

# Compiler settings - Can change these to your preferred compiler and options
CXX = g++
CXXFLAGS = -Wall -Iinclude -std=c++20

# Target library
TARGET = libminiperf.a

# Source files
SOURCES = src/mini_perf.cpp
# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET)

$(TARGET): $(OBJECTS)
	ar rcs $@ $^

# To obtain object files
%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

# To remove generated files
clean:
	rm -f $(OBJECTS) $(TARGET)