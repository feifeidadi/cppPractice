CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2

TARGETS = osiSymbolParser str2map maxProfit list

all: $(TARGETS)

list: list.C
	$(CXX) $(CXXFLAGS) -o $@ $<

maxProfit: maxProfit.C
	$(CXX) $(CXXFLAGS) -o $@ $<

osiSymbolParser: osiSymbolParser.C
	$(CXX) $(CXXFLAGS) -o $@ $<

str2map: str2map.C
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean
