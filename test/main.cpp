
#include "mtools/mtools.hpp"
using namespace mtools;




class TestImage : public Image
	{

	public:



	TestImage(int64 lx, int64 ly) : Image(lx, ly) {}
	

	/**
	* Fill the interior of a circle.
	*
	* The circle border is not drawn, use draw_filled_circle to draw both border and interior simultaneously.
	*
	* @param	P			   position of the center.
	* @param	r			   radius.
	* @param	color_interior color of the interior.
	* @param	blend		   true to use blending.
	*/
	inline void fill_circle_new(iVec2 P, int64 r, RGBc color_interior, bool blend)
		{
		if (isEmpty() || (r < 1)) return;
		iBox2 circleBox(P.X() - r, P.X() + r, P.Y() - r, P.Y() + r);
		iBox2 imBox = imageBox();
		iBox2 B = intersectionRect(circleBox, imBox);
		if (B.isEmpty()) return; // nothing to draw. 
		if (circleBox.isIncludedIn(imBox))
			{ // circle is completely inside the image
			if (blend) _draw_circle<true, false, false, true, false>(P.X(), P.Y(), r, RGBc::c_White, color_interior, 0); else _draw_circle<false, false, false, true, false>(P.X(), P.Y(), r, RGBc::c_White, color_interior, 0);
			return;
			}
		// partial drawing, use alternative drawing method
		if (blend) _draw_circle2<true, false, true, false>(B, P, r, RGBc::c_White, color_interior, 0); else _draw_circle2<false, false, true, false>(B, P, r, RGBc::c_White, color_interior, 0);
		return;
		}


	/**
	* Draw a filled circle. The border and the interior color may be different.
	*
	* @param	P			  	position of the center.
	* @param	r			  	radius.
	* @param	color_border  	color for the border.
	* @param	color_interior	color of the interior.
	* @param	blend		  	true to use blending.
	**/
	inline void draw_filled_circle_new(iVec2 P, int64 r, RGBc color_border, RGBc color_interior, bool blend)
		{
		if (isEmpty() || (r < 1)) return;
		iBox2 circleBox(P.X() - r, P.X() + r, P.Y() - r, P.Y() + r);
		iBox2 imBox = imageBox();
		iBox2 B = intersectionRect(circleBox, imBox);
		if (B.isEmpty()) return; // nothing to draw. 
		if (circleBox.isIncludedIn(imBox))
			{ // circle is completely inside the image
			if (blend) _draw_circle<true, false, true, true, false>(P.X(), P.Y(), r, color_border, color_interior, 0); else _draw_circle<false, false, true, true, false>(P.X(), P.Y(), r, color_border, color_interior, 0);
			return;
			}
		// partial drawing, use alternative drawing method
		if (blend) _draw_circle2<true, true, true, false>(B, P, r, color_border, color_interior, 0); else _draw_circle2<false, false, true, false>(B, P, r, color_border, color_interior, 0);
		return;
		}


	/**
	* Draw a circle.
	*
	* @param	P				position of the center.
	* @param	r				radius.
	* @param	color			color to use.
	* @param	blend			true to use blending.
	* @param	antialiasing	true to use antialiasing.
	* @param	penwidth		The pen width (0 = unit width)
	**/
	inline void draw_circle_new(iVec2 P, int64 r, RGBc color, bool blend, bool antialiasing, int32 penwidth = 0)
		{
		if (isEmpty() || (r < 1)) return;
		iBox2 circleBox(P.X() - r, P.X() + r, P.Y() - r, P.Y() + r);
		iBox2 imBox = imageBox();
		if (penwidth > 0)
			{ // large pen
			_correctPenOpacity(color, penwidth);
			circleBox.enlarge(penwidth);
			iBox2 B = intersectionRect(circleBox, imBox);
			if (B.isEmpty()) return; // nothing to draw.
			if (circleBox.isIncludedIn(imBox))
				{ // included
				if (antialiasing)
					{
					if (blend) _draw_circle_AA<true, false, true>(P.X(), P.Y(), r, color, penwidth); else _draw_circle_AA<false, false, true>(P.X(), P.Y(), r, color, penwidth);
					}
				else
					{
					if (blend) _draw_circle<true, false, true, false, true>(P.X(), P.Y(), r, color, RGBc::c_White, penwidth); else _draw_circle<false, false, true, false, true>(P.X(), P.Y(), r, color, RGBc::c_White, penwidth);
					}
				return;
				}
			// not included
			if (B.area() * 64 > circleBox.area())
				{ // still faster to use draw everything using the first method and checking the range
				if (antialiasing)
					{
					if (blend) _draw_circle_AA<true, true, true>(P.X(), P.Y(), r, color, penwidth); else _draw_circle_AA<false, true, true>(P.X(), P.Y(), r, color, penwidth);
					}
				else
					{
					if (blend) _draw_circle<true, true, true, false, true>(P.X(), P.Y(), r, color, RGBc::c_White, penwidth); else _draw_circle<false, true, true, false, true>(P.X(), P.Y(), r, color, RGBc::c_White, penwidth);
					}
				return;
				}
			// use alternate method
			if (antialiasing)
				{
				if (blend) _draw_circle2_AA<true, true>(B, P, r, color, penwidth); else _draw_circle2_AA<false, true>(B, P, r, color, penwidth);
				}
			else
				{
				if (blend) _draw_circle2<true, true, false, true>(B, P, r, color, RGBc::c_White, penwidth); else _draw_circle2<false, true, false, true>(B, P, r, color, RGBc::c_White, penwidth);
				}			
			return;
			}
		iBox2 B = intersectionRect(circleBox, imBox);
		if (B.isEmpty()) return; // nothing to draw.
		if (circleBox.isIncludedIn(imBox))
			{ // included
			if (antialiasing)
				{
				if (blend) _draw_circle_AA<true, false, false>(P.X(), P.Y(), r, color, 0); else _draw_circle_AA<false, false, false>(P.X(), P.Y(), r, color, 0);
				}
			else
				{
				if (blend) _draw_circle<true, false, true, false, false>(P.X(), P.Y(), r, color, RGBc::c_White, 0); else _draw_circle<false, false, true, false, false>(P.X(), P.Y(), r, color, RGBc::c_White, 0);
				}
			return;
			}
		// not included
		if (B.area() * 64 > circleBox.area())
			{ // still faster to use draw everything using the first method and checking the range
			if (antialiasing)
				{
				if (blend) _draw_circle_AA<true, true, false>(P.X(), P.Y(), r, color, 0); else _draw_circle_AA<false, true, false>(P.X(), P.Y(), r, color, 0);
				}
			else
				{
				if (blend) _draw_circle<true, true, true, false, false>(P.X(), P.Y(), r, color, RGBc::c_White, 0); else _draw_circle<false, true, true, false, false>(P.X(), P.Y(), r, color, RGBc::c_White, 0);
				}
			return;
			}
		// use alternate method
		if (antialiasing)
			{
			if (blend) _draw_circle2_AA<true, false>(B, P, r, color, 0); else _draw_circle2_AA<false, false>(B, P, r, color, 0);
			}
		else
			{
			if (blend) _draw_circle2<true, true, false, false>(B, P, r, color, RGBc::c_White, 0); else _draw_circle2<false, true, false, false>(B, P, r, color, RGBc::c_White, 0);
			}
		return;
		}

	};










