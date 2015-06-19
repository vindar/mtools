/****************************************************************************************************
* TEMPLATE CLASS GrowingLatticeZ3                                          Version 1.0, Vindar 2014 *
*                                                                                                   *
* STATUS: WORKING COMPLETELY !                                                                      * 
*                - Tested for 32 and 64 bits implementation ?                              YES !    *
*                - Can work with up to 1TB of memory ?                                     YES !    *
*                - Everything is sure to be OK as long as there is no exception thrown ?   YES !    *
*                                                                                                   *
* PURPOSE:  Used for representing Z^3 which is divided recursevely into square each square being    *
*           divided into 3x3x3 subsquare until we arrive the the leaf node with respresent block of * 
*           the lattice of size (2R+1)x(2R+1)x(2R+1) (where R is the template int parameter).       *
*           The value stored at each site is of type T. There are two particular value of T which   *
*           must be provided at creation time of the lattice : fullValue and emptyValue             *
*             - emptyValue : The value by default at each site of the lattice at construction       *
*                            (or after reset). this superseed the ctor of T if any (T must be POD)  *
*             - fullValue :  A particular value of type T which represent a full site. Used for     *
*                            compression to remove full blocks.                                     *           
*                                                                                                   *
* MAIN ADVANDAGES                                                                                   *
*                                                                                                   *
*           -> Save some space by deleting the nodes which are totally full or totally empty.       *
*              Cleanup done in real time without additionnal time cost.                             *
*                                                                                                   *
*           -> The object can be deep copied / deep copy constructed.                               *
*                                                                                                   *
*           -> The object be be safely saved/loaded to file without additionnal memory consumption  *
*              (work with file larger than 4GB)                                                     *
*                                                                                                   *
*           -> the structure of the object allow to get quick estimate about size of rectangle      *
*              comprised of full site around a given site.                                          *
*                                                                                                   *
*           -> access time to read/write the value of a site is amortized constant when performing  *
*              a walk (ie moving by 1 step) and logarithmic otherwise. Very fast :)                 *
*                                                                                                   *
* TEMPLATE PARAMETERS                                                                               *
*                                                                                                   *
*    R : Each elementary subsquare of sites of Z^3 is of size (2R+1)x(2R+1)x(2R+1)                  *
*        Choose typically 5 <= R <= 50 (default R = 20)                                             *
*                                                                                                   *
*	 T : Type of element stored in each site of the lattice (default T = unsigned char)             *
*         - Must be POD! No memory allocation allowed.                                              *
*         - ctor is useless (assigned with emptyVal afterward)                                      *
*         - Do not count on : 1 site = 1 object of type T (some object T may be shared between      *
*    	    several sites with same value).                                                         *
*         - T must be comparable with == (to check if empty of full)                                *
*         - must be assignable with =                                                               *
*                                                                                                   *
* METHODS:                                                                                          *
*                                                                                                   *
*	- reset()						reset the lattice (put every site empty)                        *
*	- save()						save the lattice to a file                                      *
*	- load()						load the lattice from a file                                    *
*	- nbFullSites()					return the number of full sites                                 *
*	- nbNonEmptySites()				return the number of non empty sites                            *
*	- range()						return the range of non-empty sites                             *
*	- emptyValue()					return an element with empty value                              *
*	- fullValue()					return an element with full value                               *
*	- get()							get the value of a site                                         *
*	- set()							set the value of a site                                         *
*	- getEnclosingRectFull()		compute a rect of full site around a given site (basic vers.)   *
*   - getEnclosingNotZero()         as above but make sure the rectangle does not contain (0,0,0)   *
*	- stats()						information about the object                                    *
*	- memory()						information about the memory used                               *
*	- debugTree()					print the tree structure of the lattice in a string (debugging) *
*	- operator==()					compare with another lattice                                    *
*	- operator=() and copy ctor		copy a lattice                                                  *
*                                                                                                   *
*                                                                                                   *
*                                                                                                   *
*                                                                                                   *
* standalone, no cpp file but uses GLZ3internals.h                                                                      *
****************************************************************************************************/
#ifndef _GROWINGLATTICEZ3_H_
#define _GROWINGLATTICEZ3_H_

