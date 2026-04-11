CXX?=g++
CXXFLAGS?=
DEFAULT_CXXFLAGS:=-std=c++17 -Werror -Wall
CXXFLAGS+=$(DEFAULT_CXXFLAGS)
CATCH2_CXXFLAGS:=-DCATCH_CONFIG_MAIN -DCATCH_CONFIG_FAST_COMPILE
CXXFLAGS+=$(CATCH2_CXXFLAGS)
INCLUDES:=-I.
NAME:=xdg_test

SOURCES=xdg_test.cpp
OBJ=$(SOURCES:.cpp=.o)

xdg_test: $(OBJ)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $(NAME)

test: xdg_test
	./xdg_test

.PHONY: clean
clean:
	rm -f $(OBJ) $(NAME)
