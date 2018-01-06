/****************************************************************************************************
* TEMPLATE CLASS ContactProcessOnTree                                      Version 1.0, Vindar 2014 *
*                                                                                                   *
* class used for simulating a classical contact process on a (possibly random) tree                 *
*                                                                                                   *
* EXAMPLE:                                                                                          *
*                                                                                                   *
#include "mylib_all.h"
#include "ContactProcessOnTree.h"
MT2004_64  gen;																				// the random generator
int16 reproGW(int64 depth) {int16 i = 1; while(gen.rand_double0() < 0.5) {i++;} return i;} // the tree is GW with geom(1/2) reproductuion law
bool startinfected(int64 depth) {if (gen.rand_double0() < 0.8) return true; return false;} // start with bernouilli(0.8) distribution of infected sites

int main (int argc, char *argv[])
	{
	out << "initialising... ";
	ContactProcess::ContactProcessOnTree<reproGW,startinfected> CP(16); // create the tree and infect some site up to level 16
	out << "start ! \n\n";
	double lambda = 0.48; // 0.477 = dies 0.478  =survival
	while(1) {CP.stats(); // write some info about the process
			  for(int i=0;i<10000000;i++) {CP.action(lambda,gen.rand_double0(),gen.rand_double0()); if (CP.nbSiteInfected() == 0) {CP.stats(); return 0;}}} // make 10000000 operations
	return 0;
	}                                                                                                 
*                                                                                                   *
* standalone, no cpp file                                                                           *
****************************************************************************************************/
#ifndef _CONTACTPROCESSONTREE_H_
#define _CONTACTPROCESSONTREE_H_

#include "crossplatform.h"
#include <vector>

