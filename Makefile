
OUTDIR = bin
TARGET = analyse create-sample analyse_uo_map
CXXFLAGS 	= -g -std=c++23 -O3 -Wall -Wextra -Werror #-Wconversion 
CXXFLAGS += -Wno-implicit-int-float-conversion
CXXFLAGS += -Wformat -Wformat=2 -Wimplicit-fallthrough 
CXXFLAGS += -Wvla -m64 -march=native -mtune=native 
CXXFLAGS += -flto 
#CXXFLAGS += -fsanitize=address,undefined,leak

HEADERS := $(wildcard *.h, wildcard *.hpp)

all: $(TARGET)

OBJECTS = $(patsubst %.cpp, $(OUTDIR)/%.o, $(wildcard *.cpp))
HEADERS := $(wildcard *.h, wildcard *.hpp)

#$(OUTDIR)/%.o:  %.cpp $(HEADERS)
#	$(CXX) $(CXXFLAGS) -o $@ -c $< 

#.PRECIOUS: $(TARGET) $(OBJECTS)

analyse:	2.cpp
	$(CXX) $<  $(LDDFLAGS) $(CXXFLAGS) -o $(OUTDIR)/$@

create-sample:	create-sample.cpp
	$(CXX) $< $(LDDFLAGS) $(CXXFLAGS) -o $(OUTDIR)/$@

analyse_uo_map:	2_unordered_map.cpp
	$(CXX) $<  $(LDDFLAGS) $(CXXFLAGS) -o $(OUTDIR)/$@

clean:
	rm $(OBJECTS)
	rm $(OUTDIR)/$(TARGET)
