/****************************************************************************************************
* TEMPLATE CLASS OnceVRRW                                                  Version 3.2, Vindar 2011 *
*                                                                                                   *
* Used for simulating a Once Vertex Reinforced Random Walk on Z^2                                   *
*                                                                                                   *
* STATUS: WORKING COMPLETELY !                                                                      * 
*                - Tested for 32 and 64 bits implementation ?                              YES !    *
*                - Can work with up to 1TB of memory ?                                     YES !    *
*                                                                                                   *
* IMPLEMENTATION: - Make use of the BitGraphZ2 class for the lattice.                               *
*                                                                                                   *
* CLASS TEMPLATE PARAMETERS                                                                         *
* - randomgen : the class of the random number generator.                                           *
* - N         : the size of each subsquare of Z^2 (for BitGraphZ2 object ie square have size 8Nx8N) *
*                                                                                                   *
*                                                                                                   *
* METHODS:                                                                                          *
* - Reset                 reset the walk                                                            *
* - GetDelta              get value of reinf. param. delta                                          *
* - SetDelta              set value of reinf. param. delta                                          *
* - MakeOneStep           perform one step of the walk                                              *
* - PerformWalk           perform a given number of steps of the walk                               *
* - PrintBMP              print the graph of the walk in a bmp file                                 *
* - stats                 some stats                                                                *
* - detailled_stats       detailled stats                                                           *
* - posX pos Y            actual position of the walk                                               *
* - MinX,MinY,MaxX,MaxY   encercling rectangle of the walk                                          *
* - Steps                 number of steps performed by the walk                                     *
* - Range                 number of site visited by the walk                                        *
* - isVisited             check whether a particular site has been visited                          *
*                                                                                                   *
* EXAMPLE:                                                                                          *
*                                                                                                   *
#include <mylib_all.h>

void Simulate_OnceVRRW_Z2()
    {
    out << "SIMULATION OF A ONCE VERTEX REINFORCED RANDOM WALK.\n";
    out << "Number of MB to allocate = ";;
    int32 sizeMB; input >> sizeMB; 
    if ((sizeMB < 500)||(sizeMB > 100000)) {sizeMB = 1000;} out << sizeMB << "\n";
    out << "Value of the reinforcment delta = ";
    double delta; input >> delta; if (delta < 0) {delta = 1.0;} out << delta << "\n";
    out << "number of billion step between each saving of image = ";
    int64 bil;  input >> bil; if ((bil < 1)||(bil > 1000)) {bil = 10;} out << bil << "\n\n\n";
    int64 nadd = 100;
    bil *= (1000000000/nadd);
    ExTab RatX(1000000,"ratio X");
    ExTab RatY(1000000,"ratio Y");
    MT2004_64 gen;
    models::OnceVRRW_Z2<MT2004_64,20> RRW(sizeMB,delta,&gen);
    while(1)
        {
        out << "\n\nSimulation from " << RRW.Steps() << " to " << RRW.Steps() + bil*nadd << "...";
        Chronometer();
        for(int64 i=0;i<bil;i++) 
            {
            RRW.PerformWalk(nadd);
            RatX.Add(((double)(RRW.MaxX() + RRW.MinX()))/((double)(RRW.MaxX() - RRW.MinX())));
            RatY.Add(((double)(RRW.MaxY() + RRW.MinY()))/((double)(RRW.MaxY() - RRW.MinY())));
            }
        out << "done in " << (Chronometer()/1000) << "seconds !\n";
        out << RRW.detailled_stats();
        std::string name = std::string("OVRRW-d") + mylib::tostring(delta);
        RRW.PrintBMP(true,true,name + "-n" + mylib::tostring(RRW.Steps()) + ".bmp",5000);
        RatX.Save(name + "-ratX.extab");
        RatY.Save(name + "-ratY.extab");
        }
    }

int main (int argc, char *argv[])
    {
    Simulate_OnceVRRW_Z2();
     return 0;
    }
*                                                                                                   *
* standalone, no cpp file                                                                           *
****************************************************************************************************/

#ifndef _ONCEVRRW_Z2_H_
#define _ONCEVRRW_Z2_H_

#include "customexc.h"
#include "mathgraph/BitGraphZ2.h"
#include "crossplatform.h"