namespace mylib
{

namespace models
{

namespace ContactProcess
{

/* forward declaration */
namespace 
{
typedef struct TreeNode * pTreeNode;
struct TreeNode;
template<int16 initsite(int64 depth)> class ContactTree;
class DepthVector;
}

/*************************************************************************************************************************************
*
* The contact tree class
*
* The main class for simulating a contact process on a tree.
*
* int16 initsite(int64 depth) : template function of the class used when dynamically constructing the underlying tree graph
*                               of the process. 
*                               - depth = depth of the current site to construct.
*                               - return = the number of children of this site (between 0 and 32767)
*
*
* bool initinfected(int64 depth) : template function used in the init phase to determine if a site is initially infected
*                                  - depth = detph of the current site
*                                  - returns = true if the site is to be initally infected and false otherwise
* 
* methods :
*
*	- int action(double lambda,double r1,double r2)		make an elementary step in the contact process (healing or infection)
*	- void stats()										print out some stats about the process
*	- int64 nbSiteInfected()							number of infected sites
*	- int64 nbEdgeInfected()							number of infected edges
*	- int64 minInfectedDepth()							depth of the current minimum infected site
*	- int64 maxInfectedDepth()							depth of the current maximum infected site
*	- int64 minAbsInfectedDepth()						depth of the minimum infected site since the last call to resetAbsDepth()
*	- int64 maxAbsInfectedDepth()						depth of the maximum infected site since the last call to resetAbsDepth()
*	- void resetAbsDepth()								reset the value of minAbsInfectedDepth() and maxAbsInfectedDepth() to those of the current min and max depths of the infected sites
*	- int64 treeDepth()									return the depth of the tree constructed so far
*	- int64 treeInfecteDepth()							return the maximum depth of all time infected sites
*
*************************************************************************************************************************************/
template<int16 initsite(int64 depth),bool initinfected(int64 depth)> class ContactProcessOnTree
{
public:

	/* ctor
	 * Initialise all the site of the tree up to some depth  
	 * for each site created, this calls the initinfected function to check
	 * whether the site is initially infected */
	ContactProcessOnTree(int depth) 
		{
		init(depth,tree.getRoot());
		return;
		}


	/* dtor */
	~ContactProcessOnTree() {return;}


	/* Make an action for the process with healing parameter 1 and infection parameter lambda
	 * lambda = infection rate (each edge infect after an exponential time of expectation 1/lambda)
	 * r1,r2 = two random number in [0,1)  (ussually obtained with the .rand_double0() method)
	 *
	 * returns : 0 if nothing happenned (infection between site already infected or from the root to its father)
	 *          -1 if a previopusly infected site was healed
	 *          +1 if a new site was infected */
	inline int action(double lambda,double r1,double r2)
	{
	int64 nbs = nbSiteInfected();
	int64 nbe = nbEdgeInfected();
	if (nbs==0) return 0;
	double a = r1*(lambda*nbe + nbs);
	if (a < nbs)// heal a site
		{
		int64 i = (int64)(r2*nbs);
		healSite(i); 
		return -1;
		}
	// infect a site
	int64 i = (int64)(r2*nbe);
	return infectSiteFromEdge(i);
	}


	/* print out some stats about the process */
	void stats()
	{
		out << "- infected sites : " << nbSiteInfected() << " / " << tree.nbSites() << "\n"; // number of site infected vs number of site created in the tree
		out << "- density : " << mylib::doubletostring_nice(((double)nbSiteInfected())/(tree.nbSites())) << "\n"; // the ratio of the number above
		out << "- infected depth range : [" << DV.minAbsDepth() << " , " << DV.maxAbsDepth() << "]   now : [" << DV.minDepth() << " , " << DV.maxDepth() << "]\n"; // min and max depth of infected sites
		out << "- maximal depth : " << tree.maxInfectedDepth() << " / " << tree.maxDepth() << "\n\n"; // total depth of the tree and all time max obtained for an infected site
	}


	/* return the number of infected sites */
	inline int64 nbSiteInfected() const {return tree.nbSiteInfected();}


	/* return the number of edge adjacent to an infected site (counted twice if both site of the edge are infected) */
	inline int64 nbEdgeInfected() const {return tree.nbEdgeInfected();}


	/* return the depth of the current minimum infected site */
	inline int64 minInfectedDepth() const {return DV.minDepth();}


	/* return the depth of the current maximum infected site */
	inline int64 maxInfectedDepth() const {return DV.maxDepth();}


	/* return the depth of the minimum infected site since the last call to resetAbsDepth()*/
	inline int64 minAbsInfectedDepth() const {return DV.minAbsDepth();}


	/* return the depth of the maximum infected site since the last call to resetAbsDepth()*/
	inline int64 maxAbsInfectedDepth() const {return DV.minAbsDepth();}


	/* reset the value of minAbsInfectedDepth() and maxAbsInfectedDepth() to those of the current min and max depths of the infected sites */
	inline void resetAbsDepth() {DV.resetAbsDepth();}


	/* return the depth of the tree constructed so far */
	inline int64 treeDepth() const {tree.maxDepth()}


	/* return the maximum depth of all time infected sites */
	inline int64 treeInfecteDepth() const {tree.maxInfectedDepth();}



/****************************************************
* Private stuffs
****************************************************/
private:

	/* recursively infect site with probability a up to some depth */
	void init(int depth,pTreeNode p)
		{
		if (initinfected(p->depth)) // yes, we must infect this site
			{
			int16 b = tree.nbSons(p); // get the number of sons
			if (tab.size() < (size_t)(b+1)) {tab.resize(b+1);} // make sure tab[b] exists, create it is needed
			tree.infectNode(p,tab[b].size()); // set the site as infected in the tree and give its future position in the list
			tab[b].push_back(p); // add the site in the list of infected sites
			DV.add(p->depth); // add the site in the list of site infected in the depth vector
			}
		if (p->depth < depth) 
			{
			for(int i=0; i< tree.nbSons(p); i++) {init(depth,tree.getSon(p,i));}	// infect the subtree recursively
			}
		return;
		}

	/* heal a given site */
	void healSite(int64 nb)
		{
		Insure((nb>=0)&&(nb<nbSiteInfected()));
		int i = 0; 
		while(nb >= (int64)(tab[i].size())) 
			{
			nb -= tab[i].size(); 
			i++;
			} // put in i the index of the list and in nb the offset in this list
		tree.healNode(tab[i].at((size_t)nb));	// set the node as healed in the tree
		DV.remove((tab[i].at((size_t)nb))->depth); // remove the site in the depth vector
		if (nb != (tab[i].size()-1))	// if we are not the last node on the list we need to swap with the last one
			{
			pTreeNode p = tab[i].back(); // get the last node in the list
			tab[i].at((size_t)nb) = p;			 // move it to position nb in the list
			tree.infectNode(p,nb);		 // inform the tree that his new position of this node in the list is nb
			}
		tab[i].pop_back(); // remove the last entry in the list
		}


	/* infect a site through a given edge.
	 * return 1 if the infection succceeded (the other site was indeed healtly)
	 * return 0 if nothing changed (the site a the end of the edge was already infected) */
	int infectSiteFromEdge(int64 nb)
		{
		Insure((nb>=0)&&(nb<nbEdgeInfected()));
		int i = 0; while(nb >= (int64)((tab[i].size()*(i+1))) ) {nb -= (tab[i].size()*(i+1)); i++;} int16 b = (int16)(nb % (i+1)); nb = nb/(i+1); // put in i the index of the list and in nb the offset in this list and in b the number of the children
		pTreeNode p = tree.getSon(tab[i].at((size_t)nb),b); // get the destination node
		if (p == NULL) {return 0;} // do nothing if we are dealing with the root
		if (p->indexinfected != -1) {return 0;} // do nothing if the destination site is already infected
		b = tree.nbSons(p);		// get the number of sons of the new infected site p
		if (tab.size() < (size_t)(b+1)) {tab.resize(b+1);} // make sure tab[b] exists, create it is needed
		tree.infectNode(p,tab[b].size());	// tell the tree that the site is now infected and give the index we will use in the list
		tab[b].push_back(p);	// add it to the list of infected site with b children. 
		DV.add(p->depth); // add the site in the vector depth
		return 1;
		}


	std::vector< std::vector<pTreeNode> > tab; // the array containing the infected sites, ordred according to their arity
	ContactTree<initsite> tree;				   // the tree
	DepthVector DV;							   // the depth vector
};







/****************************************************************************************************************************************************************************
*
*
* PRIVATE CLASSES USED BY THE CONTACT PROCESS CLASS
*
*
*****************************************************************************************************************************************************************************/
namespace 
{


/*************************************************************************************************************************************
*
* The TreeNode structure
*
* sub-struct used for representing a node in the tree grpah of the contact process
*************************************************************************************************************************************/
typedef struct TreeNode * pTreeNode;

struct TreeNode
{
pTreeNode	father;			// pointer to the father. Null if it is the root.
pTreeNode	firstson;		// pointer to the first son. Null if no son or not yet defined.
int64       indexinfected;  // index in the corresponding list of infected (-1 if not infected)
int32		depth;			// depth of this node
int16 		nbson;			// number of son of this node. -1 if not yet defined, otherwise >=0.
int16 		idbrother;		// which brother is this node with respect to its parent between 0 and the number of brother - 1

/* attach a given number of sons to the node (nb= number of sons to attach, 0 if leaf)) */
inline void create_sons(int nb) 
	{
	nbson = nb; if (nb>0) {firstson = new TreeNode[nb]; for(int i=0;i<nb;i++) {firstson[i].father = this; firstson[i].firstson = NULL; firstson[i].nbson = -1; firstson[i].idbrother = (int16)i; firstson[i].indexinfected = -1; firstson[i].depth = depth+1;}}
	}

/* delete all the subtree below this node (but do not delete itself) */
void deletesubtree()
	{
	if (nbson<=0) return;
	for(int i=0;i<nbson;i++) {firstson[i].deletesubtree();} delete firstson;
	}
};




/*************************************************************************************************************************************
*
* The tree class
*
* sub-class representing the tree graph of the contact process (uses the TreeNode struct)
*************************************************************************************************************************************/
template<int16 initsite(int64 depth)> class ContactTree
{
public:

	/* ctor, construst a tree with just the root */
	ContactTree()
		{
		root = new TreeNode;
		root->father = NULL; root->firstson = NULL; root->nbson = -1; root->idbrother = 0; root->indexinfected = -1; root->depth = 0;
		reset();
		}

	/* dtor */
	~ContactTree()
		{
		root->deletesubtree();
		delete root;
		}

	/* reset the tree */
	void reset()
		{
		root->deletesubtree();
		root->father = NULL; root->firstson = NULL; root->nbson = -1; root->idbrother = 0; root->indexinfected = -1; root->depth = 0;
		nbsites = 1;
		nbsiteinfected = 0;
		nbedgeinfected = 0;
		maxdepth = 0;
		maxinfectedepth = -1;
		}

	inline pTreeNode getRoot() const {return root;}

	inline int64 maxDepth()	const {return maxdepth;}				// return the maximum depth of the tree 
	inline int64 nbSites() const {return nbsites;}				// return the number of sites in the tree
	inline int64 nbSiteInfected() const	{return nbsiteinfected;}	// return the number of infected sites on the tree
	inline int64 nbEdgeInfected() const	{return nbedgeinfected;}	// return the number of infected edge on the tree (oriented so edge betwwen two infected site are counted twice)
	inline int64 maxInfectedDepth()	const {return maxinfectedepth;}	// return the maximum depth ever obtained by an infected site


	/* return the number of sons of the node (this creates the sons if not yet done*/
	inline int nbSons(pTreeNode n)
		{
		if (n->nbson < 0) {int16 nb = initsite(n->depth); n->create_sons(nb); nbsites += nb; if (((n->depth + 1) > maxdepth)&&(nb>0)) {maxdepth = n->depth +1;}}	// creates the sons if needed
		return n->nbson;
		}

	/* return the i-th son of the node (return the father if pos is outside the range) */ 
	inline pTreeNode getSon(pTreeNode n,int pos) const
		{
		if ((pos < 0)||(pos >= n->nbson)) return n->father;
		return ((n->firstson) + pos);
		}

	/* set a Node as infected with a given pointer for the infected node list 
	   can also be used simply to update the pos of the pointer to the node in the list of infected nodes*/
	inline void infectNode(pTreeNode n, int64 pos)
		{
		if (n->indexinfected == -1) {nbsiteinfected++; nbedgeinfected += (nbSons(n) + 1); if (n->depth > maxinfectedepth) {maxinfectedepth = n->depth;}}
		n->indexinfected = pos;
		}

	/* set a node as healed */
	inline void healNode(pTreeNode n)
		{
		Insure(n->indexinfected != -1);
		if (n->indexinfected != -1) {nbsiteinfected--; nbedgeinfected -= (nbSons(n) + 1); n->indexinfected = -1;}
		}


private: 

	pTreeNode	root;
	int64		nbsites;
	int64		nbsiteinfected;
	int64		nbedgeinfected;
	int64		maxdepth;
	int64		maxinfectedepth;
};


/*************************************************************************************************************************************
*
* The depth vector class
*
* sub-class used for keeping a vector of the number of infected site at each depth of the tree
*************************************************************************************************************************************/
class DepthVector
{
public: 

