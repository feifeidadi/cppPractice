CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2

TARGETS = singleton thread

all: $(TARGETS)

singleton: singleton.C
	$(CXX) $(CXXFLAGS) -o $@ $<

thread: thread.C
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean
