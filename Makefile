CPP=g++
CPPFLAGS=-std=c++11 -Wall -Wextra -Werror -pedantic
EXE=run_jrnz

OBJ_DIR=obj
SRC_DIR=src

_DEPS=z80.hpp bus.hpp register.hpp instructions.hpp storage_element.hpp common.hpp system.hpp debugger.hpp decoder.hpp ula.hpp
DEPS=$(patsubst %,$(SRC_DIR)/%,$(_DEPS))

_OBJ=main.o instructions.o storage_element.o register.o z80.o system.o debugger.o decoder.o ula.o
OBJ=$(patsubst %,$(OBJ_DIR)/%,$(_OBJ))

default: debug

debug: CPPFLAGS += -g
debug: build

release: CPPFLAGS += -O2
release: build

build: setup_build $(EXE)
	@echo "Build finished"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEPS)
	$(CPP) $(CPPFLAGS) -c -o $@ $<

$(EXE): $(OBJ)
	$(CPP) $(CPPFLAGS) $^ -o $@

setup_build:
	@mkdir -p $(OBJ_DIR)

.PHONY: clean

clean:
	@echo "Cleaning"
	@rm -f $(OBJ_DIR)/*.o *~ $(SRC_DIR)/*~
