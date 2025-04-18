CC = gcc
CCPP = g++
CFLAGS = -Wall
CPFLAGS = -std=c++20

BIN_DIR = bin
COSL_TARGET = $(BIN_DIR)/cosl
COSL_SRCS = cosl.cpp lexer.cpp parser.cpp utils.cpp ast.cpp resolver.cpp
# later add hmap and rtrim srcs too!

ROSL_TARGET = $(BIN_DIR)/rosl
ROSL_SRCS = rosl.c

ifeq ($(OS),Windows_NT)
    EXE_EXT = .exe
    RM = del /Q
else
    EXE_EXT =
    RM = rm -f
endif

COSL_TARGET := $(COSL_TARGET)$(EXE_EXT)
ROSL_TARGET := $(ROSL_TARGET)$(EXE_EXT)

all: $(BIN_DIR) $(COSL_TARGET) $(ROSL_TARGET)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(COSL_TARGET): $(COSL_SRCS) | $(BIN_DIR)
	$(CCPP) $(CPFLAGS) -o $(COSL_TARGET) $(COSL_SRCS)

$(ROSL_TARGET): $(ROSL_SRCS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(ROSL_TARGET) $(ROSL_SRCS)

# added timeout of 5s if the test doesn't
test: $(COSL_TARGET)
	@echo "Running tests..."
	@find ./unitTests -type f -name "*.osl" | while read -r testfile; do \
		echo "Running $$testfile..."; \
		timeout 5 ./$(COSL_TARGET) "$$testfile" || echo "Test $$testfile timed out."; \
	done

clean:
	$(RM) $(COSL_TARGET) $(ROSL_TARGET)
	$(RM) -r $(BIN_DIR)