/****************************************************************************************************
* TEMPLATE CLASS RW_Z2Site                                                 Version 4.0, Vindar 2011 *
*                                                                                                   *
* STATUS: WORKING COMPLETELY !                                                                      * 
*                - Tested for 32 and 64 bits implementation ?                              YES !    *
*                - Can work with up to 1TB of memory ?                                     YES !    *
*                - Everything is sure to be OK as long as there is no exception thrown ?   YES !    *  
*                                                                                                   *
* PURPOSE: Used for representing a walk on Z2, Z2 is decomposed of little squares which are         *
*          allocated as needed and ordered in a custom RB tree. VERY FAST:                          *
*             - move to an arbitrary location (x,y) in logarithmic time.                            *
*             - move to a neighbor in amortized constant time (almost as fast a a C array !)        *
*                                                                                                   *
*          When all the memory is exhausted, a cleaning up is done by removing the squares which    *
*          are farther away from the actual location. Later on, if one tries to access the value    *
*          at a location previously erased, an exception is thrown.                                 *
*                                                                                                   *
* CLASS PARAMETERS:                                                                                 *
*    - T                    : The type of obj attached to each site, MUST BE SIMPLE C STRUCT        *
*                             -> there must not be memory allocation and ctor/dtor shold be empty   *
*                                (initialization is done by memset(0) or operator=, see below)      *
*                                                                                                   *
*    - uint32 N             : Z2 is decomposed of subsquare of size NxN, (N=100 by default)         *
*                                                                                                   *
*    - bool useInitFct     : default false: each element of type T is initialized with 0's by memset*
*                            if set to true, T must implement : const T & operator=(int64 x,int64 y)*
*                            which should handle the initialization of the data at site (x,y)       *
* METHODS                                                                                           *
*        - Reset                                                                                    *
*    READING/SETTING VALUES                                                                         *
*        - Value (2 versions)                                                                       *
*        - ValueUp                                                                                  *
*        - ValueDown                                                                                *
*        - ValueLeft                                                                                *
*        - Valueright                                                                               *
*        - PeekValue*   <- peek the value of a site: this method is THREADSAFE and can be called    *
*                          from another thread than that which created/uses the object !            *
*    MOVING IN THE LATTICE                                                                          *
*        - MoveUp                                                                                   *
*        - MoveDown                                                                                 *
*        - MoveRight                                                                                *
*        - MoveLeft                                                                                 *
*        - Move                                                                                     *
*    INFORMATION ABOUT THE WALK                                                                     *
*        - NbSteps                                                                                  *
*        - GetX                                                                                     *
*        - GetY                                                                                     *
*        - GetMinX                                                                                  *
*        - GetMaxX                                                                                  *
*        - GetMinY                                                                                  *
*        - GetMaxY                                                                                  *
*        - PrintInBMP (4 versions)                                                                  *
*        - Stats()                                                                                  *
*                                                                                                   *
* EXAMPLE:                                                                                          *
*                                                                                                   *
#include <mylib_all.h>

void colorfct(const int & val,int & r,int & g,int & b) 
  {r = 0; g = 0; b = 255; if (val !=0) return;  r=255; g = 255; return;}

int main (int argc, char *argv[])
{
     RW_Z2Site<int,100> G(1000);
     MT2004_64 gen;
     for(int i=0;i<100000000;i++)
        {
            G.Value() = 1;
            double a = gen.rand_double0();
            if (a < 0.25) G.MoveUp(); else
            if (a < 0.5)  G.MoveDown(); else
            if (a < 0.75) G.MoveLeft(); else G.MoveRight();
        }
     G.PrintInBMP<colorfct>(true,"RW.bmp",1000);

     return 0;
}
*                                                                                                   *
* standalone, no cpp file                                                                           *
****************************************************************************************************/

#ifndef _RW_Z2SITE_H_
#define	_RW_Z2SITE_H_

/* headers */
#include <string>
#include "crossplatform.h"
#include "customexc.h"
#include "imageBMP.h"
#include "stringfct.h"
#include "logfile.h"
#include <windows.h>

namespace mylib
{
namespace mathgraph
{

