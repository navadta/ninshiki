# OCR Makefile
NAME := ocr
CC := gcc
CFLAGS := -Werror -Wall -Wextra -std=c99 -O3 # -g
LDFLAGS := -Iinclude/
C_SRC := $(shell find src/ -name *.c -printf "%P\n")
SRC := $(shell find -name *.c) $(shell find -name *.h)
SRC_DIR := src
BUILD_DIR := build
SANDBOX_DIR := sandbox
OBJ := $(patsubst %,$(BUILD_DIR)/$(SANDBOX_DIR)/%,$(C_SRC:.c=.o))
FMT := clang-format
FMT_FLAGS := -Werror -style=file
EXAMPLE_DIR := example
EXAMPLES := $(shell find $(EXAMPLE_DIR)/ -name *.c -printf "%P\n")

.PHONY: all example compile mrproper clean

all: mrproper format $(EXAMPLES)

check: $(EXAMPLES) # test-format

format:
	$(FMT) $(FMT_FLAGS) -i $(SRC)

test-format:
	$(FMT) $(FMT_FLAGS) --dry-run $(SRC)

compile: $(OBJ)
	ar -q $(BUILD_DIR)/lib$(NAME).a $^

$(BUILD_DIR)/$(SANDBOX_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(shell dirname $@)
	$(CC) -c $^ -o $@ $(CFLAGS) $(LDFLAGS)

$(EXAMPLES): compile
	mkdir -p $(BUILD_DIR)/$(EXAMPLE_DIR)
	$(CC) $(CFLAGS) `pkg-config --cflags gtk+-3.0` -o $(BUILD_DIR)/$(EXAMPLE_DIR)/$(@:.c=) $(EXAMPLE_DIR)/$@ $(LDFLAGS) -lm `pkg-config --libs gtk+-3.0` -L$(BUILD_DIR) -l$(NAME)

mrproper:
	rm -rf $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)/$(SANDBOX_DIR)
