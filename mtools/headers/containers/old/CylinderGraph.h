/****************************************************************************************************
* TEMPLATE CLASS CylinderGraph                                             Version 3.2, Vindar 2011 *
*                                                                                                   *
* USED FOR REPRESENTING THE GRAPH Zx(Z/LZ)                                                          *
*                                                                                                   *
* - can simulate almost infinite size graph by swapping on the hard drive.                          *
* - can set an object (struct type) at each edge of each site (both not both at the same time,      *
*   except for the graph Z (ie L=1).                                                                *
* - the template parameter T represent the type of object stored, must be a struct or basic type    *
*   but not a real object since the ctor/dtor/copy ctor are not correctly called                    *
* - For example, for L=3, the class is used for representing the graph:                             *
*   (the edges up of height 2 are linked with the corresponding site of height 0)                   *
*                                                                                                   *
*   y                                                                                               *
*          | | | | | | | | | |                                                                      *
*   2     -o-o-o-o-o-o-o-o-o-o-                                                                     *
*          | | | | | | | | | |                                                                      *
*   1 ... -o-o-o-o-o-o-o-o-o-o- ...                                                                 *
*          | | | | | | | | | |                                                                      *
*   0     -o-o-o-o-o-o-o-o-o-o-                                                                     *
*                                                                                                   *
*         -1 0 1 2 3 4 5 6 7 ... X                                                                  *
*                                                                                                   *
*                                                                                                   *
* optimized for efficient query of the value of an edge/site.                                       *
*                                                                                                   *
* An simple example of usage:                                                                       *
*                                                                                                   *                  
#include "mylib_all.h"  

int main()                                                                                       
 {                                                                                                
     // create a graph G containing chars (zero initialized)                                      
     // with L=3, 100MB of RAM and 1GB of swap on HD                                              
     mathgraph::CylinderGraph<char> G(3,100,1024);                                               
     while(1==1)                                                                                  
         {                                                                                        
         out << G.ToStringByEdge().c_str() << "\n";                                     
         char a = input.getnormalkey();                                                 
         switch(a)                                                                                
             {                                                                                    
             case 'z': {G.EdgeUp() = 1; G.MoveUp(); break;}                                       
             case 's': {G.EdgeDown() = 1; G.MoveDown(); break;}                                   
             case 'q': {G.EdgeLeft() = 1; G.MoveLeft(); break;}                                   
             case 'd': {G.EdgeRight() = 1; G.MoveRight(); break;}                                 
             default: break;                                                                     
             }                                                                                   
         }                                                                                       
 }
*                                                                                                  *
* standalone, no cpp file                                                                          *
****************************************************************************************************/

#ifndef _CYLINDERGRAPH_H_
#define	_CYLINDERGRAPH_H_

/* headers */
#include <string>
#include <cstring>
#include <set>
#include <ctime>
#include <cstdio>

#include "crossplatform.h"
#include "customexc.h"

namespace mylib
{
namespace mathgraph
{


#if defined (_MSC_VER) 
#pragma warning (push)
#pragma warning (disable:4996)
#endif


/* the default functions */
template<typename T> inline void def_init_cylindergraph(int64 x,uint16 y,T & val) {val = 0;}
template<typename T> inline char def_printedgeup_cylindergraph(const T & val) {if (val == 0) {return ' ';} else {return '|';}}
template<typename T> inline char def_printedgeright_cylindergraph(const T & val) {if (val == 0) {return ' ';} else {return '-';}}
template<typename T> inline char def_printsite_cylindergraph(const T & val) {if (val == 0) {return '.';} else {return 'X';}}



/* the class template parameters
 *
 * - T : type (or struct) that is associated with edge edge / site of the graph.
 *
 * - void initup(int64 x,uint16 y,T & obj): function that is called when initialising the object obj corresponding
 *                                          to the edge E = [(x,y) <-> ((y+1)%L,x)]. this function should modifiy obj to
 *                                          initialize it. Called only once for each edge E (no need to remember the
 *                                          value supplied before).
 *                                          NOTE: when storing value by site, this function coorepsond to initialising the
 *                                          object obj at site (x,y).
 *                                          NOTE2: if no function is supplied, the default behavior is zero initialization
 *                                          for object that support obj = 0 assertion.
 *
 * - void initright(int64 x,uint16 y,T & obj): similar as above, but for initialising the object obj corresponding to the edge
 *                                             E =[(x,y) <-> (x+1,y)].
 *                                             NOTE: When storing by site, this does not really matter.
 *                                             NOTE: If no function is supplied, the default behaviour is to to use the same
 *                                             function as that provided for initup above (or default init_up if no initup
 *                                             function is supplied)
 */
template<typename T = int, void initup(int64,uint16,T &) = def_init_cylindergraph<T>, void initright(int64,uint16,T &) = initup >
class CylinderGraph
{

public:

