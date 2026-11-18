CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic -Isrc
LDFLAGS ?=
LDLIBS ?= -lm
BUILD_DIR := build
TEST_BIN := $(BUILD_DIR)/test_flatten
SOURCES := src/flatten.c tests/test_flatten.c
HEADERS := src/flatten.h

.PHONY: all test clean

all: $(TEST_BIN)

$(TEST_BIN): $(SOURCES) $(HEADERS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(LDFLAGS) $(LDLIBS)

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -rf $(BUILD_DIR)