#define DIR_H   true
#define DIR_V   false



	template<class T, int N = 100, class TFloat = double> class TreeFigure
	{

		struct _ListNode;	// forward declarations.
		struct _BaseNode;	//


	public:


		/**************************************************************************************************
		* Public structures
		**************************************************************************************************/

		/**
		* typedef. Represent a bounding box.
		**/
		using BBox = Box<TFloat, 2>;


		/**
		* An object together, with its bounding box
		**/
		struct BoundedObject
		{
			BoundedObject(const T & obj, const BBox & bbox) : object(obj), boundingbox(bbox) {}

			BBox	boundingbox;
			T		object;
		};


		/** Handle to a bounded object inserted inside the tree. */
		struct Handle
		{

			/** get a const reference to the underlying bounded object. */
			const BoundedObject & boundedObject() const { return *(node()->_bobj); }

		private:

			friend class TreeFigure<T, N, TFloat>;

			/** private constructor */
			Handle(_ListNode * prevnode, _BaseNode * basenode) : _prevnode(prevnode), _basenode(basenode) {}

			/* get a pointer to the node referenced by this handle */
			MTOOLS_FORCEINLINE _ListNode * node() const { MTOOLS_ASSERT(_basenode);  return ((_node == nullptr) ? _basenode->_first : _node->_next); }

			_ListNode * _prevnode;	// The node before this object, or nullptr if it is the first object in the list
			_BaseNode * _basenode;	// the rec/sqr node where the object belong
		};



		/**************************************************************************************************
		* Public Methods
		**************************************************************************************************/



		/**
		* Default constructor, create an empty object.
		**/
		TreeFigure(bool callDtors = false) : _callDtors(callDtors), _rootNode(nullptr), _sqrNodePool(), _recNodePool(), _listNodePool()
		{
			_createRoot(); // create the root
		}


		/**
		* Destructor
		**/
		~TreeFigure()
		{
			_reset(); // release all allocated memory. 
		}


		/**
		* Move constructor.
		**/
		TreeFigure(const TreeFigure && TF) : _callDtors(TF._callDtors), _rootNode(TF._rootNode),
			_sqrNodePool(std::forward<decltype(_sqrNodePool)>(TF._sqrNodePool)),
			_recNodePool(std::forward<decltype(_recNodePool)>(TF._recNodePool)),
			_listNodePool(std::forward<decltype(_listNodePool)>(TF._listNodePool))
		{
			TF._rootNode = nullptr;
			TF._currentNode = nullptr;
		}


		/**
		* Move assignement operator.
		**/
		TreeFigure & operator=(const TreeFigure && TF)
		{
			_reset();
			_callDtors = TF._callDtors;
			_rootNode = TF._rootNode;
			_sqrNodePool = std::forward<decltype(_sqrNodePool)>(TF._sqrNodePool);
			_recNodePool = std::forward<decltype(_recNodePool)>(TF._recNodePool);
			_listNodePool = std::forward<decltype(_listNodePool)>(TF._listNodePool);
			TF._rootNode = nullptr;
			TF._currentNode = nullptr;
			return(*this);
		}


		/** Remove all objects. The TreeFigure returns to its initial state. */
		void reset()
		{
			_reset();
			_createRoot();
		}


		/**
		* Serialize this object.
		**/
		void serialize(OBaseArchive & ar, const int version = 0)
		{
			// TODO
		}


		/**
		* Deserialize this object.
		**/
		void deserialize(IBaseArchive & ar)
		{
			// TODO
		}



		/**
		* Insert an object. A copy of obj is made with the copy constructor.
		* Return a handle to the object.
		**/
		Handle insert(const T & object, const BBox & boundingBox)
		{

			return Handle(nullptr, nullptr);
			// TODO
		}


		/**
		* Remove an object referenced by its handle (fast).
		**/
		void remove(const Handle & handle)
		{
			MTOOLS_ASSERT(handle._basenode != nullptr);
			_ListNode * node;
			if (handle._prevnode != nullptr)
			{ // not the first object in the list. 
				node = (handle._prevnode)->_next;			// node to remove
				(handle._prevnode)->_next = node->_next;	// recreate link between the node neighours. 
			}
			else
			{
				node = (handle._basenode)->_first;			// node to remove
				(handle._basenode)->_first = node->_next;	// recreate link between the node neighours. 
			}
			if (_callDtors) _listNodePool.destroyAndFree(node); else _listNodePool.free(node);
			(handle._basenode)->dec_size();
		}


		/**
		* Query the number of object currently inserted.
		**/
		size_t size() const
		{
			return _listNodePool.size();
		}


		/**
		* Return the number of bytes malloced by this object.
		**/
		size_t footprint() const
		{
			return (_sqrNodePool.footprint() + _recNodePool.footprint() + _listNodePool.footprint());
		}


		/**
		* Print information about this object into a string.
		**/
		std::string toString(bool debug = false)
		{
			std::string s = std::string("TreeFigure<") + typeid(T).name() + ", " + mtools::toString(N) + ", " + typeid(TFloat).name() + ">\n";
			s += std::string(" - object inserted : ") + mtools::toString(size()) + "\n";
			s += std::string(" - memory used : ") + mtools::toStringMemSize(footprint()) + "\n";
			s += std::string(" - main bounding box : ") + mtools::toString(_rootNode->_bbox) + "\n";
			if (!debug) { return s + "---\n"; }

			// TODO

			return s;
		}



		/**************************************************************************************************
		* Private implementation.
		**************************************************************************************************/

	private:




		/** Create a new Node and put it in front of frontNode (which must be the first element of the list) */
		/*
		inline _ListNode * _createListNode(const T & obj, const BBox & bb, _ListNode * frontnode)
		{
		MTOOLS_ASSERT((frontnode == nullptr) || (frontnode->_prev == nullptr)); // make sure frontnode is the first element
		_ListNode * newnode = (_ListNode *)_listNodePool.malloc();
		::new(newnode) _ListNode(obj, bb, nullptr, frontnode);
		if (frontnode != nullptr) { frontnode->_prev = newnode; }
		return newnode;
		}
		*/

		/** delete a given list node and return the next one. */
		/*
		inline _ListNode * _deleteListNode(_ListNode * node)
		{
		MTOOLS_ASSERT(node != nullptr);
		if (node->_prev != nullptr) node->_prev->_next = node->_next->_prev;
		if (node->_next != nullptr) node->_next->_prev = node->_prev->_next;
		_ListNode * nextnode = node->_next;
		if (CALLDTORS) _listNodePool.destroyAndFree<_ListNode>(node); else _listNodePool.free(node);
		return nextnode;
		}
		*/

		/** move a list node and put it in front of fontNode.
		Return the Node after Node (before moving).  */
		/*
		inline _ListNode * _moveListNode(_ListNode * node, _ListNode * frontnode)
		{
		MTOOLS_ASSERT(node != nullptr);
		MTOOLS_ASSERT((frontnode == nullptr) || (frontnode->_prev == nullptr)); // make sure frontnode is the first element
		if (node->_prev != nullptr) node->_prev->_next = node->_next->_prev;
		if (node->_next != nullptr) node->_next->_prev = node->_prev->_next;
		_ListNode * nextnode = node->_next;
		node->_next = frontnode;
		if (frontnode != nullptr) { frontnode->_prev = node; }
		return nextnode;
		}
		*/



		/**************************************************************************************************
		* Private structures
		**************************************************************************************************/


		TreeFigure(const TreeFigure & TF) = delete;				// no copy
		TreeFigure & operator=(const TreeFigure &) = delete;	//



																/** structure for simply chained list of bounded objects. */
		struct _ListNode
		{
			/** ctor. */
			_ListNode(const BoundedObject & bobj, _ListNode * next) : _bobj(bobj), _next(next) {}
			_ListNode(const T & obj, const BBox & bbox, _ListNode * next) : _bobj(obj, bbox), _next(next) {}


			_ListNode * _next;		// next item in the list, nullptr if there are none. 
			BoundedObject _bobj;	// the bounded object.
		};


		/** base class for square and rectangle nodes. */
		struct _BaseNode
		{
			/** ctor. */
			_BaseNode(const BBox & bbox) : _bbox(bbox), _first(nullptr), _size_and_flag(0) {}

			BBox		_bbox;		   // the node bounding box
			_ListNode *	_first;        // list of item in the node

			MTOOLS_FORCEINLINE uint64 size() const { return (_size_and_flag >> 1); }								// accessors for size and sorted flag 
			MTOOLS_FORCEINLINE void inc_size() { _size_and_flag += 2; }											// 
			MTOOLS_FORCEINLINE void dec_size() { MTOOLS_ASSERT(_size_and_flag >= 2);  _size_and_flag -= 2; }		//
			MTOOLS_FORCEINLINE void set_sorted_flag() { MTOOLS_ASSERT(!(_size_and_flag & 1)); _size_and_flag++; }	//
			MTOOLS_FORCEINLINE bool sorted_flag() const { return (_size_and_flag & 1); }							//

		private:

			uint64		_size_and_flag;	       // number of items in the list and flag whether they have been sorted.

		};


		/** Rectangular node class (either vertical splitting ||| (DIR_V) or horizontal pslitting = (DIR_H). */
		template<bool DIRECTION> struct _RecNode : public _BaseNode
		{
			/** ctor. */
			_RecNode(const BBox & bbox) : _BaseNode(bbox), _son() {}

			_RecNode<DIRECTION> *  _son[3];   // pointer to the sons
		};



		/** Square node class  */
		struct _SqrNode : public _BaseNode
		{
			/** ctor. */
			_SqrNode(const BBox & bbox) : _BaseNode(bbox), _son(), _hor(), _ver() {}

			_SqrNode * 	        _son[9];	// pointer to the sons square nodes
			_RecNode<DIR_H> *   _hor[3];    // pointer to the horizontal rectangular nodes ==
			_RecNode<DIR_V> *   _ver[3];    // pointer to the vertical rectanglular node |||
		};





		/** release all allocated memory and set pointers to nullptr. */
		void _reset()
		{
			_recNodePool.freeAll();
			_sqrNodePool.freeAll();
			if (_callDtors) _listNodePool.destroyAndFreeAll<_ListNode>(); else _listNodePool.freeAll();
			_rootNode = nullptr;
		}


		/** create the root of the tree */
		void _createRoot()
		{
			MTOOLS_ASSERT((_rootNode == nullptr) && (_currentNode == nullptr));
			_rootNode = (_SqrNode *)_sqrNodePool.malloc();
			::new(_rootNode) _SqrNode(BBox(-1, 1, -1, 1));
		}

	public:
		/**
		* compute the subbox of outb to which inb belong.
		*
		* @param 		  	inb   	The bounding box to test.
		* @param 		  	outb  	The out box that contains inb.
		* @param [in,out]	subbox	the sub-box of outb containing inb is stored here.
		*
		* @return	The correponding index numbered as follow.
		*
		*    | 9  | 10 | 11 |
		*    |    |    |    |
		*    +----+----+----+------
		*    | 0  | 1  |  2 |   12
		*    +----+----+----+------         15 = no subbox
		*    | 3  | 4  |  5 |   13
		*    +----+----+----+------
		*    | 6  | 7  |  8 |   14
		*    +----+----+----+------
		**/
		static inline int _getIndex(const BBox & inb, const BBox & outb, BBox & subbox)
		{
			MTOOLS_ASSERT(outb.contain(inb));

			TFloat ax = (outb.max[0] - outb.min[0]) / 4;
			TFloat bx = ax + ax;
			TFloat cx = bx + ax;
			TFloat mix = (inb.min[0] - outb.min[0]);
			int nx_min = (mix < bx) ? ((mix < ax) ? 0 : 1) : ((mix < cx) ? 2 : 3);
			TFloat max = (inb.max[0] - outb.min[0]);
			int nx_max = (max <= bx) ? ((max <= ax) ? 0 : 1) : ((max <= cx) ? 2 : 3);
			int ix = nx_min + (4 * nx_max);
			int rx;
			switch (ix)
			{
			case 0: { rx = 0; break; }
			case 4: { rx = 0; break; }
			case 5: { rx = 1; break; }
			case 8: { rx = 3; break; }
			case 9: { rx = 1; break; }
			case 10: { rx = 1; break; }
			case 12: { rx = 3; break; }
			case 13: { rx = 3; break; }
			case 14: { rx = 2; break; }
			case 15: { rx = 2; break; }
			default: { MTOOLS_ERROR("hum... should not be possible (1)"); }
			}

			TFloat ay = (outb.max[1] - outb.min[1]) / 4;
			TFloat by = ay + ay;
			TFloat cy = by + ay;
			TFloat miy = (inb.min[1] - outb.min[1]);
			int ny_min = (miy < by) ? ((miy < ay) ? 0 : 1) : ((miy < cy) ? 2 : 3);
			TFloat may = (inb.max[1] - outb.min[1]);
			int ny_max = (may <= by) ? ((may <= ay) ? 0 : 1) : ((may <= cy) ? 2 : 3);
			int iy = ny_min + (4 * ny_max);
			int ry;
			switch (iy)
			{
			case 0: { ry = 0; break; }
			case 4: { ry = 0; break; }
			case 5: { ry = 1; break; }
			case 8: { ry = 3; break; }
			case 9: { ry = 1; break; }
			case 10: { ry = 1; break; }
			case 12: { ry = 3; break; }
			case 13: { ry = 3; break; }
			case 14: { ry = 2; break; }
			case 15: { ry = 2; break; }
			default: { MTOOLS_ERROR("hum... should not be possible (2)"); }
			}

			TFloat ox = outb.min[0];
			ax += ox;
			bx += ox;
			cx += ox;
			TFloat dx = outb.max[0];

			TFloat oy = outb.min[1];
			ay += oy;
			by += oy;
			cy += oy;
			TFloat dy = outb.max[1];

			int vv = rx + 4 * ry;
			int ind;
			//   9 10 11
			// 
			//   0  1  2    12
			//   3  4  5    13            15
			//   6  7  8    14
			// 
			switch (vv)
			{
			case 0: { subbox = { ox,bx,oy,by };  ind = 0;   break; }      //  0 , 0
			case 1: { subbox = { ax,cx,oy,by };  ind = 1;  break; }      //  1 , 0
			case 2: { subbox = { bx,dx,oy,by };  ind = 2;  break; }      //  2 , 0
			case 3: { subbox = { ox,dx,oy,by };  ind = 12; break; }      //  3 , 0

			case 4: { subbox = { ox,bx,ay,cy };  ind = 3;  break; }      //  0 , 1
			case 5: { subbox = { ax,cx,ay,cy };  ind = 4;  break; }      //  1 , 1
			case 6: { subbox = { bx,dx,ay,cy };  ind = 5;  break; }      //  2 , 1 
			case 7: { subbox = { ox,dx,ay,cy };  ind = 13; break; }      //  3 , 1

			case 8: { subbox = { ox,bx,by,dy };  ind = 6;  break; }      //  0 , 2
			case 9: { subbox = { ax,cx,by,dy };  ind = 7;  break; }      //  1 , 2
			case 10: { subbox = { bx,dx,by,dy };  ind = 8;  break; }      //  2 , 2
			case 11: { subbox = { ox,dx,by,dy };  ind = 14; break; }      //  3 , 2

			case 12: { subbox = { ox,bx,oy,dy };  ind = 9;  break; }      //  0 , 3
			case 13: { subbox = { ax,cx,oy,dy };  ind = 10; break; }      //  1 , 3
			case 14: { subbox = { bx,dx,oy,dy };  ind = 11; break; }      //  2 , 3
			case 15: { subbox = { ox,dx,oy,dy };  ind = 15; break; }      //  3 , 3
			default: { MTOOLS_ERROR("hum... should not be possible (3)"); }
			}
			return ind;
		}


		/**************************************************************************************************
		* Private global variables
		**************************************************************************************************/

		bool _callDtors;														// true if we should call the destructor when object are deleted. 

		_SqrNode * _rootNode;													// root node the "tree"

		mtools::CstSizeMemoryPool<sizeof(_SqrNode), 10000> _sqrNodePool;			// memory pool for square nodes elements
		mtools::CstSizeMemoryPool<sizeof(_RecNode<true>), 10000> _recNodePool;	// memory pool for rectangular nodes (both horizontal and vertical)
		mtools::CstSizeMemoryPool<sizeof(_ListNode), 100000> _listNodePool;	    // memory pool for listNode elements


	};


