/****************************************************************************************************
* Internal classes used for GrowingLatticeZ3.h                             Version 1.0, Vindar 2014 *
*                                                                                                   *
* This file contains the implementations of the sub classes used for creating the GrowingLatticeZ3  *
* class exposed in GrowingLatticeZ3.h                                                               *
*                                                                                                   *
* it should not be called directly except in GrowingLatticeZ3.h                                     *
*                                                                                                   *
* Here, everything is encapsulated into the namespace GLZ3internals                                 *                                                                  *
*                                                                                                   *
* standalone, no cpp file                                                                           *
****************************************************************************************************/
#ifndef _GLZ3INTERNALS_H_
#define _GLZ3INTERNALS_H_


namespace mylib
{
namespace mathgraph
{

/******************************************************
* Namespace GLZ3internals for isolating these classes
* from the rest of the world
******************************************************/
namespace GLZ3internals
{


	template<class T,int64 R> class nodesqr;			// forward declaration
	template<class T,int64 R> class leafsqr;			//



	/*****************************************************************************************************
	*
	* structure containing the info about the lattice
	*
	******************************************************************************************************/
	template<class T> struct infoGLZ3
		{
		T emptyVal;								// an empty element
		T fullVal;								// a full element for comparison
		int64 nbNE;								// number of site which are not empty
		int64 nbF;								// number of site which are full
		int64 Xmin,Xmax,Ymin,Ymax,Zmin,Zmax;	// smallest 3D rectangle containing all the non empty site
		};






