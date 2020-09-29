# OCR Makefile
NAME = ocr
CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS := -Iinclude/
SRC = $(shell find src/ -name *.c -printf "%P\n")
SRC_DIR = src
BUILD_DIR = build
SANDBOX = sandbox
OBJ= $(patsubst %,$(BUILD_DIR)/$(SANDBOX)/%,$(SRC:.c=.o))
FORMATTER = clang-format
FORMATTER_FLAGS = -Werror
EXAMPLE_DIR = example

.PHONY: all example compile mrproper clean

all: mrproper format example

check: compile test-format

format:
	$(FORMATTER) $(FORMATTER_FLAGS) -i $(patsubst %,$(SRC_DIR)/%,$(SRC))

test-format:
	$(FORMATTER) $(FORMATTER_FLAGS) --dry-run $(SRC_DIR)/$(SRC)

compile: $(OBJ)
	ar -q $(BUILD_DIR)/lib$(NAME).a $^

$(BUILD_DIR)/$(SANDBOX)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(shell dirname $@)
	$(CC) -c $^ -o $@ $(CFLAGS) $(LDFLAGS)

example: compile
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ $(shell find $(EXAMPLE_DIR)/ -name *.c) -I$(EXAMPLE_DIR)/ $(LDFLAGS) -L$(BUILD_DIR) -l$(NAME)

mrproper:
	rm -rf $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)/$(SANDBOX)