CXX=g++
CXXFLAGS=-Wall -O2 -fno-tree-vectorize -march=native -std=c++17 -I$(HOME)/.local/include -I..
# Arrow's headers require C++20, so the loader gets its own standard. Nothing in
# parquet_loader.hpp leaks Arrow types, so the rest of the project stays C++17.
PARQUET_CXXFLAGS=-Wall -O2 -march=native -std=c++20 -I$(HOME)/.local/include
LDFLAGS=-L$(HOME)/.local/lib -Wl,-rpath,$(HOME)/.local/lib
LDLIBS=-lbenchmark -lpthread -lparquet -larrow

simd: main.o parquet_loader.o
	$(CXX) $(CXXFLAGS) main.o parquet_loader.o -o simd $(LDFLAGS) $(LDLIBS)

main.o: main.cpp ../parquet_loader.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

parquet_loader.o: ../parquet_loader.cpp ../parquet_loader.hpp
	$(CXX) $(PARQUET_CXXFLAGS) -c ../parquet_loader.cpp -o parquet_loader.o

clean:
	rm -f main.o parquet_loader.o simd

run: simd
	./simd
