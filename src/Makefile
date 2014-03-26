#Makefile
CXX = g++ -std=gnu++0x -Wall -Werror -Wshadow -pedantic -g
OBJ = log.o
LIB = #-lSDL2
DEPS := $(OBJ:%.o=%.d)

LINKOBJ = $(OBJ)

RM = rm -f
BIN = ipdf

all : $(BIN)

$(BIN) : $(LINKOBJ) main.o
	$(CXX) -o $(BIN) main.o $(LINKOBJ) $(LIB)

%.o : %.cpp
	$(CXX) -c -MMD -o $@ $<

no_main : $(OBJ)

main.o : main.cpp
	$(CXX) -c main.cpp

clean :
	$(RM) $(BIN) $(OBJ) $(LINKOBJ) main.o

clean_full: #cleans up all backup files
	$(RM) $(BIN) $(OBJ) $(LINKOBJ)
	$(RM) *.*~
	$(RM) *~
	$(RM) *.o


	