    /***************************************************
    * template class
    ****************************************************/
    template<class T,uint32 N = 100,bool useInitFct = false> class RW_Z2Site
    {
    public:

        /*******************************************************
        * Constructor
        * - SizeMB the quantity of memory to use in RAM
        *******************************************************/
        RW_Z2Site(int32 SizeMB)
            {
			initCriticalSection();		// initialize the critical section for thread safe access
            CreateMemoryPool(SizeMB);   // create the memory pool;
            Reset(0,0);                 // reset the object, set the walk at the origin (0,0);
            }

        ~RW_Z2Site() {DestroyMemoryPool(); deleteCriticalSection();}


        /*******************************************************
        * Reset the object to an empty graph and set position
        * of the walk to (x,y)
        *******************************************************/
        void Reset(int64 x,int64 y)
            {
				enterCriticalSection();
                DeallocateAllBlocks();                              // free all memory
                ResetForbiddenBlocks();                             // reset all forbiddden blocks
				resetLastPeekedBlock();								// reset the last peeked block
                ResetCleanup();                                     // Reset data concerning memory cleanup
                ResetLastBlock();                                   // reset the lastblocks
                nbstep = 0;                                         // no step done
                Xblock = GetBlockCoord(x); Xr = x - Xblock;         // the position is (x,y), find
                Yblock = GetBlockCoord(y); Yr = y - Yblock;         // the block position and the offset
                ExMinX = x; ExMaxX = x; ExMinY = y; ExMaxY = y;     // the extremal positions
                treeroot = NULL;                                    // create a null tree
                actblock = Find(Xblock,Yblock);                     // create the first node of the tree and get its position
				leaveCriticalSection();
            }


        /*******************************************************
        * Query the number of step performed
        * each call to a Move fct (including Move(int x,int y))
        * counts as a step
        *******************************************************/
        inline uint64 NbSteps() const {return nbstep;}


        /*******************************************************
        * Query of the actual position (X,Y) of the walk
        *******************************************************/
        inline int64 GetX() const {return(Xblock+Xr);}
        inline int64 GetY() const {return(Yblock+Yr);}


        /*******************************************************
        * Query of the extremal value for the positions
        * i.e. min/max value for X/Y of all position since the
        * last reset
        *******************************************************/
        inline int64 GetMinX() const {return ExMinX;}
        inline int64 GetMaxX() const {return ExMaxX;}
        inline int64 GetMinY() const {return ExMinY;}
        inline int64 GetMaxY() const {return ExMaxY;}


        /*******************************************************
        * Query of the value at the actual position
        *******************************************************/
        inline const T & Value() const {return GetData(actblock)->tab[Xr + (N*Yr)];}
        inline       T & Value()       {return GetData(actblock)->tab[Xr + (N*Yr)];}


        /*******************************************************
        * Query of the value at a neigbour of the position
        * (a bit slower tan thhe value at the position but not
        * by much)
        *******************************************************/
        inline       T & ValueUp()     {sMoveUp();    T & V = Value(); sMoveDown();  return(V);}  
        inline       T & ValueDown()   {sMoveDown();  T & V = Value(); sMoveUp();    return(V);}      
        inline       T & ValueLeft()   {sMoveLeft();  T & V = Value(); sMoveRight(); return(V);}      
        inline       T & ValueRight()  {sMoveRight(); T & V = Value(); sMoveLeft();  return(V);}       


        /*******************************************************
        * Move the walk 1 step in a direction (very fast)
        *
        * WARNING: after this call, all reference to values 
        * queried previously are invalidated.
        *******************************************************/
        inline void MoveUp()           {nbstep++; if (GetY() == ExMaxY) {ExMaxY++;} if (Yr < (N-1)) {Yr++; return;} if (NbFreeBlocks() < 5) {MakeRoomMemory();} Yr=0;   Yblock += N; actblock = Find(Xblock,Yblock);}
        inline void MoveDown()         {nbstep++; if (GetY() == ExMinY) {ExMinY--;} if (Yr > 0)     {Yr--; return;} if (NbFreeBlocks() < 5) {MakeRoomMemory();} Yr=N-1; Yblock -= N; actblock = Find(Xblock,Yblock);}
        inline void MoveRight()        {nbstep++; if (GetX() == ExMaxX) {ExMaxX++;} if (Xr < (N-1)) {Xr++; return;} if (NbFreeBlocks() < 5) {MakeRoomMemory();} Xr=0;   Xblock += N; actblock = Find(Xblock,Yblock);}
        inline void MoveLeft()         {nbstep++; if (GetX() == ExMinX) {ExMinX--;} if (Xr > 0)     {Xr--; return;} if (NbFreeBlocks() < 5) {MakeRoomMemory();} Xr=N-1; Xblock -= N; actblock = Find(Xblock,Yblock);}


        
        /*******************************************************
        * Move the walk to a new position (x,y)
        * this is a bit slower than moving 1 step.
        *
        * WARNING: after this call, all reference to values 
        * queried previously are invalidated.
        *******************************************************/
        inline void Move(int64 x,int64 y) 
            {
            nbstep++;
            if (x > ExMaxX) {ExMaxX = x;} else {if (x < ExMinX) {ExMinX = x;}}
            if (y > ExMaxY) {ExMaxY = y;} else {if (y < ExMinY) {ExMinY = y;}}
            int64 BX = GetBlockCoord(x); Xr = x - BX;
            int64 BY = GetBlockCoord(y); Yr = y - BY;
            if ((BX != Xblock)||(BY != Yblock)) 
                {
                if (NbFreeBlocks() < 5) {MakeRoomMemory();}
                Xblock = BX;
                Yblock = BY;
                actblock = Find(Xblock,Yblock);
                }
            }


        /**********************************************************
        * Peek the value at the position (x,y)
        * - return a pointer to the value if already created
        * - return NULL if the site was never created or if it was
        *   destroyed.
        *   - site was destroyed: set destroyed to true
        *   - site was never created: set destroyed to false;
		*
		* THIS METHOD IS THREADSAFE: it can be called from another
		* thread that that which is using the object (for example
		* a DynamicLatticePlotter) and is pretty fast. 
        **********************************************************/
        inline const T * PeekValue(int64 x,int64 y,bool & destroyed) const
            {
            int64 BX = GetBlockCoord(x); int64 BY = GetBlockCoord(y); 
			size_t posO = PeekLastBlock(BX,BY);
			if (posO < nb_blocks) {int64 ox = x - BX; int64 oy = y - BY; return(&(GetData(posO)->tab[ox + (N*oy)]));}
	        if (posO == nb_blocks)     {destroyed = false; return NULL;}
		    if (posO == (nb_blocks+1)) {destroyed = true;  return NULL;} 
			size_t posN = DoesNodeExist(BX,BY,false); 
			SetLastPeekedBlock(BX,BY,posN);
		    if (posN == nb_blocks)     {destroyed = false; return NULL;}
			if (posN == (nb_blocks+1)) {destroyed = true;  return NULL;} 
			int64 ox = x - BX; int64 oy = y - BY;
			return (&(GetData(posN)->tab[ox + (N*oy)]));
            }



        /*******************************************************
        * PRINTING IN BMP
        *
        * Print a part of the graph in a BMP file
        *
        * - void ColorFct(const T &,int32 &,int32 &,int32 &) 
        *   -> this is the conversion function from T to a color
        *
        * - (LX,LY) : size of the image to save
        * - [xmin,xmax]x[ymin,ymax] : the rectangle to print
        * - drawaxe = true for drawing the axes (color black)
        *
        *  Note: There is no "lissage", and the color are s follow
        *   
        * - Non created site -> color cian (0,255,255)
        * - Destroyed site -> color violet (102,0,153)
        *
        * This function alos creates a log with the same name 
        * as the image (but ext .txt) containing informations
        * the characteristics of the image;
        *******************************************************/        
        template<void ColorFct(const T &,int32 &,int32 &,int32 &)> void PrintInBMP(bool drawaxes,std::string filename,int64 xmin,int64 xmax,int64 ymin,int64 ymax,int32 LX,int32 LY) const
        {
            if ((xmin >= xmax)||(ymin >= ymax)) {mylib::customexc e(0,"RW_Z2Site::PrintInBMP, xmin >= xmax or ymin >= ymax !"); e.raise();}
            if ((LX<1)||(LY<1)) {mylib::customexc e(0,"RW_Z2Site::PrintInBMP, dimension LX or LY too small !"); e.raise();}
            if ((LX > 100000)||(LY > 100000)) {mylib::customexc e(0,"RW_Z2Site::PrintInBMP, dimension LX or LY too large !"); e.raise();}
            mylib::ImageSave * Img;
            try {Img = new mylib::ImageSave(filename,LX,LY); if (Img==NULL) {throw(1);}}
            catch(...) {mylib::customexc e(0,"RW_Z2Site::PrintInBMP, not enough memory for creating image file!"); e.raise();}
            for(int32 j=0;j<LY;j++) {for(int32 i=0;i<LX;i++)
                {
                int64 xm = xmin + (int64)(((double)(xmax-xmin+1))*((double)i)/((double)LX)); 
                int64 ym = ymin + (int64)(((double)(ymax-ymin+1))*((double)j)/((double)LY));
                int64 xn = xmin + (int64)(((double)(xmax-xmin+1))*((double)(i+1))/((double)LX)); 
                int64 yn = ymin + (int64)(((double)(ymax-ymin+1))*((double)(j+1))/((double)LY));
                if ((drawaxes == true)&&(((xm==0)||(ym==0))||(((xm<0)&&(xn>0))||((ym<0)&&(yn>0))))) {Img->Add(0,0,0);}
                else
                    {
                    bool destroyed;
                    const T * v = PeekValue(xm,ym,destroyed);
                    if (v == NULL) {if (destroyed) {Img->Add(102,0,153);} else {Img->Add(0,255,255);}} 
                    else {int32 r,g,b; ColorFct(*v,r,g,b); Img->Add(r,g,b);}
                    }
                }}
            delete Img;
            mylib::logger ll(filename + ".txt",false,true);
            ll << "\nRW_Z2Site::PrintInBMP\n";
            ll << "- log for the file [" << filename << "]\n";
            ll << "- size of the image : " << LX << " x " << LY << "\n";
            ll << "- representing the rectangle : [" << xmin << "," << xmax << "] X [" <<  ymin << "," << ymax << "]\n";
            if ((LX == (xmax -xmin+1))&&(LY == (ymax -ymin+1))) {ll << "- 1 to 1 mapping : 1 pixel = 1 site of Z^2 !\n";}
            else 
                {
                int asr = (int)((((double)(ymax-ymin+1))*((double)(LX)))/(((double)(xmax-xmin+1))*((double)(LY)))*1000.0);
                ll << "- aspect ratio X:Y is " <<  ((double)(asr))/1000.0  << " : 1\n";
                }
            ll << "At the time of the saving of the bitmap, the stats were :\n" << Stats() << "\n";
        }


        /*******************************************************
        * PRINTING IN BMP : 1 to 1 CORRESPONDENCE
        * Same as before but with :
        *
        * - Each site correpsond to exactly 1 pixel.
        * - The image has size (xmax-xmin+1) x (ymax -ymin+1)
        *******************************************************/
        template<void ColorFct(const T &,int32 &,int32 &,int32 &)> void PrintInBMP(bool drawaxes,std::string filename,int64 xmin,int64 xmax,int64 ymin,int64 ymax) const
        {
            if ((xmax - xmin > 100000)||(ymax - ymin > 100000)) {mylib::customexc e(0,"RW_Z2Site::PrintInBMP, Image too large (1 to 1 mapping) !"); e.raise();}
            if ((xmin >= xmax)||(ymin >= ymax)) {mylib::customexc e(0,"RW_Z2Site::PrintInBMP, xmin >= xmax ou ymin >= ymax !"); e.raise();}
            int32 LX = (int32)(xmax - xmin +1);
            int32 LY = (int32)(ymax - ymin +1);
            PrintInBMP<ColorFct>(drawaxes,filename,xmin,xmax,ymin,ymax,LX,LY);
        }


        /*******************************************************
        * PRINTING IN BMP : WITH ASPECT RATIO RESPECTED
        *
        * As before but with 1:1 proportions
        *
        * - the size of the bitmap is chosen such that the X
        *   and Y proportion are 1:1 and the largest dimension 
        *   of the bitmap is LL
        *******************************************************/
        template<void ColorFct(const T &,int32 &,int32 &,int32 &)> void PrintInBMP(bool drawaxes,std::string filename,int64 xmin,int64 xmax,int64 ymin,int64 ymax,int32 LL) const
        {
            if ((LL < 2)||(LL > 100000)) {mylib::customexc e(0,"RW_Z2Site::PrintInBMP, dimension LL incorrect !"); e.raise();}
            int64 lx = xmax -xmin;
            int64 ly = ymax -ymin;
            if ((lx < 1)||(ly < 1)) {mylib::customexc e(0,"RW_Z2Site::PrintInBMP, xmin >= xmax or ymin >= ymax !"); e.raise();}
            if (lx > ly)
                {
                    int64 b = (int64)(((double)LL)*(((double)(ly))/((double)(lx)))); if (b<1) {b=1;}
                    PrintInBMP<ColorFct>(drawaxes,filename,xmin,xmax,ymin,ymax,LL,(int32)b);
                }
            else
                {
                    int64 a = (int64)(((double)LL)*(((double)(lx))/((double)(ly)))); if (a<1) {a=1;}
                    PrintInBMP<ColorFct>(drawaxes,filename,xmin,xmax,ymin,ymax,(int32)a,LL);
                }
        }


        /*******************************************************
        * PRINTING IN BMP : AUTOMATIC ZOOM
        *
        * - 1:1 proportion
        * - largest dimension of image LL.
        * - automatically encompass all visited sites (plus a small border)
        *******************************************************/
        template<void ColorFct(const T &,int32 &,int32 &,int32 &)> void PrintInBMP(bool drawaxes,std::string filename,int LL) const
        {
            if (LL < 10)     {mylib::customexc e(0,"RW_Z2Site::PrintInBMP, dimension LX or LY too small !"); e.raise();}
            if (LL > 100000) {mylib::customexc e(0,"RW_Z2Site::PrintInBMP, dimension LX or LY too large !"); e.raise();}
            int64 xmin = (GetMinX() - 1) - ((GetMaxX()-GetMinX())/20);
            int64 xmax = (GetMaxX() + 1) + ((GetMaxX()-GetMinX())/20);
            int64 ymin = (GetMinY() - 1) - ((GetMaxY()-GetMinY())/20);
            int64 ymax = (GetMaxY() + 1) + ((GetMaxY()-GetMinY())/20);
            PrintInBMP<ColorFct>(drawaxes,filename,xmin,xmax,ymin,ymax,LL);
        }


        /*******************************************************
        * Print some statistics about the object in a std::string
        *******************************************************/
        std::string Stats() const
        {
        std::string s;
        s += "***********************************************************\n";
        s += "RW_Z2Site object statistics\n\n";
        s += "- Memory allocated          : " + mylib::tostring(((sizeof(Node)+sizeof(size_t)+sizeof(Data))*(nb_blocks+1))/(1024*1024)) + "Mb\n";
        s += "- Size of a block           : " + mylib::tostring(N) + " x " + mylib::tostring(N) + "  (" + mylib::tostring(sizeof(Node)+sizeof(size_t)+sizeof(Data)) + " octets each)\n";
        s += "- blocks in use             : " + mylib::tostring(nb_blocks - NbFreeBlocks()) + " / " + mylib::tostring(nb_blocks) + "   (" + mylib::tostring((int)(((double)(nb_blocks - NbFreeBlocks())*100.0)/((double)nb_blocks))) + "%)\n";
        s += "- number of cleanup done    : " + mylib::tostring(nbMemCleanup) + "\n";
        s += "- number of block destroyed : " + mylib::tostring(nbBlockDestroyed) + "\n";
        s += "- Number of steps           : " + mylib::tostring(NbSteps()) + "\n";
        s += "- Position of the walk      : X = " + mylib::tostring(GetX()) + "   Y = " + mylib::tostring(GetY()) + "\n";
        s += "- Encercling rectangle      : [" + mylib::tostring(GetMinX()) + "," + mylib::tostring(GetMaxX()) + "] X [" + mylib::tostring(GetMinY()) + "," + mylib::tostring(GetMaxY()) + "]\n";
        if (treeroot == NULL) {s += "- tree is empty (treerooot = NULL) !!!\n";}
        else {s += "- depth of the tree         : " + mylib::tostring(treeroot->depth()) + "\n";}
        s += "***********************************************************\n";
        return s;
        }




/*********************************************************************************************************************
*                                                                                                                    *
* PRIVATE PART: IMPLEMENTATION OF THE CLASS                                                                          *
*                                                                                                                    *
*********************************************************************************************************************/

    private:

        RW_Z2Site(const RW_Z2Site &);                 // no copy
        RW_Z2Site & operator=(const RW_Z2Site &);     //

        /* Data structure */
        struct Data {T tab[N*N];};



    /*******************************************************************************************************************************************************
     * Some Helpers functions
     *******************************************************************************************************************************************************/


        /********************************************************************
        * MoveXXX versions which no not record the step (used by ValueXXX())
        * and do not perform memory cleanup
        ********************************************************************/
        inline void sMoveUp()              {if (Yr < (N-1)) {Yr++; return;} Yr=0; Yblock += N; actblock = Find(Xblock,Yblock);}
        inline void sMoveDown()            {if (Yr > 0) {Yr--; return;} Yr=N-1; Yblock -= N; actblock = Find(Xblock,Yblock);}
        inline void sMoveRight()           {if (Xr < (N-1)) {Xr++; return;} Xr=0; Xblock += N; actblock = Find(Xblock,Yblock);}
        inline void sMoveLeft()            {if (Xr > 0) {Xr--; return;} Xr=N-1; Xblock -= N; actblock = Find(Xblock,Yblock);}


        /******************************************************************** 
         * Initialize the datas of the block at position pos and coord x,y
         *******************************************************************/


		template<bool uif> inline void InitializeData(int64 x,int64 y,size_t pos);

		template<> inline void InitializeData<false>(int64 x,int64 y,size_t pos) {memset(GetData(pos),0,sizeof(Data)); return;}

		template<> inline void InitializeData<true>(int64 x,int64 y,size_t pos)
			{
			 Data * d = GetData(pos);
             for(size_t i=0;i<N;i++)
				{
                for(size_t j=0;j<N; j++)
                    {
					T & site = d->tab[i + (N*j)];
					site.init(x+i,y+j);					// T must implement .init(int64 x,int64 y) since useInitFct = true
                    }
                }
			}

        /******************************************************************** 
        * Calculate the coordinate of the block associated withh a position
        * then, thhe offset inside the block is simply  x - GetBlockCoord(x)
        ********************************************************************/
        inline int64 GetBlockCoord(int64 x) const {int64 BX; if (x<0) {BX = -1-((-x-1)/((int64)N));} else {BX = x/((int64)N);} BX*= N; return BX;}



    /*******************************************************************************************************************************************************
     * The Node Structure
     *******************************************************************************************************************************************************/
    struct Node
        {
        public: 
            /* getter /setter for the father */
            inline Node*  getfather() const         {return((Node *)(father & (~((size_t)3))));}
            inline void   setfather(Node* adr)      {father = (father&((size_t)3)) | ((size_t)adr);}
            /* getter /setter for the left son */
            inline Node*  getleft() const           {return left;}
            inline void   setleft(Node* adr)        {left = adr;}
            /* getter /setter for the right son */
            inline Node*  getright() const          {return right;}
            inline void   setright(Node* adr)       {right = adr;}
            /* getter setter for the color */
            inline bool   isblack() const           {return((father &((size_t)2))==0);}
            inline bool   isred() const             {return((father &((size_t)2))!=0);}
            inline void   setblack()                {father &= (~((size_t)2));}
            inline void   setred()                  {father |= ((size_t)2);}
            /* comparison operators */
            inline bool   IsSmallerThan(int64 x,int64 y) const  {return((X < x)||((X == x)&&(Y < y)));}
            inline bool   IsEqual(int64 x,int64 y) const        {return((X == x)&&(Y == y));}
            /* return the distance of the Node to position (x,y) */
            inline int64  dist(int64 x,int64 y)             {int64 u = X-x; if (u<0) {u = -u;} int64 v = Y-y; if (v<0) {v = -v;} if (u>v) {return u;} return v;}
            /* return the brother of the node */
            inline Node*  brother() const                   {Node * f = getfather(); if (f == NULL) {return NULL;} if (this == f->getleft()) {return f->getright();} return f->getleft();}
            /* return the grandfather of the Node */
            inline Node*  grandfather() const               {if (getfather() != NULL) {return getfather()->getfather();} return NULL;}
            /* return the uncle of the Node */
            inline Node*  uncle() const                     {Node * g = grandfather(); if (g == NULL) {return NULL;} if (getfather() == g->getleft()) {return g->getright();} return g->getleft();}
            /* return next when treated as a one sided list */
            inline Node* & next()                           {return right;}
            /* performs a left rotation of the subtree */
            inline void LeftRotation(Node* & newRoot)
            {
                Node* O = getfather();
                Node* D = getright();
                Node* C = D->getleft();
                if (O == NULL) {newRoot = D;} else {if (O->getleft() == this) {O->setleft(D);} else {O->setright(D);}}
                setfather(D);
                setright(C);
                D->setfather(O);
                D->setleft(this);
                if (C!=NULL) {C->setfather(this);}
            }                     
            /* same with a right rotation */
            inline void RightRotation(Node* & newRoot)
            {
                Node* O = getfather();
                Node* D = getleft();
                Node* C = D->getright();
                if (O == NULL) {newRoot = D;} else {if (O->getleft() == this) {O->setleft(D);} else {O->setright(D);}}
                setfather(D);
                setleft(C);
                D->setfather(O);
                D->setright(this);
                if (C!=NULL) {C->setfather(this);}
            }
            /* Initialise the node with given father, no son and color red */
            inline void Initialize(int64 x,int64 y, Node * pere)
            {
                setfather(pere); setleft(NULL); setright(NULL); setred();
                X = x; Y = y;
            }
            /* Return the depth of the subtree rooted at this node 
             * recursive (but no problem for balanced trees) */
            size_t depth() const
            {
                size_t L1 = 0;
                size_t L2 = 0;
                if (getleft() != NULL)  {L1 = getleft()->depth();}
                if (getright() != NULL) {L2 = getright()->depth();}
                if (L1 < L2) {return(L2+1);} 
                return(L1+1);
            }
        /* Datas of the Node */
        public:
            int64 X;                // X position of the block
            int64 Y;                // Y position of the block
        private:
            size_t   father; 
            Node*    left;   
            Node*    right;  
        };



    /*******************************************************************************************************************************************************
     * The variables of the object
     *******************************************************************************************************************************************************/
    Node * treeroot;                   // the root of the tree
    size_t actblock;                   // position of the actual block
    int64 Xblock,Yblock;               // position of the block
    int64 Xr,Yr;                       // relative position inside the block
    int64 ExMinX,ExMaxX,ExMinY,ExMaxY; // extremal positions 
    uint64 nbstep;                     // number of step performed;



    /*******************************************************************************************************************************************************
     * Access / Modification of the Red-Black tree
     *******************************************************************************************************************************************************/


        /******************************************************************** 
         * Find the Node with coordinate (x,y) and return its position      *
         * - return nb_blocks if the node was never created                 *
         * - return nb_blocks+1 if the node was destroyed                   *
         *******************************************************************/
        size_t DoesNodeExist(int64 x,int64 y,bool uselastbloc = true) const
        {
            if (IsBlockForbidden(x,y)) {return(nb_blocks+1);}   // check if the block is forbidden
			enterCriticalSection();
			if (uselastbloc) {size_t remp = FindInLastBlock(x,y); if (remp != nb_blocks) {return remp;}} // try fast search first if required
			Node * N = treeroot; // fall back to complete search
            while(1)
                {
				if (N->IsEqual(x,y)) {if (uselastbloc) {RotateLastBlock(NodePos(N));} leaveCriticalSection(); return(NodePos(N));}
                if (N->IsSmallerThan(x,y)) {if (N->getright() != NULL) {N = N->getright();} else {leaveCriticalSection(); return nb_blocks;}}
                                      else {if (N->getleft()  != NULL) {N = N->getleft(); } else {leaveCriticalSection(); return nb_blocks;}}
                }
        }


        /******************************************************************** 
         * Find the Node with coordinate (x,y) and return its position      *
         * in the memory pool. Create it if it does no exist                *
         *                                                                  *
         * - Raise an exception if the block is forbidden                   *
         * - Raise an exception if out of memory                            *
         * - may change the value of treeroot                               *
         * - do not invalidate any pointer (it just add some new stuff in   *
         *   memory but do not remove anything)                             *
         *******************************************************************/
        size_t Find(int64 x,int64 y)
            {
            if (IsBlockForbidden(x,y)) {mylib::customexc e(0,"RW_Z2Site::Find, trying to access a forbidden block !"); e.raise();}   // check if the block is forbidden
			enterCriticalSection();
            if (treeroot == NULL) {size_t pos = AllocateBlock(); InitializeData<useInitFct>(x,y,pos); treeroot = GetNode(pos); treeroot->Initialize(x,y,NULL); treeroot->setblack(); leaveCriticalSection(); return pos;} // it the tree is empty, simply create it and set the root black
            size_t remp = FindInLastBlock(x,y); if (remp != nb_blocks) {leaveCriticalSection(); return remp;} // try fast search...
            Node * N = treeroot;
            while(1)
                {
                if (N->IsEqual(x,y)) {leaveCriticalSection(); return NodePos(N);}
                if (N->IsSmallerThan(x,y)) 
                    {
                    if (N->getright() != NULL) {N = N->getright();}
                    else
                        {
                        size_t pos = AllocateBlock();    // get a position for the block in the memory pool (MAY RAISE AN EXCEPTION)
                        Node * NewNode = GetNode(pos);
                        NewNode->Initialize(x,y,N);     // initialize the node
                        InitializeData<useInitFct>(x,y,pos);// initialize the Data
                        N->setright(NewNode);           // attach it as right son of N
                        RectifyTree(NewNode);           // rectify the tree so that it is again a RB tree
                        RotateLastBlock(pos);           // note the new block as recently seen
						leaveCriticalSection(); 
                        return pos;
                        }
                    }
                else 
                    {
                    if (N->getleft() != NULL) {N = N->getleft();}
                    else
                        {
                        size_t pos = AllocateBlock();    // get a position for the block in the memory pool (MAY RAISE AN EXCEPTION)
                        Node * NewNode = GetNode(pos);
                        NewNode->Initialize(x,y,N);// initialize the node
                        InitializeData<useInitFct>(x,y,pos);// initialize the Data
                        N->setleft(NewNode);            // attach it as left son of N
                        RectifyTree(NewNode);           // rectify the tree so that it is again a RB tree
                        RotateLastBlock(pos);           // note the new block as recently seen
						leaveCriticalSection(); 
                        return pos;
                        }
                    }
                }
            }


        /******************************************************************** 
         * Rectify the tree after insertion of a red Node at position N     *
         *******************************************************************/
        void RectifyTree(Node* N)
            {
            Node* F;
            Node* U;
            Node* G;
            startRectifyTree: 
            F = N->getfather();                     // father of the node
            if (F == NULL) {N->setblack(); return;} // CASE 1: N is the root 
            if (F->isblack()) {return;}             // CASE 2: the father exist and is black, nothing to do 
            G = F->getfather();                     // the grandfather exist (not null !)
            U = F->brother();                       // the uncle (may be null)
            if ((U!=NULL)&&(U->isred()))            // CASE 3: N, F and U are red
                {
                F->setblack(); U->setblack();
                G->setred();
                N = G; 
                goto startRectifyTree;
                }
            G->setred();                            // CASE 4:  1 or 2 rotations are needed
            if (F == G->getleft())
                {
                if (N == F->getright()) 
                    {
                    N->setblack();
                    F->LeftRotation(treeroot);
                    G->RightRotation(treeroot);
                    }
                else
                    {
                    F->setblack();
                    G->RightRotation(treeroot);
                    }
                }
            else
                {
                if (N == F->getleft()) 
                    {
                    N->setblack();
                    F->RightRotation(treeroot);
                    G->LeftRotation(treeroot);
                    }
                else
                    {
                    F->setblack();
                    G->LeftRotation(treeroot);
                    }
                }
            }




/*******************************************************************************************************************************************************
 *                Fast search Method (remember a certain number of last block so that one can avoid looking a the whhole tree
 *
 * THIS PART SELF-SUFFICIENT: INDEPENDANT OF EVERYTHING ELSE (i.e. can be changed as long as the interface remain the same)
 *******************************************************************************************************************************************************/
    static const size_t nblastblock = 8;        // number of block keept in memory for fast searrch (must not be too large or it becomes slow)
    mutable size_t lastblock[nblastblock];      // the list of last block for fast search
    mutable bool  islastblock;                  // true if there is some lastblock

    /****************************************
    * Empty the fast search list and add only
    * the block with at pos
    *****************************************/
    inline void ResetLastBlock() const
        {
            islastblock = false;
        }

    /****************************************
    * Add newblock to the fast search list
    *****************************************/
    inline void RotateLastBlock(size_t newblock) const
        {
            if (islastblock == false) {for(int i=0;i<nblastblock;i++) {lastblock[i]=newblock;} islastblock = true; return;}
            for(size_t i=0;i<(nblastblock-1);i++) {lastblock[i] = lastblock[i+1];}
            lastblock[nblastblock-1] = newblock;
        }

    /****************************************
    * search for a block with given position
    * in the fast search list.
    * - return its position if found
    * - return nb_blocks if it cannot be found
    *****************************************/
    inline size_t FindInLastBlock(int64 x,int64 y) const
        {
            if (islastblock == false) {return(nb_blocks);}
            for(size_t i=0;i< nblastblock;i++) {if (GetNode(lastblock[i])->IsEqual(x,y)) {return lastblock[i];}}
            return(nb_blocks);
        }






/*******************************************************************************************************************************************************
 * Memory cleanup, removing half the block farther away from the current position
 *
 * THIS PART SELF-SUFFICIENT: INDEPENDANT OF EVERYTHING ELSE (i.e. can be changed as long as the interface remain the same)
 *******************************************************************************************************************************************************/
    uint64 nbMemCleanup;               // number of memory cleanup which have been done
    uint64 nbBlockDestroyed;           // number of block destroyed by memory cleanup

    /*****************************************
    * Init the variables for the cleanup process
    *****************************************/
    void ResetCleanup()
    {
        nbMemCleanup = 0;
        nbBlockDestroyed = 0;
    }

    /*****************************************
    * Reorder the (simply) linked list of Node 
    * in increasing order with respect to 
    * the distance of the node to (x,y)
    * return the new first node of the list
    * (use in place merge sort on linked list)
    *****************************************/
    Node * OrderList(Node * list,int64 refX,int64 refY) 
        {
        Node *p, *q, *e, *tail;
        int64 insize, nmerges, psize, qsize, i;
        insize = 1;
        while (1) 
            {
            p = list;
            list = NULL;
            tail = NULL;
            nmerges = 0;
            while(p != NULL) 
                {
                nmerges++; q = p; psize = 0;
                for (i = 0; i < insize; i++)  {psize++; q = q->next(); if (q == NULL) {break;} }
                qsize = insize;
                while((psize > 0) || (qsize > 0 && q)) 
                    {
                    if (psize == 0)                                         {e = q; q = q->next(); qsize--;} 
                    else if ((qsize == 0) || (q==NULL))                     {e = p; p = p->next(); psize--;} 
                    else if ((p->dist(refX,refY)) <= (q->dist(refX,refY)))  {e = p; p = p->next(); psize--;} 
                    else                                                    {e = q; q = q->next(); qsize--;}
		            if (tail != NULL) {tail->next() = e;} else {list = e;}
		            tail = e;
                    }
                p = q;
                }
	        tail->next() = NULL;
            if (nmerges <= 1) return list;
            insize *= 2;
            }
        }

    /*****************************************
    * Remove about half of the block in memory
    * keeping those that are closer to actblock
    * - all the block deleted are added with
    *   the AddForbiddenBlock ()
    * - reset the lastblock fast search
    *****************************************/
    void MakeRoomMemory()
        {
			enterCriticalSection();
            // check that actblock correspond to the block (Xblock,Yblock)
            Assert(((GetNode(actblock)->X) == Xblock)&&((GetNode(actblock)->Y) == Yblock));

            // check that there is less than 5 block free in memory
            uint64 nbf = NbFreeBlocks();
			Assert(nbf <= 4); // make sure there are at most 4 block still free

            // clear the fastblock buffer
            ResetLastBlock();
			// clear the last peeked block 
			resetLastPeekedBlock();

            // create a simply linked list of all the site allocated starting at startNode..
            size_t ii =0;
            while(IsBlockAllocated(ii)== false) {ii++;}
            Node * startNode = GetNode(ii);
            Node * prevN = startNode; ii++; size_t listsize = 1;
            while(ii<nb_blocks) {if (IsBlockAllocated(ii) == true) {prevN->next() = GetNode(ii); prevN = GetNode(ii); listsize++;} ii++;} prevN->next() = NULL;
            Assert(listsize>=10);
            Assert(listsize == (nb_blocks - NbFreeBlocks()));

            // order the list with respect to the position of actblock
            startNode = OrderList(startNode,Xblock,Yblock);

            // check that actblock is now the first element of the list
            Assert(startNode == GetNode(actblock));

            // cut the list after the first half, make N point on the first element of the second half
            Node * N = startNode; for(size_t j=0;j< (listsize/2);j++) {prevN = N; N = N->next();} prevN->next() = NULL;

            // iterate over the second half of the list: for each element, call AddForbiddenBlock  and deallocate the block
            while(N != NULL) {Node * nextN = N->next(); AddForbiddenBlock(N->X,N->Y); DeallocateBlock(NodePos(N)); N = nextN;}

            // check that startnode is not inside the new extended forbidden region
            if (IsBlockForbidden(Xblock,Yblock)) {leaveCriticalSection(); mylib::customexc e(0,"RW_Z2Site::MakeRoomMemory, the actual position has been destroyed !"); e.raise();}

            // make startNode the root of the new RB tree and set N the second element of the list
            N = startNode->next();
            treeroot = startNode; treeroot->setfather(NULL); treeroot->setleft(NULL); treeroot->setright(NULL); treeroot->setblack();

            // iterate over the remaining of the first half of the list. 
            // - add to the RB tree each node not in the forbidden region
            // - deallocate each node in the forbidden region
            while(N != NULL)
                {
                // save the next element
                Node * nextN = N->next();
                // check if it is in the forbidden region 
                if (IsBlockForbidden(N->X,N->Y)) {DeallocateBlock(NodePos(N));}
                else
                    {
                    // prepare the node for insertion
                    N->setleft(NULL); N->setright(NULL); N->setred();
                    // find where it should be inserted in the tree and insert it
                    Node * P = treeroot;
                    while(P != NULL)
                        {
						Assert(!(P->IsEqual(N->X,N->Y))); // strange, 2 block with same coordinates
                        if (P->IsSmallerThan(N->X,N->Y)) {if (P->getright() != NULL) {P = P->getright();} else {P->setright(N); N->setfather(P); P = NULL;}}
                        else {if (P->getleft() != NULL)  {P = P->getleft();}  else {P->setleft(N);  N->setfather(P); P = NULL;}}
                        }
                    // rectify the tree
                    RectifyTree(N);
                    }
                // go to the next element
                N = nextN;
                }

            // update the data about cleanup
            nbMemCleanup++;
            int64 dest = NbFreeBlocks() - nbf;
			Assert(dest>=5); // make sure enough block hae been freed
            nbBlockDestroyed += dest;
			leaveCriticalSection();
            return;
        }

/*******************************************************************************************************************************************************
 * Dealing with thing in the threadsafe way
 *
 * THIS PART SELF-SUFFICIENT: INDEPENDANT OF EVERYTHING ELSE (i.e. can be changed as long as the interface remain the same)
 *******************************************************************************************************************************************************/
	mutable CRITICAL_SECTION CS;

    /****************************************
    * Initialize the critical section
    *****************************************/
	inline void initCriticalSection() {InitializeCriticalSection(&CS);}

    /****************************************
    * Remove the critical section
    *****************************************/
	inline void deleteCriticalSection() {DeleteCriticalSection(&CS);}

    /****************************************
    * Get ownership of the critical section
    *****************************************/
	inline void enterCriticalSection() const {EnterCriticalSection(&CS);}

    /****************************************
    * release ownership of the critical section
    *****************************************/
	inline void leaveCriticalSection() const {LeaveCriticalSection(&CS);}


/*******************************************************************************************************************************************************
 * Acceleration of the PeekValue() method by remembering the last block used
 *
 * THIS PART SELF-SUFFICIENT: INDEPENDANT OF EVERYTHING ELSE (i.e. can be changed as long as the interface remain the same)
 *******************************************************************************************************************************************************/

	mutable int64 prevpeekBX,prevpeekBY;
	mutable volatile size_t prevpeekN;

	/*********************************************************
	* Reset the last peeked block
	*********************************************************/
	inline void resetLastPeekedBlock() const
		{
		prevpeekN = nb_blocks+2; prevpeekBX = 0; prevpeekBY = 0;
		}

	/*********************************************************
	* Return the last peeked block if equal to (BX,BY)
	* return nb_blocks if not
	*********************************************************/
	inline size_t PeekLastBlock(int64 BX,int64 BY) const  
		{
		return(((prevpeekN<(nb_blocks+2))&&(BX == prevpeekBX)&&(BY == prevpeekBY))  ? prevpeekN : nb_blocks+2);
		}
	
	/*********************************************************
	* Set the last peeked block
	*********************************************************/
	inline void SetLastPeekedBlock(int64 BX,int64 BY,size_t Npos) const 
		{
		prevpeekBX = BX; prevpeekBY = BY; prevpeekN = Npos;
		}
	




/*******************************************************************************************************************************************************
 * Managing forbidden blocks.
 *
 * THIS PART SELF-SUFFICIENT: INDEPENDANT OF EVERYTHING ELSE (i.e. can be changed as long as the interface remain the same)
 *******************************************************************************************************************************************************/
    int64 ForbidXmin,ForbidXmax,ForbidYmin,ForbidYmax;

    /****************************************
    * Reset so that no block is forbidden
    *****************************************/
    void ResetForbiddenBlocks()
    {
        ForbidXmin = 1; ForbidXmax = -1;
        ForbidYmin = 1; ForbidYmax = -1;
        return;
    }

    /****************************************
    * Query whether a block at pos (x,y) is 
    * forbidden. return true if forbidden
    *****************************************/
    inline bool IsBlockForbidden(int64 x,int64 y) const
    {
        if (((x >= ForbidXmin)&&(x <= ForbidXmax))&&((y >= ForbidYmin)&&(y <= ForbidYmax))) {return true;}
        return false;
    }
   
    /****************************************
    * Add that the block at pos (x,y) is 
    * now forbidden.
    *****************************************/
    inline void AddForbiddenBlock(int64 x,int64 y)
    {
        if (ForbidXmax < ForbidXmin) {ForbidXmin = x; ForbidXmax = x; ForbidYmin = y; ForbidYmax = y; return;}
        if (x < ForbidXmin) {ForbidXmin = x;} else {if (ForbidXmax < x) {ForbidXmax = x;}}
        if (y < ForbidYmin) {ForbidYmin = y;} else {if (ForbidYmax < y) {ForbidYmax = y;}}
        return;
    }




/*******************************************************************************************************************************************************
 * Memory Managment function
 *
 * THIS PART SELF-SUFFICIENT: INDEPENDANT OF EVERYTHING ELSE (i.e. can be changed as long as the interface remain the same)
 *******************************************************************************************************************************************************/
    size_t   nb_blocks;	    // number of blocks in the pool
    size_t   first_free;    // all the block after this on are free
    size_t   stackindex;	// position of the stack index
    size_t*  stack;	        // stack of free blocks
    Node*    bufferNode;    // the memory buffer for the nodes
    Data*    bufferData;    // the memory buffer for the datas

    /****************************************
    * Create a memory pool of size SizeMB MB
    * return the number of block allocated
    *****************************************/
    size_t CreateMemoryPool(int32 SizeMB)
        {
        int arch = checkimplementation();
        if (SizeMB<1) {mylib::customexc e(0,"RW_Z2Site::CreateMemoryPool : Cannot allocate for <1 MB of RAM !"); e.raise();}
        if (SizeMB > 100000) {mylib::customexc e(0,"RW_Z2Site::CreateMemoryPool : SizeMB too large !"); e.raise();}
        if ((arch == 32)&&(SizeMB > 2047)) {mylib::customexc e(0,"RW_Z2Site::CreateMemoryPool : Cannot allocate more than 2047MB of RAM in 32 bit mode !"); e.raise();}
        nb_blocks = (((size_t)SizeMB)*1024*1024)/(sizeof(Node)+sizeof(size_t)+sizeof(Data));
        if (nb_blocks < 16) {mylib::customexc e(0,"RW_Z2Site::CreateMemoryPool : SizeMB too small to allocate aat least 16 blocks !"); e.raise();}
        try {
            stack      = new size_t[nb_blocks]; 
            bufferNode = new Node[nb_blocks]; 
            bufferData = new Data[nb_blocks];
            if ((stack == NULL)||(bufferNode == NULL)||(bufferData == NULL)) throw(1);            
            }
        catch(...) {mylib::customexc e(0,"RW_Z2Site::CreateMemoryPool : Out of memory !"); e.raise();}
        DeallocateAllBlocks();
        return(nb_blocks);
        }

    /****************************************
    * Destroy the memory pool
    *****************************************/
    void DestroyMemoryPool()
        {
        delete [] bufferData;
        delete [] bufferNode;
        delete [] stack;        
        }

    /****************************************
    * Free all blocks in the memory pool
    *****************************************/
    inline void DeallocateAllBlocks()
        {
        for(size_t j=0;j<nb_blocks;j++) {stack[j] = 0;}
	    first_free = 0;
	    stackindex = 0;
        }

    /****************************************
    * Deallocate a block (from its number)
    * no cecking of corrrectness
    ****************************************/
    inline void DeallocateBlock(size_t pos)
        {
		Assert(IsBlockAllocated(pos));
        stack[pos] &= (~((size_t)1));
	    if (pos == (first_free-1)) {first_free--; return;}
	    stack[stackindex] = (((stack[stackindex])&((size_t)1)) | (pos << 1));
	    stackindex++;
        }

    /****************************************
    * Allocate a block (from its number)
    ****************************************/
    inline size_t AllocateBlock()
        {
        if (first_free < nb_blocks) {size_t pos = first_free; first_free++; stack[pos] |= ((size_t)1); return(pos);}
        if (stackindex == 0) {mylib::customexc e(0,"RW_Z2Site::AllocateNode : No more free block in the Memory pool !"); e.raise();}
	    stackindex--;
        size_t pos = ((stack[stackindex]) >> 1);
        stack[pos] |= ((size_t)1);
	    return(pos);
        }

    /****************************************
    * Return the number of free block
    *****************************************/
    inline size_t NbFreeBlocks() const {return(stackindex + (nb_blocks-first_free));}

    /****************************************
    * Return true if block number No is already
    * allocated
    *****************************************/
    inline bool IsBlockAllocated(size_t No) const 
        {
        Assert(No < nb_blocks);
        return(((stack[No])&((size_t)1))!=0);
        }

    /****************************************
    * Return a pointer to the Node 
    * corresponding to its number
    ****************************************/
    inline Node * GetNode(size_t pos) const 
        {
        Assert(pos < nb_blocks);
        return(bufferNode + pos);
        }

    /****************************************
    * Return a pointer to the Data
    * corresponding to its number
    ****************************************/
    inline Data * GetData(size_t pos) const 
        {
		Assert(pos < nb_blocks);
        return(bufferData + pos);
        }

    /****************************************
    * Return a the position of a given node
    * in the memory pool
    ****************************************/
    inline size_t NodePos(Node* N) const 
        {
        size_t res = (size_t)(N-bufferNode);
		Assert(res < nb_blocks);
        return(res);
        }

};


}
}

#endif
/* end of file RW_Z2Site.h */


