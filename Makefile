CPP=g++
CPPFLAGS=-std=c++11
EXE=run_jrnz

OBJ_DIR=obj
SRC_DIR=src

_DEPS=z80.hpp memory.hpp register.hpp instructions.hpp storage_element.hpp common.hpp
DEPS=$(patsubst %,$(SRC_DIR)/%,$(_DEPS))

_OBJ=main.o instructions.o storage_element.o
OBJ=$(patsubst %,$(OBJ_DIR)/%,$(_OBJ))

buid: setup_build $(EXE)
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
