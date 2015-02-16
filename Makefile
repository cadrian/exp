LIBRARIES=libcad

include /usr/share/libcad/Makefile

all: run-test exe doc
	@echo

run-test: target target/$(PROJECT) $(TST)
	@echo

exe: target target/$(PROJECT)

target/$(PROJECT): $(PIC_OBJ)
	$(CC) -o $@ $(PIC_OBJ) $(LINK_LIBS)
