TARGET = ./output/analyse ./output/create-sample
CXXFLAGS 	= -std=c++23 -O3 -Wall -Wextra -Werror -Wconversion -Wformat -Wformat=2 -Wimplicit-fallthrough -Wvla -m64 -march=native -mtune=native -flto


all: ./output ./output/analyse ./output/create-sample

./output/analyse : 1.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

./output/create-sample : create-sample.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -rf ./output/