    /*****************************************************************************************************
     * constructor of the class                                                                          *
     * - L : the main parameter, graph ZxZ/LZ                                                            *
     * - SizeRam_MB: size in MB of RAM that will be used by the object (cannot be 0)                     *
     * - SizeSwap_MB: size in MB of swap on hard drive that is allowed (0 = no swapping on HD)           *
     *                                                                                                   *
     * throw an exception is construction fails, çotherwise, everything is OK and the graph is usable    *
     * the starting position is (0,0), to change that, use the reset function                            *
     *****************************************************************************************************/
    CylinderGraph(uint16 L,uint16 SizeRam_MB,uint16 SizeSwap_MB = 0) {init(L,SizeRam_MB,SizeSwap_MB);}

    /* destructor of the class */
    ~CylinderGraph()
    {
        clearenvironment();; // remove all the files on the HD
        delete [] maintab_L; // release allocated memory
    }

    /*****************************************************************************************************
     * reset the environment and set the new initial position of the walk                                *
     *                                                                                                   *
     * this puts the object back to the same state as afer construction. Much faster than creating a     *
     * new object                                                                                        *
     *****************************************************************************************************/
    void reset(int64 startposX,uint16 startposY)
    {
        clearenvironment();         // remove all the files on the HD
        createnewfileidentifers();  // create some new identifiers for the files
        maintab_offset = startposX - NN; //position of the memory buffer
        xx = NN;    // offset of the walk inside the memory buffer
        yy = (size_t)(startposY%LL); // height of the walk
        minX = startposX;  // minimal position obtained
        maxX = startposX;  // maximal position obtained
        if (loadenvironment(maintab_L,maintab_offset)!=0) {mylib::customexc e(0,"cylindergraph::reset(), error loading left part"); e.raise();} // load the left side of the memory buffer
        if (loadenvironment(maintab_R,maintab_offset+NN)!=0) {mylib::customexc e(1,"cylindergraph::reset(), error loading right part"); e.raise();} // load the right side of the memory buffer
    }

    /*****************************************************************************************************
     * return the maximum number of files that can be used for swapping (0 if no swapping allowed)       *
     *****************************************************************************************************/
    inline int MaxSwapFiles() {return((int)max_files);}


    /*****************************************************************************************************
     * return the number of file used at the moment for swapping                                         *
     *****************************************************************************************************/
    inline int NbSwapFiles() {return((int)set_env.size());}


    /*****************************************************************************************************
     * return the minimal number on move to the left which are garanted to succeed                       *
     * no exception will be thrown before                                                                *
     *****************************************************************************************************/
    inline int64 MinLeftMoveGaranted() {return ((int64)(xx-1));}

    /*****************************************************************************************************
     * return the minimal number on move to the right which are garanted to succeed                      *
     * no exception will be thrown before                                                                *
     *****************************************************************************************************/
    inline int64 MinRightMoveGaranted() {return ((int64)((2*NN - 2)-xx));}

    /*****************************************************************************************************
     * return the minimal number on move (in any directions) which are garanted to succedd               *
     * no exception can be thrown before                                                                 *
     *****************************************************************************************************/
    inline int64 MinMoveGaranted() {int64 r = MinRightMoveGaranted(); int l = MinLeftMoveGaranted(); return ((r<l) ? r : l);}
      
    /*****************************************************************************************************
     * return the X position of the walker                                                               *
     *****************************************************************************************************/
    inline int64 GetPosX() const {return(maintab_offset +(int64)xx);}

    /*****************************************************************************************************
     * return the Y position of the walker (in 0,L-1)                                                    *
     *****************************************************************************************************/
    inline uint16 GetPosY() const {return((uint16)yy);}

