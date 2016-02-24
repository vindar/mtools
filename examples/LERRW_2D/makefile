
OUTFILE = LERRW_2D

OUTDIR = bin

ALL_FILES := $(wildcard *.cpp)
CPP_FILES := $(filter-out stdafx.cpp,$(ALL_FILES))

WORKDIR = `pwd`
CXX = g++

CXXFLAGS = -std=c++11 -Wall `fltk-config --cxxflags` `pkg-config cairo --cflags` `pkg-config pixman-1 --cflags` -Dcimg_display=0
LDFLAGS =   -lmtools `fltk-config --ldstaticflags` `pkg-config cairo --libs` -lfreetype -ljpeg -lpng `pkg-config pixman-1 --libs` -lz

CHECKCOMPILERGCC = $(shell $(CXX) --version | grep 'GCC\|gcc\|g++')
ifneq ("$(CHECKCOMPILERGCC)","")
# gcc specific options
    LDFLAGS += -latomic
    CXXFLAGS += -fopenmp	
endif

CHECKCOMPILERLLVM = $(shell $(CXX) --version | grep LLVM)
ifneq ("$(CHECKCOMPILERLLVM)","")
# clang specific options
endif

.PHONY: release nographics debug debug_nographics buildprog clean

release: CXXFLAGS += -O2 -DNDEBUG
release: buildprog

nographics: CXXFLAGS += -O2 -DNDEBUG -DMTOOLS_BASIC_CONSOLE
nographics: buildprog

debug: CXXFLAGS += -g -DDEBUG
debug: buildprog

debug_nographics: CXXFLAGS += -g -DDEBUG -DMTOOLS_BASIC_CONSOLE
debug_nographics: buildprog

buildprog: $(OUTFILE)

$(OUTFILE): $(CPP_FILES)
	@test -d $(OUTDIR) || mkdir -p $(OUTDIR)
	$(CXX) $(CPP_FILES) $(CXXFLAGS) $(LDFLAGS) -o $(OUTDIR)/$(OUTFILE)

clean: 
	@rm -rf $(OUTDIR)