	/*****************************************************************************************************
	*
	* Base class describing a square
	*
	* -> can be sub-classed into:
	*     - nodesqr : a 3x3x3 node
	*     - leafsqr : a leaf representing a grid of size (2R+1)x(2R+1)x(2R+1)sites
	*
	******************************************************************************************************/
	template<class T,int64 R> class basicsqr
		{

		public: 

			typedef basicsqr<T,R> * p_sqr;		// pointer types
			typedef leafsqr<T,R>  * p_leafsqr;	// 
			typedef nodesqr<T,R>  * p_nodesqr;	// 

			static basicsqr<T,R> fullsqr;		// dummy adress for full square
			static basicsqr<T,R> emptysqr;		// dummy adress for an empty adress

			int64 centerX,centerY,centerZ;		// position of the center of the square

			int64 sub_radius;				// radius of each sub-square of this square
											// if sub_radius = 1, this struct is a leaf square and its radius itself is R (so its diameter is 2R+1)
											// if sub_radius > 1, this struct is a node square and its radius itself is 3*sub_radius + 1

			p_nodesqr father;				// pointeur to the father node. null iff we are at the root 


		/* ctor: create a subsquare centered at (cX,cY,cZ) with subradius sR and node father f */
		basicsqr(int64 cX,int64 cY,int64 cZ,int64 sR,p_nodesqr f) : centerX(cX),centerY(cY),centerZ(cZ),sub_radius(sR),father(f)  {return;} 


		private:

		/* private: used only for creating a dummy fullsqr and emptysqr */
		basicsqr() : centerX(0),centerY(0),centerZ(0),sub_radius(0),father(0) {return;}

		public:

		/* basic copy ctor */
		basicsqr(const basicsqr<T,R> & sqr) 
			{
			centerX = sqr.centerX;
			centerY = sqr.centerY;
			centerZ = sqr.centerZ;
			sub_radius = sqr.sub_radius;
			father = NULL;
			}

		/* writing itself to a file */
		bool save(FILE * hf) const
			{
			if (fwrite(&centerX,sizeof(int64),1,hf) != 1) {return false;}
			if (fwrite(&centerY,sizeof(int64),1,hf) != 1) {return false;}
			if (fwrite(&centerZ,sizeof(int64),1,hf) != 1) {return false;}
			if (fwrite(&sub_radius,sizeof(int64),1,hf) != 1) {return false;}
			return true;
			}


		/* loading itself from a file
		 * f = adress of the father whose memory should already be allocated */
		bool load(FILE * hf,p_nodesqr f)
			{
			if (fread(&centerX,sizeof(int64),1,hf) != 1) {return false;}
			if (fread(&centerY,sizeof(int64),1,hf) != 1) {return false;}
			if (fread(&centerZ,sizeof(int64),1,hf) != 1) {return false;}
			if (fread(&sub_radius,sizeof(int64),1,hf) != 1) {return false;}
			father = f;
			return true;
			}


		/* return the depth of the node (0 for a leaf) */
		inline int depth() const
			{
			int64 r = sub_radius;
			if (r == 1) {return 0;}
			int d = 1; while(r > R) {d++; r = (r-1)/3;}
			return d;
			}


		/* return the root of the tree */
		inline p_nodesqr get_root() const
			{
			p_sqr p = (p_sqr)this; while(p->father != NULL) {p = p->father;}
			return((p_nodesqr)p);
			}


		/* destroy the tree and free all allocated ressources
		 * !!! must only be called from the root !!! */
		void destroytree()
			{
			if (sub_radius == 1) {delete((p_leafsqr)this); return;} // delete the leaf
			for(int k=0;k<3;k++) 
				{
				for(int j=0;j<3;j++) 
					{
					for(int i=0;i<3;i++) 
						{
						p_sqr p = ((p_nodesqr)this)->tab[i][j][k];
						if ((p != &emptysqr)&&(p!=&fullsqr)) {p->destroytree();}
						}
					}
				}
			delete((p_nodesqr)this);
			return;
			}


		/* compare this node (and it subtree) to p (and it subtree) and return true if they are equal */
		bool compare(const p_sqr p)
			{
			if (((father == NULL)&&(p->father != NULL))||((father != NULL)&&(p->father == NULL))) return false; // check if both are root
			if ((p->sub_radius != sub_radius)||(p->centerX != centerX)||(p->centerY != centerY)||(p->centerZ != centerZ)) return false; // check same node paramameter
			if (sub_radius ==1)
				{
				p_leafsqr pa = ((p_leafsqr)this), pb = ((p_leafsqr)p);
				if ((pa->nEmpty != pb->nEmpty)||(pa->nFull != pb->nFull)) {return false;}
				for(int i=0;i<2*R+1;i++) for(int j=0;j<2*R+1;j++) for(int k=0;k<2*R+1;k++) {if (pa->tab[i][j][k] != pb->tab[i][j][k]) {return false;}}
				return true;
				}
			p_nodesqr pa = ((p_nodesqr)this);
			p_nodesqr pb = ((p_nodesqr)p);
			for(int i=0;i<3;i++) for(int j=0;j<3;j++) for(int k=0;k<3;k++)
				{
				p_sqr ca = pa->tab[i][j][k]; p_sqr cb = pb->tab[i][j][k];
				if ((ca != &emptysqr)&&(ca != &fullsqr)&&(cb != &(cb->emptysqr))&&(cb != &(cb->fullsqr)))
					{ // both ca and cb are regular nodes : compare them
					if (ca->compare(cb) == false) {return false;}
					}
				else 
					{ // at least one node is special
					if ((ca == &emptysqr)&&(cb != &(cb->emptysqr))) return false;
					if ((ca == &fullsqr)&&(cb != &(cb->fullsqr))) return false;
					if ((ca != &emptysqr)&&(cb == &(cb->emptysqr))) return false;
					if ((ca != &fullsqr)&&(cb == &(cb->fullsqr))) return false;
					}
				}
			return true;
			}

		/* Returns the value at (x,y,z) does not affect anything 
		 * look recursively in the entire tree (faster if already near)
		 * put in newhint the node associated with (x,y,z) (helps for subsequent queries)
		 * - if (x,y,z) outside of the tree, newhint is set to the root.
		 *   otherwise, newhint is the leaf or node which directly contain (x,y,z)
		 */
		inline T get(int64 x,int64 y,int64 z,p_sqr & newhint,const infoGLZ3<T> * info) const
			{
			int64 rad = ((sub_radius == 1) ? R : (3*sub_radius + 1) );	// compute the radius of the current node
			int64 dx = (x - centerX); int64 dy = (y - centerY); int64 dz = (z - centerZ);	// position wrt current center
			if ((abs(dx) > rad)||(abs(dy) > rad)||(abs(dz) > rad))		// site not included in this square
				{ 
				if (father == NULL) {newhint = (p_sqr)this;	return info->emptyVal;}  // father not created, so the site must be initially empty
				return father->_get(x,y,z,newhint,info);						// look into the father
				}
			if (sub_radius == 1) {newhint = (p_sqr)this; return(((p_leafsqr)this)->tab[R + dx][R + dy][R + dz]);} // we are in a leaf
			p_nodesqr pn = (p_nodesqr)this; p_sqr p = pn->tab[pn->getindexX(x)][pn->getindexY(y)][pn->getindexZ(z)]; // else find the subsquare corresponding to the point
			if (p == &emptysqr) {newhint = (p_sqr)this; return info->emptyVal;}
			if (p == &fullsqr)  {newhint = (p_sqr)this; return info->fullVal;}
			return p->_get(x,y,z,newhint,info);
			}
	

		/* Recursive version that cannot be inlined */
		T _get(int64 x,int64 y,int64 z,p_sqr & newhint,const infoGLZ3<T> * info) const
			{
			int64 rad = ((sub_radius == 1) ? R : (3*sub_radius + 1) );	// compute the radius of the current node
			int64 dx = (x - centerX); int64 dy = (y - centerY); int64 dz = (z - centerZ);	// position wrt current center
			if ((abs(dx) > rad)||(abs(dy) > rad)||(abs(dz) > rad))	// site not included in this square
				{ 
				if (father == NULL) {newhint = (p_sqr)this; return info->emptyVal;}  // father not created, so the site must be initially empty
				return father->_get(x,y,z,newhint,info);						// look into the father
				}
			if (sub_radius == 1) {newhint = (p_sqr)this; return(((p_leafsqr)this)->tab[R + dx][R + dy][R + dz]);} // we are in a leaf
			p_nodesqr pn = (p_nodesqr)this; p_sqr p = pn->tab[pn->getindexX(x)][pn->getindexY(y)][pn->getindexZ(z)]; // else find the subsquare corresponding to the point
			if (p == &emptysqr) {newhint = (p_sqr)this;	return info->emptyVal;}
			if (p == &fullsqr)  {newhint = (p_sqr)this; return info->fullVal;}
			return p->_get(x,y,z,newhint,info);
			}
	


		 /* find the enclosing full square of (X,Y,Z)
		  * return true is the site (X,Y,Z) is full and put the full square in Xmin,Xmax,Ymin,Ymax,Zmin,Zmax
		  * return false if (X,Y,Z) is not full (and do change to Xmin,Xmax,Ymin,Ymax,Zmin,Zmax)
		  * 
		  * as always, newhint now contain the node for (X,Y,Z) or the root if (X,Y,Z) outside of the tree
		  */
		bool getEnclosingRectFull(int64 X,int64 Y,int64 Z,int64 & Xmin,int64 & Xmax,int64 & Ymin,int64 & Ymax,int64 & Zmin,int64 & Zmax,p_sqr & newhint,const infoGLZ3<T> * info) const
			{ 
			if (get(X,Y,Z,newhint,info) != info->fullVal) {return false;} // site not full nothing to do, return false;
			if (newhint->sub_radius == 1) {Xmin = X; Xmax = X; Ymin = Y; Ymax = Y; Zmin = Z; Zmax= Z; return true;} // we are on a leaf, no (easy) non trivial rectangle
			// not on a leaf, we find the radius and the center of the full square containing (x,y) 
			p_nodesqr pn = (p_nodesqr)newhint; 
			int64 cx = pn->computeCenterX(pn->getindexX(X));
			int64 cy = pn->computeCenterY(pn->getindexY(Y));
			int64 cz = pn->computeCenterZ(pn->getindexZ(Z));
			Xmin = cx - pn->sub_radius; Xmax = cx + pn->sub_radius;
			Ymin = cy - pn->sub_radius; Ymax = cy + pn->sub_radius;
			Zmin = cz - pn->sub_radius; Zmax = cz + pn->sub_radius;
			return true;
			}


		/* Set the value at (x,y,z) to v
		 * look recursively in the entire tree (faster if already near)
		 * this may destroy many object, including this one !
		 * return the new square containing (x,y,z) (the pointer returned is valid!)
		 */
		inline p_sqr set(const T & v,int64 x,int64 y,int64 z,infoGLZ3<T> * info)
			{
			if (sub_radius == 1) // check if we are in the right leaf to speed up things
				{
				p_leafsqr lf = (p_leafsqr)this; // yes, in a leaf
				int64 dx = (x - centerX) + R,  dy = (y - centerY) + R, dz = (z - centerZ) + R;
				if ((dx >= 0)&&(dx < 2*R +1)&&(dy >= 0)&&(dy < 2*R +1)&&(dz >= 0)&&(dz < 2*R +1)) // yes, in the right leaf
					{
					T & val = lf->tab[dx][dy][dz]; // previous value at the site
					if (val == v) {return(this);} // nothing to do
					if (val == info->emptyVal)		 {(lf->nEmpty)--; info->nbNE++;} // site previously empty 
					else {if (val == info->fullVal)  {(lf->nFull)--; info->nbF--;}} // site previously full
					if (v == info->emptyVal) 
						{
						(lf->nEmpty)++; info->nbNE--; 
						if (lf->nEmpty == (2*R+1)*(2*R+1)*(2*R+1)) {return lf->_leafremove(info);} // leaf is empty, remove it and clean the tree recursively
						} 
					else {if (v == info->fullVal)  
						{
						(lf->nFull)++; info->nbF++;  
						if (lf->nFull == (2*R+1)*(2*R+1)*(2*R+1)) {return lf->_leafremove(info);} // leaf is full, remove it and clean the tree recursively
						}} 
					val = v; // leaf is neither empty not full : set the value
					return(this);
					}
				}
			return _set(v,x,y,z,info); // call the slow method
			}


		/* slow method */
		p_sqr _set(const T & v,int64 x,int64 y,int64 z,infoGLZ3<T> * info)
			{
			p_sqr p; 
			if (get(x,y,z,p,info) == v) {return p;}      // get approximation in p and check if something must be done anyway
			if (p->father == NULL) // if at the root, we might need to zoom out to contain the point
				{
				while((abs(x) > (3*(p->sub_radius) + 1))||(abs(y) > (3*(p->sub_radius) + 1))||(abs(z) > (3*(p->sub_radius) + 1))) {p = ((p_nodesqr)p)->createfather();}
				}
			// now, we zoom in until we arrive to a leaf
			while(p->sub_radius != 1) // iterate until we are at the leaf
				{
				int i = ((p_nodesqr)p)->getindexX(x), j = ((p_nodesqr)p)->getindexY(y), k = ((p_nodesqr)p)->getindexZ(z);
				((p_nodesqr)p)->create_child(i,j,k,info);	// otherwise create corresponding the child node containing (x,y,z)
				p = ((p_nodesqr)p)->tab[i][j][k];
				}
			return(p->set(v,x,y,z,info));	// we are now at the leaf, we now call the fast method
			}


		/* for debugging purpose : print the node in an std::string for indentation indent*/
		std::string printNode(std::string firstline,std::string nextlines)
			{
				if (sub_radius==1) return firstline + "-> Leaf (" + tostring(centerX) + "," + tostring(centerY) + "," + tostring(centerZ) + ") : empty = " + tostring(((p_leafsqr)this)->nEmpty) + " : full = " + tostring(((p_leafsqr)this)->nFull) + " : other = " + tostring((2*R+1)*(2*R+1)*(2*R+1) - ((((p_leafsqr)this)->nFull) + (((p_leafsqr)this)->nEmpty) ) ) + "\n";
				std::string res = firstline + "-> Node (" + tostring(centerX) + "," + tostring(centerY) + "," + tostring(centerZ) + ") : radius = " + tostring(3*sub_radius+1) + "\n";
				p_nodesqr p = (p_nodesqr)this;
				for(int i=0;i<3;i++) for(int j=0;j<3;j++) for(int k=0;k<3;k++)
					{
					std::string s1 = nextlines + "    |-[" + tostring(i) + "][" + tostring(j) + "]["+ tostring(k) + "] ";
					std::string s2 = nextlines + "    |           ";
					if (p->tab[i][j][k] == &fullsqr) {res += s1 + "-> Full\n";}
					else { if (p->tab[i][j][k] == &emptysqr) {res += s1 + "-> Empty\n";} else {res += (p->tab[i][j][k])->printNode(s1,s2);}}
					}
				return res;
			}


		/* compute the size of the subtree stating from this node (should be called from the root
		   for the total size of the object */
		int64 computeSize(int64 & nbn,int64 & nbl) const
			{
			if (sub_radius == 1) {nbl++; return sizeof(leafsqr<T,R>);}
			nbn++;
			p_nodesqr p = (p_nodesqr)this;
			int64 s = sizeof(nodesqr<T,R>);
			for(int i=0;i<3;i++) for(int j=0;j<3;j++) for(int k=0;k<3;k++)
				{
				if ((p->tab[i][j][k] != &fullsqr)&&(p->tab[i][j][k] != &emptysqr)) {s += (p->tab[i][j][k])->computeSize(nbn,nbl);}
				}
			return s;
			}

		};



