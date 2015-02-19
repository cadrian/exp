#!/usr/bin/make -f

OBJ=$(shell { echo src/_exp_entry_registry.c ; ls -1 src/exp*.c ; } | sed -r 's|^src/|target/out/|g;s|\.c|.o|g')

ifeq "$(wildcard ../libcad)" ""
LIBCAD=
LIBCADINCLUDE=
LIBCADCLEAN=
else
ifeq "$(wildcard /etc/setup/setup.rc)" ""
LIBCAD=target/libcad.so
else
LIBCAD=target/cygcad.dll
endif
LIBCADINCLUDE=-I ../libcad/include
LIBCADCLEAN=libcadclean
endif

CFLAGS ?= -g
LDFLAGS ?=

all: exe
	@echo

clean: $(LIBCADCLEAN)
	rm -rf target
	rm -f src/_exp_entry_registry.c

exe: target/exp

target/exp: $(OBJ) $(LIBCAD)
	@echo "Compiling executable: $@"
	$(CC) $(CFLAGS) -o $@ $(OBJ) -L target -lpcre -lcad

target/out/%.o: src/%.c src/*.h Makefile
	mkdir -p target/out
	@echo "Compiling object: $<"
	$(CC) $(CFLAGS) -I src $(LIBCADINCLUDE) -Wall -c $< -o $@

src/_exp_entry_registry.c: src/exp_*_entry.c utils/generate_factory_registry.sh
	@echo "Generating factory registry: $<"
	utils/generate_factory_registry.sh

libcadclean:
	cd ../libcad && make clean

target/libcad.so:
	mkdir -p target
	cd ../libcad && make lib
	cp ../libcad/target/libcad.so* target/

target/cygcad.dll: target/libcad.dll.a
	mkdir -p target
	cp ../libcad/target/cygcad.dll target/

target/libcad.dll.a:
	mkdir -p target
	cd ../libcad && make lib
	cp ../libcad/target/libcad.dll.a target/

.PHONY: all clean libcadclean
