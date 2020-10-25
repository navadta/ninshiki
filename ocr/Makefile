# OCR Makefile
NAME := ocr
CC := gcc
CFLAGS := -Wall -Wextra -std=c99 -O3
LDFLAGS := -Iinclude/ -lm
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

check: compile test-format

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
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$(EXAMPLE_DIR)/$(@:.c=) $(EXAMPLE_DIR)/$@ $(LDFLAGS) -L$(BUILD_DIR) -l$(NAME)

mrproper:
	rm -rf $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)/$(SANDBOX_DIR)