	/* static dummy object indicating full and empty nodes */
	template<class T,int64 R> basicsqr<T,R>  basicsqr<T,R>::fullsqr;	// the fullsqr dummy object
	template<class T,int64 R> basicsqr<T,R>  basicsqr<T,R>::emptysqr;	// the emptysqr dummy object






	/*****************************************************************************************************
	*
	* Sub classe nodesqr describing the strucure of a 3x3x3 node
	*
	******************************************************************************************************/
	template<class T,int64 R>  class nodesqr : public basicsqr<T,R>
		{

		public:

		p_sqr tab[3][3][3];

		/* create an empty node square centered at (cX,cY,cZ), with sub_radius sr and father f 
		 * if isempty == true, make it empty, otherwise make it full */
		nodesqr(int64 cX,int64 cY,int64 cZ,int64 sr,p_nodesqr f,bool isempty) : basicsqr(cX,cY,cZ,sr,f)
			{
			p_sqr v = (isempty ? &emptysqr : &fullsqr);
			for(int k=0;k<3;k++) for(int j=0;j<3;j++) for(int i=0;i<3;i++) tab[i][j][k] = v; return;
			}

		/* create the starting node : empty node centered at zero at level 1 */
		nodesqr() : basicsqr(0,0,0,R,NULL)
			{
			for(int k=0;k<3;k++) for(int j=0;j<3;j++) for(int i=0;i<3;i++) tab[i][j][k] = &emptysqr; return;
			}

		/* copy ctor */
		nodesqr(const nodesqr<T,R> & node)  : basicsqr(node)
			{
			for(int k=0;k<3;k++) for(int j=0;j<3;j++) for(int i=0;i<3;i++) 
				{
				if (node.tab[i][j][k] == &(node.emptysqr)) {tab[i][j][k] = &emptysqr;} else {
				if (node.tab[i][j][k] == &(node.fullsqr)) {tab[i][j][k] = &fullsqr;} else {
					if ((node.tab[i][j][k])->sub_radius == 1)
						{
						tab[i][j][k] = new leafsqr<T,R>(*((p_leafsqr)(node.tab[i][j][k])));
						}
					else
						{
						tab[i][j][k] = new nodesqr<T,R>(*((p_nodesqr)(node.tab[i][j][k])));
						}
					(tab[i][j][k])->father = this;
					}}
				}
			}

		/* create the father of the node (ie the new root of the tree)
		 * and return it. This may destroy the object 
		 * if it is a node whose all sub square are emptsqr or all fullsqr */
		inline p_nodesqr createfather()
			{
			Assert((centerX == 0)&&(centerY == 0)&&(centerZ == 0)&&(father == NULL));// make sure we are the current root
			int64 rad = 3*sub_radius + 1;	// compute the radius of the current node
			p_nodesqr p = new nodesqr<T,R>(0,0,0,rad,NULL,true);	// create an empty node
			father = p; // set the father
			int st = _status();
			if (st == 0) // current node completely empty, remove it
				{
				father->tab[1][1][1] = &emptysqr; delete this; return p; 
				}
			if (st == 2) // current node completely full, remove it
				{
				father->tab[1][1][1] = &fullsqr; delete this; return p; 
				}
			// current node remains
			father->tab[1][1][1] = this;
			return p;
			}


		/* compute the center (for each direction) of a given subsquare of this node (ie i = 0,1,2) */
		inline int64 computeCenterX(int i) const {return(centerX + ((i==0) ? (-(2*sub_radius + 1)) : ((i==1) ? 0 : (2*sub_radius + 1))) );}
		inline int64 computeCenterY(int j) const {return(centerY + ((j==0) ? (-(2*sub_radius + 1)) : ((j==1) ? 0 : (2*sub_radius + 1))) );}
		inline int64 computeCenterZ(int k) const {return(centerZ + ((k==0) ? (-(2*sub_radius + 1)) : ((k==1) ? 0 : (2*sub_radius + 1))) );}


		/* reciprocally, compute the index of the subsquare associated with a point */
		inline int getindexX(int64 x) const {int64 d = x - centerX;	return ((d < -sub_radius) ? 0 : ((d > sub_radius) ? 2 : 1) );}
		inline int getindexY(int64 y) const {int64 d = y - centerY;	return ((d < -sub_radius) ? 0 : ((d > sub_radius) ? 2 : 1) );}
		inline int getindexZ(int64 z) const {int64 d = z - centerZ;	return ((d < -sub_radius) ? 0 : ((d > sub_radius) ? 2 : 1) );}


		/* create the child (i,j,k) of the node */
		void create_child(int i,int j,int k,const infoGLZ3<T> * info)
			{
			Assert((tab[i][j][k] == &fullsqr)||(tab[i][j][k] == &emptysqr));
			int64 cx = computeCenterX(i), cy = computeCenterY(j), cz = computeCenterZ(k); // compute the center of the new node
			if (sub_radius == R)  {tab[i][j][k] = new leafsqr<T,R>(cx,cy,cz,this, ((tab[i][j][k] == &emptysqr) ? info->emptyVal : info->fullVal),info ); return;}	// the new node is a leaf
			tab[i][j][k] = new nodesqr<T,R>(cx,cy,cz,(sub_radius-1)/3,this,(tab[i][j][k] == &emptysqr)); 											// the new node is a node
			return;
			}


		/* returns 0 if the node is empty - 2 if the node is full  - 1 if the node is neither 
		 * does not check if the sub-node themselve are clean. 
		 */
		inline int _status() const
			{
			int e=0,f=0;
			for(int k=0;k<3;k++) for(int j=0;j<3;j++) for(int i=0;i<3;i++) 
				{
					if (tab[i][j][k] == &emptysqr) {e++; if (f!=0) {return 1;}} 
					else {if (tab[i][j][k] == &fullsqr) {f++; if (e!=0) {return 1;}} else {return 1;}}
				}
			if (e == 27) return 0;
			return 2;
			}


		/* recursively clean the subsquare (going up only)
		 * this may destroy the object, return the new top one*/
		p_nodesqr clean(const infoGLZ3<T> * info)
			{
			int s = _status();
			if (father == NULL) // check if we are at the root
				{
				if (s == 2)	{p_nodesqr  p = createfather(); return p;} // root full : create the father and remove the node
				if (s == 0)	{p_nodesqr  p = new nodesqr<T,R>; delete this; return p;} // root empty, delete this and reset with a node at level 1
				if ((tab[0][0][0] == &emptysqr)&&(tab[0][0][1] == &emptysqr)&&(tab[0][0][2] == &emptysqr)&&
				    (tab[0][1][0] == &emptysqr)&&(tab[0][1][1] == &emptysqr)&&(tab[0][1][2] == &emptysqr)&&
				    (tab[0][2][0] == &emptysqr)&&(tab[0][2][1] == &emptysqr)&&(tab[0][2][2] == &emptysqr)&&
				    (tab[1][0][0] == &emptysqr)&&(tab[1][0][1] == &emptysqr)&&(tab[1][0][2] == &emptysqr)&&
				    (tab[1][1][0] == &emptysqr)                             &&(tab[1][1][2] == &emptysqr)&&
				    (tab[1][2][0] == &emptysqr)&&(tab[1][2][1] == &emptysqr)&&(tab[1][2][2] == &emptysqr)&&
				    (tab[2][0][0] == &emptysqr)&&(tab[2][0][1] == &emptysqr)&&(tab[2][0][2] == &emptysqr)&&
				    (tab[2][1][0] == &emptysqr)&&(tab[2][1][1] == &emptysqr)&&(tab[2][1][2] == &emptysqr)&&
				    (tab[2][2][0] == &emptysqr)&&(tab[2][2][1] == &emptysqr)&&(tab[2][2][2] == &emptysqr))
					{ // everything is empty except the center
					if (sub_radius == R) {return this;} // root at level 1, nothing to do
					if (tab[1][1][1] == &fullsqr) {return this;} // center subsquare full, nothing to do
					// ok, tab[1][1][1] is a real node and not a leaf, delete this root then clean the new root
					p_nodesqr  p = ((p_nodesqr)(tab[1][1][1]));
					p->father = NULL; // set as the new root
					delete(this); // delete this
					return p->clean(info); // recursiveley clean the new root
					}
				return this;
				}
			if (s == 1) return this;
			father->tab[father->getindexX(centerX)][father->getindexY(centerY)][father->getindexZ(centerZ)] = ((s==0) ? &emptysqr : &fullsqr); // set the subsquare in the father as empty or full
			p_nodesqr p = father->clean(info); // recursively clean the father and get the adress of the top one
			delete this; // delete itself, 
			return p; // return the new ancestor
			}


		/* writing itself to a file */
		bool save(FILE * hf) const
			{
			// save the common part
			if (((const basicsqr<T,R>*)(this))->save(hf) == false) {return false;}
			// save the specific part
			char tt[3][3][3];
			for(int i=0;i<3;i++) for(int j=0;j<3;j++) for(int k=0;k<3;k++)
				{
				if (tab[i][j][k] == &emptysqr) {tt[i][j][k] = 0;} else 
					{
					if (tab[i][j][k] == &fullsqr) {tt[i][j][k] = 1;} else 
						{
						if ((tab[i][j][k])->sub_radius == 1)  {tt[i][j][k] = 2;} else {tt[i][j][k] = 3;}		
						} 
					}
				}
			if (fwrite(&tt,sizeof(char),3*3*3,hf) != (3*3*3)) {return false;}
			for(int i=0;i<3;i++) for(int j=0;j<3;j++) for(int k=0;k<3;k++)
				{
				if (tt[i][j][k] == 2) {if (((p_leafsqr)(tab[i][j][k]))->save(hf) == false) {return false;} }
				else {if (tt[i][j][k] == 3) {if (((p_nodesqr)(tab[i][j][k]))->save(hf) == false) {return false;} }}
				}
			return true;
			}


		/* loading itself from file */
		bool load(FILE * hf,p_nodesqr f,const infoGLZ3<T> * info)
			{
			// load the common part
			if (((p_sqr)(this))->load(hf,f) == false) {return false;} 
			// load the specific part
			char tt[3][3][3];
			if (fread(&tt,sizeof(char),3*3*3,hf) != (3*3*3)) {return false;}
			for(int i=0;i<3;i++) for(int j=0;j<3;j++) for(int k=0;k<3;k++)
				{
				if (tt[i][j][k] == 0) {tab[i][j][k] = &emptysqr;} else 
					{
					if (tt[i][j][k] == 1) {tab[i][j][k] = &fullsqr;} else 
						{
						if (tt[i][j][k] == 2) 
							{
							p_leafsqr p =  new leafsqr<T,R>(0,0,0,NULL,info->emptyVal,info);
							if (p->load(hf,this) == false) {return false;}
							tab[i][j][k] = p;
							}
						else
							{
							p_nodesqr p =  new nodesqr<T,R>(0,0,0,0,NULL,true);
							if (p->load(hf,this,info) == false) {return false;}
							tab[i][j][k] = p;
							}
						}
					}
				}
			return true;
			}


		};