	/* ctor */
	DepthVector() {reset();}

	/* reset the object to the empty state */
	void reset()
	{
		nb = 0;
		mini = 100000000000;
		maxi = -1;
		vec.clear();
		vec.resize(10000);
		resetAbsDepth();
	}

	/* return the minimum depth of an occupied site */
	inline int64 minDepth() const {return mini;}

	/* return the maximum depth of an occupied site */
	inline int64 maxDepth() const {return maxi;}

	/* return the absolute minimum depth since the last depthreset() */
	inline int64 minAbsDepth() const {return miniA;}

	/* return the absolute maximum depth since the last depthreset() */
	inline int64 maxAbsDepth() const {return maxiA;}

	/* reset the aboslute min et max depth values */
	inline void resetAbsDepth() {miniA = mini; maxiA = maxi;}

	/* return the number of occupied site with depth i */
	inline int64 nbAtDepth(int64 i) const {return vec[(size_t)i];} 

	/* add a new ocuppied site at depth i */
	inline void add(int64 i) 
		{
		if (i < mini) {mini = i; if (i < miniA) miniA=i;}
		if (i > maxi) {maxi = i; if (i > maxiA) maxiA=i; if (((int64)vec.size()) < (i+1)) {vec.resize((size_t)i+1);}}
		vec[(size_t)i]++;
		nb++;
		}

	/* remove an occupied site at depth i */
	inline void remove(int64 i) 
		{
		if (nb <= 1) {reset(); return;}
		vec[(size_t)i]--;
		nb--;
		if ((i == mini)&&(vec[(size_t)i] <= 0))	{while(vec[(size_t)i] <=0) {i++;} mini = i; return;}
		if ((i == maxi)&&(vec[(size_t)i] <= 0)) {while(vec[(size_t)i] <=0) {i--;} maxi = i; return;}
		}

private:

	int64 nb;
	int64 mini,maxi;
	int64 miniA,maxiA;
	std::vector<int64> vec;

};


}

}

}

}
#endif
/* end of file ContactProcessOnTree.h */

