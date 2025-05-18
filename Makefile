WARNINGS = -Wall -Wextra -Wpedantic -Werror \
           -Wcast-align -Wformat=2 -Wlogical-op \
           -Wmissing-include-dirs -Wnested-externs \
           -Wdisabled-optimization -Wunsafe-loop-optimizations \
           -Wfree-nonheap-object

CC = gcc
# CFLAGS = -std=c89 -D_POSIX_C_SOURCE=200809L -Iinclude $(WARNINGS)
INCDIRS = include src
CPPFLAGS = $(addprefix -I,$(INCDIRS))
CFLAGS = -g -O0 -std=c89 -D_POSIX_C_SOURCE=200809L
LDFLAGS = -lfl
EXEC = puer

SRC = src
SRC_FILES = $(SRC)/*.c

.PHONY: all build clean run

all: clean build

build: $(EXEC)

$(EXEC): $(SRC_FILES)
	bison -v -d -o $(SRC)/parser.tab.c $(SRC)/parser.y
	flex -o $(SRC)/lexer.yy.c $(SRC)/lexer.l
	$(CC) $(CFLAGS) $(CPPFLAGS) $(SRC_FILES) $(LDFLAGS) -o $(EXEC)
	rm -f $(SRC)/parser.tab.* $(SRC)/lexer.yy.c

run:
	./$(EXEC)

clean:
	rm -f $(EXEC) $(SRC)/parser.tab.* $(SRC)/lexer.yy.c $(SRC)/parser.output $(SRC)/*.o
