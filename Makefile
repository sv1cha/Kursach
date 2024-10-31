CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -I/usr/include/openssl -I/usr/local/include
LDFLAGS = -lcryptopp -lboost_program_options -L/usr/lib -L/usr/local/lib

SRCS = main.cpp Calculator.cpp Client_Communicate.cpp Connector_to_base.cpp Interface.cpp Logger.cpp
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)
TARGET = Server

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

# Цель для запуска сервера
run: $(TARGET)
	./$(TARGET)

# Цель для запуска тестов (если они у вас есть)
test: $(TARGET)
	./$(TARGET) --test