	/*****************************************************************************************************
	*
	* Sub classe leafsqr describing the strucure of leaf representing (2R+1)(2R+1)(2R+1) sites
	*
	******************************************************************************************************/
	template<class T,int64 R> class leafsqr : public basicsqr<T,R>
		{
		public:

		T tab[2*R+1][2*R+1][2*R+1];	// the array that contain the value at each site
		int nEmpty,nFull;		// number of empty and full values


		/* create an leaf square centered at cX,cY,cZ with father f filled with val*/
		leafsqr(int64 cX,int64 cY,int64 cZ,p_nodesqr f,const T & val, const infoGLZ3<T> * info ) : basicsqr(cX,cY,cZ,1,f)
			{
			for(int k=0;k<(2*R+1);k++) for(int j=0;j<(2*R+1);j++) for(int i=0;i<(2*R+1);i++) tab[i][j][k] = val;
			nEmpty = 0; nFull = 0;
			if (val == info->emptyVal) {nEmpty = (2*R+1)*(2*R+1)*(2*R+1);} else if (val == info->fullVal) {nFull = (2*R+1)*(2*R+1)*(2*R+1);}
			}

		/* copy ctor */
		leafsqr(const leafsqr<T,R> & leaf)  : basicsqr(leaf)
			{
			nEmpty = leaf.nEmpty; nFull = leaf.nFull;
			for(int k=0;k<(2*R+1);k++) for(int j=0;j<(2*R+1);j++) for(int i=0;i<(2*R+1);i++) tab[i][j][k] = leaf.tab[i][j][k];
			}

		/* returns 0 if the node is empty - 2 if the node is full  - 1 if the node is neither */
		inline int _status() const
			{
			if (nEmpty == (2*R+1)*(2*R+1)*(2*R+1)) {return 0;}
			if (nFull == (2*R+1)*(2*R+1)*(2*R+1)) {return 2;}
			return 1;
			}


		/* destroy the leaf and set the pointer in the father to deadlink 
		 * should only be called when the leaf node is either emptysqr or fullsqr */
		p_sqr _leafremove(const infoGLZ3<T> * info)
			{
			Insure(father != NULL);
			father->tab[father->getindexX(centerX)][father->getindexY(centerY)][father->getindexZ(centerZ)] = ((nEmpty == (2*R+1)*(2*R+1)*(2*R+1)) ?  &emptysqr : &fullsqr); // remove the pointer to this in the father array
			p_sqr p = father->clean(info); // recursively clean the ancestors and get the adress of the top one
			delete this; // delete itself, return the new ancestor
			return p;
			}


		/* writing itself to a file */
		bool save(FILE * hf) const
			{
			// save the common part
			if (((const basicsqr<T,R>*)(this))->save(hf) == false) {return false;}
			// save the specific part
			if (fwrite(&nEmpty,sizeof(int),1,hf) != 1) {return false;};
			if (fwrite(&nFull,sizeof(int),1,hf) != 1) {return false;};
			if (fwrite(&tab,sizeof(T),(2*R+1)*(2*R+1)*(2*R+1),hf) != ((2*R+1)*(2*R+1)*(2*R+1))) {return false;};
			return true;
			}


		/* loading itself from file */
		bool load(FILE * hf,p_nodesqr f)
			{
			// load the common part
			if (((basicsqr<T,R>*)(this))->load(hf,f) == false) {return false;}
			// load the specific part
			if (fread(&nEmpty,sizeof(int),1,hf) != 1) {return false;};
			if (fread(&nFull,sizeof(int),1,hf) != 1) {return false;};
			if (fread(&tab,sizeof(T),(2*R+1)*(2*R+1)*(2*R+1),hf) != ((2*R+1)*(2*R+1)*(2*R+1))) {return false;};
			return true;
			}


		};




}

}

}

#endif
/* end of file GLZ3internals.h */

