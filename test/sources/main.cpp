#include "stdafx_test.h"

#include "mtools.hpp"
#include "misc/threadworker.hpp"

#include <limits>

using namespace mtools;






volatile int inIt = 64; // initial number of iterations 

/* Mandelbrot
simple RGBc return type : multiple call for the same pixel are blended together
*/
RGBc mandelbrot(const fVec2 & pos, const fBox2 & R, int32 nbiter)
    {
    int nbi = inIt + nbiter * (inIt/10);
    const double cx = pos.X();
    const double cy = pos.Y();
    double X = 0.0;
    double Y = 0.0;
    for (int i = 0; i < nbi; i++)
        {
        const double sX = X;
        const double sY = Y;
        X = sX*sX - sY*sY + cx;
        Y = 2 * sX*sY + cy;
        if ((X*X + Y*Y) > 4) { return RGBc::jetPalette(i, 1, nbi); }
        }
    return RGBc::c_Black;
    }


/* Douady's rabbit
return type std::pair<RGBc,bool> : setting the bool to true force color returned to overwrite previous color at the same pixel. 
*/
std::pair<RGBc,bool> rabbit(const fVec2 & pos, const fBox2 & R, int32 nbiter)
    {
    const int nbi = 64;
    const double cx = -0.122561;
    const double cy = 0.744862;
    double X = pos.X();
    double Y = pos.Y();
    for (int i = 0; i < nbi; i++)
        {
        const double sX = X;
        const double sY = Y;
        X = sX*sX - sY*sY + cx;
        Y = 2 * sX*sY + cy;
        if ((X*X + Y*Y) > 4) { return std::pair<RGBc,bool>(RGBc::jetPalette(i, 1,64),true); }
        }
    return std::pair<RGBc, bool>(RGBc::c_Black,true);
    }









    void test()
        {
        const int LLX = 2200;
        const int LLY = 1400;
        const int UX = 2000;
        const int UY = 1000;

        ProgressImg progIm(LLX, LLY);
        progIm.clear((RGBc64)RGBc::c_Red);
        
        mtools::Img<unsigned char> dispIm(LLX, LLY, 1, 4);
        cimg_library::CImg<unsigned char> * cim = (cimg_library::CImg<unsigned char> *)&dispIm;

        fBox2 r(-2.0, 2.0, -1.0, 1.0);

        PlaneDrawer<decltype(mandelbrot)> TPD(mandelbrot, 6);

        iBox2 SubB(50, 50 + UX - 1, 20, 20 + UY - 1);

        SubB.clear(); 

        TPD.setParameters(r, &progIm, SubB);
        TPD.sync();
        TPD.enable(true);
        TPD.sync();
        int drawtype = 0, isaxe = 1, isgrid = 0, iscell = 1; // flags
        cimg_library::CImgDisplay DD(*cim); // display
        while ((!DD.is_closed())) 
            {
            uint32 k = DD.key();
            if ((DD.is_key(cimg_library::cimg::keyA))) { TPD.enable(!TPD.enable()); std::this_thread::sleep_for(std::chrono::milliseconds(50)); }   // type A for toggle axe (with graduations)
            if ((DD.is_key(cimg_library::cimg::keyG))) { isgrid = 1 - isgrid; std::this_thread::sleep_for(std::chrono::milliseconds(50)); } // type G for toggle grid
            if ((DD.is_key(cimg_library::cimg::keyC))) { iscell = 1 - iscell; std::this_thread::sleep_for(std::chrono::milliseconds(50)); } // type C for toggle cell
            if (DD.is_key(cimg_library::cimg::keyESC)) { TPD.redraw(); } // [ESC] to force complete redraw
            if (DD.is_key(cimg_library::cimg::keyARROWUP)) { double sh = r.ly() / 20; r.min[1] += sh; r.max[1] += sh; TPD.setParameters(r, &progIm, SubB); } // move n the four directions
            if (DD.is_key(cimg_library::cimg::keyARROWDOWN)) { double sh = r.ly() / 20; r.min[1] -= sh; r.max[1] -= sh; TPD.setParameters(r, &progIm, SubB);} //
            if (DD.is_key(cimg_library::cimg::keyARROWLEFT)) { double sh = r.lx() / 20; r.min[0] -= sh; r.max[0] -= sh; TPD.setParameters(r, &progIm, SubB); } //
            if (DD.is_key(cimg_library::cimg::keyARROWRIGHT)) { double sh = r.lx() / 20; r.min[0] += sh; r.max[0] += sh; TPD.setParameters(r, &progIm, SubB); } //
            if (DD.is_key(cimg_library::cimg::keyPAGEDOWN)) { double lx = r.max[0] - r.min[0]; double ly = r.max[1] - r.min[1]; r.min[0] = r.min[0] - (lx / 8.0); r.max[0] = r.max[0] + (lx / 8.0); r.min[1] = r.min[1] - (ly / 8.0);  r.max[1] = r.max[1] + (ly / 8.0); TPD.setParameters(r, &progIm, SubB); }
            if (DD.is_key(cimg_library::cimg::keyPAGEUP)) { double lx = r.max[0] - r.min[0]; double ly = r.max[1] - r.min[1]; r.min[0] = r.min[0] + (lx / 10.0); r.max[0] = r.max[0] - (lx / 10.0); r.min[1] = r.min[1] + (ly / 10.0); r.max[1] = r.max[1] - (ly / 10.0); TPD.setParameters(r, &progIm, SubB); }
       
            TPD.sync();
            std::cout << "quality = " << TPD.progress() << "\n";
            progIm.blit(dispIm);
//            if (isaxe) { dispIm.fBox2_drawAxes(r).fBox2_drawGraduations(r).fBox2_drawNumbers(r); }
//            if (isgrid) { dispIm.fBox2_drawGrid(r); }
//            if (iscell) { dispIm.fBox2_drawCells(r); }
            DD.display(*cim);
            }
        return;
        }


    double ff(double x) { return -x; }

    RGBc colorLattice(iVec2 pos) 
        {
        if (pos.norm() < 100) return (RGBc::c_Green).getOpacity(0.5);
        return (RGBc::c_Lime).getOpacity(0.5);
        }

    RGBc colorPlane(fVec2 pos)
        {
        if (pos.norm() < 50) return (RGBc::c_Red).getOpacity(0.5);
        return RGBc::c_TransparentWhite;
        }

    std::vector<double> vv1;

    int * vv2;

    mtools::Img<unsigned char> imm;


    int main(int argc, char * argv[])
        {

  
        /*
        imm.load("lenna.jpg");
        vv2 = new int[1000];
        for (int i = 0;i < 1000; i++) { vv1.push_back(i - 50); vv2[i] = i - 100; }
        Plotter2D Pl;
        Pl.sensibility(1);
        Pl.fourChannelImage(true);

        auto P1 = makePlot2DFun(ff, "function");
        auto P2 = makePlot2DLattice(colorLattice, "lattice");
        auto P3 = makePlot2DPlane(mandelbrot, 6, "plane");
        auto P4 = makePlot2DCImg(imm, "image");
        auto P5 = makePlot2DVector(vv1, true, "vector");
        auto P6 = makePlot2DArray(vv2, 1000, "array");

        Pl.gridObject(true);
        Pl[P3];
        //Pl[P1][P2][P3][P4][P5][P6];
        Pl.plot();
        */

        MTOOLS_SWAP_THREADS(argc, argv);
        parseCommandLine(argc, argv, true);

        Plotter2D Plotter;  // create the plotter
        //Plotter.fourChannelImage(true);
        auto M = makePlot2DPlane(mandelbrot, 7, "Mandelbrot Set"); // the mandelbrot set
        Plotter[M]; 
        M.opacity(1.0);
        Plotter.range().setRange(fBox2(-0.65, -0.15, 0.4, 0.8));
        std::cout << "\n\nok\n";

        watch("Nb of iterations", inIt);
        Plotter.sensibility(1);
        Plotter.plot();
        watch.remove("Nb of iterations");
        return 0;
        }




