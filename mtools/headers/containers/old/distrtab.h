/****************************************************************************************
 * distrtab.h                                                 Version 3.2, Vindar, 2011 *
 *                                                                                      *
 * Class for computing empirical distribution of a sequence of i.i.d. random variables  *
 *                                                                                      *
 * - use the add(double) method to insert a new realization.                            *
 * - beware not to use any query method until at least 1 entry has been inserted        *
 *                                                                                      *
 * example of usage:                                                                    *
 *                                                                                      *
    #include "mylib_all.h"

    int main(void)
        { 
        random::MT2004_64 gen;
        random::NormalLaw N(0,0.1);
        DistrTab tab(-5.0,5.0,10000);
        for(int i=0;i<10000000;i++) {
            double v = N.get(gen.rand_double0(),gen.rand_double0());
            tab.add(v); }
        out << "Mean = " << tab.Mean() << "\n";
        out << "Variance^2 = " << tab.Variance()*tab.Variance() << "\n";
        Plotter P;
        P.insert(tab.PlotDensity());
        P.setrangeX(-3,3); P.setrangeY();
        P.plot();
        P.remove(tab.PlotDensity()); // so that it does not crash if tab is destroyed before P.
        return 0;
    }
 *                                                                                      *
 * implementation in distrtab.cpp                                                       *
  ****************************************************************************************/

#ifndef _DISTRTAB_H_
#define _DISTRTAB_H_

#include "../crossplatform.h"
#include "../graphics/plotobj.h"

namespace mylib
{
namespace containers
{

/***************************************************************
 * sub class of DistrTab used for plotting graphs              *
 * associated with an DistrTab object                          *
 *                                                             *
 * Cannot be created, must be obtained via a PlotXXX() method  *
 * from the main DistrTab object                               *
 ***************************************************************/
class DistrPlot: public mylib::graphics::PlotObj
{
    friend class DistrTab;
    public:
	    double minplotdomain() const;
        double maxplotdomain() const;

    protected:
        double plotvalue(double x) const;

    private:
        DistrPlot(DistrTab * obj,int type,std::string name = std::string("")) : PlotObj(name,false), mainobj(obj), ty(type) {return;}
        ~DistrPlot() {return;}
        DistrTab * mainobj;
        int ty;
};



/***************************************************************
 * The DistrTab class itself                                   *
 *                                                             *
 ***************************************************************/
class DistrTab
{
   friend class DistrPlot;

public:

    /**************************************************************************
     * Constructor: create the object, initially empty                        *
     * [minval,maxval]:  the interval where we want to record the density     *
     * size: the size of the buffer to hold the density (the memory used is   *
     *       about 16*size bytes).                                            *
     * name: optionnal name for this object.                                  *
     **************************************************************************/
    DistrTab(double minval,double maxval,size_t size,std::string name = std::string(""));

    
    /**************************************************************************
     * Constructor: create the object from a file                             *
     **************************************************************************/
    DistrTab(const std::string & filename); // NOT YET IMPLETMENTED


    /**************************************************************************
     * reset the object to the inital state                                   *
     * i.e. exactly the same as just after its construction                   *
     **************************************************************************/
    void Reset();


    /**************************************************************************
     * Save the object into a file                                            *
     **************************************************************************/
    void Save(const std::string & filename); // NOT YET IMPLETMENTED


    /**************************************************************************
     * Add a new realization into the object                                  *
     **************************************************************************/
    inline void add(double val)
        {
        if (val!=val) {return;} nbentry++;
        if (val < mini) {mini = val;} if (val > maxi) {maxi = val;}
        if (val < minv) {nboutmin++; return;} if (val >= maxv) {nboutmax++; return;} 
        size_t p = (size_t)((val-minv)*epsilon); tab[p]++; meanv += val; squarev += (val*val);
        }


    /**************************************************************************
     * return the empirical mean E[X]                                         *
     * !!! This may crash if no entry has been made in the object !!!         *
     **************************************************************************/
    inline double Mean() const {return(meanv/nbentry);}


    /**************************************************************************
     * return the empirical second moment E[X^2]                              *
     * !!! This may crash if no entry has been made in the object !!!         *
     **************************************************************************/
    inline double SquareMean() const {return(squarev/nbentry);}


    /**************************************************************************
     * return the empirical variance sqrt(E[X^2] - E[X]^2)                    *
     * !!! This may crash if no entry has been made in the object !!!         *
     **************************************************************************/
    inline double Variance() const {return sqrt(SquareMean() - Mean()*Mean());}


    /**************************************************************************
     * return an approximation of the density P(X \in dx) at point x          *
     * !!! This may crash if no entry has been made in the object !!!         *
     **************************************************************************/
    inline double Density(double x) const
        {
        if (x < mini) {return 0.0;} if (x < minv) {return((((double)nboutmin)/nbentry)/(minv-mini));}
        size_t p = (size_t)((x - minv)*epsilon);
        if (p < siz) {return((((double)tab[p])/nbentry)*siz/(maxv-minv));} if (x < maxi) {return((((double)nboutmax)/nbentry)/(maxi-maxv));}
        return 0.0;
        }