    /*****************************************************************************************************
     * return the minimal X position obtained by the walker since the last reset                         *
     *****************************************************************************************************/
    inline int64 GetMinX() const {return(minX);}

    /*****************************************************************************************************
    * return the maximal X position obtained by the walker since the last reset                          *
     *****************************************************************************************************/
    inline int64 GetMaxX() const {return(maxX);}



    
    /*****************************************************************************************************
     * move a step up (cannot throw an exception, always succeds)                                        *
     *****************************************************************************************************/
    inline uint16 MoveUp()                {yy = ((++yy)%LL); return (uint16)yy;}

    /*****************************************************************************************************
     * move a step down (cannot throw an exception, always succeds)                                      *
     *****************************************************************************************************/
    inline uint16 MoveDown()              {yy = ((yy + LL-1)%LL); return (uint16)yy;}

    /*****************************************************************************************************
     * change the y position of the walker (cannot throw an exception, always succeds)                   *
     *****************************************************************************************************/
    inline uint16 MoveVertical(uint16 y)  {yy = ((size_t)y)%LL; return (uint16)yy;}

    /*****************************************************************************************************
     * move a step left                                                                                  *
     * if it is not possible, throws an exception of type mylib::customexc (object not usable anymore)     *
     *****************************************************************************************************/
    inline int64 MoveLeft()
    {
        if (xx == 1) {if (shiftleft()!=0) {mylib::customexc e(2,"cylindergraph::MoveLeft(), cannot move the the left"); e.raise(); }}
        --xx;
        int64 newposX = maintab_offset + (int64)xx; // the new position
        if (newposX < minX) {minX = newposX;}  // update minX if necessary
        return newposX;
    }

    /*****************************************************************************************************
     * move a step right                                                                                 *
     * if it is not possible, throw an exception of type mylib::customexc (object not usable anymore)      *
     *****************************************************************************************************/
    inline int64 MoveRight()
    {
        if (xx == (2*NN - 2)) {if (shiftright()!=0) {mylib::customexc e(3,"cylindergraph::MoveRight(), cannot move the the right"); e.raise();}}
        ++xx;
        int64 newposX = maintab_offset + (int64)xx; // the new position
        if (newposX > maxX) {maxX = newposX;}  // update maxX if necessary
        return newposX;
    }


    /*****************************************************************************************************
     * QUERY OF THE VALUE OF SITE / EDGE.                                                                *
     * - very fast function, the result is return by reference/const reference                           *
     * - the reference is sure to be valid until at least the next move to the left or to the right      *
     *****************************************************************************************************/

    /* value of the site at the actual position */
    inline       T &  Site()         {return(maintab_L[(xx*LL) + yy].up);}
    inline const T &  Site() const   {return(maintab_L[(xx*LL) + yy].up);}

    /* value of a site of same X position */
    inline       T &  Site(uint16 y)        {return(maintab_L[(xx*LL) + (((size_t)y)%LL)].up);}
    inline const T &  Site(uint16 y) const  {return(maintab_L[(xx*LL) + (((size_t)y)%LL)].up);}

    /* value of the 4 neigbouring site */
    inline       T &  SiteUp()            {return(maintab_L[(xx*LL) + ((yy+1)%LL)].up);}
    inline const T &  SiteUp()    const   {return(maintab_L[(xx*LL) + ((yy+1)%LL)].up);}
    inline       T &  SiteDown()          {return(maintab_L[(xx*LL) + ((yy+LL-1)%LL)].up);}
    inline const T &  SiteDown()  const   {return(maintab_L[(xx*LL) + ((yy+LL-1)%LL)].up);}
    inline       T &  SiteRight()         {return(maintab_L[((xx+1)*LL) + yy].up);}
    inline const T &  SiteRight() const   {return(maintab_L[((xx+1)*LL) + yy].up);}
    inline       T &  SiteLeft()          {return(maintab_L[((xx-1)*LL) + yy].up);}
    inline const T &  SiteLeft()  const   {return(maintab_L[((xx-1)*LL) + yy].up);}

