CC 			= gcc
CFLAGS 		= -Wall -Wextra -Iinclude

SRC_DIR 	= src
BUILD_DIR 	= build
BIN_DIR 	= bin

SRC_FILES 	= $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES 	= $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC_FILES))
TARGET 		= $(BIN_DIR)/main

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	$(CC) $(OBJ_FILES) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all clean
