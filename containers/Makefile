CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2
LDFLAGS = 

SOURCES = $(wildcard *.C)

TARGETS = $(patsubst %.C,%,$(SOURCES))

all: $(TARGETS)

# Test target with Google Test
gtest: CXXFLAGS += -D__GOOGLE_TEST__
gtest: LDFLAGS += -lgtest -lgtest_main -pthread
gtest: clean $(TARGETS)

%: %.C
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean
