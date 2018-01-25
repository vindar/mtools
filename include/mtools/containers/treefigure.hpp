/** @file treefigure.hpp */
//
// Copyright 2015 Arvind Singh
//
// This file is part of the mtools library.
//
// mtools is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with mtools  If not, see <http://www.gnu.org/licenses/>.


#pragma once

#include "../misc/internal/mtools_export.hpp"
#include "../misc/error.hpp"
#include "../maths/vec.hpp"
#include "../maths/box.hpp"
#include "../misc/metaprog.hpp"
#include "../io/serialization.hpp"
#include "../graphics/rgbc.hpp"
#include "../graphics/image.hpp"


#include <string>
#include <vector>


namespace mtools
{

	/**
	 * TreeFigure class. 
	 * 
	 * Container for 'spatial data': hold objects associated with a bounding box. 
	 * 
	 * The container is 'tree-like' and organized in such way: 
	 *   -  Fast iteration through object that intersect/contain/are contained in a given region. 
	 *   -  When iterating through a region, larger object are returned earlier than smaller ones.  
	 *
	 * - Contrarily to a r-tree, splitting of region is binary, as in a  which insure that the distance of R^2
	 *   is still reflected in the tree structure. 
	 * - Contrarily to the usual quadtree, region represented by the subnodes of a node overlap so that small  
	 *   objects cannot get stuck in big nodes because they intersect their boundaries. 
	 * 
	 * This class if useful to store figure objects (such a line/rectangle/point...) cf the 
	 * plot2DFigure class. 
	 * 
	 * @tparam	T	   template parameter representing the type of objects contained in the container.
	 * @tparam	N	   Max number of 'reducible' object per node (typically between 2 and 100). 
	 * @tparam	TFloat Type use for floating point computation (default double).
	 * 				   
	 */
	template<class T, int N = 10, class TFloat = double> class TreeFigure
	{


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
			BoundedObject() : boundingbox(), object() {}
			BoundedObject(const BBox & bbox, const T & obj) : boundingbox(bbox), object(obj) {}

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
		TreeFigure(const TreeFigure && TF) : _callDtors(TF._callDtors), _rootNode(TF._rootNode), _treeNodePool(std::forward<decltype(_treeNodePool)>(TF._sqrNodePool)), _listNodePool(std::forward<decltype(_listNodePool)>(TF._listNodePool))
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
		* Serialize this object into an archive.
		**/
		void serialize(OBaseArchive & ar, const int version = 0) const
			{
			ar << std::string("TreeFigure< ") + typeid(T).name() + ", " + mtools::toString(N) + ", " + typeid(TFloat).name() + ">\n";
			ar & size(); // number of items
			size_t nb = iterate_all([&ar](const BoundedObject & bo) -> void  { ar & bo.boundingbox; ar & bo.object; }); 
			ar << std::string("\nend of TreeFigure\n");
			MTOOLS_ASSERT(nb == size());
			}


		/**
		* Deserialize this object. 
		* 
		* The object is NOT EMPTIED before de-serializing so the content of the archive is added 
		* to the existing content. 
		**/
		void deserialize(IBaseArchive & ar)
			{
			size_t nb; ar & nb; // number of items to add
			for (size_t i = 0; i < nb; i++)
				{
				BoundedObject bo; 
				ar & bo.boundingbox;
				ar & bo.object;
				insert(bo);
				}
			}


		/**
		* Insert an object. 
		* 
		* A copy of object is made with the copy constructor.
		**/
		MTOOLS_FORCEINLINE void insert(const BBox & boundingbox, const T & object)
			{
			insert({ boundingbox , object });
			}


		/**
		* Insert a bounded object.
		* 
		* A copy is made with the copy constructor.
		**/
		void insert(const BoundedObject & boundedObject)
			{
			MTOOLS_INSURE(!(boundedObject.boundingbox.isEmpty())); // bounding box should not be empty. 
			// create new roots until we contain the object's bounding box
			while (!(_rootNode->_bbox.contain(boundedObject.boundingbox))) { _reRootUp(); }
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
					return;
					}
				// go to the correct son and continue
				node = node->_son[i]; 
				}
			}


		/**
		 * Iterate over all objects whose bounding box intersect 'box'. 
		 * the function 'fun' must be callable in the form 'fun(boundedObject)'.
		 */
		template<typename FUNCTION> size_t iterate_intersect(BBox box, FUNCTION fun) const
			{
			MTOOLS_INSURE(!(box.isEmpty())); // box should not be empty.
			std::vector<_TreeNode* > stack1; 
			std::vector<_TreeNode* > stack2;
			std::vector<_TreeNode* > * pcurrentStack = &stack1;
			std::vector<_TreeNode* > * pnextStack = &stack2;
			if (intersectionRect(_rootNode->_bbox, box).isEmpty()) return 0;
			pcurrentStack->push_back(_rootNode);
			size_t nb = 0;
			while (1)
				{
				const size_t currentsize = pcurrentStack->size();
				if (currentsize == 0)
					{
					MTOOLS_ASSERT(nb == size());
					return nb;
					}
				for (size_t i = 0; i < currentsize; i++)
					{
					_TreeNode * node = pcurrentStack->operator[](i);					
					_ListNode * LN = node->_first_irreducible;
					while (LN != nullptr) 
						{ 
						if (!(intersectionRect(LN->_bobj.boundingbox, box).isEmpty())) { fun(LN->_bobj); nb++; }
						LN = LN->_next; 
						}
					LN = node->_first_reducible;
					while (LN != nullptr) 
						{ 
						if (!(intersectionRect(LN->_bobj.boundingbox, box).isEmpty())) { fun(LN->_bobj); nb++; }
						LN = LN->_next; 
						}
					for (int j = 0; j < 15; j++)
						{
						if (node->_son[j] != nullptr) 
							{
							if (!(intersectionRect(node->_son[j]->_bbox, box).isEmpty()))
								{
								pnextStack->push_back(node->_son[j]);
								}
							}
						}
					}
				pcurrentStack->clear();
				mtools::swap(pcurrentStack, pnextStack);
				}
			}


		/**
		* Iterate over all objects whose bounding box is contained in 'box'.
		* the function 'fun' must be callable in the form 'fun(boundedObject)'.
		*/
		template<typename FUNCTION> size_t iterate_contained_in(const BBox & box, FUNCTION fun) const
			{
			MTOOLS_INSURE(!(box.isEmpty())); // box should not be empty.
			std::vector<_TreeNode* > stack1;
			std::vector<_TreeNode* > stack2;
			std::vector<_TreeNode* > * pcurrentStack = &stack1;
			std::vector<_TreeNode* > * pnextStack = &stack2;
			if (intersectionRect(_rootNode->_bbox, box).isEmpty()) return 0;
			pcurrentStack->push_back(_rootNode);
			size_t nb = 0;
			while (1)
				{
				const size_t currentsize = pcurrentStack->size();
				if (currentsize == 0)
					{
					MTOOLS_ASSERT(nb == size());
					return nb;
					}
				for (size_t i = 0; i < currentsize; i++)
					{
					_TreeNode * node = pcurrentStack->operator[](i);					
					_ListNode * LN = node->_first_irreducible;
					while (LN != nullptr) 
						{ 
						if (box.contain(LN->_bobj.boundingbox)) { fun(LN->_bobj); nb++; }
						LN = LN->_next; 
						}
					LN = node->_first_reducible;
					while (LN != nullptr) 
						{ 
						if (box.contain(LN->_bobj.boundingbox)) { fun(LN->_bobj); nb++; }
						LN = LN->_next; 
						}
					for (int j = 0; j < 15; j++)
						{
						if (node->_son[j] != nullptr) 
							{
							if (!(intersectionRect(node->_son[j]->_bbox, box).isEmpty()))
								{
								pnextStack->push_back(node->_son[j]);
								}
							}
						}
					}
				pcurrentStack->clear();
				mtools::swap(pcurrentStack, pnextStack);
				}
			}


		/**
		* Iterate over all objects whose bounding box contain 'box'.
		* the function 'fun' must be callable in the form 'fun(boundedObject)'.
		*/
		template<typename FUNCTION> size_t iterate_contain(const BBox & box, FUNCTION fun) const
			{
			MTOOLS_INSURE(!(box.isEmpty())); // box should not be empty.
			std::vector<_TreeNode* > stack1;
			std::vector<_TreeNode* > stack2;
			std::vector<_TreeNode* > * pcurrentStack = &stack1;
			std::vector<_TreeNode* > * pnextStack = &stack2;
			if (intersectionRect(_rootNode->_bbox, box).isEmpty()) return 0;
			pcurrentStack->push_back(_rootNode);
			size_t nb = 0;
			while (1)
				{
				const size_t currentsize = pcurrentStack->size();
				if (currentsize == 0)
					{
					MTOOLS_ASSERT(nb == size());
					return nb;
					}
				for (size_t i = 0; i < currentsize; i++)
					{
					_TreeNode * node = pcurrentStack->operator[](i);					
					_ListNode * LN = node->_first_irreducible;
					while (LN != nullptr) 
						{ 
						if (LN->_bobj.boundingbox.contain(box)) { fun(LN->_bobj); nb++; }
						LN = LN->_next; 
						}
					LN = node->_first_reducible;
					while (LN != nullptr) 
						{ 
						if (LN->_bobj.boundingbox.contain(box)) { fun(LN->_bobj); nb++; }
						LN = LN->_next; 
						}
					for (int j = 0; j < 15; j++)
						{
						if (node->_son[j] != nullptr) 
							{
							if (node->_son[j]->_bbox.contain(box))
								{
								pnextStack->push_back(node->_son[j]);
								}
							}
						}
					}
				pcurrentStack->clear();
				mtools::swap(pcurrentStack, pnextStack);
				}
			}


		/**
		* Iterate over all objects.
		* the function 'fun' must be callable in the form 'fun(boundedObject)'.
		*/
		template<typename FUNCTION> size_t iterate_all(FUNCTION fun) const 
			{
			std::vector<_TreeNode* > stack1; 
			std::vector<_TreeNode* > stack2;
			std::vector<_TreeNode* > * pcurrentStack = &stack1;
			std::vector<_TreeNode* > * pnextStack = &stack2;
			pcurrentStack->push_back(_rootNode);
			size_t nb = 0;
			while (1)
				{
				const size_t currentsize = pcurrentStack->size();
				if (currentsize == 0)
					{
					MTOOLS_ASSERT(nb == size());
					return nb;
					}
				for (size_t i = 0; i < currentsize; i++)
					{
					_TreeNode * node = pcurrentStack->operator[](i);					
					_ListNode * LN = node->_first_irreducible;
					while (LN != nullptr) { fun(LN->_bobj);  LN = LN->_next; nb++; }
					LN = node->_first_reducible;
					while (LN != nullptr) { fun(LN->_bobj);	LN = LN->_next; nb++; }
					for (int j = 0; j < 15; j++)
						{
						if (node->_son[j] != nullptr) { pnextStack->push_back(node->_son[j]);  }
						}
					}
				pcurrentStack->clear();
				mtools::swap(pcurrentStack, pnextStack);
				}
			}


		/**
		* Return the main bounding box that contains all items currently inserted.
		**/
		BBox mainBoundingBox() const  { return _rootNode->_bbox; }


		/**
		* Query the number of objects currently inserted.
		**/
		size_t size() const { return _listNodePool.size(); }


		/**
		* Return the number of bytes malloced by this object.
		**/
		size_t footprint() const { return (_treeNodePool.footprint() + _listNodePool.footprint()); }


		/**
		* Print information about this object into a string.
		**/
		std::string toString() const
			{
			std::string s = std::string("TreeFigure<") + typeid(T).name() + ", " + mtools::toString(N) + ", " + typeid(TFloat).name() + ">\n";
			s += std::string(" - object inserted : ") + mtools::toString(size()) + "\n";
			s += std::string(" - memory used : ") + mtools::toStringMemSize(footprint()) + "\n";
			s += std::string(" - main bounding box : ") + mtools::toString(_rootNode->_bbox) + "\n";
			return s + "---\n"; 
			}


		/**
		* Draw the tree structure into an image. For debug purposes.
		**/
		void drawTreeDebug(Image & im, fBox2 R, RGBc objColor = RGBc::c_Blue, RGBc treeColor = RGBc::c_Red) const 
			{ 
			std::vector<_TreeNode* > stack1; 
			std::vector<_TreeNode* > stack2;
			std::vector<_TreeNode* > * pcurrentStack = &stack1;
			std::vector<_TreeNode* > * pnextStack = &stack2;
			pcurrentStack->push_back(_rootNode);
			while (1)
				{
				const size_t currentsize = pcurrentStack->size();
				if (currentsize == 0) { return; }
				for (size_t i = 0; i < currentsize; i++)
					{
					_TreeNode * node = pcurrentStack->operator[](i);
					_drawNodedebug(im, R, node, treeColor , RGBc(180, 180, 180).getOpacity(0.1f), objColor, objColor);
					for (int j = 0; j < 15; j++)
						{
						if (node->_son[j] != nullptr) { pnextStack->push_back(node->_son[j]);  }
						}
					}
				pcurrentStack->clear();
				mtools::swap(pcurrentStack, pnextStack);
				}
			}



	/**************************************************************************************************
	* Private implementation.
	**************************************************************************************************/

	private:


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
			_TreeNode(const BBox & bbox) : _bbox(bbox), _first_reducible(nullptr), _last_reducible(nullptr), _first_irreducible(nullptr), _nb_reducible(0), _nb_irreducible(0), _son {nullptr} {}

			BBox		_bbox;				// the node bounding box
			_ListNode *	_first_reducible;	// pointeur to the first reducible item
			_ListNode *	_last_reducible;	// pointeur to the first reducible item
			_ListNode * _first_irreducible;	// pointeur to the first irreducible item
			size_t		_nb_reducible;		// number of reducible items
			size_t		_nb_irreducible;	// number of irreducible items
			_TreeNode *	_son[15];			// pointer to the sons
			};



		/** link a reducible node at the end */
		inline void _linkReducible(_ListNode * LN, _TreeNode * node)
			{
			MTOOLS_ASSERT(LN != nullptr);
			MTOOLS_ASSERT(node != nullptr);
			if (node->_last_reducible == nullptr)
				{
				MTOOLS_ASSERT(node->_first_reducible == nullptr);
				node->_first_reducible = LN;
				}
			else
				{
				node->_last_reducible->_next = LN;
				}
			LN->_prev = node->_last_reducible;
			LN->_next = nullptr;
			node->_last_reducible = LN;
			node->_nb_reducible++;
			}


		/** Add a new list node at the end of the list of reducible node. */
		inline void _addReducible(const BoundedObject & bo, _TreeNode * node)
			{
			_ListNode * LN = (_ListNode *)_listNodePool.malloc();
			::new(LN) _ListNode(bo);
			_linkReducible(LN, node);
			}


		/** remove a reducible node from the chain and return its successor. */
		inline _ListNode * unlinkReducible(_ListNode * LN, _TreeNode * node)
			{
			MTOOLS_ASSERT(LN != nullptr);
			MTOOLS_ASSERT(node != nullptr);
			if (LN->_prev != nullptr) { LN->_prev->_next = LN->_next; } else { node->_first_reducible = LN->_next; }
			if (LN->_next != nullptr) { LN->_next->_prev = LN->_prev; } else {  node->_last_reducible = LN->_prev; }
			node->_nb_reducible--;
			return LN->_next;
			}



		/** Add a new list node at the beginning of the list of irreducible node. */
		inline void _linkIrreducible(_ListNode * LN, _TreeNode * node)
			{
			MTOOLS_ASSERT(LN != nullptr);
			MTOOLS_ASSERT(node != nullptr);
			if (node->_first_irreducible != nullptr)
				{
				MTOOLS_ASSERT(node->_first_irreducible->_prev == nullptr);
				node->_first_irreducible->_prev = LN;
				}
			LN->_next = node->_first_irreducible;
			LN->_prev = nullptr;
			node->_first_irreducible = LN;
			node->_nb_irreducible++;
			}


		/** Add a new list node at the beginning of the list of irreducible node. */
		inline void _addIrreducible(const BoundedObject & bo, _TreeNode * node)
			{
			MTOOLS_ASSERT(node != nullptr);
			_ListNode * LN = (_ListNode *)_listNodePool.malloc();
			::new(LN) _ListNode(bo);
			_linkIrreducible(LN, node);
			}


		/** remove an irreducible node from the chain and return its successor. */
		inline _ListNode * unlinkIrreducible(_ListNode * LN, _TreeNode * node)
			{
			MTOOLS_ASSERT(LN != nullptr);
			MTOOLS_ASSERT(node != nullptr);
			if (LN->_prev != nullptr) { LN->_prev->_next = LN->_next; } else { node->_first_irreducible = LN->_next; }
			if (LN->_next != nullptr) { LN->_next->_prev = LN->_prev; }
			node->_nb_irreducible--;
			return LN->_next;
			}


		/** deal with overflowing node. (Recursive version, to be improved). */
		void _overflow(_TreeNode * node)
			{
			MTOOLS_ASSERT(node != nullptr);
			if ((node->_nb_reducible == 0) ||(node->_nb_reducible + node->_nb_irreducible <= N)) return; // no overflow: nothing to do. 
			size_t nb = (node->_nb_irreducible >= N) ? (node->_nb_reducible) : (node->_nb_reducible + node->_nb_irreducible - N); // number of reducible items to dispatch
			_ListNode * LN = node->_first_reducible;
			for (size_t k = 0; k<nb; k++)
				{
				MTOOLS_ASSERT(LN != nullptr);
				int index = _getIndex(LN->_bobj.boundingbox, node->_bbox); // get the son index
				MTOOLS_ASSERT(index >= 0);
				MTOOLS_ASSERT(index < 15);
				if (node->_son[index] == nullptr) { _createChildNode(node, index); } // create the son since it does not exist yet. 
				_ListNode * nextLN = unlinkReducible(LN, node);	// unlink from node
				int newindex = _getIndex(LN->_bobj.boundingbox, node->_son[index]->_bbox); // get the index inside the child node. 
				if (newindex == 15)
					{ // the item is now irreducible, good :)
					_linkIrreducible(LN, node->_son[index]);
					}
				else
					{ // the item is still reducible in the subnode
					_linkReducible(LN, node->_son[index]);
					}
				LN = nextLN;
				}
			// and we deal with the possible overflow of the child nodes. 
			for (int i = 0; i < 15; i++)
				{
				if (node->_son[i] != nullptr) _overflow(node->_son[i]);
				}
			}


		/** Release all allocated memory and set pointers to nullptr. */
		void _reset()
			{
			_treeNodePool.freeAll();
			if (_callDtors) _listNodePool.destroyAndFreeAll<_ListNode>(); else _listNodePool.freeAll();
			_rootNode = nullptr;
			}


		/** Create the initial root of the tree */
		void _createRoot()
			{
			MTOOLS_ASSERT(_rootNode == nullptr);
			_rootNode = (_TreeNode *)_treeNodePool.malloc();
			::new(_rootNode) _TreeNode({ (TFloat)-1, (TFloat)1, (TFloat)-1, (TFloat)1 });
			}


		/** create a given child node of a node */
		void _createChildNode(_TreeNode * node, int index)
			{
			MTOOLS_ASSERT(index >= 0);
			MTOOLS_ASSERT(index < 15);
			MTOOLS_ASSERT(node != nullptr);
			MTOOLS_ASSERT(node->_son[index] == nullptr);
			_TreeNode * nn = (_TreeNode *)_treeNodePool.malloc();
			::new(nn) _TreeNode(_getSubBox(index, node->_bbox));
			node->_son[index] = nn;
			return;
			}


		/** Create the father of the root and set it as the new root. */
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
			MTOOLS_ASSERT(index >= 0);
			MTOOLS_ASSERT(index < 15);
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


		/** draw a node into an image. For debug purpose only. */
		void _drawNodedebug(Image & im, fBox2 R, _TreeNode * node, RGBc nodecolor, RGBc nodecolorinterior, RGBc obj_red, RGBc obj_irred) const
			{
			im.canvas_draw_box(R, node->_bbox, nodecolorinterior, true);
			im.canvas_draw_rectangle(R, node->_bbox, nodecolor, false);
			_ListNode * LN = node->_first_reducible; 
			while (LN != nullptr)
				{
				im.canvas_draw_box(R, LN->_bobj.boundingbox, obj_red, true);
				LN = LN->_next;
				}
			LN = node->_first_irreducible;
			while (LN != nullptr)
				{
				im.canvas_draw_box(R, LN->_bobj.boundingbox, obj_irred, true);
				LN = LN->_next;
				}
			}




		/**
		* compute the subbox of outb to which inb belong.
		*
		* @param 		  	inb   	The bounding box to test.
		* @param 		  	outb  	The out box that contains inb.
		*
		* @return	The corresponding index numbered as follow.
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



}

/* end of file */




