CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2

SOURCES = $(wildcard *.C)

TARGETS = $(patsubst %.C,%,$(SOURCES))

all: $(TARGETS)

%: %.C
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean
