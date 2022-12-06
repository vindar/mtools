#include <mtools/mtools.hpp>
using namespace mtools;



int main(int argc, char *argv[])
{
	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows
	mtools::parseCommandLine(argc, argv, true); // parse the command line, interactive mode
	
	cout << "Hello World\n";
	cout.getKey();
	return 0; 

}


























