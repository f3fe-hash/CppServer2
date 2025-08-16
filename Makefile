CXX := g++

DEBUG = #-g

CXXVERSION := 11

CXXFLAGS := -Wall -Wextra -Wpedantic -O3 -Os -funroll-loops -std=c++$(CXXVERSION)

BUILD_DIR   := build
INCLUDE_DIR := include
SRC_DIR     := src

TARGET := server

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.cpp.o,$(SRCS))

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJS) $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) $(OBJS) -o $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/%.cpp.o: $(SRC_DIR)/%.cpp $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) $(DEBUG) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

run:
	@clear
	@./$(BUILD_DIR)/$(TARGET)

test:
	@$(CXX) $(CXXFLAGS) test.cpp -o test
	@clear
	@./test

clean:
	@rm -rf $(BUILD_DIR) test

size:
	@wc -c $(BUILD_DIR)/$(TARGET)