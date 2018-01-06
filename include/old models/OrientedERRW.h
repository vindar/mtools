/****************************************************************************************************
* TEMPLATE CLASS OrientedERRW                                              Version 1.0, Vindar 2011 *
*                                                                                                   *
* Used for simulating a 2D Oriented Edge Reinforced Random Walk                                     *
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




template<class randomgen,int32 N = 100> class OrientedERRW
{
public:

	/* structure of a site */
	struct SiteOERRW
		{
		uint64 up,down,left,right; // number of visit of each directed edge
		uint64 lastvisit;		   // time of the last visit of the site
		uint64 firstvisit;		   // time of the first visit of the site
		uint64 v() const {return up+down+left+right;} // the total number of visit of the site
		};

	/* ctor */
	OrientedERRW(int memoryMB,randomgen * generator)
		{
		gen = generator;	// the random generator
		G = new	RW_Z2Site<SiteOERRW,100>(memoryMB); // the lattice
		}

	/* dtor */
	~OrientedERRW()
		{
		delete G;
		}



private:

	RW_Z2Site<SiteOERRW,100> * G;   // the Z2 lattice of Site
	MT2004_64  * gen;		   // the random number generator

	volatile uint64 range;
	volatile uint64 N;
	uint64 maxv;				// max de visite en 1 site

	CImg<unsigned char> im;    // the site image


};



}
}
#endif 

/* end of file OrientedERRW.h */

