CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
LDFLAGS = -lreadline

TARGET = main.out
SRCS = main.cpp utils.cpp history.cpp autocomplete.cpp redirection.cpp builtins.cpp process.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

rebuild: clean all
