CXX := g++

CXXVERSION := 17

CXXFLAGS := -Wall -Wextra -Wpedantic -Ofast -Os -static -funroll-loops -std=c++$(CXXVERSION)

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
	@$(CXX) test/test.cpp -O3 -funroll-loops -static -o test/test
	@./test/test

clean:
	@rm -rf $(BUILD_DIR) logs test/test

size:
	@wc -c $(BUILD_DIR)/$(TARGET)

SERVICE_NAME := cppserver
SERVICE_FILE := $(SERVICE_NAME).service
SERVICE_DEST := /etc/systemd/system/$(SERVICE_FILE)
SERVICE_DIR  := /etc/CppServer

SOURCE_LINK := https://github.com/f3fe-hash/CppServer2.git

#
# Install the server
#
install: $(BUILD_DIR)/$(TARGET)
	@sudo mkdir -p /etc/CppServer
	@sudo cp $(BUILD_DIR)/$(TARGET) /etc/CppServer/server
	@sudo mkdir -p /etc/CppServer/site
	@sudo cp -r site/* /etc/CppServer/site/

	@sudo cp $(SERVICE_FILE) $(SERVICE_DEST)
	@sudo systemctl daemon-reload
	@sudo systemctl enable $(SERVICE_NAME)
	@sudo systemctl start  $(SERVICE_NAME)
	@sudo systemctl status $(SERVICE_NAME)

# Create an alias to check the server's status
	@echo "alias status='sudo systemctl status $(SERVICE_NAME)'" > ~/.bash_aliases

# Then reboot
	@sudo reboot

#
# Uninstall the server
#
uninstall:
	@sudo systemctl stop    $(SERVICE_NAME)
	@sudo systemctl disable $(SERVICE_NAME)
	@sudo rm -rf $(SERVICE_DEST) $(SERVICE_DIR)
	@sudo systemctl daemon-reload

#
# Do a full re-install
#
reinstall-full:
# First uninstall service
	@sudo systemctl stop    $(SERVICE_NAME)
	@sudo systemctl disable $(SERVICE_NAME)
	@sudo rm -rf $(SERVICE_DEST) $(SERVICE_DIR)
	@sudo systemctl daemon-reload

# Remove existing source, clone fresh, rebuild and install
	@cd .. && \
	rm -rf CppServer2 && \
	git clone $(SOURCE_LINK) && \
	cd CppServer2 && \
	make install

#
# Do a basic re-compile and service restart
#
reinstall-basic: $(BUILD_DIR)/$(TARGET)
# First uninstall service
	@sudo systemctl stop    $(SERVICE_NAME)
	@sudo systemctl disable $(SERVICE_NAME)
	@sudo rm -rf $(SERVICE_DEST) $(SERVICE_DIR)
	@sudo systemctl daemon-reload

# Then reinstall service
	@sudo mkdir -p /etc/CppServer
	@sudo cp $(BUILD_DIR)/$(TARGET) /etc/CppServer/server
	@sudo mkdir -p /etc/CppServer/site
	@sudo cp -r site/* /etc/CppServer/site/

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
