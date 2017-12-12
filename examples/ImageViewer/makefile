OUTFILE = imageViewer

OUTDIR = bin

ALL_FILES := $(wildcard *.cpp)
CPP_FILES := $(filter-out stdafx.cpp,$(ALL_FILES))

WORKDIR = `pwd`
CXX = g++

CXXFLAGS = -Wall `mtools-config --cxxflags`
LDFLAGS = `mtools-config --ldflags`

CHECKCOMPILERGCC = $(shell $(CXX) --version | grep 'GCC\|gcc\|g++')
ifneq ("$(CHECKCOMPILERGCC)","")
# gcc specific options
    LDFLAGS += -latomic
endif

.PHONY: release debug buildprog clean

release: CXXFLAGS += -O2 -DNDEBUG
release: buildprog

debug: CXXFLAGS += -g -DDEBUG
debug: buildprog

buildprog: $(OUTFILE)

$(OUTFILE): $(CPP_FILES)
	@test -d $(OUTDIR) || mkdir -p $(OUTDIR)
	$(CXX) $(CPP_FILES) $(CXXFLAGS) $(LDFLAGS) -o $(OUTDIR)/$(OUTFILE)

clean: 
	@rm -rf $(OUTDIR)


