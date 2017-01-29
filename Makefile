BIN       = jsonrpctest

CXX       = clang++
STRIP     = strip
CXXFLAGS  = -std=c++0x -Wall -O3 -Ilib -isystem lib/externals

OBJ       = jsonrpctest.o lib/jsonrp.o


all: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(BIN) $(OBJ) $(LDFLAGS)
	$(STRIP) $(BIN)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BIN) $(OBJ) *~