    /**************************************************************************
     * return an lower bound on the empirical value P(X <= x)                 *
     * !!! This may crash if no entry has been made in the object !!!         *
     **************************************************************************/
    inline double RepartMin(double x) const
        {
        maketabrep();
        if (x < mini) {return 0.0;} if (x < minv) {return 0.0;}
        size_t p = (size_t)((x - minv)*epsilon);
        if (p < siz) {return(((double)tabrep[p])/nbentry);} if (x < maxi) {return(1.0-((double)nboutmax)/nbentry);}
        return 1.0;
        }


    /**************************************************************************
     * return an upper bound on the empirical value P(X <= x)                 *
     * !!! This may crash if no entry has been made in the object !!!         *
     **************************************************************************/
    inline double RepartMax(double x) const
        {
        maketabrep();
        if (x < mini) {return 0.0;} if (x < minv) {return(((double)nboutmin)/nbentry);}
        size_t p = (size_t)((x - minv)*epsilon);
        if (p < siz) {return(((double)tabrep[p] + (double)tab[p])/nbentry);}
        return 1.0;
        }

    /**************************************************************************
     * return an lower bound on the empirical value P(X > x)                  *
     * !!! This may crash if no entry has been made in the object !!!         *
     **************************************************************************/
    inline double TailMin(double x) const {return (1.0-RepartMax(x));}


    /**************************************************************************
     * return an upper bound on the empirical value P(X > x)                  *
     * !!! This may crash if no entry has been made in the object !!!         *
     **************************************************************************/
    inline double TailMax(double x) const {return (1.0-RepartMin(x));}


    /**************************************************************************
     * return the plot corresponding the empirical repartition function       *
     * (approximated from below)                                              *
     * !!! plotting this object may crash if no entry has benn made !!!       *
     **************************************************************************/
    DistrPlot & PlotRepartMin() const {return(*Prepmin);}


    /**************************************************************************
     * return the plot corresponding the empirical repartition function       *
     * (approximated from above)                                              *
     * !!! plotting this object may crash if no entry has benn made !!!       *
     **************************************************************************/
    DistrPlot & PlotRepartMax() const {return(*Prepmax);}


    /**************************************************************************
     * return the plot corresponding the empirical tail distr. function       *
     * (approximated from below)                                              *
     * !!! plotting this object may crash if no entry has benn made !!!       *
     **************************************************************************/
    DistrPlot & PlotTailMin() const {return(*Ptailmin);}


    /**************************************************************************
     * return the plot corresponding the empirical tail distr. function       *
     * (approximated from above)                                              *
     * !!! plotting this object may crash if no entry has benn made !!!       *
     **************************************************************************/
    DistrPlot & PlotTailMax() const {return(*Ptailmax);}
    

    /**************************************************************************
     * return the plot corresponding the                                      *
     * empirical density                                                      *
     * !!! plotting this object may crash if no entry has benn made !!!       *
     **************************************************************************/
    DistrPlot & PlotDensity() const {return(*Pdensity);}


    /**************************************************************************
     * return the number of realization inserted into the object              *
     **************************************************************************/
    inline uint64 NBentry() const {return nbentry;}


    /**************************************************************************
     * return the minimal realization so far                                  *
     **************************************************************************/
    inline double MinValue() const {return mini;}


    /**************************************************************************
     * return the maximal realization so far                                  *
     **************************************************************************/
    inline double MaxValue() const {return maxi;}


    /**************************************************************************
     * return the number of realization so far which are smaller than minval  *
     * (i.e. out of bounds from below)                                        *
     **************************************************************************/
    inline uint64 NbOutMin() const {return nboutmin;}


    /**************************************************************************
    * return the number of realization so far which are larger than maxval    *
     * (i.e. out of bounds from above)                                        *
     **************************************************************************/
    inline uint64 NbOutMax() const {return nboutmax;}


    /* dtor */
    ~DistrTab();

private:

    /* create tabrep from tab if needed */
    void inline maketabrep() const
        {
        if (nbentry == lastrep) {return;}
        lastrep = nbentry; uint64 tot = nboutmin;
        for(size_t i=0;i<siz;i++) {tabrep[i] = tot; tot += tab[i];}
        }

    /* no copy */
    DistrTab & operator=(const DistrTab &);
    DistrTab(const DistrTab &);

    /* private variables */
    static int nbo;     // number of object created (for choosing a default name)
    std::string nam;    // name of the object
    double minv;        // keep point in the interval [minv,maxv]
    double maxv;        //
    size_t siz;         // size of the tab
    double epsilon;     // to make add method faster;
    uint64 * tab;       // the tab itself
    uint64 * tabrep;    // the tab for repartition
    DistrPlot * Prepmin;    // the differents plots
    DistrPlot * Prepmax;    //
    DistrPlot * Ptailmin;   //
    DistrPlot * Ptailmax;   //
    DistrPlot * Pdensity;   //
    uint64 nbentry;     // total number of entry
    mutable uint64 lastrep; // no of the entry when we last recomputed tabrep
    uint64 nboutmin;    // number of value < minv
    uint64 nboutmax;    // number of value > maxv
    double mini;        // smallest value (minv if no value < minv)
    double maxi;        // highest value  (maxv if no value > maxv)
    double meanv;       // empirical mean
    double squarev;     // empirical square mean
};


}
}
#endif
/* end of file distrtab.h */

