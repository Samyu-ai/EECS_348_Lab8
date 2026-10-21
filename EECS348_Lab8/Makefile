CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic
TARGET = calculator

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o $(TARGET)

test: $(TARGET)
	./$(TARGET) tests.txt > actual_output.txt
	diff -u expected_output.txt actual_output.txt

clean:
	rm -f $(TARGET) actual_output.txt

.PHONY: all test clean
