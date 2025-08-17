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

.PHONY: all run test clean size

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
	@rm -rf $(BUILD_DIR) logs test

size:
	@wc -c $(BUILD_DIR)/$(TARGET)

SERVICE_NAME := cppserver
SERVICE_FILE := $(SERVICE_NAME).service
SERVICE_DEST := /etc/systemd/system/$(SERVICE_FILE)

# Install the server
install: $(BUILD_DIR)/$(TARGET)
	@sudo mkdir -p /etc/CppServer
	@sudo cp $(BUILD_DIR)/$(TARGET) /etc/CppServer/server
	@sudo cp -r site/ /etc/CppServer/

	@sudo cp $(SERVICE_FILE) $(SERVICE_DEST)
	@sudo systemctl daemon-reload
	@sudo systemctl enable $(SERVICE_NAME)
	@sudo systemctl start  $(SERVICE_NAME)
	@sudo systemctl status $(SERVICE_NAME)

# Create an alias to check the server's status
	@echo "alias status='sudo systemctl status $(SERVICE_NAME)'" > ~/.bash_aliases

# Then reboot
	@sudo reboot

# Uninstall the server
uninstall:
	@sudo systemctl stop    $(SERVICE_NAME)
	@sudo systemctl disable $(SERVICE_NAME)
	@sudo rm -rf $(SERVICE_DEST)
	@sudo systemctl daemon-reload

reinstall: $(BUILD_DIR)/$(TARGET)
# First uninstall
	@sudo systemctl stop    $(SERVICE_NAME)
	@sudo systemctl disable $(SERVICE_NAME)
	@sudo rm -rf $(SERVICE_DEST)
	@sudo systemctl daemon-reload

# Then reinstall
	@sudo mkdir -p /etc/CppServer
	@sudo cp $(BUILD_DIR)/$(TARGET) /etc/CppServer/server

	@sudo cp $(SERVICE_FILE) $(SERVICE_DEST)
	@sudo systemctl daemon-reload
	@sudo systemctl enable $(SERVICE_NAME)
	@sudo systemctl start  $(SERVICE_NAME)
	@sudo systemctl status $(SERVICE_NAME)

# Finally reboot
	@sudo reboot

restart:
	@sudo systemctl restart $(SERVICE_NAME)
	@sudo systemctl status  $(SERVICE_NAME)

# Check the status of the server
status:
	@sudo systemctl status $(SERVICE_NAME)
	@echo "-------------------------------------"
	@cat /etc/CppServer/logs/log.txt
