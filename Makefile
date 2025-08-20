# ---------------------------
# General config
# ---------------------------

IP   := $(shell ip route get 1.1.1.1 | grep -oP 'src \K[0-9.]+')
PORT := 8080

SOURCE_LINK  := https://github.com/f3fe-hash/CppServer2.git

# ---------------------------
# HTTPS
# ---------------------------

KEY  = keys/key.pem
CERT = keys/cert.pem

ifeq ($(https),1)
    HTTPS_FLAGS += -DUSE_HTTPS
    HTTPS_FLAGS += -DKEY=\"$(KEY)\"
    HTTPS_FLAGS += -DCERT=\"$(CERT)\"
    HTTPS_ENABLED := 1
else
    HTTPS_ENABLED := 0
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

SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.cpp.o)
DIRS := $(sort $(dir $(OBJS)))

# ---------------------------
# Colors
# ---------------------------

RED    := \033[1;31m
YELLOW := \033[1;33m
GREEN  := \033[1;32m
BLUE   := \033[1;34m
RESET  := \033[0m

# ---------------------------
# Main Targets
# ---------------------------

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJS) | $(DIRS)
	@printf "$(BLUE)  LD\tLinking files...$(RESET)"
	@$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) $(OBJS) -o $@ $(LIBS)
	@printf "$(BLUE)Done$(RESET)\n"
ifeq ($(HTTPS_ENABLED),0)
	@printf "$(RED)  WARN\tHTTPS is disabled.$(RESET)\n"
else
	@printf "$(YELLOW)  NOTE\tHTTPS is enabled.$(RESET)\n"
endif

$(BUILD_DIR)/%.cpp.o: $(SRC_DIR)/%.cpp | $(DIRS)
	@mkdir -p $(dir $@)
	@printf "$(GREEN)  CXX\tBuilding file $@...$(RESET)"
	@$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@
	@printf "$(GREEN)Done\n$(RESET)"

$(DIRS):
	@mkdir -p $(DIRS)

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

todo:
	@echo "1) git clone $(SOURCE_LINK)"
	@echo "2) cd CppServer2"
	@echo "3) make key-gen"
	@echo "4) make reinstall https=1"
	@echo "5) sudo reboot"

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
			-subj "/C=$(COUNTRY)/ST=$(STATE)/L=$(CITY)/O=Organization/OU=Unit/CN=$(IP)"; \
		echo "Done: $(KEY) and $(CERT) generated."; \
	fi

force-key-gen:
	@echo "Generating self-signed certificate..."
	@mkdir -p $(dir $(KEY))
	@openssl req -x509 -newkey rsa:4096 -sha256 -days 365 \
		-keyout $(KEY) -out $(CERT) -nodes \
		-subj "/C=$(COUNTRY)/ST=$(STATE)/L=$(CITY)/O=Organization/OU=Unit/CN=$(IP)"
	@echo "Done: $(KEY) and $(CERT) generated."

# ---------------------------
# Paths & Service Setup
# ---------------------------

SERVICE_NAME := cppserver
SERVICE_FILE := install/$(SERVICE_NAME).service
SERVICE_DEST := /etc/systemd/system/$(SERVICE_NAME).service
SERVICE_DIR  := /etc/CppServer

# ---------------------------
# Install
# ---------------------------

install: $(BUILD_DIR)/$(TARGET)
	@echo "Installing $(SERVICE_NAME)..."
	@sudo install -d $(SERVICE_DIR)/site
	@sudo install -D $< $(SERVICE_DIR)/server
	@sudo cp -r site/* $(SERVICE_DIR)/site/

ifeq ($(HTTPS_ENABLED),1)
	@echo "Copying HTTPS certificates..."
	@sudo install -d $(SERVICE_DIR)/keys
	@sudo install -m 600 $(KEY) $(SERVICE_DIR)/keys/key.pem
	@sudo install -m 644 $(CERT) $(SERVICE_DIR)/keys/cert.pem
endif

	@echo "Generating systemd service file..."
	@sed "s|__IP__|$(IP)|g; s|__PORT__|$(PORT)|g" install/$(SERVICE_NAME).service.tmp > install/$(SERVICE_NAME).service

	@echo "Setting up systemd service..."
	@sudo install -D $(SERVICE_FILE) $(SERVICE_DEST)
	@sudo systemctl daemon-reload
	@sudo systemctl enable --now $(SERVICE_NAME)

	@echo "Installation complete."

# ---------------------------
# Uninstall
# ---------------------------

uninstall:
	@echo "Uninstalling $(SERVICE_NAME)..."
	@sudo systemctl stop $(SERVICE_NAME) || true
	@sudo systemctl disable $(SERVICE_NAME) || true
	@sudo rm -f $(SERVICE_DEST)
	@sudo rm -rf $(SERVICE_DIR)
	@sudo systemctl daemon-reload
	@echo "Uninstallation complete."

# ---------------------------
# Reinstall
# ---------------------------

reinstall: uninstall install

restart:
	@sudo systemctl restart $(SERVICE_NAME)
	@sudo systemctl status  $(SERVICE_NAME)

status:
	@sudo systemctl status $(SERVICE_NAME)
	@echo "-------------------------------------"
	@tail -n 20 /etc/CppServer/logs/log.log || true

.PHONY: all run test clean deep-clean key-gen force-key-gen \
        size install uninstall reinstall restart status todo show
