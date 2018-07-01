BIN       = jsonrpcpp_example

CXX       = clang++
STRIP     = strip
CXXFLAGS  = -std=c++11 -Wall -O3 -Iinclude -pedantic -Wextra -Wshadow -Wconversion

OBJ       = jsonrpcpp_example.o


all: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(BIN) $(OBJ) $(LDFLAGS)
	$(STRIP) $(BIN)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BIN) $(OBJ) *~

