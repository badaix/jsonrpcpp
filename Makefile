BIN       = jsonrpctest

CXX       = clang++
STRIP     = strip
CXXFLAGS  = -std=c++0x -Wall -O3 -Isrc -isystem src/externals

OBJ       = jsonrpctest.o src/jsonrp.o


all: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(BIN) $(OBJ) $(LDFLAGS)
	$(STRIP) $(BIN)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BIN) $(OBJ) *~

