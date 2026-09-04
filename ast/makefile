CXX=g++
CXXFLAGS=-Wall -O2 -std=c++17

ast: ast.cpp
	$(CXX) $(CXXFLAGS) ast.cpp -o ast

clean:
	rm -f ast

run: ast
	./ast
