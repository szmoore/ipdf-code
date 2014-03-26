#Makefile
CXX = g++ -std=gnu++0x -Wall -Werror -Wshadow -pedantic -g
OBJ = log.o main.o document.o view.o
LIB = #-lSDL2
OBJPATHS = $(OBJ:%=../obj/%)
DEPS := $(OBJPATHS:%.o=%.d)

LINKOBJ = $(OBJPATHS)

RM = rm -f
BIN = ../bin/ipdf

-include $(DEPS)

all : $(BIN)

$(BIN) : $(LINKOBJ)
	@mkdir -p $(dir $@)
	$(CXX) -o $(BIN) $(LINKOBJ) $(LIB)

../obj/%.o : %.cpp
	@mkdir -p $(dir $@)
	$(CXX) -c -MMD -o $@ $<

clean :
	$(RM) $(BIN) $(DEPS) $(LINKOBJ)

clean_full: #cleans up all backup files
	$(RM) $(BIN) $(DEPS) $(LINKOBJ)
	$(RM) *.*~
	$(RM) *~
	$(RM) *.o


	