    /* value of the edge around the actual position */
    inline       T & EdgeUp()                  {return(maintab_L[(xx*LL) + yy].up);}
    inline const T & EdgeUp()     const        {return(maintab_L[(xx*LL) + yy].up);}
    inline       T & EdgeDown()                {return(maintab_L[(xx*LL) + ((yy+LL-1)%LL)].up);}
    inline const T & EdgeDown()   const        {return(maintab_L[(xx*LL) + ((yy+LL-1)%LL)].up);}
    inline       T & EdgeRight()               {return(maintab_L[(xx*LL) + yy].right);}
    inline const T & EdgeRight()  const        {return(maintab_L[(xx*LL) + yy].right);}
    inline       T & EdgeLeft()                {return(maintab_L[((xx-1)*LL) + yy].right);}
    inline const T & EdgeLeft()   const        {return(maintab_L[((xx-1)*LL) + yy].right);}

    /* value of the edge around a site of same X position as the actual position */
    inline       T & EdgeUp(uint16 y)          {return(maintab_L[(xx*LL) + (((size_t)y)%LL)].up);}
    inline const T & EdgeUp(uint16 y)   const  {return(maintab_L[(xx*LL) + (((size_t)y)%LL)].up);}
    inline       T & EdgeDown(uint16 y)        {return(maintab_L[(xx*LL) + ((((size_t)y)+LL-1)%LL)].up);}
    inline const T & EdgeDown(uint16 y) const  {return(maintab_L[(xx*LL) + ((((size_t)y)+LL-1)%LL)].up);}
    inline       T & EdgeRight(uint16 y)       {return(maintab_L[(xx*LL) + (((size_t)y)%LL)].right);}
    inline const T & EdgeRight(uint16 y)const  {return(maintab_L[(xx*LL) + (((size_t)y)%LL)].right);}
    inline       T & EdgeLeft(uint16 y)        {return(maintab_L[((xx-1)*LL) + (((size_t)y)%LL)].right);}
    inline const T & EdgeLeft(uint16 y) const  {return(maintab_L[((xx-1)*LL) + (((size_t)y)%LL)].right);}


    /*************************************************************************
     * FOR TEST AND DEBUGGING PURPOSES                                       *
     * print a part of the graph in a std::string                            *
     * - S = print tha ladder on [posX-S,posX+S]                             *
     * - site_printfct: custom function for the char associated with a site  *
     *   (by default, '.' if obj==0 and 'X' otherwise)                       *
     *                                                                       *
     * may throw an exception and invalid the object                         *
     *************************************************************************/
    std::string ToStringBySite(uint16 S = 10,
                               char site_printfct(const T &) = def_printsite_cylindergraph<T>)
    {
        if (S<1) {S=1;} // at least One site to print
        int64 oldmaxpos = maxX; int64 oldminpos = minX; // save old extremal position
        uint32 tx = 2*(2*S+1) + 2; uint32 ty = 2*LL; // dimension for the buffer
        char * buf;
        try {buf = new char[tx*ty + 1];} catch(...) {mylib::customexc e(4,"cylindergraph::ToStringBySite(), not enough memory"); e.raise();} // create the buffer
        if (buf == NULL) {mylib::customexc e(5,"cylindergraph::ToStringBySite(), not enough memory"); e.raise();}  // and check if the memory allocation succeded
        memset(buf,' ',tx*ty); buf[tx*ty]= 0; // clear the buffer
        for(uint32 j=0;j<ty;j++) {buf[j*tx + tx-1]='\n';} // add the endline
        for(uint32 i=0;i<S;i++)  {MoveLeft();} // move the the left by S steps
        // draw the configuration
        for(uint32 i=0;i<((2*S)+1);i++) {
        for(uint32 j=0;j<(uint32)LL;j++) {
            buf[tx*(2*j+1) + 2*(i+1)-1] = site_printfct(EdgeUp((uint32)(LL-1 - j)));
            } MoveRight();}
        for(uint32 i=0;i<(S+1);i++)  {MoveLeft();} // come back to the original position
        buf[tx*(2*(LL-1-GetPosY())+1) + (2*S + 1) -1]='['; // draw the actual position
        buf[tx*(2*(LL-1-GetPosY())+1) + (2*S + 1) +1]=']';
        maxX = oldmaxpos; minX = oldminpos; // restore the old extremas
        try { // copy the buffer inside a string
        std::string res = buf;
        delete [] buf;  // delete the buffer
        res+= "Position X = " + int64tostring(GetPosX()) + ", trace in [" + int64tostring(minX) + "," + int64tostring(maxX) + "]\n\n"; // append the position at the end of the string
        return res;
        } catch (...) {mylib::customexc e(6,"cylindergraph::ToStringBySite(), not enough memory"); e.raise();}
    }