#undef DIR_H
#undef DIR_V





	void testindex(fBox2 & tb)
	{
		fBox2 out = { -100, 500 , 100 , 700 };

		Image im(800, 800);
		im.clear(RGBc::c_White);

		fBox2 R = { -200, 600 , 0 , 800 };


		im.canvas_draw_box(R, out, RGBc(240, 240, 240), false);
		im.canvas_draw_rectangle(R, out, RGBc::c_Black, false, 1);

		double ox = out.min[0];
		double ax = ox + out.lx() / 4;
		double bx = ox + 2 * out.lx() / 4;
		double cx = ox + 3 * out.lx() / 4;
		double dx = out.max[0];

		double oy = out.min[1];
		double ay = oy + out.ly() / 4;
		double by = oy + 2 * out.ly() / 4;
		double cy = oy + 3 * out.ly() / 4;
		double dy = out.max[1];

		im.canvas_draw_line(R, { ax,oy }, { ax,dy }, RGBc::c_Black, true, false, false, 1);
		im.canvas_draw_line(R, { bx,oy }, { bx,dy }, RGBc::c_Black, true, false, false, 1);
		im.canvas_draw_line(R, { cx,oy }, { cx,dy }, RGBc::c_Black, true, false, false, 1);

		im.canvas_draw_line(R, { ox,ay }, { dx,ay }, RGBc::c_Black, true, false, false, 1);
		im.canvas_draw_line(R, { ox,by }, { dx,by }, RGBc::c_Black, true, false, false, 1);
		im.canvas_draw_line(R, { ox,cy }, { dx,cy }, RGBc::c_Black, true, false, false, 1);

		fBox2 subbox;
		TreeFigure<void *>::_getIndex(tb, out, subbox);

		im.canvas_draw_box(R, subbox, RGBc(180, 180, 180), false);
		im.canvas_draw_box(R, tb, RGBc::c_Red, false);


		auto P1 = makePlot2DImage(im);
		Plotter2D plotter;
		plotter[P1];
		plotter.autorangeXY();
		plotter.range().zoomOut();
		plotter.plot();


	}




	int main(int argc, char *argv[])
	{
		MTOOLS_SWAP_THREADS(argc, argv); // required on OSX, does nothing on Linux/Windows
		mtools::parseCommandLine(argc, argv, true); // parse the command line, interactive mode

		TreeFigure<void *> TF;

		fBox2 B;

		B = { -100,110,100,380 };


		for (int i = 0; i < 500; i += 50)
		{
			for (int j = 0; j < 400; j += 50)
			{
				fVec2 T(i, j);
				fBox2 C = B;
				C.min += T;
				C.max += T;
				testindex(C);
			}

		}




		mtools::cout << "Hello World\n";
		mtools::cout.getKey();
		return 0;
	}
