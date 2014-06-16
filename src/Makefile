#Makefile
ARCH := $(shell uname -m)
# TODO: stb_truetype doesn't compile with some of these warnings.
CXX = g++ -std=gnu++0x -g -Wall -Werror -Wshadow -pedantic
MAIN = main.o
OBJ = log.o real.o document.o objectrenderer.o view.o screen.o vfpu.o graphicsbuffer.o framebuffer.o shaderprogram.o stb_truetype.o gl_core44.o
LIB_x86_64 = ../contrib/lib/libSDL2-2.0.so.0 -lGL
LIB_i386 = ../contrib/lib32/libSDL2-2.0.so.0 -lGL

MAINRPATH_x86_64 = -Wl,-rpath,'$$ORIGIN/../contrib/lib'
MAINRPATH_i386 = -Wl,-rpath,'$$ORIGIN/../contrib/lib32'
TESTRPATH_x86_64 = -Wl,-rpath,'$$ORIGIN/../../contrib/lib'
TESTRPATH_i386 = -Wl,-rpath,'$$ORIGIN/../../contrib/lib32'
OBJPATHS = $(OBJ:%=../obj/%)
DEPS := $(OBJPATHS:%.o=%.d)
CFLAGS_x86_64 := -I../contrib/include/SDL2 -I`pwd`
CFLAGS_i386 := -I../contrib/include32/SDL2 -I`pwd`


LIB := $(LIB_$(ARCH))
MAINRPATH := $(MAINRPATH_$(ARCH))
TESTRPATH := $(TESTRPATH_$(ARCH))
CFLAGS := $(CFLAGS_$(ARCH))

DEF = -DREAL=1

LINKOBJ = $(OBJPATHS)

RM = rm -f
BIN = ../bin/ipdf



all : $(BIN)

single : DEF = -DREAL=0
single : $(BIN)

double : DEF = -DREAL=1
double : $(BIN)

# The tests will compile with the default REAL definition
# To change that you can run as `make DEFS="REAL=X" tests/<target>` where X is your chosen type
# But remember to make clean first.
tests/% : tests/%.cpp ../obj/tests/%.o $(LINKOBJ)
	$(CXX) -o $@.test $(LINKOBJ) ../obj/$@.o $(LIB) $(TESTRPATH)

-include $(DEPS)

runtests : tests/runtests.sh
	cd tests; ./runtests.sh


$(BIN) : $(LINKOBJ) ../obj/$(MAIN)
	echo $(LINKOBJ)
	@mkdir -p $(dir $@)
	$(CXX) -o $(BIN) $(LINKOBJ) ../obj/$(MAIN) $(LIB) $(MAINRPATH)

-include $(DEPS)

../obj/%.o : %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(DEF) -c -MMD -o $@ $<

-include $(DEPS)

clean_bin :
	$(RM) $(BIN)

clean :
	$(RM) $(BIN) $(DEPS) $(LINKOBJ) ../obj/$(MAIN)
	$(RM) tests/*~
	$(RM) tests/*.test
	$(RM) tests/*.out
	$(RM) tests/*.err
	

clean_full: clean
	$(RM) *.*~
	$(RM) *~



	