    /*************************************************************************
     * AS ABOVE BUT FOR PRINTING BY EDGE                                     *
     *                                                                       *
     * may throw an exception and invalid the object                         *
     *************************************************************************/

    std::string ToStringByEdge(uint16 S = 10,
                               char edgeup_printfct(const T &) = def_printedgeup_cylindergraph<T>,
                               char edgeright_printfct(const T &) = def_printedgeright_cylindergraph<T>)
    {
        if (S<1) {S=1;} // at least One site to print
        int64 oldmaxpos = maxX; int64 oldminpos = minX; // save old extremal position
        uint32 tx = 2*(2*S+1) + 2; uint32 ty = (uint32)(2*LL); // dimension for the buffer
        char * buf = NULL;
        try {buf = new char[tx*ty + 1];} catch(...) {mylib::customexc e(7,"cylindergraph::ToStringByEdge(), not enough memory"); e.raise();} // create the buffer
        if (buf == NULL) {mylib::customexc e(8,"cylindergraph::ToStringByEdge(), not enough memory"); e.raise();}  // and check if the memory allocation succeded
        memset(buf,' ',tx*ty); buf[tx*ty]= 0; // clear the buffer
        for(uint32 j=0;j<ty;j++) {buf[j*tx + tx-1]='\n';} // add the endline
        for(uint32 i=0;i<S;i++)  {MoveLeft();} // move the the left by S steps
        // draw the configuration
        for(uint32 j=0;j<(uint32)LL;j++)  {buf[tx*(2*j+1)] = edgeright_printfct(EdgeLeft((uint32)(LL-1 - j)));}
        for(uint32 i=0;i<((2*(uint32)S)+1);i++) {
        for(uint32 j=0;j<(uint32)LL;j++) {
            buf[tx*(2*j+1) + 2*(i+1)] = edgeright_printfct(EdgeRight((uint32)(LL-1 - j)));
            buf[tx*(2*j) + 2*(i+1) -1]= edgeup_printfct(EdgeUp((uint32)(LL-1 - j)));
            buf[tx*(2*j+1) + 2*(i+1)-1]='o';
            } MoveRight();}
        for(uint32 i=0;i<((uint32)S+1);i++)  {MoveLeft();} // come back to the original position
        buf[tx*(2*(LL-1-GetPosY())+1) + (2*S + 1)]='X'; // draw the actual position
        maxX = oldmaxpos; minX = oldminpos; // restore the old extremas
        try { // copy the buffer inside a string
        std::string res = buf;
        delete [] buf;  // delete the buffer
        res+= "Position X = " + int64tostring(GetPosX()) + ", trace in [" + int64tostring(minX) + "," + int64tostring(maxX) + "]\n\n"; // append the position at the end of the string
        return res;
        } catch (...) {mylib::customexc e(9,"cylindergraph::ToStringByEdge(), not enough memory"); e.raise();}
        return "";
    }


private:

    /**************************************************************************************
     * PRIVATE IMPLEMENTATION                                                             *
     **************************************************************************************/

    typedef struct {T up; T right;} tab_el; // structure for containing the information of a site

    /* variables set only once in the ctor */
    int64 max_files;                        // maximum number of files allowed simultaneously (0 = no swapping on HD)
    size_t NN;                              // the length of the array is 2*N
    size_t LL;                              // the height of the array is L;
    tab_el * maintab_L;                     // the array itself left part
    tab_el * maintab_R;                     // the array itself right part

    /* variables used for the position of the walk */
    size_t xx;      // position X of the walk reltive to the begining of maintab
    size_t yy;      // position Y of the walk
    int64 minX;     // minimal position obtained by the walk
    int64 maxX;      // maximal position obtained by the walk
    int64 maintab_offset; // absolute position of the begining of the memory array

    /* variables for the set of environment already visited */
    int64 minloaded_env;
    int64 maxloaded_env;
    std::set<int64> set_env;

    /* variables used to create the file identifier*/
    int64 file_identifier1;
    int64 file_identifier2;

