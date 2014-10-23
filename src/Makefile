#Makefile
ARCH := $(shell uname -m)
# TODO: stb_truetype doesn't compile with some of these warnings.

# This seemed like a good idea 2 hours ago ok...
ifeq ($(ARCH), win32)
	CXX = i686-w64-mingw32-g++ -std=c++11 -g -Wall -Werror -Wshadow -pedantic
else
	CXX = g++ -std=c++11 -g -Wall -Werror -Wshadow -pedantic -rdynamic
endif

MAIN = main.o
OBJ = log.o real.o bezier.o objectrenderer.o view.o screen.o graphicsbuffer.o framebuffer.o shaderprogram.o stb_truetype.o gl_core44.o  path.o document.o debugscript.o paranoidnumber.o

QT_INCLUDE := -I/usr/share/qt4/mkspecs/linux-g++-64 -I. -I/usr/include/qt4/QtCore -I/usr/include/qt4/QtGui -I/usr/include/qt4 -I. -Itests -I.
QT_DEF := -DQT_NO_DEBUG -DQT_GUI_LIB -DQT_CORE_LIB
QT_LIB :=  -L/usr/lib/x86_64-linux-gnu -lQtGui -lQtCore -lpthread 

LIB_x86_64 = ../contrib/lib/libSDL2-2.0.so.0 -lGL -lgmp -lmpfr
LIB_i386 = ../contrib/lib32/libSDL2-2.0.so.0 -lGL -lgmp -lmpfr
LIB_i686 = $(LIB_i386)
LIB_win32 = -mwindows -lmingw32 -L../contrib/win32/lib/ -lSDL2main -lSDL2 -lgmp -static-libgcc -lopengl32 -static-libstdc++ -lmpfr

MAINRPATH_x86_64 = -Wl,-rpath,'$$ORIGIN/../contrib/lib'
MAINRPATH_i386 = -Wl,-rpath,'$$ORIGIN/../contrib/lib32'
MAINRPATH_i686 = $(MAINRPATH_i386)
MAINRPATH_win32 = -Wl,-rpath,'$$ORIGIN/../contrib/win32/lib'
TESTRPATH_x86_64 = -Wl,-rpath,'$$ORIGIN/../../contrib/lib'
TESTRPATH_i386 = -Wl,-rpath,'$$ORIGIN/../../contrib/lib32'
TESTRPATH_i686 = $(TESTRPATH_i386)
TESTRPATH_win32 = -Wl,-rpath,'$$ORIGIN/../../contrib/win32/lib'
OBJPATHS = $(OBJ:%=../obj/%)
DEPS := $(OBJPATHS:%.o=%.d)
CFLAGS_x86_64 := -I../contrib/include/SDL2 -I`pwd`
CFLAGS_i386 := -I../contrib/include32/SDL2 -I`pwd`
CFLAGS_i686 := $(CFLAGS_i386)
CFLAGS_win32 := -I../contrib/win32/include/SDL2 -I`pwd` -I../contrib/win32/include


LIB := $(LIB_$(ARCH))
MAINRPATH := $(MAINRPATH_$(ARCH))
TESTRPATH := $(TESTRPATH_$(ARCH))

CFLAGS := $(CFLAGS_$(ARCH))

RM = rm -f
BIN = ../bin/ipdf
REALTYPE=1
CONTROLPANEL=enabled
QUADTREE=disabled
TRANSFORMATIONS=direct
MPFR_PRECISION=23
PATHREAL=0
DEF = -DREALTYPE=$(REALTYPE)



## Only link with things we care about

ifeq ($(QUADTREE),enabled)
	OBJ := $(OBJ) quadtree.o
else
	DEF := $(DEF) -DQUADTREE_DISABLED
endif

ifeq ($(CONTROLPANEL),enabled)
	LIB := $(LIB) $(QT_LIB)
	DEF := $(DEF) $(QT_DEF)
	CFLAGS := $(CFLAGS) $(QT_INCLUDE)
	OBJ := $(OBJ) moc_controlpanel.o controlpanel.o
