/****************************************************************************************************
* Internal classes used for GrowingLatticeZ2.h                             Version 1.0, Vindar 2014 *
*                                                                                                   *
* This file contains the implementations of the sub classes used for creating the GrowingLatticeZ2  *
* class exposed in GrowingLatticeZ2.h                                                               *
*                                                                                                   *
* it should not be called directly exept in GrowingLatticeZ2.h                                      *
*                                                                                                   *
* Here, everything is encupsulated into the namespace GLZ2internals                                 *                                                                  *
*                                                                                                   *
* standalone, no cpp file                                                                           *
****************************************************************************************************/
#ifndef _GLZ2INTERNALS_H_
#define _GLZ2INTERNALS_H_


namespace mylib
{
namespace mathgraph
{

/******************************************************
* Namespace GLZ2internals for isolating these classes
* from the rest of the world
******************************************************/
namespace GLZ2internals
{


	template<class T,int64 R> class nodesqr;			// forward declaration
	template<class T,int64 R> class leafsqr;			//



	/*****************************************************************************************************
	*
	* structure containing the info about the lattice
	*
	******************************************************************************************************/
	template<class T> struct infoGLZ2
		{
		T emptyVal;						// an empty element
		T fullVal;						// a full element for comparison
		int64 nbNE;						// number of site which are not empty
		int64 nbF;						// number of site which are full
		int64 Xmin,Xmax,Ymin,Ymax;		// smallest rectangle containing all the non empty site
		};






