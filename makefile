CXX			= g++
CXXFLAGS	= --std=c++14
SOURCES		= $(wildcard *.cpp)
ifeq ($(OS),Windows_NT)
EXE			= $(SOURCES:%.cpp=%.exe)
else
EXE			= $(SOURCES:%.cpp=%)
endif

.PHONY: all clean

all: $(EXE)

ifeq ($(OS),Windows_NT)
$(EXE): %.exe : %.cpp
	$(CXX) -Wall -Wextra $(CXXFLAGS) -o $@ $<
else
$(EXE): % : %.cpp
	$(CXX) -Wall -Wextra $(CXXFLAGS) -o $@ $<
endif

clean:
ifeq ($(OS),Windows_NT)
	del /f $(EXE)
else
	rm -f $(EXE)
endif