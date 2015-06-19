/****************************************************************************************************
* TEMPLATE CLASS GrowingLatticeZ2                                          Version 1.0, Vindar 2014 *
*                                                                                                   *
* STATUS: WORKING COMPLETELY !                                                                      * 
*                - Tested for 32 and 64 bits implementation ?                              YES !    *
*                - Can work with up to 1TB of memory ?                                     YES !    *
*                - Everything is sure to be OK as long as there is no exception thrown ?   YES !    *
*                                                                                                   *
* PURPOSE:  Used for representing Z^2 which is divided recursevely into square each square being    *
*           divided into 3x3 subsquare until we arrive the the leaf node with respresent block of   * 
*           the lattice of size (2R+1)x(2R+1)  (where R is the template int parameter).             *
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
*    R : Each elementary subsquare of sites of Z^2 is of size (2R+1)x(2R+1)                         *
*        Choose typically 5 <= R <= 100 (default R = 30)                                            *
*                                                                                                   *
*	 T : Type of element stored in each site of the lattice (default T = unsigned char)             *
*         - Must be POD! No memory allocation allowed.                                              *
*         - ctor is useless (assigned with emptyVal afterward)                                      *
*         - Do not count on : 1 site = 1 object of type T (some object T may be shared between      *
*    	    several sites with same value).                                                         *
*         - T must be comparable with == (to check if empty of full)                                *
*         - must be assignable with =                                                               *
*                                                                                                   *
* GENERAL FUNCTION                                                                                  *
*                                                                                                   *
* - printGrowingLatticeZ2()         load a lattice from a file and draw it on a LatticePlotter      *
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
*	- improvedEnclosingRectFull()	compute a rect of full site around a given site (improved vers.)*
*   - improvedEnclosingNotZero()    as above but make sure the rectangle does not contain (0,0)     *
*	- stats()						information about the object                                    *
*	- memory()						information about the memory used                               *
*	- debugTree()					print the tree structure of the lattice in a string (debugging) *
*	- operator==()					compare with another lattice                                    *
*	- operator=() and copy ctor		copy a lattice                                                  *
*                                                                                                   *
* EXAMPLE 1:                                                                                        *
*                                                                                                   *
#include "mylib_all.h"

class iDLA
	{
	public: 
		void makeWalk(int64 nbParticules) {
			L1 = new GrowingLatticeZ2<char,5>(0,1); L2 = new GrowingLatticeZ2<int64,50>(0,-1);
			N = nbParticules; int64 step = 0;
			Chronometer();
			for(int64 i = 0;i< nbParticules;i++) {
				int64 x=0,y=0;
				while(L1->get(x,y) == 1) {// move until we exit the set of visited sites
					int64 xmin,xmax,ymin,ymax; L1->improvedEnclosingRectFull(x,y,xmin,xmax,ymin,ymax);
					if ((xmax -x > 1)&&(ymax - y > 1)&&(x-xmin > 1)&&(y -  ymin > 1)) {step += random::SRW_Z2_exitRectangle(x,y,xmin,xmax,ymin,ymax,gen);	}
					else {random::SRW_Z2_make1step(x,y,gen.rand_double0()); step++;} }
				L1->set(1,x,y); L2->set(i,x,y); }
			out << N << " particules in " << step << "steps.\nSimulation done in " << (double)(Chronometer())/1000 << " secondes\n";
			LatticePlotter<iDLA,false> Plotter2(*this); Plotter2.startPlot();
			delete L1; delete L2; }

		RGBc getColor(int64 x, int64 y) {if (L1->get(x,y) == 0) {return RGBc::c_White;} return RGBc::jet_palette(L2->get(x,y),0,N);}

	private: 
		int64 N;							// number of particles
		MT2004_64  gen;						// the random number generator
		GrowingLatticeZ2<char,5> * L1;		// to check if a site is occupied
		GrowingLatticeZ2<int64,50> * L2;	// time of visit of this site
	};

int main(int argc, char *argv[])
	{
	iDLA O;	O.makeWalk(50000);
	return 0;
	}

*                                                                                                   *
*                                                                                                   *
* EXAMPLE 2: Programme simple qui affiche une GrowingLattice passé en ligne de commande ou          *
*            fournie par l'utilisateur.                                                             *
*                                                                                                   *
#include "mylib_all.h"

int main (int argc, char *argv[])
{
	out.disablelogfile();
	out << "***************************************\n";
	out << "*   Growing Lattice Z2 Reader 1.0     *\n";
	out << "***************************************\n\n";
	std::string filename;
	if (argc == 2)   {filename = argv[1]; out << "Fichier : " << filename << "\n\n";}
	else {out << "nom du fichier .Z2 ? "; input >> filename; out << filename << "\n\n";}
	printGrowingLatticeZ2<0>(filename);
	return 0;
}
*                                                                                                   *
*                                                                                                   *
* standalone, no cpp file but uses GLZ2internals.h                                                  *
****************************************************************************************************/
#ifndef _GROWINGLATTICEZ2_H_
#define _GROWINGLATTICEZ2_H_

