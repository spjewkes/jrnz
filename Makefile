CPP=g++
CPPFLAGS=-std=c++11

OBJ_DIR=obj

DEPS=memory.hpp registers.hpp instructions.hpp
_OBJ=main.o instructions.o
OBJ=$(patsubst %,$(OBJ_DIR)/%,$(_OBJ))

$(OBJ_DIR)/%.o: %.cpp $(DEPS)
	$(CPP) $(CPPFLAGS) -c -o $@ $<

run_jrnz: $(OBJ)
	$(CPP) $(CPPFLAGS) $^ -o $@

.PHONY: clean

clean:
	rm -f $(OBJ_DIR)/*.o *~

