CXXFLAGS += $(ADD_CFLAGS) -std=c++0x -Wall -Wno-unused-function -O3 -I.
CXX       = /usr/bin/g++
STRIP     = strip
BIN       = jsonrpctest

OBJ       = jsonrpctest.o jsonrpc.o


all: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(BIN) $(OBJ) $(LDFLAGS)
	$(STRIP) $(BIN)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BIN) $(OBJ) *~

