
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





	template<class T, int N = 50, class TFloat = double> class TreeFigure
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



		/**************************************************************************************************
		* Public Methods
		**************************************************************************************************/



		/**
		* Default constructor, create an empty object.
		**/
		TreeFigure(bool callDtors = false) : _callDtors(callDtors), _rootNode(nullptr), _treeNodePool(), _listNodePool()
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
			_treeNodePool(std::forward<decltype(_treeNodePool)>(TF._sqrNodePool)),
			_listNodePool(std::forward<decltype(_listNodePool)>(TF._listNodePool))
			{
			TF._rootNode = nullptr;
			}


		/**
		* Move assignement operator.
		**/
		TreeFigure & operator=(const TreeFigure && TF)
			{
			_reset();
			_callDtors = TF._callDtors;
			_rootNode = TF._rootNode;
			_treeNodePool = std::forward<decltype(_treeNodePool)>(TF._treeNodePool);
			_listNodePool = std::forward<decltype(_listNodePool)>(TF._listNodePool);
			TF._rootNode = nullptr;
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
		void insert(const BoundedObject & boundedObject)
			{
			// create new roots until we contain the object's bounding box
			while (!(_rootNode->_bbox.contains(boundedObject.boundingbox))) { _reRootUp(); }
			// start from the root and go down
			_TreeNode * node = _rootNode;
			while (1)
				{
				int i = _getIndex(boundedObject.boundingbox, node->_bbox);
				if (i == 15)
					{ // irreducible item, we just put it there (in front)
					_addIrreducible(boundedObject, node);
					// deal with overflowing nodes if needed.  
					if ((node->_nb_reducible >0) &&(node->_nb_reducible + node->_nb_irreducible > N)) _overflow(node);
					return;
					}
				if (node->_son[i] == nullptr)
					{ // son not created. Put reducible object right here. 
					_addReducible(boundedObject, node);
					// and check for overflow
					if (node->_nb_reducible + node->_nb_irreducible > N) _overflow(node);
					}
				// go to the correct son and continue
				node = node->_son[i]; 
				}
			}




		/**
		* Query the number of object currently inserted.
		**/
		size_t size() const { return _listNodePool.size(); }


		/**
		* Return the number of bytes malloced by this object.
		**/
		size_t footprint() const { return (_treeNodePool.footprint() + _listNodePool.footprint()); }


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

	//private:


		TreeFigure(const TreeFigure & TF) = delete;				// no copy
		TreeFigure & operator=(const TreeFigure &) = delete;	//



		/** structure for doubly chained list of bounded objects. */
		struct _ListNode
		{
			/** ctor. */
			_ListNode(const BoundedObject & bobj) : _prev(nullptr), _next(nullptr), _bobj(bobj) {}

			_ListNode *   _prev;	// next item in the list, nullptr if there are none. 
			_ListNode *   _next;	// next item in the list, nullptr if there are none. 
			BoundedObject _bobj;	// the bounded object.
		};


		/** base class for square and rectangle nodes. */
		struct _TreeNode
		{
			/** ctor. */
			_TreeNode(const BBox & bbox) : _bbox(bbox), _first_reducible(nullptr), _last_reducible, _first_irreducible(nullptr), _nb_reducible(0), _nb_irreducible(0), _son {nullptr} {}

			BBox		_bbox;				// the node bounding box
			_ListNode *	_first_reducible;	// pointeur to the first reducible item
			_ListNode *	_last_reducible;	// pointeur to the first reducible item
			_ListNode * _first_irreducible;	// pointeur to the first irreducible item
			size_t		_nb_reducible;		// number of reducible items
			size_t		_nb_irreducible;	// number of irreducible items
			_TreeNode *	_son[15];			// pointer to the sons

		};


		/** Add a new list node at the end of the list of reducible node. */
		void _addReducible(const BoundedObject & bo, _TreeNode * node)
			{
			_ListNode * LN = (_ListNode *)_listNodePool.malloc();
			::new(LN) _ListNode(boundedObject);
			if (node->_last_reducible == nullptr)
				{
				MTOOLS_ASSERT(node->_first_reducible == nullptr);
				node->_first_reducible = LN;
				}
			else
				{
				node->_last_reducible->_next = LN;
				}
			LN->prev = node->_last_reducible;
			node->_last_reducible = LN;
			node->_nb_reducible++;
			}


		/** Add a new list node at the beginning of the list of irreducible node. */
		void _addIrreducible(const BoundedObject & bo, _TreeNode * node)
			{
			MTOOLS_ASSERT(node != nullptr);
			_ListNode * LN = (_ListNode *)_listNodePool.malloc();
			::new(LN) _ListNode(boundedObject);
			if (node->_first_irreducible != nullptr)
			{
				MTOOLS_ASSERT(node->_first_irreducible->_prev == nullptr);
				node->_first_irreducible->_prev = LN;
			}
			LN->_next = node->_first_irreducible;
			node->_first_reducible = LN;
			node->_nb_irreducible++;
			}


		/* remove a reducible node from the chain and return its successor. */
		_ListNode * unlinkReducible(_ListNode * LN, _TreeNode * node)
			{
			MTOOLS_ASSERT(LN != nullptr);
			MTOOLS_ASSERT(node != nullptr);
			if (LN->prev != nullptr) { LN->_prev->_next = LN->_next; } else { node->_first_reducible = LN->_next; }
			if (LN->next != nullptr) { LN->_next->_prev = LN->_prev; } else {  node->_last_reducible = LN->_prev; }
			node->_nb_reducible--;
			return LN->next;
			}

		/* remove an irreducible node from the chain and return its successor. */
		_ListNode * unlinkIrreducible(_ListNode * LN, _TreeNode * node)
			{
			MTOOLS_ASSERT(LN != nullptr);
			MTOOLS_ASSERT(node != nullptr);
			if (LN->prev != nullptr) { LN->_prev->_next = LN->_next; } else { node->_first_irreducible = LN->_next; }
			if (LN->next != nullptr) { LN->_next->_prev = LN->_prev; }
			node->_nb_irreducible--;
			return LN->next;
			}


		/** create a given child node of a node */
		void _createChildNode(_TreeNode * node, int index)
			{
			MTOOLS_ASSERT(index >= 0);
			MTOOLS_ASSERT(index < 15);
			MTOOLS_ASSERT(node != nullptr);
			MTOOLS_ASSERT(node->_son[index] == nullptr);
			_TreeNode * nn = (_TreeNode *)_treeNodePool.malloc();
			::new(nn) _TreeNode(_getSubBox(index,node->_bbox));
			node->_son[index] = nn;
			return;
			}


		/** deal with overflowing node. (Recursive version, could be improved). */
		void _overflow(_TreeNode * node)
		{
			MTOOLS_ASSERT(node != nullptr);
			if (node->_nb_reducible + node->_nb_irreducible <= N) return; // no overflow, nothing to do. 
			size_t nb = (node->_nb_irreducible >= N) ? (node->_nb_reducible) : (node->_nb_reducible + node->_nb_irreducible - N); // number of reducible item to dispatch
			for (size_t k = 0; k<nb; k++)
				{

				}
			node->_nb_reducible -= nb;
			// and we deal with the possible overflow of the child nodes. 
			for (int i = 0; i < 15; i++)
				{
				if (node->_son[i] != nullptr) overflow(node->_son[i]);
				}
			}



		/** release all allocated memory and set pointers to nullptr. */
		void _reset()
			{
			_treeNodePool.freeAll();
			if (_callDtors) _listNodePool.destroyAndFreeAll<_ListNode>(); else _listNodePool.freeAll();
			_rootNode = nullptr;
			}


		/**   
		 * create the initial root of the tree 
		 **/
		void _createRoot()
			{
			MTOOLS_ASSERT((_rootNode == nullptr) && (_currentNode == nullptr));
			_rootNode = (_TreeNode *)_treeNodePool.malloc();
			::new(_rootNode) _TreeNode({ (TFloat)-1, (TFloat)1, (TFloat)-1, (TFloat)1 });
			}


		/**   
		 * Create the father of the root and set it as the new root.
		 **/
		void _reRootUp()
			{
			_TreeNode * newrootnode = (_TreeNode *)_treeNodePool.malloc();
			::new(newrootnode) _TreeNode({ 2 * _rootNode->_bbox.min[0], 2 * _rootNode->_bbox.max[0], 2 * _rootNode->_bbox.min[1], 2 * _rootNode->_bbox.max[1] });
			newrootnode->_son[5] = _rootNode;
			_rootNode = newrootnode;
			}


		/**
		 * Return the subbox corresponding to the son with index i.
		 * 
		 * Should not be called with i = 15.
		 * 
		 *    | 12 | 13 | 14 |
		 *    |    |    |    |
		 *    +----+----+----+------
		 *    | 0  | 1  | 2  |    3
		 *    +----+----+----+------         15 = no subbox
		 *    | 4  | 5  | 6  |    7
		 *    +----+----+----+------
		 *    | 8  | 9  | 10 |   11
		 *    +----+----+----+------
		 **/
		static inline BBox _getSubBox(int index, const BBox & box)
			{
			MTOOLS_ASSERT(i >= 0);
			MTOOLS_ASSERT(i < 15);
			TFloat ex = (box.max[0] - box.min[0]) / 4;
			TFloat ox = box.min[0];
			TFloat ax = ox + ex;
			TFloat bx = ax + ex;
			TFloat cx = bx + ex;
			TFloat dx = box.max[0];
			MTOOLS_ASSERT((ox < ax) && (ax < bx) && (bx < cx) && (cx < dx));
			TFloat ey = (box.max[1] - box.min[1]) / 4;
			TFloat oy = box.min[1];
			TFloat ay = oy + ey;
			TFloat by = ay + ey;
			TFloat cy = by + ey;
			TFloat dy = box.max[1];
			MTOOLS_ASSERT((oy < ay) && (ay < by) && (by < cy) && (cy < dy));
			BBox subbox;
			switch (index)
				{
				case 0:  { subbox = { ox,bx,oy,by }; break; }      //  0 , 0
				case 1:  { subbox = { ax,cx,oy,by }; break; }      //  1 , 0
				case 2:  { subbox = { bx,dx,oy,by }; break; }      //  2 , 0
				case 3:  { subbox = { ox,dx,oy,by }; break; }      //  3 , 0
				case 4:  { subbox = { ox,bx,ay,cy }; break; }      //  0 , 1
				case 5:  { subbox = { ax,cx,ay,cy }; break; }      //  1 , 1
				case 6:  { subbox = { bx,dx,ay,cy }; break; }      //  2 , 1 
				case 7:  { subbox = { ox,dx,ay,cy }; break; }      //  3 , 1
				case 8:  { subbox = { ox,bx,by,dy }; break; }      //  0 , 2
				case 9:  { subbox = { ax,cx,by,dy }; break; }      //  1 , 2
				case 10: { subbox = { bx,dx,by,dy }; break; }      //  2 , 2
				case 11: { subbox = { ox,dx,by,dy }; break; }      //  3 , 2
				case 12: { subbox = { ox,bx,oy,dy }; break; }      //  0 , 3
				case 13: { subbox = { ax,cx,oy,dy }; break; }      //  1 , 3
				case 14: { subbox = { bx,dx,oy,dy }; break; }      //  2 , 3
				case 15: { subbox = { ox,dx,oy,dy }; break; }      //  3 , 3
				default: { MTOOLS_ERROR("hum... should not be possible"); }
				}
			return subbox;
			}


		/**
		* compute the subbox of outb to which inb belong.
		*
		* @param 		  	inb   	The bounding box to test.
		* @param 		  	outb  	The out box that contains inb.
		*
		* @return	The correponding index numbered as follow.
		*
		*    | 12 | 13 | 14 |
		*    |    |    |    |
		*    +----+----+----+------
		*    | 0  | 1  | 2  |    3
		*    +----+----+----+------         15 = no subbox
		*    | 4  | 5  | 6  |    7
		*    +----+----+----+------
		*    | 8  | 9  | 10 |   11
		*    +----+----+----+------
		**/
		static inline int _getIndex(const BBox & inb, const BBox & outb)
			{
			MTOOLS_ASSERT(outb.contain(inb));

			TFloat ax = (outb.max[0] - outb.min[0])/4;
			TFloat ex = ax/8; 
			if (((outb.max[0] - ex) >= outb.max[0]) || ((outb.min[0] + ex) <= outb.min[0])) return 15; // loosing precison, do not go further down.

			TFloat ay = (outb.max[1] - outb.min[1])/4;
			TFloat ey = ay/8;
			if (((outb.max[1] - ey) >= outb.max[1]) || ((outb.min[1] + ey) <= outb.min[1])) return 15; // loosing precison, do not go further down.

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

			return rx + 4 * ry;
			}



		/**************************************************************************************************
		* Private global variables
		**************************************************************************************************/

		bool _callDtors;														// true if we should call the destructor when object are deleted. 

		_TreeNode * _rootNode;													// root node the "tree"

		mtools::CstSizeMemoryPool<sizeof(_TreeNode), 10000> _treeNodePool;		// memory pool for the tree nodes elements
		mtools::CstSizeMemoryPool<sizeof(_ListNode), 100000> _listNodePool;	    // memory pool for listNode elements


	};









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
		int ind = TreeFigure<void *>::_getIndex(tb, out);
		subbox = TreeFigure<void *>::_getSubBox(ind, out);

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
