WARNINGS    = -Wall -Wextra -Wpedantic -Werror \
              -Wcast-align -Wformat=2 -Wlogical-op \
              -Wmissing-include-dirs -Wnested-externs \
              -Wdisabled-optimization -Wunsafe-loop-optimizations \
              -Wfree-nonheap-object

NOWARN      = -w
CC          = gcc
INCDIRS     = include src deps
CPPFLAGS    = $(addprefix -I,$(INCDIRS))
CFLAGS      = -O2 -std=c89 -D_POSIX_C_SOURCE=200809L $(WARNINGS)
LDFLAGS     = -lfl
EXEC        = puer
SRC         = src
BUILD_DIR   = build

USER_CS     = $(filter-out $(SRC)/parser.tab.c $(SRC)/lexer.yy.c,$(wildcard $(SRC)/*.c))

.PHONY: all debug clean FORCE
all: $(EXEC)

debug: CFLAGS += -g -O0
debug: all

FORCE:

$(EXEC): FORCE $(SRC)/parser.y $(SRC)/lexer.l $(USER_CS)
	mkdir -p $(BUILD_DIR)

	bison -v -d -o $(SRC)/parser.tab.c $(SRC)/parser.y
	flex    -o $(SRC)/lexer.yy.c  $(SRC)/lexer.l

	$(CC) $(NOWARN) $(CPPFLAGS) -c $(SRC)/parser.tab.c -o $(BUILD_DIR)/parser.tab.o
	$(CC) $(NOWARN) $(CPPFLAGS) -c $(SRC)/lexer.yy.c  -o $(BUILD_DIR)/lexer.yy.o

	@for src in $(USER_CS); do \
	  obj=$(BUILD_DIR)/$$(basename $$src .c).o; \
	  $(CC) $(CFLAGS) $(CPPFLAGS) -c $$src -o $$obj; \
	done

	$(CC) $(BUILD_DIR)/*.o $(LDFLAGS) -o $(EXEC)

	rm -f \
	  $(SRC)/parser.tab.c \
	  $(SRC)/parser.tab.h \
	  $(SRC)/lexer.yy.c

clean:
	rm -rf $(EXEC) $(BUILD_DIR) $(SRC)/parser.tab.* $(SRC)/lexer.yy.c

