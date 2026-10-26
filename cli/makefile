include ../makefile

CXXFLAGS += -std=c++17 -Wall -Wextra $(SCAS_CXXFLAGS)

.PHONY: run clean

run: cli
	./cli

cli: main.cpp $(SCAS_SRCS)
	$(CXX) $(CXXFLAGS) -o cli main.cpp $(SCAS_SRCS)

clean:
	rm -f cli
