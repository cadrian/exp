#!/usr/bin/make -f

PROJECT=exp
OBJ=$(shell { echo src/_exp_entry_registry.c ; ls -1 src/exp*.c ; } | sed -r 's|^src/|target/out/|g;s|\.c|.o|g')

ifeq "$(wildcard ../libcad)" ""
LIBCAD=
LIBCADINCLUDE=
LIBCADCLEAN=
GENDOC=/usr/share/libcad/gendoc.sh
else
ifeq "$(wildcard /etc/setup/setup.rc)" ""
LIBCAD=target/libcad.so
else
LIBCAD=target/cygcad.dll
endif
LIBCADINCLUDE=-I ../libcad/include
LIBCADCLEAN=libcadclean
GENDOC=$(shell cd ../libcad; echo $$(pwd)/gendoc.sh)
endif

CFLAGS ?= -g
LDFLAGS ?=

all: exe doc
	@echo

clean: $(LIBCADCLEAN)
	rm -rf target
	rm -f src/_exp_entry_registry.c

exe: target/$(PROJECT)

doc: target/$(PROJECT).pdf target/$(PROJECT)-htmldoc.tgz

target/$(PROJECT): $(OBJ) $(LIBCAD)
	@echo "Compiling executable: $@"
	$(CC) $(CFLAGS) -o $@ $(OBJ) -L target -lpcre -lcad

target/out/%.o: src/%.c src/*.h Makefile
	mkdir -p target/out
	@echo "Compiling object: $<"
	$(CC) $(CFLAGS) -I src $(LIBCADINCLUDE) -Wall -c $< -o $@

src/_exp_entry_registry.c: src/exp_*_entry.c utils/generate_factory_registry.sh
	@echo "Generating factory registry: $<"
	utils/generate_factory_registry.sh

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# libcad

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

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# release

install: exe doc target/version
	mkdir -p $(DESTDIR)/usr/bin
	mkdir -p $(DESTDIR)/usr/share/$(PROJECT)
	mkdir -p $(DESTDIR)/usr/share/doc/$(PROJECT)-doc
	cp target/$(PROJECT) $(DESTDIR)/usr/bin/
	cp -a target/*.pdf target/doc/html $(DESTDIR)/usr/share/doc/$(PROJECT)-doc/

release: debuild target/version
	@echo "Releasing version $(shell cat target/version)"
	mkdir target/dpkg
	mv ../$(PROJECT)*_$(shell cat target/version)_*.deb    target/dpkg/
	mv ../$(PROJECT)_$(shell cat target/version).dsc       target/dpkg/
	mv ../$(PROJECT)_$(shell cat target/version).tar.[gx]z target/dpkg/
	mv ../$(PROJECT)_$(shell cat target/version)_*.build   target/dpkg/
	mv ../$(PROJECT)_$(shell cat target/version)_*.changes target/dpkg/
	(cd target && tar cfz $(PROJECT)_$(shell cat target/version)_$(shell gcc -v 2>&1 | grep '^Target:' | sed 's/^Target: //').tgz $(PROJECT) $(PROJECT).pdf $(PROJECT)-htmldoc.tgz dpkg)

debuild: exe doc
	debuild -us -uc

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# doc (copied from libcad with a few changes)

target/$(PROJECT).pdf: target/doc/latex/refman.pdf
	@echo "	   Saving PDF"
	cp $< $@

target/doc/latex/refman.pdf: target/doc/latex/Makefile target/doc/latex/version.tex
	@echo "	 Building PDF"
#	remove the \batchmode on the first line:
	mv target/doc/latex/refman.tex target/doc/latex/refman.tex.orig
	tail -n +2 target/doc/latex/refman.tex.orig > target/doc/latex/refman.tex
	-yes x | $(MAKE) -C target/doc/latex > target/doc/make.log 2>&1
#	cat target/doc/latex/refman.log

target/$(PROJECT)-htmldoc.tgz: target/doc/html/index.html
	@echo "	 Building HTML archive"
	(cd target/doc/html; tar cfz - *) > $@

target/doc/latex/version.tex: target/version
	cp $< $@

target/version: debian/changelog
	head -n 1 debian/changelog | awk -F'[()]' '{print $$2}' > $@

debian/changelog: debian/changelog.raw
	sed "s/#DATE#/$(shell date -R)/;s/#SNAPSHOT#/$(shell date -u +'~%Y%m%d%H%M%S')/" < $< > $@

target/doc/latex/Makefile: target/doc/.doc
	sleep 1; touch $@

target/doc/html/index.html: target/doc/.doc
	sleep 1; touch $@

target/doc/.doc: Doxyfile target/gendoc.sh $(shell ls -1 src/*.[ch] doc/*) Makefile
	@echo "Generating documentation"
	target/gendoc.sh
	-doxygen -u $<
	doxygen $< && touch $@

target/gendoc.sh:
	mkdir -p target/doc target/gendoc
	ln -sf $(GENDOC) $@

.PHONY: all clean libcadclean doc install release debuild
