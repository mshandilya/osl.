CC = gcc
CCPP = g++
CFLAGS = -Wall -Werror

COSL_TARGET = cosl.exe
COSL_SRCS = cosl.cpp lexer.cpp parser.cpp utils.cpp ast.cpp

ROSL_TARGET = rosl.exe
ROSL_SRCS = rosl.c

all: $(COSL_TARGET) $(ROSL_TARGET)

$(COSL_TARGET): $(COSL_SRCS)
	$(CCPP) $(CFLAGS) -o $(COSL_TARGET) $(COSL_SRCS)

$(ROSL_TARGET): $(ROSL_SRCS)
	$(CC) $(CFLAGS) -o $(ROSL_TARGET) $(ROSL_SRCS)

clean:
	MSYS_NO_PATHCONV=1 MSYS2_ARG_CONV_EXCL="*" rm -f $(COSL_TARGET) $(ROSL_TARGET)