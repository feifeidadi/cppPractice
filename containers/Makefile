CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2

TARGETS = osiSymbolParser str2map

all: $(TARGETS)

osiSymbolParser: osiSymbolParser.C
	$(CXX) $(CXXFLAGS) -o $@ $<

str2map: str2map.C
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean
