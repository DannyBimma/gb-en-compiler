# C to British English Compiler - Makefile
# For Linux and macOS builds

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -O2
SRC_DIR = src
BUILD_DIR = build
TARGET = c2en

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Default target
all: $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	@echo ""
	@echo "Build successful! Compiler executable: ./$(TARGET)"
	@echo "Usage: ./$(TARGET) <input.c> [options]"
	@echo ""

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	@echo "Cleaned build artifacts"

# Install to system (requires sudo)
install: $(TARGET)
	install -d /usr/local/bin
	install -m 755 $(TARGET) /usr/local/bin/
	@echo "Installed to /usr/local/bin/$(TARGET)"

# Uninstall from system (requires sudo)
uninstall:
	rm -f /usr/local/bin/$(TARGET)
	@echo "Uninstalled $(TARGET)"

# Run with example
test: $(TARGET)
	@echo "Building test example..."
	@mkdir -p examples
	./$(TARGET) examples/hello.c -v
	@echo ""
	@echo "Test output:"
	@cat examples/hello.txt

# Display help
help:
	@echo "C to British English Compiler - Build System"
	@echo ""
	@echo "Targets:"
	@echo "  make           - Build the compiler"
	@echo "  make clean     - Remove build artifacts"
	@echo "  make install   - Install to /usr/local/bin (requires sudo)"
	@echo "  make uninstall - Remove from /usr/local/bin (requires sudo)"
	@echo "  make test      - Build and run test example"
	@echo "  make help      - Display this help message"
	@echo ""

.PHONY: all clean install uninstall test help
