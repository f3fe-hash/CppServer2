#!/bin/bash

# Exit on error
set -e

# Usage check
if [ "$#" -ne 4 ]; then
    echo "Usage: $0 <service_name> <service_dest> <service_dir> <source_link>"
    exit 1
fi

# Input arguments
SERVICE_NAME="$1"
SERVICE_DEST="$2"
SERVICE_DIR="$3"
SOURCE_LINK="$4"

# Stop and disable the service
echo "Stopping and disabling service: $SERVICE_NAME"
sudo systemctl stop "$SERVICE_NAME" || true
sudo systemctl disable "$SERVICE_NAME" || true

# Remove service files/directories
echo "Removing: $SERVICE_DEST and $SERVICE_DIR"
sudo rm -rf "$SERVICE_DEST" "$SERVICE_DIR"

# Reload systemd
echo "Reloading systemd daemon"
sudo systemctl daemon-reexec
sudo systemctl daemon-reload

# Go up one directory
cd ..

# Clean up old repo
echo "Removing old CppServer2 directory"
rm -rf CppServer2

# Clone new repo
echo "Cloning repository from $SOURCE_LINK"
git clone "$SOURCE_LINK"
cd CppServer2