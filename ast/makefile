CXX = g++
CXXFLAGS = -Wall -O2 -std=c++17

# Source files and target binaries
SRCS = ast.cpp SimpleSQLParser.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = ast

.PHONY: all clean run

all: $(TARGET)

# Link object files into final executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

# Compile individual .cpp files into .o object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)