namespace mylib
{
namespace models
{

template<class randomgen,int32 N = 25> class OnceVRRW_Z2
{
public:

    /*********************************************************************
    * Constructor
    * - MB : memory to allocate for the object
    * - delta : value of the reinforcement:
    *           - non visited site has value 1.0
    *           - visited site has value delta (ie delta > 1 gives reinforcment)
    *********************************************************************/
    OnceVRRW_Z2(int MB,double delta,randomgen * gener)
        {
        gen = gener;
        if (d<0) {mylib::customexc e(0,"OnceVRRW_Z2::OnceVRRW_Z2, invalid value for delta !"); e.raise();}
        if ((MB < 100)||(MB > 1000000)) {mylib::customexc e(0,"OnceVRRW_Z2::OnceVRRW_Z2, invalid value for MB !"); e.raise();}
        G = new mylib::mathgraph::BitGraphZ2<N>(MB);
        Reset(delta);
        }


    /*********************************************************************
    * dtor 
    *********************************************************************/
    ~OnceVRRW_Z2() {delete G;}
    


    /*********************************************************************
    * Reset the walk/object
    * - starting position of the walk to (0,0)
    *********************************************************************/
    void Reset(double delta)
        {
        G->Clear();
        n = 0; x = 0; y = 0;
        G->Set(x,y);
        SetDelta(delta);
        }


    /*********************************************************************
    * change the value of delta (after this call, all the site visited
    * are considered having the new value delta)
    *********************************************************************/
    void SetDelta(double delta) {d = delta;}


    /*********************************************************************
    * return the actual value of delta
    *********************************************************************/
    double GetDelta() const {return d;}


    /*********************************************************************
    * Perform 1 step of the walk and return the new position in (nX,nY)
    * (very fast)
    *********************************************************************/
    inline void MakeOneStep(int64 & nX,int64 & nY)
        {
        n++;
        double d_up    = ((G->Get(x,y+1)) ? d : 1.0);
        double d_left  = ((G->Get(x-1,y)) ? d : 1.0);
        double d_right = ((G->Get(x+1,y)) ? d : 1.0);
        double d_down  = ((G->Get(x,y-1)) ? d : 1.0);
        double a = gen->rand_double0()*(d_up + d_left + d_right + d_down);
        if (a < d_up)                       {y++; G->Set(x,y);}   else {
        if (a < (d_up + d_left))            {x--; G->Set(x,y);}   else {
        if (a < (d_up + d_left + d_right))  {x++; G->Set(x,y);}   else {y--; G->Set(x,y);} }}
        nX = x; nY = y;
        return;
        }


    /*********************************************************************
    * Perform nbsteps of the walk. 
    *********************************************************************/
    inline void PerformWalk(int64 nbsteps)
        {
        for(int64 i=0;i<nbsteps;i++)
            {
            double d_up    = ((G->Get(x,y+1)) ? d : 1.0);
            double d_left  = ((G->Get(x-1,y)) ? d : 1.0);
            double d_right = ((G->Get(x+1,y)) ? d : 1.0);
            double d_down  = ((G->Get(x,y-1)) ? d : 1.0);
            double a = gen->rand_double0()*(d_up + d_left + d_right + d_down);
            if (a < d_up)                       {y++; G->Set(x,y);}   else {
            if (a < (d_up + d_left))            {x--; G->Set(x,y);}   else {
            if (a < (d_up + d_left + d_right))  {x++; G->Set(x,y);}   else {y--; G->Set(x,y);} }}
            }
        n += nbsteps;
        return;
        }

    /*********************************************************************
    * Print the graph of the walk in a bmp file
    * - with 1:1 aspect ratio, largest dimension LL, autorange.
    * - drawaxes = true for the axes
    * - filledsquare = true for drawing the completely filled subsquare 
    *   (of size 8*N x 8N) with a darker red
    *********************************************************************/
    void PrintBMP(bool filledsquare,bool drawaxes,std::string filename, int32 LL)
        {
            G->SaveBMP(filledsquare,drawaxes,filename,LL);
        }


    /*********************************************************************
    * Print some stats about the walk
    *********************************************************************/
    std::string stats()
        {
            std::string s;
            s += "*****************************************************\n";
            s += "Stats for the object OnceVRRW_Z2\n\n";
            s += " - reinforcement delta : " + mylib::tostring(GetDelta()) + "\n";
            s += " - number of steps performed : " + mylib::tostring(Steps()) + "\n";
            s += " - number of site visited    : " + mylib::tostring(Range()) + "\n";
            s += " - position of the walk : X = " + mylib::tostring(PosX()) + ",  Y = " + mylib::tostring(PosY()) + "\n";
            s += " - min-max values : [ " + mylib::tostring(MinX()) + " , " + mylib::tostring(MaxX()) + " ] x [ " + mylib::tostring(MinY()) + " , " + mylib::tostring(MaxY()) + " ]\n\n";
            return s;
        }


    /*********************************************************************
    * Print detailled stats about walk + object
    *********************************************************************/
    std::string detailled_stats()
        {
            return(stats() + G->Stats() + "\n");
        }

    /*********************************************************************
    * Return the actual position of the walk
    *********************************************************************/
    inline int64 PosX() const {return x;}
    inline int64 PosY() const {return y;}

    /*********************************************************************
    * Return the encercling rectangle containing the range of the walk
    *********************************************************************/
    inline int64 MinX() const {return G->MinX();}
    inline int64 MaxX() const {return G->MaxX();}
    inline int64 MinY() const {return G->MinY();}
    inline int64 MaxY() const {return G->MaxY();}

    /*********************************************************************
    * Return the number of steps performed by the walk
    *********************************************************************/
    uint64 Steps() const {return n;}

    /*********************************************************************
    * Return the number of site visited by the walk
    *********************************************************************/
    uint64 Range() const {return G->NbSet();}

    /*********************************************************************
    * return true if a particular site has been visited
    *********************************************************************/
    bool isVisited(int64 px,int64 py) const {return G->Get(px,py);}

private:

    /* No copy */
    OnceVRRW_Z2(const OnceVRRW_Z2 &);
    OnceVRRW_Z2 & operator=(const OnceVRRW_Z2 &);

    uint64 n;           // number of step;
    int64 x,y;          // position of the walk;
    double d;           // delta value;
    mylib::mathgraph::BitGraphZ2<N> * G;  // the graph
    randomgen * gen;    // random number generator;
};

}
}
#endif 

/* end of file OnceVRRW.h */

