#include "stdafx_test.h"

#include "mtools.hpp"
using namespace mtools;





namespace mtools
	{

	namespace internals_rstartree
		{

		/* type representing a position */
		template<size_t DIM, typename TFloat> using Pos = mtools::Vec<TFloat, DIM>;


		/* type representing a minimal bounding rectangle */
		template<size_t DIM, typename TFloat> using MBR = mtools::Box<TFloat, DIM>;  


		/* base stucture for an element of the tree */
		template<size_t DIM, uint32 MAXFILL, typename TFloat> struct RTreeBase
			{
			uint32			nbchildren;			// number of children
			uint32			indexinfather;		// index of this node in its father's children list.
			RTreeBase*		father;				// pointer to the father
			MBR<DIM,TFloat>	mbrs[MAXFILL];		// MBRs of children
			};


		/* structure for a node */
		template<size_t DIM, uint32 MAXFILL, typename TFloat> struct RTreeNode : public RTreeBase<DIM,MAXFILL,TFloat>
			{
			RTreeBase* sons[MAXFILL];		// pointers to the children of the node
			};


		/* structure for a leaf */
		template<typename T, size_t DIM, uint32 MAXFILL, typename TFloat> struct RTreeLeaf : public RTreeBase<DIM,MAXFILL,TFloat>
			{
			T data[MAXFILL];				// data associated with the children
			};


		/* default delete function, return true */
		template<typename T, size_t DIM, typename TFloat> inline bool defaultDeleteFun(const T & obj, const MBR<DIM,TFloat> & mbr) { return true; }


		/* return the distance between a point and a filled mbr
		  ie it is zero if the point is contained in the mbr */
		template<size_t DIM, typename TFloat> inline TFloat SqrDistPosMBR(const Pos<DIM, TFloat> & pos, const MBR<DIM, TFloat> & mbr)
			{
			TFloat d2 = 0;
			for (size_t d = 0; d < DIM; d++)
				{
				auto a = mbr.min[d] - pos[d]; 
				if (a > 0) { d2 += a*a; }
				else
					{ 
					auto b = pos[d] - mbr.max[d];
					if (b > 0) { d2 += b*b; }
					}
				}
			return d2;
			}

		/* default distance from a point to an object: return the distance from the point to the filled MBR.
		(i.e. 0 if the pos is contained in mbr) */
		template<typename T, size_t DIM, typename TFloat> inline TFloat defaultSqrDistFun(const Pos<DIM,TFloat> & pos, const T & obj, const MBR<DIM, TFloat> & mbr) { return SqrDistPosMBR<DIM,TFloat>(pos,mbr); }

		}






	template<typename T, size_t DIM, uint32 MINFILL = 5, uint32 MAXFILL = 12, typename TFloat = double> class RStarTree
		{


		public:

			typedef mtools::internals_rstartree::Pos<DIM,TFloat>	Pos;	
			typedef mtools::internals_rstartree::MBR<DIM, TFloat>	MBR;  
			typedef mtools::internals_rstartree::RTreeBase<DIM, MAXFILL, TFloat>		RTreeBase;
			typedef mtools::internals_rstartree::RTreeBase<DIM, MAXFILL, TFloat>*		pRTreeBase;
			typedef mtools::internals_rstartree::RTreeNode<DIM, MAXFILL, TFloat>		RTreeNode;
			typedef mtools::internals_rstartree::RTreeNode<DIM, MAXFILL, TFloat>*		pRTreeNode;
			typedef mtools::internals_rstartree::RTreeLeaf<T, DIM, MAXFILL, TFloat>		RTreeLeaf;
			typedef mtools::internals_rstartree::RTreeLeaf<T, DIM, MAXFILL, TFloat>*	pRTreeLeaf;


			/**
			 * Constructor.
			 *
			 * @param	callDtors	true to call the destructors when deleting objects (default).
			 **/
			RStarTree(bool callDtors = true) : _size(0), _depth(0), _root((pRTreeBase)nullptr), _globalMBR(), _callDtors(callDtors)
				{
				}


			/**
			 * Move Constructor.
			 **/
			RStarTree(RStarTree && otree) : _size(otree._size), _depth(otree.depth), _root(otree._root), _globalMBR(otree._globalMBR), _callDtors(otree._callDtors), _poolLeaf(std::move(otree._poolLeaf)), _poolNode(std::move(otree._poolNode))
				{
				_size = 0;
				_depth = 0;
				_root = (pRTreeBase)nullptr;
				_globalMBR = MBR();
				_callDtors = false;
				}


			/**
			* Constructor. Loads the tree from a file
			*
			* @param   filename    Filename of the file.
			**/
			RStarTree(const std::string & filename) : _size(0), _depth(0), _root((pRTreeBase)nullptr), _globalMBR(), _callDtors(true)
				{
				load(filename);
				}


			/**
			* Constructor. Loads the tree from a file
			*
			* @param   filename    Filename of the file.
			**/
			RStarTree(const char * str) : _size(0), _depth(0), _root((pRTreeBase)nullptr), _globalMBR(), _callDtors(true)
				{ // ctor needed together with the std::string ctor to prevent implicit conversion to bool and call of the wrong ctor.
				load(std::string(str));
				}


			/**
			* Destructor.
			**/
			~RStarTree()
				{
				reset();
				}


			/**
			 * Return the number of objects inside the tree.
			 **/
			size_t size() const { return _size; }


			/**
			* Return the depth of the tree. (0 if empty, 1 if the root is a leaf...)
			**/
			size_t depth() const { return _depth; }


			/**
			 * Return the global MBR enclosing all the objects in the tree.
			 *
			 * @return	The global MBR for the whole tree or a completely empty rectangle if the tree is empty.
			 **/
			MBR globalMBR() const { return _globalMBR; }


			/**
			* Resets the tree to its initial empty state.
			* Call the destructor of all the T objects if the flag callDtors is set.
			**/
			void reset()
				{
				_size = 0;
				_depth = 0;
				_root = (pRTreeBase)nullptr;
				_globalMBR = MBR();
				_poolNode.deallocateAll();
				if (_callDtors) { _poolLeaf.destroyAndDeallocateAll(); }
				else { _poolLeaf.deallocateAll(); }
				}


			/**
			* Serializes the tree into an OArchive. If T implement a serialize method recognized by
			* OArchive, it is used for serialization otherwise OArchive uses its default serialization
			* method (which correspond to a basic memcpy() of the object memory).
			*
			* @param [in,out]  ar  The archive object to serialise the tree into.
			* @sa  class OArchive, class IArchive
			**/
			void serialize(OArchive & ar) const
				{
				MTOOLS_ERROR("Not yet implemented");		// TODO
				}


			/**
			* Deserializes the tree from an IArchive. If T has a constructor of the form T(IArchive &), it
			* is used for deserializing the T objects in the tree. Otherwise, if T implements one of the
			* serialize methods recognized by IArchive, the objects in the tree are first position/default
			* constructed and then deserialized using those methods. If no specific deserialization method
			* is found, IArchive falls back to its default derialization method.
			*
			* If the tree is non-empty, it is first reset, possibly calling the ctor of the existing T
			* objects depending on the status of the callCtor flag.
			*
			* @param [in,out]  ar  The archive to deserialize the tree from.
			* @sa  class OArchive, class IArchive
			**/
			void deserialize(IArchive & ar)
				{
				MTOOLS_ERROR("Not yet implemented");		// TODO
				}


			/**
			* Saves the tree into a file (using the archive format). The file is compressed if it ends
			* with the extension ".gz", ".gzip" or ".z".
			*
			* The method simply call serialize() to create the archive file.
			*
			* @param   filename    The filename to save.
			* @return  true on success, false on failure.
			* @sa  load, serialize, deserialize, class OArchive, class IArchive
			**/
			bool save(const std::string & filename) const
				{
				try
					{
					OArchive ar(filename);
					ar & (*this); // use the serialize method.
					}
				catch (...)
					{
					MTOOLS_DEBUG("Error saving RStarTree object");
					return false;
					}
				return true;
				}


			/**
			* Loads a tree from a file.
			*
			* If the tree is non-empty, it is first reset, possibly calling the dtors of T object depending
			* on the status of the callDtor flag.
			*
			* The method simply call deserialize() to recreate the tree from the archive file.
			*
			* @param   filename    The filename to load.
			* @return  true on success, false on failure [in this case, the lattice is reset].
			* @sa  save, class OArchive, class IArchive
			**/
			bool load(const std::string & filename)
				{
				try
					{
					IArchive ar(filename);
					ar & (*this); // use the deserialize method.
					}
				catch (...)
					{
					MTOOLS_DEBUG("Error loading RStarTree object");
					_callDtors = false; reset(); _callDtors = true;
					return false;
					}
				return true;
				}


			/**
			* Check if we should call the destructors of T objects when they are not needed anymore.
			*
			* @return  true if we call the dtors and false otherwise.
			**/
			bool callDtors() const { return _callDtors; }


			/**
			* Set whether we should, from now on, call the destructor of object when they are removed.
			* @param   callDtor    true to call the destructors.
			**/
			void callDtors(bool callDtor) { _callDtors = callDtor; }


			/**
			* Return the memory currently allocated by the tree (in bytes).
			**/
			size_t memoryAllocated() const { return sizeof(*this) + _poolLeaf.footprint() + _poolNode.footprint(); }


			/**
			* Return the memory currently used by the tree (in bytes).
			**/
			size_t memoryUsed() const { return sizeof(*this) + _poolLeaf.used() + _poolNode.used(); }


			/**
			* Returns a string with some information concerning the object.
			*
			* @param   debug   Set this flag to true to enable the debug mode where the whole tree structure
			*                  is written into the string [should not be used for large trees].
			*
			* @return  an info string.
			**/
			std::string toString(bool debug = false) const
				{
				std::string s;
				s += std::string("RStarTree< T =") + typeid(T).name() + " , Dim = " + mtools::toString(DIM) + " , minfill = " + mtools::toString(MINFILL) + " , maxfill = " + mtools::toString(MAXFILL) + " , TFloat = " + typeid(TFloat).name() + " >\n";
				s += std::string(" - Memory : ") + mtools::toStringMemSize(memoryUsed()) + " / " + mtools::toStringMemSize(memoryAllocated()) + "\n";
				s += std::string(" - calling Dtors : ") + mtools::toString(_callDtors) + "\n";
				s += std::string(" - number of objects  : ") + mtools::toString(_size) + "\n";
				s += std::string(" - height of the tree : ") + mtools::toString(_depth) + "\n";
				s += std::string(" - global MBR : ") + mtools::toString(_globalMBR) + "\n";
				if (_root == nullptr) { s += "\n *** Empty tree ! ***\n"; return s; }
				if (!debug) return s;
				s += "\n";
				size_t nbo = 0;
				s +=_printTree(_root, 0,nbo);
				MTOOLS_INSURE(nbo == _size);
				return s;
				}

			
			/* for testing */
			void make()
				{
				_root = _poolNode.allocate();

				pRTreeLeaf leaf1 = _poolLeaf.allocate();
				leaf1->father = _root;
				leaf1->nbchildren = 2;
				leaf1->indexinfather = 0;
				leaf1->mbrs[0] = mtools::Box<TFloat, 2>(-1,2, -3,3);
				leaf1->mbrs[1] = mtools::Box<TFloat, 2>(0, 1, 5, 5);

				pRTreeLeaf leaf2 = _poolLeaf.allocate();
				leaf2->father = _root;
				leaf2->nbchildren = 3;
				leaf2->indexinfather = 1;
				leaf2->mbrs[0] = mtools::Box<TFloat, 2>(1, 1, 1, 1);
				leaf2->mbrs[1] = mtools::Box<TFloat, 2>(2, 4, 3, 3);
				leaf2->mbrs[2] = mtools::Box<TFloat, 2>(-1, 2, 0, 0);

				_root->nbchildren = 2;
				_root->father = nullptr;

				((pRTreeNode)_root)->sons[0] = leaf1;
				_root->mbrs[0] = unionRect(leaf1->mbrs[0], leaf1->mbrs[1]);

				((pRTreeNode)_root)->sons[1] = leaf2;
				_root->mbrs[1] = unionRect(leaf2->mbrs[0], unionRect(leaf2->mbrs[1], leaf2->mbrs[2]));

				_depth = 2;
				_size = 5;
				_globalMBR = unionRect(_root->mbrs[0], _root->mbrs[1]);
				}



			/************ INSERT METHODS *************/

			/**
			 * Insert a new item in the tree with a given mbr. 
			 *
			 * @param	obj	The object to insert
			 * @param	mbr	The associated minimal bounding rectangle. 
			 **/
			void insertItem(const T & obj, const MBR & mbr)
				{
				}


			/**
			* Insert a new item in the tree at a given position. Its mbr is reduced to a point
			*
			* @param	obj	The object to insert
			* @param	pos	The position of the object (i.e. mbr reduced to a point). 
			**/
			void insertItem(const T & obj, const Pos & pos)
				{
				}


			// TODO : add bulk insert method



			/************ DELETE METHODS *************/


			/**
			 * Removes all the item at position pos. 
			 * Only item with mbr reduced to point pos are removed.
			 *
			 * @param	pos	The position of the item to remove (the mbr of the item must match pos exactly). 
			 *
			 * @return	the number of item removed. 
			 **/
			template<typename FUNCTOR> size_t removeItemAtPos(const Pos & pos, FUNCTOR && deletefun = mtools::internals_rstartree::defaultDeleteFun<T,DIM,TFloat>)
				{

				}

			template<typename FUNCTOR> size_t removeItemContaining(const Pos & pos, FUNCTOR && deletefun = mtools::internals_rstartree::defaultDeleteFun<T, DIM, TFloat>)
				{

				}

			template<typename FUNCTOR> size_t removeItemContaining(const MBR & mbr, FUNCTOR && deletefun = mtools::internals_rstartree::defaultDeleteFun<T, DIM, TFloat>)
				{

				}

			template<typename FUNCTOR> size_t removeItemContainedInside(const MBR & mbr, FUNCTOR && deletefun = mtools::internals_rstartree::defaultDeleteFun<T, DIM, TFloat>)
				{

				}

			template<typename FUNCTOR> size_t removeItemIntersecting(const MBR & mbr, FUNCTOR && deletefun = mtools::internals_rstartree::defaultDeleteFun<T, DIM, TFloat>)
				{

				}

			template<typename FUNCTOR> size_t removeAll(FUNCTOR && deletefun = mtools::internals_rstartree::defaultDeleteFun<T, DIM, TFloat>)
				{

				}


			/************ FIND METHODS *************/


			template<typename FUNCTOR> size_t findItemAtPos(const Pos & pos, FUNCTOR && fun)
				{

				}

			template<typename FUNCTOR> size_t findItemContaining(const Pos & pos, FUNCTOR && fun)
				{

				}

			template<typename FUNCTOR> size_t findItemContaining(const MBR & mbr, FUNCTOR && fun)
				{

				}

			template<typename FUNCTOR> size_t findItemContainedInside(const MBR & mbr, FUNCTOR && fun)
				{

				}

			template<typename FUNCTOR> size_t findItemIntersecting(const MBR & mbr, FUNCTOR && fun)
				{

				}

			template<typename FUNCTOR> size_t findAll(FUNCTOR && fun)
				{

				}



			/************ OTHER METHODS *************/


			template<typename FUNCTOR> std::pair<T,MBR> findNearest(const Pos & pos, FUNCTOR && fun = mtools::internals_rstartree::defaultSqrDistFun<T,DIM,TFloat>)
				{

				}




		private:




			/* compute the MBR of all the children of N */
			inline MBR _computeMBR(const pRTreeBase N) const
				{
				if (N->nbchildren == 0) return MBR();
				MBR mbr = N->mbrs[0];
				for (uint32 i = 1; i < N->nbchildren; i++) { mbr = mtools::unionRect(mbr, N->mbrs[i]); }
				return mbr; 
				}



			/* recursive method to print the tree (debug purpose only) */
			std::string _printTree(RTreeBase * N, int depth, size_t & nbo) const
				{
				std::string tab(4*depth, ' ');
				std::string s;
				MTOOLS_INSURE(N != nullptr);
				MTOOLS_INSURE(depth < _depth);
				MBR mbr = _computeMBR(N);				
				if (depth == 0)
					{
					MTOOLS_INSURE(N->father == nullptr);
					MTOOLS_INSURE(mbr == _globalMBR);
					}
				else
					{
					MTOOLS_INSURE(N->father != nullptr);
					MTOOLS_INSURE(N->indexinfather < N->father->nbchildren);
					MTOOLS_INSURE(N->father->mbrs[N->indexinfather] == mbr);
					}					
				if (depth +1 < _depth)
					{ // deal with a node
					MTOOLS_INSURE(_poolNode.isInPool(N));
					RTreeNode * NN = (RTreeNode *)N;
					s += tab + "NODE [" + mtools::toString((uintptr_t)NN) + "]\n";
					s += tab + "- depth = " + mtools::toString(depth) + "]\n";
					s += tab + "- index in father = " + mtools::toString(NN->indexinfather) + "\n";
					s += tab + "- nb children     = " + mtools::toString(NN->nbchildren) + "\n";
					s += tab + "- total MBR       = " + mtools::toString(mbr) + "\n";
					for (uint32 i = 0;i < NN->nbchildren; i++)
						{
						s += tab + "-- child " + mtools::toString(i) + "   MBR = " + mtools::toString(NN->mbrs[i]) + " --> [" + mtools::toString((uintptr_t)NN->sons[i]) + "]\n";
						MTOOLS_INSURE(NN->sons[i] != nullptr);
						MTOOLS_INSURE(NN->sons[i]->father == N);
						}
					s += "\n";
					for (uint32 i = 0;i < NN->nbchildren; i++) { s += _printTree(NN->sons[i], depth + 1, nbo); }
					return s;
					}
				if (depth + 1 == _depth)
					{ // deal with a leaf
					MTOOLS_INSURE(_poolLeaf.isInPool(N));
					RTreeLeaf * LL = (RTreeLeaf *)N;
					s += tab + "LEAF [" + mtools::toString((uintptr_t)LL) + "]\n";
					s += tab + "- depth = " + mtools::toString(depth) + "\n";
					s += tab + "- index in father = " + mtools::toString(LL->indexinfather) + "\n";
					s += tab + "- nb children     = " + mtools::toString(LL->nbchildren) + "\n";
					s += tab + "- total MBR       = " + mtools::toString(mbr) + "\n";
					for (uint32 i = 0; i < LL->nbchildren; i++)
						{
						s += tab + "-- child " + mtools::toString(i) + "   MBR = " + mtools::toString(LL->mbrs[i]) + "\n";
						}
					nbo += LL->nbchildren;
					s += "\n";
					return s;
					}
				MTOOLS_ERROR("DEPTH ERROR!\n");
				}



			size_t      _size;		// number of elements in the tree
			size_t      _depth;		// depth of the tree
			pRTreeBase  _root;		// pointer to the root
			MBR         _globalMBR; // global MBR for the whole tree. 

			bool _callDtors;		// should we call the destructors
			SingleObjectAllocator<RTreeLeaf>  _poolLeaf;       // the two memory pools
			SingleObjectAllocator<RTreeNode>  _poolNode;       //


		};



	}