#include "crossplatform.h"
#include <stdio.h>
#include <string>
#include "mathgraph/GLZ3internals.h" // include all the internal code for the object with the node/leaf sub classes


namespace mylib
{
namespace mathgraph
{




/********************************************************************************************************************
*
*
* Class GrowingLatticeZ3
*
*
*********************************************************************************************************************/
template<class T = unsigned char,int R = 20> class GrowingLatticeZ3
{

			typedef GLZ3internals::basicsqr<T,R> * p_sqr;		// pointer types
			typedef GLZ3internals::leafsqr<T,R>  * p_leafsqr;	// 
			typedef GLZ3internals::nodesqr<T,R>  * p_nodesqr;	// 



public:


        /************************************************************************
        * ctor : the lattice is created initially empty (all site = emptyVal)
		*
		* - emptyV : an empty element
		* - fullV  : a full element
		*
        ************************************************************************/
		GrowingLatticeZ3(const T & emptyV,const T & fullV)
			{
			info.emptyVal = emptyV;		// save the model for empty value
			info.fullVal = fullV;		// and full value
			treenode = NULL;
			reset();
			}


        /************************************************************************
        * dtor
        ************************************************************************/
		~GrowingLatticeZ3()
			{
			if (treenode != NULL) {treenode->get_root()->destroytree(); treenode=NULL;}	// release all alocated memory if any
			}


        /************************************************************************
        * copy ctor : the object is obtained as a deep copy of GL
		*             so that *this and GL are completely independent
        ************************************************************************/
		GrowingLatticeZ3(const GrowingLatticeZ3<T,R> & GL)
			{
			treenode = new GLZ3internals::nodesqr<T,R>(*(GL.treenode->get_root())); 
			info = GL.info; // copy the additionnal info
			return;
			}


        /************************************************************************
        * assignement operator : copy the lattice from GL, erasing the current
		* one. 
		* deep copy : the two object are completely independent.
        ************************************************************************/
		GrowingLatticeZ3<T,R> & operator=(const GrowingLatticeZ3<T,R> & GL)
			{
			if (&GL == this) return(*this);
			treenode->get_root()->destroytree(); // release all allocated memory
			treenode = new GLZ3internals::nodesqr<T,R>(*(GL.treenode->get_root())); 
			info = GL.info; // copy the additionnal info
			return *this;
			}


        /************************************************************************
        * comparison operator : return true if the two lattices are equal ie 
		* if all sites have exactly the same value.
        ************************************************************************/
		bool operator==(const GrowingLatticeZ3<T,R> & GL)
			{
			return treenode->get_root()->compare(GL.treenode->get_root());
			}


		/************************************************************************
        * Reset the lattice setting all site to empty value
        ************************************************************************/
		void reset()
			{
			if (treenode != NULL) {treenode->get_root()->destroytree();}	// release all allocated memory if any
			treenode = new GLZ3internals::nodesqr<T,R>; // create the root node				
			info.nbF  = 0; info.nbNE = 0; info.Xmin = 0; info.Xmax = 0; info.Ymin = 0; info.Ymax = 0; info.Zmin = 0; info.Zmax= 0;// initialize the info
			}


		/************************************************************************
        * Save the lattice into a file in binary format
		* return false if an error occured
		*
		* - Works even if the file is larger than 4GB. 
		*
		* - File format is archtecture independent: same for 32 and 64 bit program :)
		*
		* does not use additionnal memory while doing the save
        ************************************************************************/
		bool save(std::string filename) const
			{
			FILE * hf = fopen(filename.c_str(),"wb");
			if (hf == NULL) return false;
			if (fwrite("glZ3!",1,5,hf) != 5) {fclose(hf); return false;} 
			uint64 tailleT = sizeof(T); if (fwrite(&tailleT,sizeof(uint64),1,hf) != 1) {fclose(hf); return false;}	
			uint64 vR = R; if (fwrite(&vR,sizeof(uint64),1,hf) != 1) {fclose(hf); return false;} 			
			if (fwrite(&info,sizeof(GLZ3internals::infoGLZ3<T>),1,hf) != 1) {fclose(hf); return false;}
			if (((p_nodesqr)(treenode->get_root()))->save(hf) == false) {fclose(hf);  return false;}
			if (fwrite("end!",1,4,hf) != 4) {fclose(hf); return false;}
			fclose(hf);
			return true;
			}


		/************************************************************************
        * Erase the current lattice and load it from a file
		* return false if an error occured (in this case, the lattice is empty)
		*
		* Works even if the file is larger than 4GB. 
		* does not use more memory than the size of the object to create
        ************************************************************************/
		bool load(std::string filename)
			{
			treenode->get_root()->destroytree(); treenode = NULL; 	// release all allocated memory
			FILE * hf = fopen(filename.c_str(),"rb");
			if (hf == NULL) {reset(); return false;}
			char begin[6]; memset(begin,0,6);			
			if (fread(begin,1,5,hf) != 5) {reset(); fclose(hf); return false;}
			if ((std::string(begin)) != std::string("glZ3!")) {reset(); fclose(hf); return false;}				
			uint64 tailleT=0,vR=0; // new file format, contains the size of T and R
			if (fread(&tailleT,sizeof(uint64),1,hf) != 1) {reset(); fclose(hf); return false;}
			if (fread(&vR,sizeof(uint64),1,hf) != 1) {reset(); fclose(hf); return false;}
			if ((tailleT != sizeof(T))||(vR != R)) {reset(); fclose(hf); return false;} // make sure the template param are correct
			if (fread(&info,sizeof(GLZ3internals::infoGLZ3<T>),1,hf) != 1) {reset(); fclose(hf); return false;}
			treenode = new GLZ3internals::nodesqr<T,R>;
			if (((p_nodesqr)treenode)->load(hf,NULL,&info) == false) {reset(); fclose(hf); return false;}
			char end[5]; end[0]= 0; end[1] = 0; end[2] = 0; end[3] = 0;
			if ((fread(end,1,5,hf) != 4)||(end[0] != 'e')||(end[1] != 'n')||(end[2] != 'd')||(end[3]!='!')) {reset(); fclose(hf); return false;}
			fclose(hf);
			return true;
			}


		/************************************************************************
        * Return the number of sites which are full
        ************************************************************************/
		inline int64 nbFullSites() const {return info.nbF;}


		/************************************************************************
        * Return the number of sites which are not empty
        ************************************************************************/
		inline int64 nbNonEmptySites() const {return info.nbNE;}


		/************************************************************************
        * Return a rectangle containing all the non empty sites
		*
		* more precisely, after the function return, evry non empty site (X,Y,Z) is
		* such that  (Xmin <= X <= Xmax) & (Ymin <= Y <= Ymax) & (Zmin <= Z <= Zmax)
		* (the rectangle returned may not be the smallest one if some site were
		* change back from non-empty to empty).
		*
		* If the lattice is empty, return false and set the rectangle to (0,0,0,0,0,0) 
        ************************************************************************/
		inline bool range(int64 & Xmin,int64 & Xmax,int64 & Ymin,int64 & Ymax,int64 & Zmin,int64 & Zmax) const 
			{
			if (info.nbNE == 0) {Xmin = 0; Xmax = 0; Ymin = 0, Ymax = 0;  Zmin = 0, Zmax = 0; return false;}
			Xmin = info.Xmin; Xmax = info.Xmax; Ymin = info.Ymin; Ymax = info.Ymax; Zmin = info.Zmin; Zmax = info.Zmax;
			return true;
			}


		/************************************************************************
        * Return an empty element of type T
        ************************************************************************/
		inline T emptyValue() const {return info.emptyVal;}


		/************************************************************************
        * Return a full element of type T
        ************************************************************************/
		inline T fullValue() const {return info.fullVal;}


		/************************************************************************
        * Get the value of the site at position (x,y)
        ************************************************************************/
		inline T get(int64 x,int64 y,int64 z) const {return treenode->get(x,y,z,treenode,&info);}


		/************************************************************************
        * Set the value of the site at position (x,y)
        ************************************************************************/
		inline void set(const T & v,int64 x,int64 y,int64 z) 
			{
			if (v != info.emptyVal)
				{
				if (info.nbNE == 0) {info.Xmin = x; info.Xmax = x; info.Ymin = y; info.Ymax = y; info.Zmin = z; info.Zmax = z;}
				else 
					{
					if (x < info.Xmin) {info.Xmin = x;} else if (x > info.Xmax) {info.Xmax = x;}
					if (y < info.Ymin) {info.Ymin = y;} else if (y > info.Ymax) {info.Ymax = y;}
					if (z < info.Zmin) {info.Zmin = z;} else if (z > info.Zmax) {info.Zmax = z;}
					}
				}
			treenode = treenode->set(v,x,y,z,&info);
			}


		/************************************************************************
		* Find a rectangle of full point in the lattice containing (X,Y,Z) and put 
		* the result in (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax)
		* 
		* - return false if (X,Y,Z) is not full 
		*                (in this case Xmin,Xmax,Ymin,Ymax,Zmin,Zmax are not modified)
		*
		* - return true  if (X,Y,Z) is indeed full
		*                 in this case, change Xmin,Xmax,Ymin,Ymax,Zmin,Zmax such that
	    *				 the rectangle contain (X,Y,Z) and all points (i,j,k) such 
		*				 that (Xmin <= i <= Xmax)&&(Ymin <= j <= Ymax)&&(Zmin <= k <= Zmax) are full.
		*
		* This method is very fast ! (as fast as the get method)
		* but the rectangle returned is not necessarily very good because (X,Y,Z)
		* may be very close to the boundary of the returned rectangle.
        ************************************************************************/
		inline bool getEnclosingRectFull(int64 X,int64 Y,int64 Z,int64 & Xmin,int64 & Xmax,int64 & Ymin,int64 & Ymax,int64 & Zmin,int64 & Zmax) const
			{
			return treenode->getEnclosingRectFull(X,Y,Z,Xmin,Xmax,Ymin,Ymax,Zmin,Zmax,treenode,&info);
			}



		/************************************************************************
		* Find a rectangle of full point in the lattice containing (X,Y,Z) but
		* not containing the origin (0,0,0). Put the result in (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax)
		* 
		* This is similar as getEnclosingRectFull() but the returned rectangle
		* is "truncated" in such way that it does not contain the origin (but still
		* contain (X,Y,Z) ).
		*
		* - Return false if (X,Y,Z) is not full or if (X,Y,Z) == (0,0,0). In this case,
        *                Xmin,Xmax,Ymin,Ymax,Zmin,Zmax are not modified
		*
		* - Return true otherwise and change Xmin,Xmax,Ymin,Ymax,Zmin,Zmax to the obtained 
		*               rectangle
		*
		* Same time complexity as getEnclosingRectFull(). 
        ************************************************************************/
		inline bool improvedEnclosingNotZero(int64 X,int64 Y,int64 Z,int64 & Xmin,int64 & Xmax,int64 & Ymin,int64 & Ymax,int64 & Zmin,int64 & Zmax) const
			{
			if ((X == 0)&&(Y == 0)&&(Z == 0)) return false;	// do nothing if at the origin
			if (!(treenode->getEnclosingRectFull(X,Y,Z,Xmin,Xmax,Ymin,Ymax,Zmin,Zmax,treenode,&info))) {return false;} // do nothing if site not full				
			if ((Xmin <= 0)&&(Xmax >= 0)&&(Ymin <= 0)&&(Ymax >=0)&&(Zmin <= 0)&&(Zmax >=0)) // 0 is inside the rectangle, truncate it !
				{
				if (abs(X) >= max(abs(Y),abs(Z))) {if (X>0) {Xmin = 1;} else {Xmax= -1;} return true;}
				if (abs(Y) >= max(abs(X),abs(Z))) {if (Y>0) {Ymin = 1;} else {Ymax= -1;} return true;}
				if (Z>0) {Zmin = 1;} else {Zmax= -1;}
				}
			return true;
			}




		/************************************************************************
		* Print some statistics about the object into an std::string
		*
		* - set debug to true to print the whole tree structure in the string
        ************************************************************************/
		std::string stats(bool debug = false) const
			{
			std::string s = "Growing Lattice Z3 statistics :\n";
			s += "  -> R = " + tostring(R) + " (each leaf is " + tostring(2*R+1) + "x" + tostring(2*R+1) + "x" + tostring(2*R+1) + " sites)\n";
			s += "  -> T = object of size " + tostring(sizeof(T)) + " bytes\n";
			s += "  -> Number of site non empty = " + tostring(nbNonEmptySites()) + "\n"; 
			s += "  -> Number of site full      = " + tostring(nbFullSites()) + "\n"; 
			int64 xmin,xmax,ymin,ymax,zmin,zmax;
			if (range(xmin,xmax,ymin,ymax,zmin,zmax)) {s += "  -> enclosing rectangle      = [" + tostring(xmin) + "," + tostring(xmax) + "]x[" + tostring(ymin) + "," + tostring(ymax) + "]x[" + tostring(zmin) + "," + tostring(zmax) + "]\n";}
			int64 nbn,nbl,d,mem; mem = memory(&nbn,&nbl,&d);
			s += "  -> Depth of the tree  = " + tostring(d) + "\n"; 
			s += "  -> Number of nodes    = " + tostring(nbn) + "\n"; 
			s += "  -> Number of leafs    = " + tostring(nbl) + "\n"; 
			s += "  -> Size of the object = " + tostring(mem) + "bytes (" + tostring(mem/(1024*1024)) + "MB)\n\n"; 
			if (debug) {s+= debugTree();}
			return s;
			}


		/************************************************************************
		* Return the size of the allocated memory
		*
		* - Return the number of bytes used by the lattice
		*
		* Additonnaly, for the pointer which are not NULL : 
		*    - put in nb_nodes the number of nodes in the lattice.
	    *    - put in nb_leaf the number of leaf in the lattice.
		*    - put in depth the depth of the tree.
		*
		* This method is slow since it examine the whole tree.
        ************************************************************************/
		int64 memory(int64 * nb_nodes,int64 * nb_leafs,int64 * depth) const
			{
			if (depth != NULL) {(*depth) = treenode->get_root()->depth();}
			int64 nbl = 0,nbn = 0;
			int64 s = treenode->get_root()->computeSize(nbn,nbl);
			if (nb_nodes != NULL) {(*nb_nodes) = nbn;}
			if (nb_leafs != NULL) {(*nb_leafs) = nbl;}
			return s + sizeof(GrowingLatticeZ3<T,R>);
			}


		/************************************************************************
		* Print the structure of the tree into an std string
		*
		* For debugging purpose only (should only be used for small tree)
        ************************************************************************/
		std::string debugTree() const
			{
			return treenode->get_root()->printNode("","");
			}


	private:

	mutable p_sqr treenode; // the current node 
	GLZ3internals::infoGLZ3<T> info;	// info about the object;

};


}

}
#endif

/* end of file GrowingLatticeZ3 */


