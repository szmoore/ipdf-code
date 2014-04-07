#Makefile
CXX = g++ -std=gnu++0x -Wall -Werror -Wshadow -pedantic -g
MAIN = main.o
OBJ = log.o document.o view.o screen.o
LIB = ../contrib/lib/libSDL2-2.0.so.0 -lGL
MAINRPATH = -Wl,-rpath,'$$ORIGIN/../contrib/lib'
TESTRPATH = -Wl,-rpath,'$$ORIGIN/../../contrib/lib'
OBJPATHS = $(OBJ:%=../obj/%)
DEPS := $(OBJPATHS:%.o=%.d)
CFLAGS += -I../contrib/include/SDL2 -I`pwd`

LINKOBJ = $(OBJPATHS)

RM = rm -f
BIN = ../bin/ipdf


all : $(BIN)

tests/% : tests/%.cpp ../obj/tests/%.o $(LINKOBJ)
	$(CXX) -o $@.test $(LINKOBJ) ../obj/$@.o $(LIB) $(TESTRPATH)

runtests : tests/runtests.sh
	cd tests; ./runtests.sh


$(BIN) : $(LINKOBJ) ../obj/$(MAIN)
	@mkdir -p $(dir $@)
	$(CXX) -o $(BIN) $(LINKOBJ) ../obj/$(MAIN) $(LIB) $(MAINRPATH)

../obj/%.o : %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) -c -MMD -o $@ $<

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



	