    void init(uint16 L,uint16 SizeRam_MB,uint16 SizeSwap_MB)
    {
        // check correct implementation of fixed sized type
        checkimplementation();
        // check that the parameters are acceptable
        if (L==0) {mylib::customexc e(10,"cylindergraph::init(), invalid L parameter!"); e.raise();}
        if (SizeRam_MB == 0)  {mylib::customexc e(11,"cylindergraph::init(), invalid SizeRam_MB parameter!"); e.raise();}
        LL = (size_t)L;  // save the height of the graph
        // check that a size_t can hold at least twice more that SizeSwap_MB mb of ram
        uint64 maxsizet = (uint64)(((size_t)-1)/2);
        uint64 request = ((uint64)SizeRam_MB)*1024*1024;
        if (request >= maxsizet) {mylib::customexc e(12,"cylindergraph::init(), SizeRam_MB too large for this architecture"); e.raise();}
        NN = ((size_t)request)/(2*sizeof(tab_el)*LL); // compute the value of NN
        if (NN < 3) {mylib::customexc e(13,"cylindergraph::init(), SizeRam_MB too small for this value of L"); e.raise();} // check that it is not too small
        max_files = (SizeSwap_MB/SizeRam_MB); // compute the number of files on the disk (cannot be more the 65535)
        if (max_files < 3) {max_files = 0;}  // at least 3 files, otherwise, do not swap
        try {maintab_L = new tab_el[2*LL*NN];} // create the memory buffer
        catch(...) {mylib::customexc e(14,"cylindergraph::init(), cannot allocate memory"); e.raise();}
        if (maintab_L == NULL) {mylib::customexc e(15,"cylindergraph::init(), cannot allocate memory"); e.raise();}
        maintab_R = maintab_L + (NN*LL); // note the position of the right part of the buffer
        reset(0,0); // reset the environment and put the walker at position (0,0)
    }

    /* make a shift to the right
     * return 0 if OK */
    int shiftright()
    {
        saveenvironment(maintab_L,maintab_offset); // save the left part of the environment
        maintab_offset = maintab_offset + (int64)NN; // change the offset of maintab
        xx = xx - NN;   // change the relative position of xx
        moveenvironment(maintab_R,maintab_L); // copy the right part of the environment onto the left part
        return loadenvironment(maintab_R,maintab_offset + (int64)NN);  // load the new right part environment and return the result
    }

    /* make a shift to the left
     * return 0 if OK */
    int shiftleft()
    {
        saveenvironment(maintab_R,maintab_offset + (int64)NN); // save the right part of the environment
        maintab_offset = maintab_offset - (int64)NN;     // change the offset of maintab
        xx = xx + NN;   // change the relative position of xx
        moveenvironment(maintab_L,maintab_R); // copy the left part of the environment onto the right part
        return loadenvironment(maintab_L,maintab_offset);  // load the new left part environment and return the result
    }

    /* reset the environment, erase all the files on disk */
    void clearenvironment()
    {
        for(std::set<int64>::iterator it = set_env.begin();it!=set_env.end();it++) {deletefile(*it);} // delete each file in the set
        set_env.clear();    // empty the set
        minloaded_env = 1;  maxloaded_env = 0; // set minloaded_env and maxloaded_env to the default value
        return;
    }

    /* create the initial environment at pos in the buffer tab_dest */
    void makeinitialenvironment(tab_el * tab_dest,int64 pos)
    {
        for(size_t x = 0;x < NN; x++) { for(size_t y = 0; y < LL; y++ ) { // init each edge of the array
                initup((int64)x + pos,(uint16)y,tab_dest[x*LL + y].up);
                initright((int64)x + pos,(uint16)y,tab_dest[x*LL + y].right); }}
    }

