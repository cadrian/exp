#!/usr/bin/make -f

OBJ=$(shell ls -1 src/*.c | sed -r 's|^src/|target/out/|g;s|\.c|.o|g')

CFLAGS ?= -g
LDFLAGS ?=

all: exe
	@echo

clean:
	rm -rf target

exe: target target/exp

target:
	mkdir target

target/exp: $(OBJ)
	@echo "Compiling executable: $<"
	$(CC) $(CFLAGS) -g -o $@ -lpcre -lcad $(OBJ)

target/out/%.o: src/%.c src/*.h
	@echo "Compiling object: $<"
	$(CC) $(CFLAGS) -Wall -c $< -o $@

.PHONY: all clean
