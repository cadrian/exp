#!/usr/bin/make -f

OBJ=$(shell { echo src/_exp_entry_registry.c ; ls -1 src/*.c ; } | sed -r 's|^src/|target/out/|g;s|\.c|.o|g' | sort -u)

ifeq "$(wildcard ../libcad)" ""
LIBCADINCLUDE=
else
LIBCADINCLUDE=-I ../libcad/include
endif

CFLAGS ?= -g
LDFLAGS ?=

all: exe
	@echo

clean:
	rm -rf target
	rm -f src/_exp_entry_registry.c

exe: target/exp

target:
	mkdir -p target

target/out: target
	mkdir -p target/out

target/exp: $(OBJ) libcad
	@echo "Compiling executable: $<"
	$(CC) $(CFLAGS) -o $@ $(OBJ) -L target -lpcre -lcad

target/out/%.o: src/%.c src/*.h target/out
	@echo "Compiling object: $<"
	$(CC) $(CFLAGS) -I src $(LIBCADINCLUDE) -Wall -c $< -o $@

src/_exp_entry_registry.c: src/exp_*_entry.c
	@echo "Generating factory registry: $<"
	utils/generate_factory_registry.sh

ifeq "$(wildcard ../libcad)" ""
libcad:
	@echo "libcad sources not found, using pre-install library"
else
ifeq "$(wildcard /etc/setup/setup.rc)" ""
libcad: target/libcad.so
else
libcad: target/cygcad.dll
endif

target/libcad.so: target
	cd ../libcad && make lib
	cp ../libcad/target/libcad.so* target/

target/cygcad.dll: target/libcad.dll.a
	cp ../libcad/target/cygcad.dll target/

target/libcad.dll.a: target
	cd ../libcad && make lib
	cp ../libcad/target/libcad.dll.a target/
endif

.PHONY: all clean libcad