	/*****************************************************************************************************
	*
	* Base class describing a square
	*
	* -> can be sub-classed into:
	*     - nodesqr : a 3x3 node
	*     - leafsqr : a leaft representing a grid of size (2R+1)x(2R+1) sites
	*
	******************************************************************************************************/
	template<class T,int64 R> class basicsqr
		{

		public: 

			typedef basicsqr<T,R> * p_sqr;		// pointer types
			typedef leafsqr<T,R>  * p_leafsqr;	// 
			typedef nodesqr<T,R>  * p_nodesqr;	// 

			static basicsqr<T,R> fullsqr;	// dummy adress for full square
			static basicsqr<T,R> emptysqr; // dummy adress for an empty adress

			int64 centerX,centerY;		// position of the center of the square

			int64 sub_radius;			// radius of each sub-square of this square
										// if sub_radius = 1, this struct is a leaf square and its radius itself is R (so its diameter is 2R+1)
										// if sub_radius > 1, this struct is a node square and its radius itself is 3*sub_radius + 1

			p_nodesqr father;		    // pointeur to the father node. null iff we are at the root 


		/* ctor: create a subsquare centered at cX,cY with subradius sR and node father f */
		basicsqr(int64 cX,int64 cY,int64 sR,p_nodesqr f) : centerX(cX),centerY(cY),sub_radius(sR),father(f)  {return;} 


		private:

		/* private: used only for creating a dummy fullsqr and emptysqr */
		basicsqr() : centerX(0),centerY(0),sub_radius(0),father(0) {return;}

		public:

		/* basic copy ctor */
		basicsqr(const basicsqr<T,R> & sqr) 
			{
			centerX = sqr.centerX;
			centerY = sqr.centerY;
			sub_radius = sqr.sub_radius;
			father = NULL;
			}

		/* writing itself to a file */
		bool save(FILE * hf) const
			{
			if (fwrite(&centerX,sizeof(int64),1,hf) != 1) {return false;}
			if (fwrite(&centerY,sizeof(int64),1,hf) != 1) {return false;}
			if (fwrite(&sub_radius,sizeof(int64),1,hf) != 1) {return false;}
			return true;
			}


		/* loading itself from a file
		 * f = adress of the father whose memory should already be allocated */
		bool load(FILE * hf,p_nodesqr f)
			{
			if (fread(&centerX,sizeof(int64),1,hf) != 1) {return false;}
			if (fread(&centerY,sizeof(int64),1,hf) != 1) {return false;}
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
			for(int j=0;j<3;j++) 
				{
				for(int i=0;i<3;i++) 
					{
					p_sqr p = ((p_nodesqr)this)->tab[i][j];
					if ((p != &emptysqr)&&(p!=&fullsqr)) {p->destroytree();}
					}
				}
			delete((p_nodesqr)this);
			return;
			}


		/* compare this node (and it subtree) to p (and it subtree) and return true if they are equal */
		bool compare(const p_sqr p)
			{
			if (((father == NULL)&&(p->father != NULL))||((father != NULL)&&(p->father == NULL))) return false; // check if both are root
			if ((p->sub_radius != sub_radius)||(p->centerX != centerX)||(p->centerY != centerY)) return false; // check same node paramameter
			if (sub_radius ==1)
				{
				p_leafsqr pa = ((p_leafsqr)this), pb = ((p_leafsqr)p);
				if ((pa->nEmpty != pb->nEmpty)||(pa->nFull != pb->nFull)) {return false;}
				for(int i=0;i<2*R+1;i++) for(int j=0;j<2*R+1;j++) {if (pa->tab[i][j] != pb->tab[i][j]) {return false;}}
				return true;
				}
			p_nodesqr pa = ((p_nodesqr)this);
			p_nodesqr pb = ((p_nodesqr)p);
			for(int i=0;i<3;i++) for(int j=0;j<3;j++)
				{
				p_sqr ca = pa->tab[i][j]; p_sqr cb = pb->tab[i][j];
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

		/* Returns the value at x,y does not affect anything 
		 * look recursively in the entire tree (faster if already near)
		 * put in newhint the node associated with (x,y) (helps for subsequent queries)
		 * - if (x,y) outside of the tree, newhint is set to the root.
		 *   otherwise, newhint is the leaf or node which directly contain (x,y)
		 */
		inline T get(int64 x,int64 y,p_sqr & newhint,const infoGLZ2<T> * info) const
			{
			int64 rad = ((sub_radius == 1) ? R : (3*sub_radius + 1) );	// compute the radius of the current node
			int64 dx = (x - centerX); int64 dy = (y - centerY);			// position wrt current center
			if ((abs(dx) > rad)||(abs(dy) > rad))						// site not included in this square
				{ 
				if (father == NULL) {newhint = (p_sqr)this;	return info->emptyVal;}  // father not created, so the site must be initially empty
				return father->_get(x,y,newhint,info);						// look into the father
				}
			if (sub_radius == 1) {newhint = (p_sqr)this; return(((p_leafsqr)this)->tab[R + dx][R + dy]);} // we are in a leaf
			p_nodesqr pn = (p_nodesqr)this; p_sqr p = pn->tab[pn->getindexX(x)][pn->getindexY(y)]; // else find the subsquare corresponding to the point
			if (p == &emptysqr) {newhint = (p_sqr)this; return info->emptyVal;}
			if (p == &fullsqr)  {newhint = (p_sqr)this; return info->fullVal;}
			return p->_get(x,y,newhint,info);
			}
	

		/* Recursive version that cannot be inlined */
		T _get(int64 x,int64 y,p_sqr & newhint,const infoGLZ2<T> * info) const
			{
			int64 rad = ((sub_radius == 1) ? R : (3*sub_radius + 1) );	// compute the radius of the current node
			int64 dx = (x - centerX); int64 dy = (y - centerY);			// position wrt current center
			if ((abs(dx) > rad)||(abs(dy) > rad))						// site not included in this square
				{ 
				if (father == NULL) {newhint = (p_sqr)this; return info->emptyVal;}  // father not created, so the site must be initially empty
				return father->_get(x,y,newhint,info);						// look into the father
				}
			if (sub_radius == 1) {newhint = (p_sqr)this; return(((p_leafsqr)this)->tab[R + dx][R + dy]);} // we are in a leaf
			p_nodesqr pn = (p_nodesqr)this; p_sqr p = pn->tab[pn->getindexX(x)][pn->getindexY(y)]; // else find the subsquare corresponding to the point
			if (p == &emptysqr) {newhint = (p_sqr)this;	return info->emptyVal;}
			if (p == &fullsqr)  {newhint = (p_sqr)this; return info->fullVal;}
			return p->_get(x,y,newhint,info);
			}
	


		 /* find the enclosing full square of (X,Y)
		  * return true is the site (X,Y) is full and put the full square in Xmin,Xmax,Ymin,Ymax 
		  * return false if (X,Y) is not full (and do change to Xmin,Xmax,Ymin,Ymax )
		  * 
		  * as always, newhint now contain the node for (X,Y) or the root is (X,Y) outside of the tree
		  */
		bool getEnclosingRectFull(int64 X,int64 Y,int64 & Xmin,int64 & Xmax,int64 & Ymin,int64 & Ymax,p_sqr & newhint,const infoGLZ2<T> * info) const
			{ 
			if (get(X,Y,newhint,info) != info->fullVal) {return false;} // site not full nothing to do, return false;
			if (newhint->sub_radius == 1) {Xmin = X; Xmax = X; Ymin = Y; Ymax = Y; return true;} // we are on a leaf, no (easy) non trivial rectangle
			// not on a leaf, we find the radius and the center of the full square containing (x,y) 
			p_nodesqr pn = (p_nodesqr)newhint; 
			int64 cx = pn->computeCenterX(pn->getindexX(X));
			int64 cy = pn->computeCenterY(pn->getindexY(Y));
			Xmin = cx - pn->sub_radius; Xmax = cx + pn->sub_radius;
			Ymin = cy - pn->sub_radius; Ymax = cy + pn->sub_radius;
			return true;
			}


		/* Improved version 
		 * try to expand the square into a larger rectangle 
		 * more time expensive but usually return a bigger rectangle where (X,Y) is farther from the boundary
		 */
		bool improvedEnclosingRectFull(int64 X,int64 Y,int64 & Xmin,int64 & Xmax,int64 & Ymin,int64 & Ymax,p_sqr & newhint,const infoGLZ2<T> * info) const
			{ 
			if (!getEnclosingRectFull(X,Y,Xmin,Xmax,Ymin,Ymax,newhint,info)) {return false;}
			if (newhint->sub_radius == 1) {return true;} // we are on a leaf, cannot improve the result
			// ok we are on a node
			p_sqr p = newhint; // save the pointer
			p_sqr p2;
			int64 x,y;
			int64 Xmin_b,Xmax_b,Ymin_b,Ymax_b;
			// down-left corner, how much more can we go ?
			int64 c_down = Ymin; int64 c_left = Xmin;
			if (getEnclosingRectFull(Xmin - 1,Ymin - 1,Xmin_b,Xmax_b,Ymin_b,Ymax_b,p,info)) { c_down = Ymin_b; c_left = Xmin_b;}
			p2 = p;	// save the pointer to the lower left corner
			// left border, how much more can we go ?
			x = Xmin - 1; y = Ymin;
			int64 left = Xmin;
			if (getEnclosingRectFull(x,y,Xmin_b,Xmax_b,Ymin_b,Ymax_b,p,info))  
				{
				left  = Xmin_b;
				while((left < Xmin-1)&&(Ymax_b < Ymax))
					{
					y = Ymax_b +1;
					if (getEnclosingRectFull(x,y,Xmin_b,Xmax_b,Ymin_b,Ymax_b,p,info)) {left = max(left,Xmin_b);} else {left = Xmin;}
					}
				if (left >= Xmin-1) {left = Xmin;}
				}
			// up-left corner, how much more can we go ?
			int64 a_up = Ymax; int64 a_left = Xmin;
			if (getEnclosingRectFull(Xmin - 1,Ymax + 1,Xmin_b,Xmax_b,Ymin_b,Ymax_b,p,info)) { a_up = Ymax_b; a_left = Xmin_b;}
			// up border, how much more can we go ?
			x = Xmin; y = Ymax + 1;
			int64 up = Ymax;
			if (getEnclosingRectFull(x,y,Xmin_b,Xmax_b,Ymin_b,Ymax_b,p,info))  
				{
				up  = Ymax_b;
				while((up > Ymax+1)&&(Xmax_b < Xmax))
					{
					x = Xmax_b + 1;
					if (getEnclosingRectFull(x,y,Xmin_b,Xmax_b,Ymin_b,Ymax_b,p,info)) {up = min(up,Ymax_b);} else {up = Ymax;}
					}
				if (up <= Ymax+1) {up = Ymax;}
				}
			// up-right corner, how much more can we go ?
			int64 b_up = Ymax; int64 b_right = Xmax;
			if (getEnclosingRectFull(Xmax + 1,Ymax + 1,Xmin_b,Xmax_b,Ymin_b,Ymax_b,p,info)) { b_up = Ymax_b; b_right = Xmax_b;}
			// down border, how much more can we go ?
			p = p2; // restore the pointer to the lower left corner
			x = Xmin; y = Ymin - 1;
			int64 down = Ymin;
			if (getEnclosingRectFull(x,y,Xmin_b,Xmax_b,Ymin_b,Ymax_b,p,info))  
				{
				down  = Ymin_b;
				while((down < Ymin-1)&&(Xmax_b < Xmax))
					{
					x = Xmax_b +1;
					if (getEnclosingRectFull(x,y,Xmin_b,Xmax_b,Ymin_b,Ymax_b,p,info)) {down = max(down,Ymin_b);} else {down = Ymin;}
					}
				if (down >= Ymin-1) {down = Ymin;}
				}
			//down-right corner, how much more can we go ?
			int64 d_down = Ymin; int64 d_right= Xmax;
			if (getEnclosingRectFull(Xmax + 1,Ymin - 1,Xmin_b,Xmax_b,Ymin_b,Ymax_b,p,info)) { d_down = Ymin_b; d_right = Xmax_b;}
			// right border, how much more can we go ?
			x = Xmax + 1; y = Ymin;
			int64 right = Xmax;
			if (getEnclosingRectFull(x,y,Xmin_b,Xmax_b,Ymin_b,Ymax_b,p,info))  
				{
				right  = Xmax_b;
				while((right > Xmax+1)&&(Ymax_b < Ymax))
					{
					y = Ymax_b +1;
					if (getEnclosingRectFull(x,y,Xmin_b,Xmax_b,Ymin_b,Ymax_b,p,info)) {right = min(right,Xmax_b);} else {right = Xmax;}
					}
				if (right <= Xmax+1) {right = Xmax;}
				}
			// some constrain on the corner are removed if there is no extension in some direction
			if (up    == Ymax) {a_left = left; b_right = right;}
			if (down  == Ymin) {c_left = left; d_right = right;}
			if (left  == Xmin) {a_up = up; c_down = down;}
			if (right == Xmax) {b_up = up; d_down = down;}
			// compute a new (hopefully better) rectangle
			Xmax = min(b_right,min(right,d_right));
			Xmin = max(a_left,max(left,c_left));
			Ymax = min(a_up,min(up,b_up));
			Ymin = max(c_down,max(down,d_down));
			return true;
			}


		/* Set the value at x,y to v
		 * look recursively in the entire tree (faster if already near)
		 * this may destroy many object, including this one !
		 * return the new square containing (x,y) (the pointer returned is valid!)
		 */
		inline p_sqr set(const T & v,int64 x,int64 y,infoGLZ2<T> * info)
			{
			if (sub_radius == 1) // check if we are in the right leaf to speed up things
				{
				p_leafsqr lf = (p_leafsqr)this; // yes, in a leaf
				int64 dx = (x - centerX) + R,  dy = (y - centerY) + R;
				if ((dx >= 0)&&(dx < 2*R +1)&&(dy >= 0)&&(dy < 2*R +1)) // yes, in the right leaf
					{
					T & val = lf->tab[dx][dy]; // previous value at the site
					if (val == v) {return(this);} // nothing to do
					if (val == info->emptyVal)		 {(lf->nEmpty)--; info->nbNE++;} // site previously empty 
					else {if (val == info->fullVal)  {(lf->nFull)--; info->nbF--;}} // site previously full
					if (v == info->emptyVal) 
						{
						(lf->nEmpty)++; info->nbNE--; 
						if (lf->nEmpty == (2*R+1)*(2*R+1)) {return lf->_leafremove(info);} // leaf is empty, remove it and clean the tree recursively
						} 
					else {if (v == info->fullVal)  
						{
						(lf->nFull)++; info->nbF++;  
						if (lf->nFull == (2*R+1)*(2*R+1)) {return lf->_leafremove(info);} // leaf is full, remove it and clean the tree recursively
						}} 
					val = v; // leaf is neither empty not full : set the value
					return(this);
					}
				}
			return _set(v,x,y,info); // call the slow method
			}


		/* slow method */
		p_sqr _set(const T & v,int64 x,int64 y,infoGLZ2<T> * info)
			{
			p_sqr p; 
			if (get(x,y,p,info) == v) {return p;}      // get approximation in p and check if something must be done anyway
			if (p->father == NULL) // if at the root, we might need to zoom out to contain the point
				{
				while((abs(x) > (3*(p->sub_radius) + 1))||(abs(y) > (3*(p->sub_radius) + 1))) {p = ((p_nodesqr)p)->createfather();}
				}
			// now, we zoom in until we arrive to a leaf
			while(p->sub_radius != 1) // iterate until we are at the leaf
				{
				int i = ((p_nodesqr)p)->getindexX(x), j = ((p_nodesqr)p)->getindexY(y);
				((p_nodesqr)p)->create_child(i,j,info);	// otherwise create corresponding the child node containing (x,y)
				p = ((p_nodesqr)p)->tab[i][j];
				}
			return(p->set(v,x,y,info));	// we are now at the leaf, we now call the fast method
			}


		/* for debugging purpose : print the node in an std::string for indentation indent*/
		std::string printNode(std::string firstline,std::string nextlines)
			{
				if (sub_radius==1) return firstline + "-> Leaf (" + tostring(centerX) + "," + tostring(centerY) + ") : empty = " + tostring(((p_leafsqr)this)->nEmpty) + " : full = " + tostring(((p_leafsqr)this)->nFull) + " : other = " + tostring((2*R+1)*(2*R+1) - ((((p_leafsqr)this)->nFull) + (((p_leafsqr)this)->nEmpty) ) ) + "\n";
				std::string res = firstline + "-> Node (" + tostring(centerX) + "," + tostring(centerY) + ") : radius = " + tostring(3*sub_radius+1) + "\n";
				p_nodesqr p = (p_nodesqr)this;
				for(int i=0;i<3;i++) for(int j=0;j<3;j++)
					{
					std::string s1 = nextlines + "    |-[" + tostring(i) + "][" + tostring(j) + "] ";
					std::string s2 = nextlines + "    |        ";
					if (p->tab[i][j] == &fullsqr) {res += s1 + "-> Full\n";}
					else { if (p->tab[i][j] == &emptysqr) {res += s1 + "-> Empty\n";} else {res += (p->tab[i][j])->printNode(s1,s2);}}
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
			for(int i=0;i<3;i++) for(int j=0;j<3;j++)
				{
				if ((p->tab[i][j] != &fullsqr)&&(p->tab[i][j] != &emptysqr)) {s += (p->tab[i][j])->computeSize(nbn,nbl);}
				}
			return s;
			}

		};




	/* static dummy object indicating full and empty nodes */
	template<class T,int64 R> basicsqr<T,R>  basicsqr<T,R>::fullsqr;	// the fullsqr dummy object
	template<class T,int64 R> basicsqr<T,R>  basicsqr<T,R>::emptysqr;	// the emptysqr dummy object






	/*****************************************************************************************************
	*
	* Sub classe nodesqr describing the strucure of a 3x3 node
	*
	******************************************************************************************************/
	template<class T,int64 R>  class nodesqr : public basicsqr<T,R>
		{

		public:

		p_sqr tab[3][3];

		/* create an empty node square centered at cX,cY, with sub_radius sr and father f 
		 * if isempty == true, make it empty, otherwise make it full */
		nodesqr(int64 cX,int64 cY,int64 sr,p_nodesqr f,bool isempty) : basicsqr(cX,cY,sr,f)
			{
			p_sqr v = (isempty ? &emptysqr : &fullsqr);
			for(int j=0;j<3;j++) for(int i=0;i<3;i++) tab[i][j] = v; return;
			}

		/* create the starting node : empty node centered at zero at level 1 */
		nodesqr() : basicsqr(0,0,R,NULL)
			{
			for(int j=0;j<3;j++) for(int i=0;i<3;i++) tab[i][j] = &emptysqr; return;
			}

		/* copy ctor */
		nodesqr(const nodesqr<T,R> & node)  : basicsqr(node)
			{
			for(int j=0;j<3;j++) for(int i=0;i<3;i++) 
				{
				if (node.tab[i][j] == &(node.emptysqr)) {tab[i][j] = &emptysqr;} else {
				if (node.tab[i][j] == &(node.fullsqr)) {tab[i][j] = &fullsqr;} else {
					if ((node.tab[i][j])->sub_radius == 1)
						{
						tab[i][j] = new leafsqr<T,R>(*((p_leafsqr)(node.tab[i][j])));
						}
					else
						{
						tab[i][j] = new nodesqr<T,R>(*((p_nodesqr)(node.tab[i][j])));
						}
					(tab[i][j])->father = this;
					}}
				}
			}

		/* create the father of the node (ie the new root of the tree)
		 * and return it. This may destroy the object 
		 * if it is a node whose all sub square are emptsqr or all fullsqr */
		inline p_nodesqr createfather()
			{
			Assert((centerX == 0)&&(centerY == 0)&&(father == NULL));// make sure we are the current root
			int64 rad = 3*sub_radius + 1;	// compute the radius of the current node
			p_nodesqr p = new nodesqr<T,R>(0,0,rad,NULL,true);	// create an empty node
			father = p; // set the father
			int st = _status();
			if (st == 0) // current node completely empty, remove it
				{
				father->tab[1][1] = &emptysqr; delete this; return p; 
				}
			if (st == 2) // current node completely full, remove it
				{
				father->tab[1][1] = &fullsqr; delete this; return p; 
				}
			// current node remains
			father->tab[1][1] = this;
			return p;
			}


		/* compute the center (for each direction) of a given subsquare of this node (ie i = 0,1,2) */
		inline int64 computeCenterX(int i) const {return(centerX + ((i==0) ? (-(2*sub_radius + 1)) : ((i==1) ? 0 : (2*sub_radius + 1))) );}
		inline int64 computeCenterY(int j) const {return(centerY + ((j==0) ? (-(2*sub_radius + 1)) : ((j==1) ? 0 : (2*sub_radius + 1))) );}


		/* reciprocally, compute the index of the subsquare associated with a point */
		inline int getindexX(int64 z) const {int64 d = z - centerX;	return ((d < -sub_radius) ? 0 : ((d > sub_radius) ? 2 : 1) );}
		inline int getindexY(int64 z) const {int64 d = z - centerY;	return ((d < -sub_radius) ? 0 : ((d > sub_radius) ? 2 : 1) );}


		/* create the child (i,j) of the node */
		void create_child(int i,int j,const infoGLZ2<T> * info)
			{
			Assert((tab[i][j] == &fullsqr)||(tab[i][j] == &emptysqr));
			int64 cx = computeCenterX(i), cy = computeCenterY(j);	// compute the center of the new node
			if (sub_radius == R)  {tab[i][j] = new leafsqr<T,R>(cx,cy,this, ((tab[i][j] == &emptysqr) ? info->emptyVal : info->fullVal),info ); return;}	// the new node is a leaf
			tab[i][j] = new nodesqr<T,R>(cx,cy,(sub_radius-1)/3,this,(tab[i][j] == &emptysqr)); 											// the new node is a node
			return;
			}


		/* returns 0 if the node is empty - 2 if the node is full  - 1 if the node is neither 
		 * does not check if the sub-node themselve are clean. 
		 */
		inline int _status() const
			{
			int e=0,f=0;
			for(int j=0;j<3;j++) for(int i=0;i<3;i++) 
				{
					if (tab[i][j] == &emptysqr) {e++; if (f!=0) {return 1;}} 
					else {if (tab[i][j] == &fullsqr) {f++; if (e!=0) {return 1;}} else {return 1;}}
				}
			if (e == 9) return 0;
			return 2;
			}


		/* recursively clean the subsquare (going up only)
		 * this may destroy the object, return the new top one*/
		p_nodesqr clean(const infoGLZ2<T> * info)
			{
			int s = _status();
			if (father == NULL) // check if we are at the root
				{
				if (s == 2)	{p_nodesqr  p = createfather(); return p;} // root full : create the father and remove the node
				if (s == 0)	{p_nodesqr  p = new nodesqr<T,R>; delete this; return p;} // root empty, delete this and reset with a node at level 1
				if ((tab[0][0] == &emptysqr)&&(tab[0][1] == &emptysqr)&&(tab[0][2] == &emptysqr)&&(tab[1][0] == &emptysqr)&&(tab[1][2] == &emptysqr)&&(tab[2][0] == &emptysqr)&&(tab[2][1] == &emptysqr)&&(tab[2][2] == &emptysqr))
					{ // everything is empty except the center
					if (sub_radius == R) {return this;} // root at level 1, nothing to do
					if (tab[1][1] == &fullsqr) {return this;} // center subsquare full, nothing to do
					// ok, tab[1][1] is a real node and not a leaf, delete this root then clean the new root
					p_nodesqr  p = ((p_nodesqr)(tab[1][1]));
					p->father = NULL; // set as the new root
					delete(this); // delete this
					return p->clean(info); // recursiveley clean the new root
					}
				return this;
				}
			if (s == 1) return this;
			father->tab[father->getindexX(centerX)][father->getindexY(centerY)] = ((s==0) ? &emptysqr : &fullsqr); // set the subsquare in the father as empty or full
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
			char tt[3][3];
			for(int i=0;i<3;i++) for(int j=0;j<3;j++)
				{
				if (tab[i][j] == &emptysqr) {tt[i][j] = 0;} else 
					{
					if (tab[i][j] == &fullsqr) {tt[i][j] = 1;} else 
						{
						if ((tab[i][j])->sub_radius == 1)  {tt[i][j] = 2;} else {tt[i][j] = 3;}		
						} 
					}
				}
			if (fwrite(&tt,sizeof(char),3*3,hf) != (3*3)) {return false;}
			for(int i=0;i<3;i++) for(int j=0;j<3;j++)
				{
				if (tt[i][j] ==2) {if (((p_leafsqr)(tab[i][j]))->save(hf) == false) {return false;} }
				else {if (tt[i][j] ==3) {if (((p_nodesqr)(tab[i][j]))->save(hf) == false) {return false;} }}
				}
			return true;
			}


		/* loading itself from file */
		bool load(FILE * hf,p_nodesqr f,const infoGLZ2<T> * info)
			{
			// load the common part
			if (((p_sqr)(this))->load(hf,f) == false) {return false;} 
			// load the specific part
			char tt[3][3];
			if (fread(&tt,sizeof(char),3*3,hf) != (3*3)) {return false;}
			for(int i=0;i<3;i++) for(int j=0;j<3;j++)
				{
				if (tt[i][j] == 0) {tab[i][j] = &emptysqr;} else 
					{
					if (tt[i][j] == 1) {tab[i][j] = &fullsqr;} else 
						{
						if (tt[i][j] == 2) 
							{
							p_leafsqr p =  new leafsqr<T,R>(0,0,NULL,info->emptyVal,info);
							if (p->load(hf,this) == false) {return false;}
							tab[i][j] = p;
							}
						else
							{
							p_nodesqr p =  new nodesqr<T,R>(0,0,0,NULL,true);
							if (p->load(hf,this,info) == false) {return false;}
							tab[i][j] = p;
							}
						}
					}
				}
			return true;
			}


		};






	/*****************************************************************************************************
	*
	* Sub classe leafsqr describing the strucure of leaf representing (2R+1)(2R+1) sites
	*
	******************************************************************************************************/
	template<class T,int64 R> class leafsqr : public basicsqr<T,R>
		{
		public:

		T tab[2*R+1][2*R+1];	// the array that contain the value at each site
		int nEmpty,nFull;		// number of empty and full values


		/* create an leaf square centered at cX,cY, with father f filled with val*/
		leafsqr(int64 cX,int64 cY,p_nodesqr f,const T & val, const infoGLZ2<T> * info ) : basicsqr(cX,cY,1,f)
			{
			for(int j=0;j<(2*R+1);j++) for(int i=0;i<(2*R+1);i++) tab[i][j] = val;
			nEmpty = 0; nFull = 0;
			if (val == info->emptyVal) {nEmpty = (2*R+1)*(2*R+1);} else if (val == info->fullVal) {nFull = (2*R+1)*(2*R+1);}
			}

		/* copy ctor */
		leafsqr(const leafsqr<T,R> & leaf)  : basicsqr(leaf)
			{
			nEmpty = leaf.nEmpty; nFull = leaf.nFull;
			for(int j=0;j<(2*R+1);j++) for(int i=0;i<(2*R+1);i++) tab[i][j] = leaf.tab[i][j];
			}

		/* returns 0 if the node is empty - 2 if the node is full  - 1 if the node is neither */
		inline int _status() const
			{
			if (nEmpty == (2*R+1)*(2*R+1)) {return 0;}
			if (nFull == (2*R+1)*(2*R+1)) {return 2;}
			return 1;
			}


		/* destroy the leaf and set the pointer in the father to deadlink 
		 * should only be called when the leaf node is either emptysqr or fullsqr */
		p_sqr _leafremove(const infoGLZ2<T> * info)
			{
			Insure(father != NULL);
			father->tab[father->getindexX(centerX)][father->getindexY(centerY)] = ((nEmpty == (2*R+1)*(2*R+1)) ?  &emptysqr : &fullsqr); // remove the pointer to this in the father array
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
			if (fwrite(&tab,sizeof(T),(2*R+1)*(2*R+1),hf) != ((2*R+1)*(2*R+1))) {return false;};
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
			if (fread(&tab,sizeof(T),(2*R+1)*(2*R+1),hf) != ((2*R+1)*(2*R+1))) {return false;};
			return true;
			}


		};






}

}

}

#endif
/* end of file GLZ2internals.h */

