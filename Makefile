BIN       = jsonrpcpp_example

CXX       = clang++
STRIP     = strip
CXXFLAGS  = -std=c++11 -Wall -O3 -Iinclude -pedantic -Wextra -Wshadow -Wconversion

OBJ       = example/jsonrpcpp_example.o

reformat:
	clang-format -i include/jsonrpcpp.hpp
	clang-format -i example/jsonrpcpp_example.cpp
	clang-format -i test/test_main.cpp

all: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(BIN) $(OBJ) $(LDFLAGS)
	$(STRIP) $(BIN)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BIN) $(OBJ) *~

