BIN       = jsonrpctest

CXX       = g++
STRIP     = strip
CXXFLAGS  = -std=c++0x -Wall -O3 -I.

OBJ       = jsonrpctest.o jsonrpc.o


all: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(BIN) $(OBJ) $(LDFLAGS)
	$(STRIP) $(BIN)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BIN) $(OBJ) *~

