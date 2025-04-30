# The C++ compiler to use
CXX = g++

# Compiler flags:
# -Wall: enable all warnings
# -Wextra: enable even more warnings
# -std=c++17: use the C++17 standard
CXXFLAGS = -Wall -Wextra -std=c++17

# Linker flags:
# -lssl: link against the OpenSSL library
# -lcrypto: link against the OpenSSL crypto library
LDFLAGS = -lssl -lcrypto

# List of source files in the project
# (update this list if you add or rename source files)
SRC = Sudoku.cpp SudokuGraph.cpp Utils.cpp

# List of object files generated from source files
# (by replacing .cpp with .o)
OBJ = $(SRC:.cpp=.o)

# The final name of the executable program
TARGET = sudoku_zkp

# Default build target: what happens when you run 'make' with no arguments
all: $(TARGET)

# Rule to build the final executable:
# If any object files have changed, re-link them into the TARGET executable
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

# Pattern rule:
# How to compile .cpp files into .o (object) files
# $< is an automatic variable that stands for the source file
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

# Clean rule:
# Deletes object files and the executable, so you can rebuild from scratch
clean:
	rm -f $(OBJ) $(TARGET)