else
	DEF := $(DEF) -DCONTROLPANEL_DISABLED
endif

ifeq ($REALTYPE),3)
	OBJ := $(OBJ) vfpu.o
endif

ifeq ($(REALTYPE),5)
	OBJ := $(OBJ) add_digits_asm.o sub_digits_asm.o mul_digits_asm.o div_digits_asm.o arbint.o
	LIB := $(LIB) -lgmp
endif

ifeq ($(REALTYPE),6)
	LIB := $(LIB) -lgmp -lmpfr
	DEF := $(DEF) -DMPFR_PRECISION=$(MPFR_PRECISION)
endif

ifeq ($(TRANSFORMATIONS),cumulative)
	DEF := $(DEF) -DTRANSFORM_OBJECTS_NOT_VIEW
endif

ifeq ($(TRANSFORMATIONS),path)
	DEF := $(DEF) -DTRANSFORM_OBJECTS_NOT_VIEW -DTRANSFORM_BEZIERS_TO_PATH -DPATHREAL=$(PATHREAL)
	ifeq ($(PATHREAL), mpfr)
		LIB:= $(LIB) -lmpfr
	endif
endif

ifeq ($(REALTYPE),7)
	LIB := $(LIB) -L../contrib/iRRAM/lib -liRRAM -lgmp -lmpfr
	CFLAGS :=  $(CFLAGS) -I../contrib/iRRAM/include
endif

ifeq ($(REALTYPE),8)
	OBJ := $(OBJ) 
endif 

ifeq ($(REALTYPE),9)
	LIB := $(LIB) -lgmp
endif

LINKOBJ = $(OBJPATHS)


all : REALTYPE = 1
all : $(BIN)

single : REALTYPE = 0
single : $(BIN)

double : REALTYPE = 1
double : $(BIN)



CFLAGS := $(CFLAGS) $(QT_DEF)

demo : $(BIN) ../tools/stream_plot.py
	mkdir -p ../data/
	$(RM) ../data/performance.dat
	./ipdf | tee ../data/performance.dat | ../tools/stream_plot.py 2>/dev/null

movie : $(BIN) ../tools/stream_plot.py
	mkdir -p ../data/
	$(RM) ../data/performance.dat
	$(RM) ../data/movie.ogv
	./ipdf | tee ../data/performance.dat | ../tools/stream_plot.py 2>/dev/null & recordmydesktop --fps 10 --on-the-fly-encoding -o ../data/movie.ogv

# The tests will compile with the default REALTYPE definition
# To change that you can run as `make DEFS="REALTYPE=X" tests/<target>` where X is your chosen type
# But remember to make clean first.
tests/% : tests/%.cpp ../obj/tests/%.o $(LINKOBJ)
	$(CXX) $(CFLAGS) -o $@ $(LINKOBJ) ../obj/$@.o $(LIB) $(TESTRPATH)

-include $(DEPS)

runtests : tests/runtests.sh
	cd tests; ./runtests.sh


$(BIN) : $(LINKOBJ) ../obj/$(MAIN)
	echo $(LINKOBJ)
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) -o $(BIN) $(LINKOBJ) ../obj/$(MAIN) $(LIB) $(MAINRPATH)

-include $(DEPS)

moc_controlpanel.cpp : controlpanel.cpp controlpanel.h
	moc-qt4 $(DEF) controlpanel.h -o moc_controlpanel.cpp

../obj/%.o : %.cpp main.h
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(DEF) -c -MMD -o $@ $<

../obj/%_asm.o : %_asm.S main.h
	@mkdir -p $(dir $@)
	$(CXX) -c -o $@ $<

-include $(DEPS)

clean_bin :
	$(RM) $(BIN)

clean :
	$(RM) $(BIN) $(DEPS) $(LINKOBJ) ../obj/$(MAIN) ../obj/quadtree.?
	$(RM) tests/*~
	$(RM) tests/*.test
	$(RM) tests/*.out
	$(RM) tests/*.err

clean_full: clean
	$(RM) *.*~
	$(RM) *~



	
