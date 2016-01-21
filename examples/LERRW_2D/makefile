OUTFILE = LERRW_2D

OUTDIR = bin

CPP_FILES := $(wildcard *.cpp)

WORKDIR = `pwd`
CXX = g++

# Add include directory and library directory in case the mtools
# librarie is not in a standard directory. (or add the path to 
# mtools in LIBRARY_PATH and CPATH env. var.)
#HEADERDIR_MTOOLS = ../../mtools/headers/
#LIBDIR_MTOOLS = ../../linux/bin/

CXXFLAGS = -std=c++11 -Wall  `fltk-config --cxxflags`
LDFLAGS =   -lmtools -latomic `fltk-config --ldstaticflags` -lcairo -lfreetype -ljpeg -lpng -lpixman-1 -lz

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
	$(CXX) -I$(HEADERDIR_MTOOLS) -L$(LIBDIR_MTOOLS) $(CPP_FILES) $(CXXFLAGS) $(LDFLAGS) -o $(OUTDIR)/$(OUTFILE)

clean: 
	@rm -rf $(OUTDIR)


