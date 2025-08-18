# ---------------------------
# General config
# ---------------------------

IP   := 192.168.1.39
PORT := 8080

# ---------------------------
# HTTPS
# ---------------------------

KEY  = keys/key.pem
CERT = keys/cert.pem

# Enable HTTPS via: make https=1
ifeq ($(https),1)
    HTTPS_FLAGS += -DUSE_HTTPS
    HTTPS_FLAGS += -DKEY=\"$(KEY)\"
    HTTPS_FLAGS += -DCERT=\"$(CERT)\"
endif

# ---------------------------
# Compiler and Flags
# ---------------------------

CXX         := g++
CXXVERSION  := 17
CXXFLAGS    := -Wall -Wextra -Wpedantic -Ofast -Os -funroll-loops -std=c++$(CXXVERSION) -static-libstdc++ -static-libgcc $(HTTPS_FLAGS)
LIBS        := -lssl -lcrypto

# ---------------------------
# Directories
# ---------------------------

SRC_DIR     := src
INCLUDE_DIR := include
BUILD_DIR   := build
TARGET      := server

SRCS        := $(wildcard $(SRC_DIR)/*.cpp)
OBJS        := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.cpp.o,$(SRCS))

# ---------------------------
# Main Targets
# ---------------------------

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) $(OBJS) -o $@ $(LIBS)

$(BUILD_DIR)/%.cpp.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# ---------------------------
# Convenience Targets
# ---------------------------

run:
	@clear
	@./$(BUILD_DIR)/$(TARGET) $(IP) $(PORT)

test:
	@$(CXX) test/test.cpp -O3 -funroll-loops -static -o test/test
	@./test/test

clean:
	@rm -rf $(BUILD_DIR) logs test/test

deep-clean:
	@rm -rf $(BUILD_DIR) logs test/test $(KEY) $(CERT)

size:
	@wc -c $(BUILD_DIR)/$(TARGET)

show:
	@printf "\033[?25h"

# ---------------------------
# Generate HTTPS Certificates
# ---------------------------

COUNTRY := US
STATE   := Maryland
CITY    := City

key-gen:
	@if [ -f $(KEY) ] && [ -f $(CERT) ]; then \
		echo "Keys already exist at $(KEY) and $(CERT). Skipping."; \
	else \
		echo "Generating self-signed certificate..."; \
		mkdir -p $(dir $(KEY)); \
		openssl req -x509 -newkey rsa:4096 -sha256 -days 365 \
			-keyout $(KEY) -out $(CERT) -nodes \
			-subj "/C=$(COUNTRY)/ST=$(STATE)/L=$(CITY)/O=Organization/OU=Unit/CN=$(IP)";
		echo "Done: $(KEY) and $(CERT) generated."; \
	fi


force-key-gen:
	@echo "Generating self-signed certificate..."
	@mkdir -p $(dir $(KEY))
	@openssl req -x509 -newkey rsa:4096 -sha256 -days 365 \
		-keyout $(KEY) -out $(CERT) -nodes \
		-subj "/C=$(COUNTRY)/ST=$(STATE)/L=$(CITY)/O=Organization/OU=Unit/CN=$(IP)";
	@echo "Done: $(KEY) and $(CERT) generated."

# ---------------------------
# Service Installation
# ---------------------------

SERVICE_NAME := cppserver
SERVICE_FILE := install/$(SERVICE_NAME).service
SERVICE_DEST := /etc/systemd/system/$(SERVICE_NAME).service
SERVICE_DIR  := /etc/CppServer
SOURCE_LINK  := https://github.com/f3fe-hash/CppServer2.git

install: $(BUILD_DIR)/$(TARGET)
	@sudo mkdir -p $(SERVICE_DIR)
	@sudo cp $< $(SERVICE_DIR)/server
	@sudo mkdir -p $(SERVICE_DIR)/site
	@sudo cp -r site/* $(SERVICE_DIR)/site/

	@if [ "$(https)" = "1" ]; then \
		echo "🔐 Copying HTTPS certificate and key..."; \
		sudo mkdir -p $(SERVICE_DIR)/keys; \
		sudo cp $(KEY) $(SERVICE_DIR)/keys/key.pem; \
		sudo cp $(CERT) $(SERVICE_DIR)/keys/cert.pem; \
	fi

	@sudo cp $(SERVICE_FILE) $(SERVICE_DEST)
	@sudo systemctl daemon-reload
	@sudo systemctl enable $(SERVICE_NAME)
	@sudo systemctl start  $(SERVICE_NAME)
	@sudo systemctl status $(SERVICE_NAME)

	@echo "alias status='sudo systemctl status $(SERVICE_NAME)'" > ~/.bash_aliases
	@sudo reboot

uninstall:
	@sudo systemctl stop    $(SERVICE_NAME)
	@sudo systemctl disable $(SERVICE_NAME)
	@sudo rm -rf $(SERVICE_DEST) $(SERVICE_DIR)
	@sudo systemctl daemon-reload

update:
	@chmod +x install/update.sh
	@./install/update.sh \
	$(SERVICE_NAME) $(SERVICE_DEST) \
	$(SERVICE_DIR) $(SOURCE_LINK)

reinstall: $(BUILD_DIR)/$(TARGET)
	@sudo systemctl stop    $(SERVICE_NAME)
	@sudo systemctl disable $(SERVICE_NAME)
	@sudo rm -rf $(SERVICE_DEST) $(SERVICE_DIR)
	@sudo systemctl daemon-reload

	@sudo mkdir -p $(SERVICE_DIR)
	@sudo cp $< $(SERVICE_DIR)/server
	@sudo mkdir -p $(SERVICE_DIR)/site
	@sudo cp -r site/* $(SERVICE_DIR)/site/

	ifeq ($(https),1)
		@echo "🔐 Copying HTTPS certificate and key..."
		@sudo mkdir -p $(SERVICE_DIR)/keys
		@sudo cp $(KEY) $(SERVICE_DIR)/keys/key.pem
		@sudo cp $(CERT) $(SERVICE_DIR)/keys/cert.pem
	endif

	@sudo cp $(SERVICE_FILE) $(SERVICE_DEST)
	@sudo systemctl daemon-reload
	@sudo systemctl enable $(SERVICE_NAME)
	@sudo systemctl start  $(SERVICE_NAME)
	@sudo systemctl status $(SERVICE_NAME)

	@sudo reboot

restart:
	@sudo systemctl restart $(SERVICE_NAME)
	@sudo systemctl status  $(SERVICE_NAME)

status:
	@sudo systemctl status $(SERVICE_NAME)
	@echo "-------------------------------------"
	@cat /etc/CppServer/logs/log.txt | tail -n 20

.PHONY: all run test clean deep-clean gen-keys force-gen-keys \
        size install uninstall reinstall-full reinstall-basic restart status