    /* load the environment at absolute position pos into the buffer pointed by tab_dest
       return 0 if OK */
    int loadenvironment(tab_el * tab_dest,int64 pos)
    {
        if (minloaded_env > maxloaded_env)  // is it the first time we load an environment ?
            {
            makeinitialenvironment(tab_dest,pos); // yes : load the initiale enviroment
            minloaded_env = pos; maxloaded_env = pos; // and set minloaded_env and maxloaded_env to the correct values
            return 0;
            }
        if ((pos < minloaded_env)||(pos > maxloaded_env)) // check if this environment was previoulsy created/loaded
            {
            makeinitialenvironment(tab_dest,pos); // no it is the first time: create the initial environment
            if (pos < minloaded_env) {minloaded_env = pos;} else {maxloaded_env = pos;} // and adjust set minloaded_env and maxloaded_env
            return 0;
            }
        if (max_files == 0) {return 1;} // if we do not use HD swapping, we cannot load the environment !
        if (set_env.find(pos) == set_env.end()) {return 1;} // not in the set, there it has been erased, cannot load !
        return loadfile(tab_dest,pos);  // load the environment from a file
    }

    /* Save the environment pointed by tab_source as the new environmentat absolute position pos. */
    void saveenvironment(tab_el * tab,int64 pos)
    {
        if (max_files == 0) {return;}   // if no saving on disk, don't do anything
        if ((set_env.size() == (size_t)max_files)&&(set_env.find(pos) == set_env.end())) // check if we need to delete a file before saving this one
            {
            int64 a = (*(set_env.begin())); // yes: remove the furthest files from pos
            int64 b = (*(set_env.rbegin()));
            int64 va = a-pos; if (va < 0) {va = -va;}
            int64 vb = b-pos; if (vb < 0) {vb = -vb;}
            if (va>vb) {deletefile(a); set_env.erase(a);} else {deletefile(b); set_env.erase(b);}
            }
        set_env.insert(pos);    // add the file in the list if it isn't already in it
        savefile(tab,pos);      // save on disk (overwrite if it already exists)
        return;
    }

    /* Copie the environment from source to dest */
    void moveenvironment(tab_el * source,tab_el * dest)
    {
        memcpy(dest,source,sizeof(tab_el)*LL*NN);
    }

    /* load the file corresponding to position pos into the buffer pointed 
     * by tab f size NN*LL elements of type tab_el
     * return 0 if OK */
    int loadfile(tab_el * tab,int64 pos)
    {
        FILE * handle = fopen(filename(pos).c_str(),"rb");
        if (handle==NULL) {return 1;}
        if (fread((void*)tab,sizeof(tab_el),LL*NN,handle)!= LL*NN) {fclose(handle); deletefile(pos); return 2;}
        if (fclose(handle)!=0) {deletefile(pos); return 3;}
        return 0;
    }

    /* save the buffer pointed by tab of size LL*NN elements of type type_el
     * into the file corresponding to position pos.
     * return 0 if OK */
    int savefile(tab_el * tab,int64 pos)
    {
        FILE * handle = fopen(filename(pos).c_str(),"wb");
        if (handle == NULL) {return 1;}
        if (fwrite((void*)tab,sizeof(tab_el),LL*NN,handle)!= LL*NN) {fclose(handle); deletefile(pos); return 2;}
        if (fclose(handle)!=0) {deletefile(pos); return 3;}
        return 0;
    }

    /* delete the file corresponding to position pos
     * return 0 if OK */
    int deletefile(int64 pos)
    {
        return remove(filename(pos).c_str());
    }

    /* Create new unique identifer for the files */
    void createnewfileidentifers()
    {
        file_identifier1 = (int64)(this);
        file_identifier2 = (int64)(time(NULL));
        return;
    }

    /* return the name of the file corresponding to the position pos */
    std::string filename(int64 pos)
    {
        std::string r = "CylinderGraph_";
        r += int64tostring(file_identifier1) + "_";
        r += int64tostring(file_identifier2) + "_";
        r += int64tostring(pos) + ".part";
        return r;
    }

    /* return a string containing the decimal representation of the int64 */
    std::string int64tostring(int64 val)
    {
        if (val==0) {return "0";}
        std::string r,v;
        if (val<0) {val=-val; v = "-";}
        while(val!=0) {r += ((char)('0' + (char)(val%10))); val = val/10;}
        std::string::reverse_iterator rit; for(rit=r.rbegin() ; rit < r.rend(); rit++) {v += *rit;}
        return v;
    }

    CylinderGraph(const CylinderGraph &);    // no copy contructor
    CylinderGraph & operator=(const CylinderGraph &); // no assignement operator
};


#if defined (_MSC_VER) 
#pragma warning (pop)
#endif

}
}

#endif
/* end of cylindergraph.h */

