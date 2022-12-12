#include <mtools/mtools.hpp>
using namespace mtools;




double f(double x) { return x * x;  }



void thr(Console* cons)
	{
	Plotter2D plotter;

	auto P = makePlot2DFun(f);
	plotter[P];
	plotter.autorangeXY(); 
	plotter.startPlot(); 

	while (1)
		{
		plotter.remove(P);
		(*cons) << "Hello !\n";
		Sleep(1000);
		plotter[P];
		Sleep(1000);
		}

	}







int main(int argc, char *argv[])
{
	MTOOLS_SWAP_THREADS(argc, argv);         // required on OSX, does nothing on Linux/Windows
	mtools::parseCommandLine(argc, argv, true); // parse the command line, interactive mode
	
	auto cons = new Console; 

	

	auto t = new std::thread(thr, cons);
	std::this_thread::yield();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::this_thread::yield();



	cout << "Hello World\n";
	cout.getKey();
	return 0; 

}


























