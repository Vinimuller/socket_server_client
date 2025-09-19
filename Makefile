# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

# Output directory
OUT_DIR = out

# Targets
SERVER = $(OUT_DIR)/tcp_server
CLIENT = $(OUT_DIR)/tcp_client

# Source files
SERVER_SRC = tcp_server.cpp
CLIENT_SRC = tcp_client.cpp

# Default target
all: $(SERVER) $(CLIENT)

# Ensure output directory exists
$(OUT_DIR):
	mkdir -p $(OUT_DIR)

# Build server
$(SERVER): $(SERVER_SRC) | $(OUT_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

# Build client
$(CLIENT): $(CLIENT_SRC) | $(OUT_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

# Run server
run-server: $(SERVER)
	./$(SERVER)

# Run client
run-client: $(CLIENT)
	./$(CLIENT)

# Clean build artifacts
clean:
	rm -rf $(OUT_DIR)
