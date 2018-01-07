/***********************************************
 * Project : ImageViewer
 ***********************************************/

#include "mtools.hpp"
using namespace mtools;


/* display the image in filename */
void display(const std::string & filename)
    {
    Image im;
    try
        {
        mtools::ProgressBar<int> PB(0, 1, std::string("Loading file [") + toString(filename) + "]", true);
        PB.update(1);
        im.load(filename.c_str());
        }
    catch (...) { return; }
    auto image = makePlot2DImage(im, 1, mtools::toString(filename));
    Plotter2D Plotter;
    Plotter.useSolidBackground(false);
    Plotter[image];
    image.autorangeXY();
    Plotter.plot();
    }


#if defined(_WIN32)

/* windows version, do not display the default window console */
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
    {
    std::wstring filename(pCmdLine); // the image to load
    // remove surrounding double quote if any
    if ((filename.size() >= 1) && (filename[0] == '"')) { filename = filename.substr(1); };
    if ((filename.size() >= 1) && (filename[filename.size()-1] == '"')) { filename = filename.substr(0, filename.size()-1); };
    if (filename.size() < 1) { MessageBox(0, "The viewer must be called with the image filename as its unique argument.", "Image Viewer", MB_OK | MB_ICONEXCLAMATION); return 0; }
    display(toString(filename)); 
    return 0;
    }

#else

int main(int argc, char ** argv)
    {
    MTOOLS_SWAP_THREADS(argc, argv);
    parseCommandLine(argc, argv);
    if (argc < 2) { MTOOLS_ERROR("ImageViewer : the image to open must be passed from the command line"); }
    display(argv[1]);
    return 0;
    }

#endif

/* end of file main.cpp */

