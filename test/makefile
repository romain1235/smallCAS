include ../makefile

CXXFLAGS += -std=c++17 -Wall -Wextra $(SCAS_CXXFLAGS)

.PHONY: run clean

run: test
	./test

test: test.cpp $(SCAS_SRCS)
	$(CXX) $(CXXFLAGS) -o test test.cpp $(SCAS_SRCS)

clean:
	rm -f test
