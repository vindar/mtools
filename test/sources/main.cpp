#include "stdafx_test.h"

#include "mtools.hpp"
using namespace mtools;







template<typename T, size_t DIM, uint32 MINFILL = 5, uint32 MAXFILL = 12, typename TFloat = double> class RStarTree
	{


	public:


	typedef mtools::Vec<TFloat, DIM> Pos;	// type representing a position
	typedef mtools::Box<TFloat, DIM> MBR;   // type representing a minimal bounding rectangle


	/* forward declaration */
	struct RTreeBase;
	typedef RTreeBase* pRTreeBase;
	struct RTreeNode;
	typedef RTreeNode* pRTreeNode;
	struct RTreeLeaf;
	typedef RTreeLeaf* pRTreeLeaf;

	/* base stucture for an element of the tree */
	struct RTreeBase
		{
		uint32 nbchilden;			// number of children
		MBR mbrs[MAXFILL];			// MBRs of children
		};

	/* structure for a node */ 
	struct RTreeNode : public RTreeBase
		{
		pRTreeBase links[MAXFILL];		// pointers to the children of the node
		};

	/* structure for a leaf */
	struct RTreeLeaf : public RTreeBase
		{
		T data[MAXFILL];			// data associated with the children
		};


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
		_size  = 0;
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
	* Return the depth of the tree. (0 if empty)
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
		if (_callDtors) { _poolLeaf.destroyAndDeallocateAll(); } else { _poolLeaf.deallocateAll(); }
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
/*		s += std::string("Grid_basic<") + mtools::toString(D) + " , " + typeid(T).name() + " , " + mtools::toString(R) + ">\n";
		s += std::string(" - Memory : ") + mtools::toStringMemSize(memoryUsed()) + " / " + mtools::toStringMemSize(memoryAllocated()) + "\n";
		s += std::string(" - Range min = ") + _rangemin.toString(false) + "\n";
		s += std::string(" - Range max = ") + _rangemax.toString(false) + "\n";
		if (debug) { s += "\n" + _printTree(_getRoot(), ""); }*/
		return s;
		}



	private:







		size_t      _size;		// number of elements in the tree
		size_t      _depth;		// depth of the tree
		pRTreeBase  _root;		// pointer to the root
		MBR         _globalMBR; // global MBR for the whole tree. 

		bool _callDtors;		// should we call the destructors
		SingleObjectAllocator<RTreeLeaf>  _poolLeaf;       // the two memory pools
		SingleObjectAllocator<RTreeNode>  _poolNode;       //


	

	};









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

MT2004_64 gen(5679); // RNG with 2M vertices.
int main(int argc, char *argv[])
    {


	MTOOLS_SWAP_THREADS(argc, argv);
	parseCommandLine(argc, argv);


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



    }

/* end of file main.cpp */






