#include "stdafx_test.h"

#include "mtools.hpp"
using namespace mtools;



MT2004_64 gen(5679); // RNG with 2M vertices.
int main(int argc, char *argv[])
    {

	MTOOLS_SWAP_THREADS(argc, argv);
	parseCommandLine(argc, argv);

	return 0;



    }

/* end of file main.cpp */