#include "../crossplatform.h"
#include "../graphics/fRect_iRect.h"
#include "../graphics/RGBc.h"
#include "../graphics/LatticePlotter.h"
#include "../mathgraph/GLZ2internals.h" // include all the internal code for the object with the node/leaf sub classes

#include <stdio.h>
#include <string>


namespace mylib
{
namespace mathgraph
{




/********************************************************************************************************************
*
*
* Class GrowingLatticeZ2
*
*
*********************************************************************************************************************/
template<class T = unsigned char,int R = 30> class GrowingLatticeZ2
{

			typedef GLZ2internals::basicsqr<T,R> * p_sqr;		// pointer types
			typedef GLZ2internals::leafsqr<T,R>  * p_leafsqr;	// 
			typedef GLZ2internals::nodesqr<T,R>  * p_nodesqr;	// 



public:


        /************************************************************************
        * ctor : the lattice is created initially empty (all site = emptyVal)
		*
		* - emptyV : an empty element
		* - fullV  : a full element
		*
        ************************************************************************/
		GrowingLatticeZ2(const T & emptyV,const T & fullV)
			{
			info.emptyVal = emptyV;		// save the model for empty value
			info.fullVal = fullV;		// and full value
			treenode = NULL;
			reset();
			}


        /************************************************************************
        * dtor
        ************************************************************************/
		~GrowingLatticeZ2()
			{
			if (treenode != NULL) {treenode->get_root()->destroytree(); treenode=NULL;}	// release all alocated memory if any
			}


        /************************************************************************
        * copy ctor : the object is obtained as a deep copy of GL
		*             so that *this and GL are completely independent
        ************************************************************************/
		GrowingLatticeZ2(const GrowingLatticeZ2<T,R> & GL)
			{
			treenode = new GLZ2internals::nodesqr<T,R>(*(GL.treenode->get_root())); 
			info = GL.info; // copy the additionnal info
			return;
			}


        /************************************************************************
        * assignement operator : copy the lattice from GL, erasing the current
		* one. 
		* deep copy : the two object are completely independent.
        ************************************************************************/
		GrowingLatticeZ2<T,R> & operator=(const GrowingLatticeZ2<T,R> & GL)
			{
			if (&GL == this) return(*this);
			treenode->get_root()->destroytree(); // release all allocated memory
			treenode = new GLZ2internals::nodesqr<T,R>(*(GL.treenode->get_root())); 
			info = GL.info; // copy the additionnal info
			return *this;
			}


        /************************************************************************
        * comparison operator : return true if the two lattices are equal ie 
		* if all sites have exactly the same value.
        ************************************************************************/
		bool operator==(const GrowingLatticeZ2<T,R> & GL)
			{
			return treenode->get_root()->compare(GL.treenode->get_root());
			}


		/************************************************************************
        * Reset the lattice setting all site to empty value
        ************************************************************************/
		void reset()
			{
			if (treenode != NULL) {treenode->get_root()->destroytree();}	// release all allocated memory if any
			treenode = new GLZ2internals::nodesqr<T,R>; // create the root node				
			info.nbF  = 0; info.nbNE = 0; info.Xmin = 0; info.Xmax = 0; info.Ymin = 0; info.Ymax = 0; // initialize the info
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
			if (fwrite("glZ2!",1,5,hf) != 5) {fclose(hf); return false;} // old file format started with "GLZ2!"
			uint64 tailleT = sizeof(T);									 // new format with "glZ2!" and saves the size of T
			if (fwrite(&tailleT,sizeof(uint64),1,hf) != 1) {fclose(hf); return false;}	 // 
			uint64 vR = R;
			if (fwrite(&vR,sizeof(uint64),1,hf) != 1) {fclose(hf); return false;} 			
			if (fwrite(&info,sizeof(GLZ2internals::infoGLZ2<T>),1,hf) != 1) {fclose(hf); return false;}
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
			if ((std::string(begin)) != std::string("GLZ2!")) // old file format, do not contain the size of T
				{
				if ((std::string(begin)) != std::string("glZ2!")) {reset(); fclose(hf); return false;}
				uint64 tailleT=0,vR=0; // new file format, contains the size of T and R
				if (fread(&tailleT,sizeof(uint64),1,hf) != 1) {reset(); fclose(hf); return false;}
				if (fread(&vR,sizeof(uint64),1,hf) != 1) {reset(); fclose(hf); return false;}
				if (tailleT != sizeof(T)) {reset(); fclose(hf); return false;} // make sure the size is correct
				if (vR != R) {reset(); fclose(hf); return false;} // make sure the size is correct
				}
			if (fread(&info,sizeof(GLZ2internals::infoGLZ2<T>),1,hf) != 1) {reset(); fclose(hf); return false;}
			treenode = new GLZ2internals::nodesqr<T,R>;
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
		* more precisely, after the function return, evry non empty site (X,Y) is
		* such that  (Xmin <= X <= Xmax) & (Ymin <= Y <= Ymax)
		* (the rectangle returned may not be the smallest one if some site were
		* change back from non empty to empty).
		*
		* If the lattice is empty, return false and set the rectangle to (0,0,0,0) 
        ************************************************************************/
		inline bool range(int64 & Xmin,int64 & Xmax,int64 & Ymin,int64 & Ymax) const 
			{
			if (info.nbNE == 0) {Xmin = 0; Xmax = 0; Ymin = 0, Ymax=0; return false;}
			Xmin = info.Xmin; Xmax = info.Xmax; Ymin = info.Ymin; Ymax = info.Ymax;
			return true;
			}

		inline bool range(mylib::graphics::fRect & r) const 
			{
			int64 xmin,xmax,ymin,ymax;
			bool b = range(xmin,xmax,ymin,ymax);
			if (b) {r.xmin = (double)xmin; r.xmax = (double)xmax; r.ymin = (double)ymin, r.ymax = (double)ymax; return true;}
			return false;
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
		inline T get(int64 x,int64 y) const {return treenode->get(x,y,treenode,&info);}


		/************************************************************************
        * Set the value of the site at position (x,y)
        ************************************************************************/
		inline void set(const T & v,int64 x,int64 y) 
			{
			if (v != info.emptyVal)
				{
				if (info.nbNE == 0) {info.Xmin = x; info.Xmax = x; info.Ymin = y; info.Ymax = y;}
				else 
					{
					if (x < info.Xmin) {info.Xmin = x;} else if (x > info.Xmax) {info.Xmax = x;}
					if (y < info.Ymin) {info.Ymin = y;} else if (y > info.Ymax) {info.Ymax = y;}
					}
				}
			treenode = treenode->set(v,x,y,&info);
			}


		/************************************************************************
		* Find a rectangle of full point in the lattice containing (X,Y) and put 
		* the result in (Xmin,Xmax,Ymin,Ymax)
		* 
		* - return false if (X,Y) is not full 
		*                (in this case Xmin,Xmax,Ymin,Ymax are not modified)
		*
		* - return true  if (X,Y) is indeed full
		*                 in this case, change Xmin,Xmax,Ymin,Ymax such that
	    *				 the rectangle contain (X,Y) and all points (i,j) such 
		*				 that (Xmin <= i <= Xmax)&&(Ymin <= j <= Ymax) are full.
		*
		* This method is very fast ! (as fast as the get method)
		* but the rectangle returned is not necessarily very good because (X,Y)
		* may be very close to the boundary of the returned rectangle.
        ************************************************************************/
		inline bool getEnclosingRectFull(int64 X,int64 Y,int64 & Xmin,int64 & Xmax,int64 & Ymin,int64 & Ymax) const
			{
			return treenode->getEnclosingRectFull(X,Y,Xmin,Xmax,Ymin,Ymax,treenode,&info);
			}


		/************************************************************************
		* Find a rectangle of full point in the lattice containing (X,Y) and put 
		* the result in (Xmin,Xmax,Ymin,Ymax)
		*
		* - return false if (X,Y) is not full 
		*                (in this case Xmin,Xmax,Ymin,Ymax are not modified)
		*
		* - return true  if (X,Y) is indeed full
		*                 in this case, change Xmin,Xmax,Ymin,Ymax such that
	    *				 the rectangle contain (X,Y) and all points (i,j) such 
		*				 that (Xmin <= i <= Xmax)&&(Ymin <= j <= Ymax) are full.
		*
		* Same as before but the method is more expensive (about 20 time slower)
		* but usually provide a better rectangle 
		*
		* The returned rectangle always bigger or equal to the one obtained by
		* getEnclosingRectFull but is usually such that (X,Y) is more centered 
		* inside the rectangle.
        ************************************************************************/
		inline bool improvedEnclosingRectFull(int64 X,int64 Y,int64 & Xmin,int64 & Xmax,int64 & Ymin,int64 & Ymax) const
			{
			return treenode->improvedEnclosingRectFull(X,Y,Xmin,Xmax,Ymin,Ymax,treenode,&info);
			}


		/************************************************************************
		* Find a rectangle of full point in the lattice containing (X,Y) but
		* not containing the origin (0,0). Put the result in (Xmin,Xmax,Ymin,Ymax)
		* 
		* This is similar as improvedEnclosingRectFull() but the returned rectangle
		* is "truncated" in such way that it does not contain the origin (but still
		* contain (X,Y) ).
		*
		* - Return false if (X,Y) is not full or if (X,Y) == (0,0). In this case,
        *                Xmin,Xmax,Ymin,Ymax are not modified
		*
		* - Return true otherwise and change Xmin,Xmax,Ymin,Ymax to the obtained 
		*               rectangle
		*
		* Same time complexity as improvedEnclosingRectFull(). 
        ************************************************************************/
		inline bool improvedEnclosingNotZero(int64 X,int64 Y,int64 & Xmin,int64 & Xmax,int64 & Ymin,int64 & Ymax) const
			{
			if ((X == 0)&&(Y==0)) return false;	// do nothing if at the origin
			if (!(treenode->improvedEnclosingRectFull(X,Y,Xmin,Xmax,Ymin,Ymax,treenode,&info))) {return false;} // do nothing if site not full				
			if ((Xmin <= 0)&&(Xmax >= 0)&&(Ymin <= 0)&&(Ymax >=0)) // 0 is inside the rectangle, truncate it !
				{
				if (abs(X) > abs(Y)) {if (X>0) {Xmin = 1;} else {Xmax= -1;}} // trucate along the y axis
				else {if (Y>0) {Ymin = 1;} else {Ymax= -1;}} // truncate along the x axis
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
			std::string s = "Growing Lattice Z2 statistics :\n";
			s += "  -> R = " + tostring(R) + " (each leaf is " + tostring(2*R+1) + "x" + tostring(2*R+1) + " sites)\n";
			s += "  -> T = object of size " + tostring(sizeof(T)) + " bytes\n";
			s += "  -> Number of site non empty = " + tostring(nbNonEmptySites()) + "\n"; 
			s += "  -> Number of site full      = " + tostring(nbFullSites()) + "\n"; 
			int64 xmin,xmax,ymin,ymax;
			if (range(xmin,xmax,ymin,ymax)) {s += "  -> enclosing rectangle      = [" + tostring(xmin) + "," + tostring(xmax) + "]x[" + tostring(ymin) + "," + tostring(ymax) + "]\n";}
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
			return s + sizeof(GrowingLatticeZ2<T,R>);
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
	GLZ2internals::infoGLZ2<T> info;	// info about the object;

};



namespace GLZ2internals
{

	/* Simple class used by the printGrowingLatticeZ2 function */
	template<class T,int R> class _printGLZ2class {
		public:		_printGLZ2class(GrowingLatticeZ2<T,R> * L) : Lat(L) {return;}
					inline mylib::graphics::RGBc getColor(int64 i, int64 j) const {int64 v = Lat->get(i,j); if (v == Lat->emptyValue()) {return RGBc::c_White;} if (v == Lat->fullValue()) {return RGBc::c_Black;} return RGBc::c_Red;}
		private:	GrowingLatticeZ2<T,R> * Lat;
		};

	/* Used by the printGrowingLatticeZ2 function */
	template<class T,int R> bool _printGrowingLatticeZ2(bool tc,std::string filename)
		{
		GrowingLatticeZ2<T,R> G(0,1);
		out << "Loading " << filename<< "..."; if (G.load(filename) == false) {out << "Erreur de lecture du fichier !!!\n\n"; return false;} out << "ok!\n\n";
		if (tc == true) {
			out << "Format ancien, voulez-vous realiser la conversion vers le nouveau format (o/n) ?";
			int k=0;  while((k!='o')&&(k!='O')&&(k!='N')&&(k!='n')){k = input.getkey();} if ((k=='o')||(k=='O')) {out << "Saving..."; G.save(filename); out << "ok!\n\n";}
			}
		_printGLZ2class<T,R> GLC(&G);
		out << G.stats() << "\n\n";
		fRect r; G.range(r);
		LatticePlotter<const _printGLZ2class<T,R>,false> Plotter(GLC); 
		Plotter.setRange(r,true); Plotter.startPlot();
		return true;
		}
}


   /*********************************************************************************************************
	* Try to draw a GrowingLattice file into a Lattice plotter. 
	*
	* The color convention is: empty site = white
	*                          full site  = black
	*                          other site = red
	*
	* In order to work, the lattice sotred in the file must be such that
	*		- sizeof(T) = 1,4 or 8
	*		- R = 5,10,30,50 or 100
	* Otherwise, the lattice file cannot be drawn.
	*
	* If the lattice file is in the old format, the user is askedif he wants toconvert it by providing R and T.
	*
	* Return true if successful and the lattice was indeed printed.
	*
	* the template parameter unused is irrelevant (only used for putting the function in a header file), set it to 0.
	*********************************************************************************************************/
	template<int unused> bool printGrowingLatticeZ2(std::string filename)
		{
		bool tc = false;
		uint64 tailleT =0; uint64 vR = 0;
		FILE * hf = fopen(filename.c_str(),"rb"); if (hf == NULL) {out << "Erreur de lecture du fichier [" << filename<< "] !!!\n\n"; return false;}
		char begin[6]; memset(begin,0,6);if (fread(begin,1,5,hf) != 5) {out << "Erreur de lecture du fichier [" << filename<< "] !!!\n\n"; fclose(hf); return false;}
		if ((std::string(begin)) == std::string("glZ2!"))  
			{
			if (fread(&tailleT,sizeof(uint64),1,hf) != 1) {out << "Erreur de lecture du fichier [" << filename<< "] !!!\n\n"; fclose(hf); return false;}
			if (fread(&vR,sizeof(uint64),1,hf) != 1) {out << "Erreur de lecture du fichier [" << filename<< "] !!!\n\n"; fclose(hf); return false;}
			}
		else
			{
			if (std::string(begin) != std::string("GLZ2!")) {out << "Le fichier [" << filename<< "] n'est pas de type GLZ2 !\n\n"; fclose(hf); return false;}
			tc= true;
			out << "Fichier dans l'ancien format.\n\n";
			out << "Quel est la taille d'un element T (en octet) ? "; input >> tailleT;  out << tailleT << "\n\n";
			out << "Quel est la valeur de R ? "; input >> vR; out << vR << "\n\n";
			}
		fclose(hf);
		if ((tailleT != 1)&&(tailleT != 2)&&(tailleT != 4)&&(tailleT != 8)) {out << "Unsupported sizeof(T) = " << tailleT << " !!!\n\n"; fclose(hf); return false;}
		if ((vR != 5)&&(vR != 10)&&(vR != 30)&&(vR != 50)&&(vR != 100)) {out << "Unsupported value R = " << tailleT << " !!!\n\n"; fclose(hf); return false;}
		// sizeof(T) = 1	
		if ((tailleT == 1)&&(vR == 5))  {return GLZ2internals::_printGrowingLatticeZ2<char,5>(tc,filename);}
		if ((tailleT == 1)&&(vR == 10)) {return GLZ2internals::_printGrowingLatticeZ2<char,10>(tc,filename);}
		if ((tailleT == 1)&&(vR == 30)) {return GLZ2internals::_printGrowingLatticeZ2<char,30>(tc,filename);}
		if ((tailleT == 1)&&(vR == 50)) {return GLZ2internals::_printGrowingLatticeZ2<char,50>(tc,filename);}
		if ((tailleT == 1)&&(vR == 100)) {return GLZ2internals::_printGrowingLatticeZ2<char,100>(tc,filename);}
		// sizeof(T) = 2	
		if ((tailleT == 2)&&(vR == 5))  {return GLZ2internals::_printGrowingLatticeZ2<int16,5>(tc,filename);}
		if ((tailleT == 2)&&(vR == 10)) {return GLZ2internals::_printGrowingLatticeZ2<int16,10>(tc,filename);}
		if ((tailleT == 2)&&(vR == 30)) {return GLZ2internals::_printGrowingLatticeZ2<int16,30>(tc,filename);}
		if ((tailleT == 2)&&(vR == 50)) {return GLZ2internals::_printGrowingLatticeZ2<int16,50>(tc,filename);}
		if ((tailleT == 2)&&(vR == 100)) {return GLZ2internals::_printGrowingLatticeZ2<int16,100>(tc,filename);}
		// sizeof(T) = 4	
		if ((tailleT == 4)&&(vR == 5))  {return GLZ2internals::_printGrowingLatticeZ2<int32,5>(tc,filename);}
		if ((tailleT == 4)&&(vR == 10)) {return GLZ2internals::_printGrowingLatticeZ2<int32,10>(tc,filename);}
		if ((tailleT == 4)&&(vR == 30)) {return GLZ2internals::_printGrowingLatticeZ2<int32,30>(tc,filename);}
		if ((tailleT == 4)&&(vR == 50)) {return GLZ2internals::_printGrowingLatticeZ2<int32,50>(tc,filename);}
		if ((tailleT == 4)&&(vR == 100)) {return GLZ2internals::_printGrowingLatticeZ2<int32,100>(tc,filename);}
		// sizeof(T) = 8	
		if ((tailleT == 8)&&(vR == 5))  {return GLZ2internals::_printGrowingLatticeZ2<int64,5>(tc,filename);}
		if ((tailleT == 8)&&(vR == 10)) {return GLZ2internals::_printGrowingLatticeZ2<int64,10>(tc,filename);}
		if ((tailleT == 8)&&(vR == 30)) {return GLZ2internals::_printGrowingLatticeZ2<int64,30>(tc,filename);}
		if ((tailleT == 8)&&(vR == 50)) {return GLZ2internals::_printGrowingLatticeZ2<int64,50>(tc,filename);}
		if ((tailleT == 8)&&(vR == 100)) {return GLZ2internals::_printGrowingLatticeZ2<int64,100>(tc,filename);}
		out << "euh...Erreur impossible\n\n";
		return false;
		}




}

}
#endif

/* end of file GrowingLatticeZ2 */


