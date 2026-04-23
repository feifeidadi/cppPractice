# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
CXXFLAGS_DEBUG = $(CXXFLAGS) -g -DDEBUG

# Target executables
TARGET = ouchParser
TARGET_DEBUG = ouchParserDebug

# Source files
SOURCES = main.C ouchParser.C hexDump.C

# Header files
HEADERS = ouchProtocol.H ouchParser.H hexDump.H 

# Object files
OBJECTS = $(SOURCES:.C=.o)
OBJECTS_DEBUG = $(SOURCES:.C=.debug.o)

# Default target (build both)
all: $(TARGET) $(TARGET_DEBUG)

# Normal build
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

# Debug build
$(TARGET_DEBUG): $(OBJECTS_DEBUG)
	$(CXX) $(CXXFLAGS_DEBUG) -o $(TARGET_DEBUG) $(OBJECTS_DEBUG)

# Compile normal object files
%.o: %.C $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile debug object files
%.debug.o: %.C $(HEADERS)
	$(CXX) $(CXXFLAGS_DEBUG) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(OBJECTS_DEBUG) $(TARGET) $(TARGET_DEBUG)

# Phony targets
.PHONY: all clean
