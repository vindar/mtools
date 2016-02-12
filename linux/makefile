# makefile for the mtools library

WORKDIR = `pwd`
CXX = g++
AR = ar
LD = g++

HEADERDIR_MTOOLS = ../mtools/headers

OBJDIR = obj

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
CXXFLAGS_MTOOLS = `fltk-config --cxxflags` -std=c++11 -Wall -I../mtools -Dcimg_display=0

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

CXXFLAGS_TEST = `fltk-config --cxxflags` -std=c++11 -Wall -I../test -Dcimg_display=0

LDFLAGS_TEST =  $(OUT_MTOOLS) `fltk-config --ldstaticflags` -lcairo -lfreetype -ljpeg -lpng -lpixman-1 -lz
CHECKCOMPILER = $(shell $(CXX) --version | grep LLVM)
ifeq ("$(CHECKCOMPILER)","")
# using gcc
    LDFLAGS_TEST += -latomic
endif

ADDLIBDIR_TEST =

prefix = ./testinst

.PHONY: all clean before_mtools before_mtools_debug mtools mtools_debug before_test before_test_debug test test_debug install uninstall msg

all: CXXFLAGS_MTOOLS += -O2 -DNDEBUG
all: CXXFLAGS_TEST += -O2 -DNDEBUG
all: mtools test msg

debug: CXXFLAGS_MTOOLS += -g -DDEBUG
debug: CXXFLAGS_TEST += -g -DDEBUG
debug: mtools_debug test_debug


before_mtools:
	@echo
	@echo "*************************"
	@echo "**** Building mtools ****"
	@echo "*************************"
	@test -d $(OUTDIR_MTOOLS) || mkdir -p $(OUTDIR_MTOOLS)
	@test -d $(OBJDIR_MTOOLS) || mkdir -p $(OBJDIR_MTOOLS)

before_mtools_debug: 
	@echo
	@echo "*********************************"
	@echo "**** Building mtools (debug) ****"
	@echo "*********************************"
	@test -d $(OUTDIR_MTOOLS) || mkdir -p $(OUTDIR_MTOOLS)
	@test -d $(OBJDIR_MTOOLS) || mkdir -p $(OBJDIR_MTOOLS)
	
mtools: before_mtools $(PCHOBJ_MTOOLS) $(OBJ_MTOOLS)
	$(AR) rcs $(OUT_MTOOLS) $(OBJ_MTOOLS)
	@echo "*************************"
	@echo "done. Static library: $(WORKDIR)/$(OUT_MTOOLS)"
	@echo

mtools_debug: before_mtools_debug $(PCHOBJ_MTOOLS) $(OBJ_MTOOLS)
	$(AR) rcs $(OUT_MTOOLS) $(OBJ_MTOOLS)
	@echo "*************************"
	@echo "done. Static library: $(WORKDIR)/$(OUT_MTOOLS)"
	@echo
	
msg:
	@echo
	@echo "You can now run 'sudo make install (prefix=/usr/local/)' to install the library"
	@echo "Alternatively, you can set the following paths in your .bash_profile : "
	@echo "         export CPATH=$CPATH:$(HEADERDIR_MTOOLS)"


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

before_test_debug: 
	@echo
	@echo "************************************************"
	@echo "**** Building mtools's test program (debug) ****"
	@echo "************************************************"
	@test -d $(OUTDIR_TEST) || mkdir -p $(OUTDIR_TEST)
	@test -d $(OBJDIR_TEST) || mkdir -p $(OBJDIR_TEST)

test: before_test $(PCHOBJ_TEST) $(OBJ_TEST)
	$(LD) $(ADDLIBDIR_TEST) -o $(OUT_TEST) $(OBJ_TEST) $(LDFLAGS_TEST)
	@echo "****************************************"
	@echo "done. Test program: $(WORKDIR)/$(OUT_TEST)"
	@echo

test_debug: before_test_debug $(PCHOBJ_TEST) $(OBJ_TEST)
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
	@echo
	@echo "**** Cleaned ****"
	@echo
	
install:
	@echo
	@echo "**** Installing mtools in $(prefix) ****"
	@echo
	@echo " Make sure that: $(prefix)/bin is in PATH"
	@echo "                 $(prefix)/lib is in LIBRARY_PATH"
	@echo "                 $(prefix)/include is in CPATH"	
	@echo
	@test -d $(prefix)/bin || mkdir -p $(prefix)/bin
	@test -d $(prefix)/lib || mkdir -p $(prefix)/lib
	@test -d $(prefix)/include || mkdir -p $(prefix)/include
	@test -d $(prefix)/include/mtools || mkdir -p $(prefix)/include/mtools
	@install -m 0755 ../mtoolsproject.py $(prefix)/bin
	@install -m 0755 $(OUT_MTOOLS) $(prefix)/lib
	@cp -r $(HEADERDIR_MTOOLS) $(prefix)/include/mtools
	@echo "#include\"mtools/mtools.hpp\" // redirection" > $(prefix)/include/mtools.hpp
	@echo "Installation complete !"
	@echo	
	@echo " - Use  'make uninstall' to remove the library"
	@echo " - The examples can be built by running '/examples/makeall.sh'."
	@echo " - Run 'mtoolsproject.py [projectname]' to create a new project."
	@echo
	
uninstall:	
	@echo
	@echo "**** Removing mtools from $(prefix) ****"
	@echo
	@rm $(prefix)/bin/mtoolsproject.py
	@rm $(prefix)/lib/libmtools.a
	@rm $(prefix)/include/mtools.hpp
	@rm -rf $(prefix)/mtools	
	@echo "done !"
	@echo
	
	
