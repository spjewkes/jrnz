CPP=g++
CPPFLAGS=-std=c++11 -Wall -Wextra -Werror -pedantic
MAIN_EXE=run_jrnz
TEST_EXE=run_tests

OBJ_DIR=obj
SRC_DIR=src

_DEPS=z80.hpp bus.hpp register.hpp instructions.hpp storage_element.hpp common.hpp system.hpp debugger.hpp decoder.hpp ula.hpp options.hpp keyboard.hpp test.hpp
DEPS=$(patsubst %,$(SRC_DIR)/%,$(_DEPS))

_MAIN_OBJ=main.o instructions.o storage_element.o register.o z80.o system.o debugger.o decoder.o ula.o options.o keyboard.o bus.o
MAIN_OBJ=$(patsubst %,$(OBJ_DIR)/%,$(_MAIN_OBJ))

_TEST_OBJ=test_main.o instructions.o storage_element.o register.o z80.o decoder.o bus.o keyboard.o test_adc.o test_sbc.o
TEST_OBJ=$(patsubst %,$(OBJ_DIR)/%,$(_TEST_OBJ))

OS := $(shell uname)

ifeq ($(OS),Darwin)
# Mac OS
	CPPFLAGS+=-F/Library/Frameworks
	LIBS+=-framework SDL2
else
# Assume Linux for now
	LIBS+=-lSDL2
endif

default: debug

debug: CPPFLAGS += -g
debug: build

release: CPPFLAGS += -O2
release: build

build: setup_build $(MAIN_EXE) $(TEST_EXE)
	@echo "Build finished"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEPS)
	$(CPP) $(CPPFLAGS) -c -o $@ $<

$(MAIN_EXE): $(MAIN_OBJ)
	$(CPP) $(CPPFLAGS) $^ -o $@ $(LIBS)

$(TEST_EXE): $(TEST_OBJ)
	$(CPP) $(CPPFLAGS) $^ -o $@ $(LIBS)

setup_build:
	@mkdir -p $(OBJ_DIR)

.PHONY: clean

clean:
	@echo "Cleaning"
	@rm -f $(OBJ_DIR)/*.o *~ $(SRC_DIR)/*~
