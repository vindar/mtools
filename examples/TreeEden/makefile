OUTFILE = TreeEden

OUTDIR = bin
SOURCE = main.cpp

WORKDIR = `pwd`
CXX = g++

# Add include directory and library directory in case the mtools was not installed in /usr/local/ 
HEADERDIR_MTOOLS = ../../mtools/headers
LIBDIR_MTOOLS = ../../linux/bin/

CXXFLAGS = `fltk-config --cxxflags` -std=c++11 -O2 -Wall -DNDEBUG
LDFLAGS =  -lmtools `fltk-config --ldstaticflags` -lz  -lpng -ljpeg -latomic 

$(OUTFILE): $(SOURCE)
	@test -d $(OUTDIR) || mkdir -p $(OUTDIR)
	$(CXX) -I$(HEADERDIR_MTOOLS) -L$(LIBDIR_MTOOLS) $(SOURCE) $(CXXFLAGS) $(LDFLAGS) -o $(OUTDIR)/$(OUTFILE)

.PHONY: clean	

clean: 
	@rm -rf $(OUTDIR)
