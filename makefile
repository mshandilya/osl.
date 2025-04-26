CC = gcc
CCPP = g++
CFLAGS = -Wall
CPFLAGS = -std=c++20

BIN_DIR = bin
COSL_TARGET = $(BIN_DIR)/cosl
COSL_SRCS = cosl.cpp lexer.cpp parser.cpp utils.cpp ast.cpp resolver.cpp typechecker.cpp

ROSL_TARGET = $(BIN_DIR)/rosl
# ROSL_SRCS = rosl.c
ROSL_SRCS = legacyRosl.c

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

# added timeout of 10s if the test doesn't
test: $(COSL_TARGET)
	@echo "Running tests..."
	@find ./unitTests -type f -name "*.osl" | while read -r testfile; do \
		echo ""; \
		echo "Running $$testfile..."; \
		timeout 10 ./$(COSL_TARGET) "$$testfile" > actual_output.txt || { \
			echo "\033[31mTest $$testfile timed out.\033[0m"; \
			continue; \
		}; \
		if diff actual_output.txt expected_output.txt > /dev/null; then \
			echo "\033[32mTest $$testfile passed.\033[0m"; \
		else \
			echo "\033[31mTest $$testfile failed.\033[0m"; \
		fi; \
	done

clean:
	$(RM) $(COSL_TARGET) $(ROSL_TARGET)
	$(RM) -r $(BIN_DIR)