WARNINGS = -Wall -Wextra -Wpedantic -Werror \
           -Wcast-align -Wformat=2 -Wlogical-op \
           -Wmissing-include-dirs -Wnested-externs \
           -Wdisabled-optimization -Wunsafe-loop-optimizations \
           -Wfree-nonheap-object

CC = gcc
# CFLAGS = -std=c89 -D_POSIX_C_SOURCE=200809L -Iinclude $(WARNINGS)
CFLAGS = -std=c89 -D_POSIX_C_SOURCE=200809L -Iinclude
LDFLAGS = -lfl
EXEC = build/puer

SRC = src
SRC_FILES = $(SRC)/*.c

.PHONY: all build clean run

all: build

build: $(EXEC)

$(EXEC): $(SRC_FILES)
	bison -d -o $(SRC)/parser.tab.c $(SRC)/parser.y
	flex -o $(SRC)/lexer.yy.c $(SRC)/lexer.l
	$(CC) $(CFLAGS) $(SRC_FILES) $(LDFLAGS) -o $(EXEC)

run:
	./$(EXEC)

clean:
	rm -f $(EXEC) $(SRC)/parser.tab.* $(SRC)/lexer.yy.c
