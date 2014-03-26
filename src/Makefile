#Makefile
CXX = g++ -std=gnu++0x -Wall -Werror -Wshadow -pedantic -g
OBJ = log.o main.o document.o view.o screen.o
LIB = `sdl2-config --libs` -lGL
OBJPATHS = $(OBJ:%=../obj/%)
DEPS := $(OBJPATHS:%.o=%.d)
CFLAGS += `sdl2-config --cflags`

LINKOBJ = $(OBJPATHS)

RM = rm -f
BIN = ../bin/ipdf


all : $(BIN)

$(BIN) : $(LINKOBJ)
	@mkdir -p $(dir $@)
	$(CXX) -o $(BIN) $(LINKOBJ) $(LIB)

../obj/%.o : %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) -c -MMD -o $@ $<

-include $(DEPS)

clean :
	$(RM) $(BIN) $(DEPS) $(LINKOBJ)

clean_full: #cleans up all backup files
	$(RM) $(BIN) $(DEPS) $(LINKOBJ)
	$(RM) *.*~
	$(RM) *~
	$(RM) *.o


	
