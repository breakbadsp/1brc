TARGET = ./bin/analyse ./bin/create-sample
CXXFLAGS 	= -std=c++23 -O3 -Wall -Wextra -Werror -Wconversion -Wformat -Wformat=2 -Wimplicit-fallthrough -Wvla -m64 -march=native -mtune=native -flto


all: ./bin ./bin/analyse ./bin/create-sample

./bin/analyse : 1.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

./bin/create-sample : create-sample.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -rf ./bin/