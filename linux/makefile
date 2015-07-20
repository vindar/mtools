# makefile for the mtools library

WORKDIR = `pwd`
CXX = g++
AR = ar
LD = g++

OBJDIR = obj
OUTDIR = bin

HEADERDIR_MTOOLS = ../mtools/headers

SRCDIR_MTOOLS = ../mtools/sources
OBJDIR_MTOOLS = $(OBJDIR)/mtools
OUTDIR_MTOOLS = $(OUTDIR)
SRC_MTOOLS = $(wildcard $(SRCDIR_MTOOLS)/*.cpp)
OBJ_MTOOLS = $(addprefix $(OBJDIR_MTOOLS)/, $(notdir $(SRC_MTOOLS:.cpp=.o)))
OUT_MTOOLS = $(OUTDIR_MTOOLS)/libmtools.a
PCHDIR_MTOOLS = ../mtools
PCHFILE_MTOOLS = stdafx_mtools.h
PCHOBJ_MTOOLS = $(OBJDIR_MTOOLS)/$(PCHFILE_MTOOLS).gch
INC_MTOOLS = -I$(OBJDIR_MTOOLS) -I$(HEADERDIR_MTOOLS)
CXXFLAGS_MTOOLS = `fltk-config --cxxflags` -std=c++11 -O2 -Wall -DNDEBUG
INSTALLDIR_MTOOLS = /usr/local

SRCDIR_TEST = ../test/sources
OBJDIR_TEST = $(OBJDIR)/test
OUTDIR_TEST = $(OUTDIR)
SRC_TEST = $(wildcard $(SRCDIR_TEST)/*.cpp)
OBJ_TEST = $(addprefix $(OBJDIR_TEST)/, $(notdir $(SRC_TEST:.cpp=.o)))
OUT_TEST = $(OUTDIR_TEST)/test
PCHDIR_TEST = ../test
PCHFILE_TEST = stdafx_test.h
PCHOBJ_TEST = $(OBJDIR_TEST)/$(PCHFILE_TEST).gch
INC_TEST = -I$(OBJDIR_TEST) -I$(HEADERDIR_MTOOLS)
CXXFLAGS_TEST = `fltk-config --cxxflags` -std=c++11 -O2 -Wall -DNDEBUG
LDFLAGS_TEST =  $(OUT_MTOOLS) `fltk-config --ldstaticflags` -lz -lpng -ljpeg -latomic
ADDLIBDIR_TEST =


.PHONY: all clean before_mtools mtools before_test test install uninstall


all: mtools test


before_mtools:
	@echo
	@echo "*************************"
	@echo "**** Building mtools ****"
	@echo "*************************"
	@test -d $(OUTDIR_MTOOLS) || mkdir -p $(OUTDIR_MTOOLS)
	@test -d $(OBJDIR_MTOOLS) || mkdir -p $(OBJDIR_MTOOLS)

	
mtools: before_mtools $(PCHOBJ_MTOOLS) $(OBJ_MTOOLS)
	$(AR) rcs $(OUT_MTOOLS) $(OBJ_MTOOLS)
	@echo "*************************"
	@echo "done. Static library: $(WORKDIR)/$(OUT_MTOOLS)"
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

	
test: before_test $(PCHOBJ_TEST) $(OBJ_TEST)
	$(LD) $(ADDLIBDIR_TEST) -o $(OUT_TEST) $(OBJ_TEST) $(LDFLAGS_TEST)
	@echo "****************************************"
	@echo "done. Test program: $(WORKDIR)/$(OUT_TEST)"
	@echo


$(PCHOBJ_TEST): 
	$(CXX) $(CXXFLAGS_TEST) $(INC_TEST) -x c++-header $(PCHDIR_TEST)/$(PCHFILE_TEST) -o $(PCHOBJ_TEST)

	
$(OBJ_TEST): $(OBJDIR_TEST)/%.o : $(SRCDIR_TEST)/%.cpp
	$(CXX) $(CXXFLAGS_TEST) $(INC_TEST) -c $< -o $@


clean: 
	@rm -rf $(OUTDIR)
	@rm -rf $(OBJDIR)
	@echo
	@echo "**** Cleaned ****"
	@echo

	
uninstall:
	@echo
	@echo "*****************************"
	@echo "**** Uninstalling mtools ****"
	@echo "*****************************"
	@rm -rf $(INSTALLDIR_MTOOLS)/include/mtools
	@rm -f $(INSTALLDIR_MTOOLS)/include/mtools.hpp
	@rm -f $(INSTALLDIR_MTOOLS)/lib/libmtools.a
	@echo "done."
	@echo
	
	
install:
	@echo
	@echo "***************************"
	@echo "**** Installing mtools ****"
	@echo "***************************"
	@rm -rf $(INSTALLDIR_MTOOLS)/include/mtools
	@rm -f $(INSTALLDIR_MTOOLS)/include/mtools.hpp
	@rm -f $(INSTALLDIR_MTOOLS)/lib/libmtools.a
	@printf '#pragma once\n#include "mtools/mtools.hpp"\n' > $(INSTALLDIR_MTOOLS)/include/mtools.hpp
	@mkdir -p $(INSTALLDIR_MTOOLS)/include/mtools
	@cp -rf ../mtools/headers/* $(INSTALLDIR_MTOOLS)/include/mtools/
	@cp -f ./bin/libmtools.a $(INSTALLDIR_MTOOLS)/lib/
	@chmod -R go+rX,go-w $(INSTALLDIR_MTOOLS)/include/mtools
	@chmod go+r,go-w,ugo-x $(INSTALLDIR_MTOOLS)/include/mtools.hpp
	@chmod go+rx,go-w $(INSTALLDIR_MTOOLS)/lib/libmtools.a
	@echo "done. Installed in $(INSTALLDIR_MTOOLS)"
	@echo

