/****************************************************************************************
 * distrtab.cpp                                               Version 3.2, Vindar, 2011 *
 *                                                                                      *
 * implementation of distrtab.h                                                         *
 ****************************************************************************************/
#include "headers/containers/distrtab.h"

#include "headers/customexc.h"

namespace mylib
{
namespace containers
{

    int DistrTab::nbo = 1;

    DistrTab::DistrTab(double minval,double maxval,size_t size,std::string name)
    {
        minv = minval;
        maxv = maxval;
        siz = size;
        nam = name;
        if (siz <= 2) {mylib::customexc e(0,"DistrTab::DistrTab, incorrect size value !"); e.raise();}
        if (((maxv-minv)/(2*siz)) <= 0) {mylib::customexc e(0,"DistrTab::DistrTab, incorrect [minval,maxval] !"); e.raise();}
        epsilon = ((double)siz)/(maxv-minv);
        if (nam.length() == 0) {nam = "DistrTab " + mylib::tostring(nbo); nbo++;}
        tab = new uint64[siz];      if (tab == NULL) {mylib::customexc e(0,"DistrTab::DistrTab, not enough memory for tab!"); e.raise();}
        tabrep = new uint64[siz];   if (tabrep == NULL) {mylib::customexc e(0,"DistrTab::DistrTab, not enough memory for tabrep!"); e.raise();}
        Prepmin = new DistrPlot(this,1,nam + " (repart min)");
        Prepmax = new DistrPlot(this,2,nam + " (repart max)");
        Ptailmin = new DistrPlot(this,3,nam + " (tail min)");
        Ptailmax = new DistrPlot(this,4,nam + " (tail max)");
        Pdensity = new DistrPlot(this,5,nam + " (density)");
        if ((Prepmin == NULL)||(Prepmax == NULL)||(Ptailmin == NULL)||(Ptailmax == NULL)||(Pdensity == NULL)) {mylib::customexc e(0,"DistrTab::DistrTab, not enough memory for the plots"); e.raise();}
        Reset();
    }


    DistrTab::~DistrTab()
    {
        delete Pdensity;
        delete Ptailmax;
        delete Ptailmin;
        delete Prepmax;
        delete Prepmin;
        delete [] tabrep;
        delete [] tab;
    }


    void DistrTab::Reset()
    {
        memset(tab,0,sizeof(uint64)*siz);
        memset(tabrep,0,sizeof(uint64)*siz);
        nbentry = 0;
        lastrep = 0;
        nboutmin = 0;
        nboutmax = 0;
        mini = std::numeric_limits<double>::infinity();
        maxi = -std::numeric_limits<double>::infinity();
        meanv = 0;
        squarev = 0;
    }

    // NOT YET IMPLEMENTED
    DistrTab::DistrTab(const std::string & filename)
        {
            mylib::customexc e(0,"DistrTab::DistrTab(const std::string & filename), not yet implemented!"); e.raise();
        }

    // NOT YET IMPLEMENTED
    void DistrTab::Save(const std::string & filename)
        {
            mylib::customexc e(0,"DistrTab::Save(const std::string & filename), not yet implemented!"); e.raise();
        }


    double DistrPlot::plotvalue(double x) const
        {
        switch(ty)
            {
            case 1: {return mainobj->RepartMin(x);}
            case 2: {return mainobj->RepartMax(x);}
            case 3: {return mainobj->TailMin(x);}
            case 4: {return mainobj->TailMax(x);}
            case 5: {return mainobj->Density(x);}
            }
        return 0.0;
        }

    double DistrPlot::minplotdomain() const {return mainobj->mini;}
    double DistrPlot::maxplotdomain() const {return mainobj->maxi;}

}
}
/* end of file distrtab.cpp */
