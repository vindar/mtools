# makefile for the mtools library

WORKDIR := $(shell pwd)
CXX = g++
AR = ar
LD = g++

BASE_DIR := $(shell dirname $(WORKDIR))

HEADERDIR_MTOOLS = ../mtools/headers

OBJDIR = obj

TOOLSDIR_MTOOLS = ../tools
TOOLSBIN_MTOOLS = $(TOOLSDIR_MTOOLS)/bin

SRCDIR_MTOOLS = ../mtools/sources
OBJDIR_MTOOLS = $(OBJDIR)/mtools
OUTDIR_MTOOLS = lib
SRC_MTOOLS = $(wildcard $(SRCDIR_MTOOLS)/*.cpp)
OBJ_MTOOLS = $(addprefix $(OBJDIR_MTOOLS)/, $(notdir $(SRC_MTOOLS:.cpp=.o)))
OUT_MTOOLS = $(OUTDIR_MTOOLS)/libmtools.a
PCHDIR_MTOOLS = ../mtools
PCHFILE_MTOOLS = stdafx_mtools.h
PCHOBJ_MTOOLS = $(OBJDIR_MTOOLS)/$(PCHFILE_MTOOLS).gch
INC_MTOOLS = -I$(OBJDIR_MTOOLS) -I$(HEADERDIR_MTOOLS)
CXXFLAGS_MTOOLS = `$(TOOLSBIN_MTOOLS)/mtools-config --cxxflags` -I../mtools

SRCDIR_TEST = ../test/sources
OBJDIR_TEST = $(OBJDIR)/test
OUTDIR_TEST = bin
SRC_TEST = $(wildcard $(SRCDIR_TEST)/*.cpp)
OBJ_TEST = $(addprefix $(OBJDIR_TEST)/, $(notdir $(SRC_TEST:.cpp=.o)))
OUT_TEST = $(OUTDIR_TEST)/test
PCHDIR_TEST = ../test
PCHFILE_TEST = stdafx_test.h
PCHOBJ_TEST = $(OBJDIR_TEST)/$(PCHFILE_TEST).gch
INC_TEST = -I$(OBJDIR_TEST) -I$(HEADERDIR_MTOOLS)

CXXFLAGS_TEST = `$(TOOLSBIN_MTOOLS)/mtools-config --cxxflags` -I../test
LDFLAGS_TEST =  -L$(OUTDIR_MTOOLS) `$(TOOLSBIN_MTOOLS)/mtools-config --ldflags`


CHECKCOMPILERGCC = $(shell $(CXX) --version | grep 'GCC\|gcc\|g++')
ifneq ("$(CHECKCOMPILERGCC)","")
# gcc specific options
    LDFLAGS_TEST += -latomic
endif


ADDLIBDIR_TEST =

prefix = /usr/local

.PHONY: all clean before_mtools mtools before_mtools_debug mtools_debug before_test test before_test_debug test_debug install uninstall msg

all: CXXFLAGS_MTOOLS += -Wall -O2 -DNDEBUG
all: CXXFLAGS_TEST += -Wall -O2 -DNDEBUG
all: mtools-config before_mtools mtools before_test test msg

debug: CXXFLAGS_MTOOLS += -Wall -g -DDEBUG
debug: CXXFLAGS_TEST += -Wall -g -DDEBUG
debug: mtools-config before_mtools_debug mtools_debug before_test_debug test_debug


mtools-config:
	@echo
	@echo "****************************************"
	@echo "****    Building mtools-config      ****"
	@echo "****************************************"
	@test -d $(TOOLSBIN_MTOOLS) || mkdir -p $(TOOLSBIN_MTOOLS)
	$(LD) -Wall -O2 $(TOOLSDIR_MTOOLS)/mtools-config.cpp -o $(TOOLSBIN_MTOOLS)/mtools-config



before_mtools: 
	@echo
	@echo "****************************************"
	@echo "****        Building mtools         ****"
	@echo "****************************************"
	@test -d $(OUTDIR_MTOOLS) || mkdir -p $(OUTDIR_MTOOLS)
	@test -d $(OBJDIR_MTOOLS) || mkdir -p $(OBJDIR_MTOOLS)
		
mtools: $(PCHOBJ_MTOOLS) $(OBJ_MTOOLS)
	$(AR) rcs $(OUT_MTOOLS) $(OBJ_MTOOLS)
	@echo "****************************************"
	@echo "done. Static library: $(WORKDIR)/$(OUT_MTOOLS)"
	@echo



before_mtools_debug: 
	@echo
	@echo "****************************************"
	@echo "****    Building mtools (debug)     ****"
	@echo "****************************************"
	@test -d $(OUTDIR_MTOOLS) || mkdir -p $(OUTDIR_MTOOLS)
	@test -d $(OBJDIR_MTOOLS) || mkdir -p $(OBJDIR_MTOOLS)

mtools_debug: $(PCHOBJ_MTOOLS) $(OBJ_MTOOLS)
	$(AR) rcs $(OUT_MTOOLS) $(OBJ_MTOOLS)
	@echo "****************************************"
	@echo "done. Static (debug) library: $(WORKDIR)/$(OUT_MTOOLS)"
	@echo
	
msg:
	@echo
	@echo "****************************************"
	@echo "****************************************"
	@echo
	@echo "You can now install the library with"
	@echo
	@echo "         sudo make install [prefix=/usr/local]"
	@echo
	@echo "or, if you do not wish to install the library, add the following"
	@echo "lines to your .profile / .bash_profile / .bashrc"
	@echo
	@echo '         export CPATH="$$CPATH:$(BASE_DIR)/mtools/headers/"'
	@echo '         export PATH="$$PATH:$(BASE_DIR)"'
	@echo '         export LIBRARY_PATH="$$LIBRARY_PATH:$(BASE_DIR)/linux/lib"'
	@echo
	@echo "Subsequently, you can:"
	@echo
	@echo "   - Build the examples with '../examples/makeall.sh'."
	@echo "   - Create a new empty project with the python script '../mtoolsproject.py'."
	@echo


$(PCHOBJ_MTOOLS): 
	$(CXX) $(CXXFLAGS_MTOOLS) $(INC_MTOOLS) -x c++-header $(PCHDIR_MTOOLS)/$(PCHFILE_MTOOLS) -o $(PCHOBJ_MTOOLS)
	
$(OBJ_MTOOLS): $(OBJDIR_MTOOLS)/%.o : $(SRCDIR_MTOOLS)/%.cpp
	$(CXX) $(CXXFLAGS_MTOOLS) $(INC_MTOOLS) -c $< -o $@
	


before_test:
	@echo
	@echo "****************************************"
	@echo "**** Building mtools's test program ****"
	@echo "****************************************"
	@test -d $(OUTDIR_TEST) || mkdir -p $(OUTDIR_TEST)
	@test -d $(OBJDIR_TEST) || mkdir -p $(OBJDIR_TEST)

test: $(PCHOBJ_TEST) $(OBJ_TEST)
	$(LD) $(ADDLIBDIR_TEST) -o $(OUT_TEST) $(OBJ_TEST) $(LDFLAGS_TEST)
	@echo "****************************************"
	@echo "done. Test program: $(WORKDIR)/$(OUT_TEST)"
	@echo



before_test_debug:
	@echo
	@echo "************************************************"
	@echo "**** Building mtools's test program (debug) ****"
	@echo "************************************************"
	@test -d $(OUTDIR_TEST) || mkdir -p $(OUTDIR_TEST)
	@test -d $(OBJDIR_TEST) || mkdir -p $(OBJDIR_TEST)

test_debug: $(PCHOBJ_TEST) $(OBJ_TEST)
	$(LD) $(ADDLIBDIR_TEST) -o $(OUT_TEST) $(OBJ_TEST) $(LDFLAGS_TEST)
	@echo "****************************************"
	@echo "done. Test program: $(WORKDIR)/$(OUT_TEST)"
	@echo


$(PCHOBJ_TEST): 
	$(CXX) $(CXXFLAGS_TEST) $(INC_TEST) -x c++-header $(PCHDIR_TEST)/$(PCHFILE_TEST) -o $(PCHOBJ_TEST)
	
$(OBJ_TEST): $(OBJDIR_TEST)/%.o : $(SRCDIR_TEST)/%.cpp
	$(CXX) $(CXXFLAGS_TEST) $(INC_TEST) -c $< -o $@



clean: 
	@rm -rf $(OUTDIR_MTOOLS)
	@rm -rf $(OUTDIR_TEST)
	@rm -rf $(OBJDIR)
	@rm -rf $(TOOLSBIN_MTOOLS)
	@echo
	@echo "**** Cleaned ****"
	@echo
	


install:
	@echo
	@echo "****************************************"
	@echo "****       Installing mtools        ****"
	@echo "****************************************"
	@echo
	@echo "Installation in $(prefix)"
	@echo
	@echo "Make sure that: $(prefix)/bin is in PATH"
	@echo "                $(prefix)/lib is in LIBRARY_PATH"
	@echo "                $(prefix)/include is in CPATH"	
	@echo
	@test -d $(prefix)/bin || mkdir -p $(prefix)/bin
	@test -d $(prefix)/lib || mkdir -p $(prefix)/lib
	@test -d $(prefix)/include || mkdir -p $(prefix)/include
	@test -d $(prefix)/include/mtools || mkdir -p $(prefix)/include/mtools
	@install -m 0755 ../mtoolsproject.py $(prefix)/bin
	@install -m 0755 $(TOOLSBIN_MTOOLS)/mtools-config $(prefix)/bin
	@install -m 0755 $(OUT_MTOOLS) $(prefix)/lib
	@cp -r $(HEADERDIR_MTOOLS)/* $(prefix)/include/mtools
	@echo "#include \"mtools/mtools.hpp\" // redirection" > $(prefix)/include/mtools.hpp
	@echo "Installation complete !"
	@echo
	@echo "Now, you can"
	@echo
	@echo "    - Run 'make uninstall' to remove the library"
	@echo "    - Build the example with '../examples/makeall.sh'"
	@echo "    - Create an empty project with '../mtoolsproject.py'"
	@echo
	

uninstall:	
	@echo
	@echo "****************************************"
	@echo "****        Removing mtools         ****"
	@echo "****************************************"
	@echo
	@rm $(prefix)/bin/mtools-config
	@rm $(prefix)/bin/mtoolsproject.py
	@rm $(prefix)/lib/libmtools.a
	@rm $(prefix)/include/mtools.hpp
	@rm -rf $(prefix)/include/mtools	
	@echo "done !"
	@echo
	
	