RGBc color(int64 x, int64 y)
	{
	if (x*x + y*y < 10000)
		{
		uint32 c = 1;
		for (int i = 0; i < 100; i++) { c += (x*x) % 100 + (y*y) % 200; }
		c = c % 255;
		return RGBc(c,c,c);
		}
	return RGBc::c_TransparentWhite;
	}



std::vector<double> tvv;




/*

template<typename T> class opaque_fltk
	{


		void * operator->() {return }
	private:

		void * _p;
	}

struct opaque_p_Fl_Widget 
	{
	void * _p;
	};





Fl_Widget  & operator*(opaque_p_Fl_Widget & p)
	{
	return *((Fl_Widget*)p._p);
	}

const Fl_Widget  & operator*(const opaque_p_Fl_Widget & p)
	{
	return *((Fl_Widget*)p._p);
	}

Fl_Widget * operator->(opaque_p_Fl_Widget & p)
	{
	return ((Fl_Widget*)p._p);
	}


const Fl_Widget  & operator*(const opaque_p_Fl_Widget & p)
	{
	return *((Fl_Widget*)p._p);
	}

	*/



MT2004_64 gen(5679); // RNG with 2M vertices.
int main(int argc, char *argv[])
	{
	tvv.resize(1);
	tvv[0] = 77;



	tvv.resize(1000);

	for (int i = 0;i < 1000;i++)
		{
		tvv[i] = sin((double)i / 100);
		}

	Plotter2D plot;

	auto P1 = makePlot2DVector(tvv);
	plot[P1];

	plot.plot();
	return 0;


	MTOOLS_SWAP_THREADS(argc, argv);
	parseCommandLine(argc, argv);

	{
	OArchive ar("test.tst.gz");

	int z = 1234567;
	ar & z;
	}
	
	{
	IArchive ir("test.tst.gz");

	int e = 0;

	ir & e;
	cout << e << "\n";
	}

	cout.getKey();

	return 0;

	RStarTree<int, 2> RST;

	RST.make();

	cout << RST.toString(true) << "\n";

	cout.getKey();

	/*
	Img<unsigned char> im; 
	im.load("lenna.jpg");

	Plotter2D plotter;


	auto P3 = makePlot2DCImg(nullptr, 6,"image");

	plotter[P3];

	plotter.startPlot(); 

	cout.getKey();

	P3.image(im);

	cout.getKey();

	P3.image(nullptr);
	
	cout.getKey();
	
	P3.image(im);

	cout.getKey();

	plotter.endPlot();

	return 0;

	auto P1 = makePlot2DLattice(color, "Lattice");
	auto P2 = makePlot2DPixel(color, 6, "Pixel");

	plotter[P1][P2];
	plotter.plot();
	return 0;
	*/


    }

/* end of file main.cpp */






