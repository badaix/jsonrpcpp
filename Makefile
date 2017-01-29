BIN       = jsonrpctest

CXX       = clang++
STRIP     = strip
CXXFLAGS  = -std=c++0x -Wall -O3 -Ijsonrpcpp -isystem jsonrpcpp/externals

OBJ       = jsonrpctest.o jsonrpcpp/jsonrp.o


all: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(BIN) $(OBJ) $(LDFLAGS)
	$(STRIP) $(BIN)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BIN) $(OBJ) *~

