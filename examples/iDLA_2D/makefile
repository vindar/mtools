OUTFILE = iDLA_2D

OUTDIR = bin
SOURCE = main.cpp

WORKDIR = `pwd`
CXX = g++

HEADERDIR_MTOOLS = ../../mtools/headers
LIB_MTOOLS = ../../linux/bin/libmtools.a

CXXFLAGS = `fltk-config --cxxflags` -std=c++11 -O2 -Wall -DNDEBUG
LDFLAGS =  `fltk-config --ldstaticflags` -lz -lpng -ljpeg -latomic

$(OUTFILE): $(SOURCE)
	@test -d $(OUTDIR) || mkdir -p $(OUTDIR)
	$(CXX) $(SOURCE) $(LIB_MTOOLS) -I$(HEADERDIR_MTOOLS) $(CXXFLAGS) $(LDFLAGS) -o $(OUTDIR)/$(OUTFILE)


.PHONY: clean	

clean: 
	@rm -rf $(OUTDIR)
