# OCR Makefile
NAME := gui
CC := gcc
OCR := `realpath ../ocr`
CFLAGS := `pkg-config --cflags gtk+-3.0` -I$(OCR)/include -Werror -Wall -Wextra -std=c99 -O3 # -g
LDFLAGS := -L$(OCR)/build `pkg-config --libs gtk+-3.0` -locr -lhunspell -lm -l pthread
C_SRC := $(shell find src/ -name *.c -printf "%P\n")
SRC := $(shell find -name *.c) $(shell find -name *.h)
SRC_DIR := src
BUILD_DIR := build
SANDBOX_DIR := sandbox
RESOURCES_DIR := resources
OBJ := $(patsubst %,$(BUILD_DIR)/$(SANDBOX_DIR)/%,$(C_SRC:.c=.o))
FMT := clang-format
FMT_FLAGS := -Werror -style=file

.PHONY: all compile mrproper clean

all: mrproper format compile

check: compile test-format

format:
	$(FMT) $(FMT_FLAGS) -i $(SRC)

test-format:
	$(FMT) $(FMT_FLAGS) --dry-run $(SRC)

compile: $(OBJ)
	cp -r $(RESOURCES_DIR) $(BUILD_DIR)
	$(CC) -o $(BUILD_DIR)/$(NAME) $(OBJ) $(LDFLAGS)

$(BUILD_DIR)/$(SANDBOX_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -c $^ -o $@ -Iinclude/ -lhunspell -lm -l pthread

mrproper:
	rm -rf $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)/$(SANDBOX_DIR)
