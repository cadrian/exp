#!/usr/bin/make -f

OBJ=$(shell ls -1 src/*.c | sed -r 's|^src/|target/out/|g;s|\.c|.o|g')

LIBCAD=$(shell if [ -r ../libcad ]; then echo ../libcad ; else echo /usr/share/libcad ; fi)
CFLAGS ?= -g
LDFLAGS ?=

all: exe
	@echo

clean:
	rm -rf target

exe: target/exp

target:
	mkdir -p target

target/out: target
	mkdir -p target/out

target/exp: $(OBJ) libcad
	@echo "Compiling executable: $<"
	$(CC) $(CFLAGS) -g -o $@ $(OBJ) -L target -lpcre -lcad

target/out/%.o: src/%.c src/*.h target/out
	@echo "Compiling object: $<"
	$(CC) $(CFLAGS) -I src -I $(LIBCAD)/include -Wall -c $< -o $@

ifneq "$(wildcard /cygdrive)" ""
libcad: target/libcad.so
else
libcad: target/cygcad.dll
endif

target/libcad.so: target
	cd $(LIBCAD) && make lib
	cp $(LIBCAD)/target/libcad.so* target/

target/cygcad.dll: target/libcad.dll.a
	cp $(LIBCAD)/target/cygcad.dll target/

target/libcad.dll.a: target
	cd $(LIBCAD) && make lib
	cp $(LIBCAD)/target/libcad.dll.a target/

.PHONY: all clean
