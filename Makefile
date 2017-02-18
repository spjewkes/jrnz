CPP=g++
CPPFLAGS=-std=c++11
EXE=run_jrnz

OBJ_DIR=obj

DEPS=memory.hpp registers.hpp instructions.hpp
_OBJ=main.o instructions.o
OBJ=$(patsubst %,$(OBJ_DIR)/%,$(_OBJ))

buid: setup_build $(EXE)
	@echo "Build finished"

$(OBJ_DIR)/%.o: %.cpp $(DEPS)
	$(CPP) $(CPPFLAGS) -c -o $@ $<

$(EXE): $(OBJ)
	$(CPP) $(CPPFLAGS) $^ -o $@

setup_build:
	@mkdir -p $(OBJ_DIR)

.PHONY: clean

clean:
	@echo "Cleaning"
	@rm -f $(OBJ_DIR)/*.o *~
