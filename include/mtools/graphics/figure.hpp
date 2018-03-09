/** @file figure.hpp */
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
#include "image.hpp"
#include "font.hpp"
#include "../containers/treefigure.hpp"

#include <type_traits>

namespace mtools
{



	/* Forward declarations */
	class FigureInterface;				// interface for a figure object
	template<int N> class FigureCanvas;	// main figure canvas class

	/* available figures classes*/
	class FigureCircle;
	class FigureCirclePart;
	class FigureEllipse;
	class FigureEllipsePart;
	class FigureBox;
	class FigureDot;
	class FigureLine;
	class FigurePolyLine;
	class FigureTriangle;
	class FigureConvexPolygon;
	class FigureQuadBezier;
	class FigureRatQuadBezier;
	class FigureCubicBezier;
	template<typename FIGURE1, typename FIGURE2>  class FigurePair;
	template<typename FIGURE1, typename FIGURE2, typename FIGURE3>  class FigureTriplet;
	template<typename FIGURE1, typename FIGURE2, typename FIGURE3, typename FIGURE4>  class FigureQuadruplet;
	template<class... FIGURES> class FigureTuple;
	class FigureGroup;



	

	/**
	 * Factory function to create an empty Figure canvas with a given number of layers
	 */
	template<int N = 5> FigureCanvas<N> makeFigureCanvas(size_t nbLayers = 1)
		{
		return FigureCanvas<N>(nbLayers);
		}



	/**
	 * Class that holds figure objects.
	 * use makePlot2DFigure() to create a plot object that encapsulate the FigureCanvas object. 
	 * 
	 * NOT THREADSAFE : do not insert objects while accessing (ie drawing) the canvas. 
	 *
	 * @tparam	N	   template parameter for the TreeFigure container : max number of 'reducible' object per node.
	 */
	template<int N = 5> class FigureCanvas
	{

	public: 


		/**
		 * Constructor: create an empty canvas with a given number of layers. 
		 **/
		FigureCanvas(size_t nbLayers = 1) : _vecallocp(), _nbLayers(nbLayers), _figLayers(nullptr)
			{
			MTOOLS_INSURE(nbLayers > 0);
			_figLayers = new TreeFigure<FigureInterface*,N> [nbLayers];
			}


		 /**
		 * Destructor
		 **/
		~FigureCanvas()
			{
			clear();
			delete [] _figLayers;
			}


		/**
		* Move constructor
		**/
		FigureCanvas(FigureCanvas && o) : _vecallocp(std::move(o._vecallocp)), _nbLayers(o._nbLayers), _figLayers(o._figLayers)
			{
			o._figLayers = new TreeFigure<FigureInterface*, N>[o._nbLayers]; // create empty objects to replace to ones moved.
			}


		/**
		* Move assignement operator 
		**/
		FigureCanvas & operator=(FigureCanvas && o) 
			{
			if (&o == this) return *this;
			_vecallocp = std::move(o._vecallocp);
			_nbLayers = o._nbLayers;
			_figLayers = o._figLayers;
			o._figLayers = new TreeFigure<FigureInterface*, N>(o._nbLayers); // create empty objects to replace to ones moved.
			return *this;
			}


		/**
		 * Insert a figure into the canvas, inside a given layer
		 */
		template<typename FIGURECLASS> MTOOLS_FORCEINLINE void operator()(const FIGURECLASS & figure, size_t layer = 0)
			{
			MTOOLS_INSURE(layer < _nbLayers);
			FigureInterface * pf = _copyInPool(figure);			// save a copy of the object in the memory pool
			_figLayers[layer].insert(pf->boundingBox(), pf);	// add to the corresponding layer. 
			return;
			}


		/**   
		 * Empty the canvas. 
		 **/
		void clear()
			{
			_deallocateAll();
			for (size_t i = 0; i < _nbLayers; i++) _figLayers[i].reset();
			}


		/**
		* Return the number of layers
		**/
		MTOOLS_FORCEINLINE size_t nbLayers() const { return _nbLayers; }


		/**
		 * Return the number of objects in the canvas. 
		 */
		MTOOLS_FORCEINLINE size_t size() const
			{
			size_t tot = 0;
			for (size_t i = 0; i < _nbLayers; i++) tot += _figLayers[0].size();
			return tot;  
			}


		/**
		* Return the number of objects in a given layer.
		*/
		MTOOLS_FORCEINLINE size_t size(size_t layer) const
			{
			MTOOLS_ASSERT(layer < _nbLayers);
			return _figLayers[layer].size();
			}



		/** Return a pointer to the TreeFigure object associated with a given layer. */
		MTOOLS_FORCEINLINE TreeFigure<FigureInterface*, N> * getTreeLayer(size_t layer) const
			{
			MTOOLS_ASSERT(layer < _nbLayers);
			return _figLayers + layer;
			}


	private: 

		/* no copy */
		FigureCanvas(const FigureCanvas &) = delete;
		FigureCanvas & operator=(const FigureCanvas &) = delete;

		
		/* Make a copy of the figure object inside the memory pool */
		template<typename FIGURECLASS> MTOOLS_FORCEINLINE FigureInterface * _copyInPool(const FIGURECLASS & figure)
			{
			void * p = _allocate(sizeof(FIGURECLASS));	// allocate memory in the memory pool for the figure object
			new (p) FIGURECLASS(figure);				// placement new : copy constructor. 
			return ((FigureInterface *)p);				// cast to base class. 
			}


		/******************** MEMORY POOL IMPLEMENTATION (TODO : REPLACE BY BETTER VERSION) **********************/

		/* allocate size bytes in the memory pool. TODO: REPLACE A BY BETTER VERSION THAN MALLOC/FREE */
		MTOOLS_FORCEINLINE void * _allocate(size_t size)
			{
			void * p = malloc(size);
			_vecallocp.push_back(p); 
			return  p;
			}


		/* delete all allocated memory TODO: REPLACE A BY BETTER VERSION THAN MALLOC/FREE */
		void _deallocateAll()
			{
			for (void *  p : _vecallocp) { free(p); }
			_vecallocp.clear();
			}

		std::vector<void *>				_vecallocp;	// vector containing pointers to all allocated figures (TEMP WHILE USING MALLOC/FREE). 

		/*********************************************************************************************************/


		size_t								_nbLayers;	// number of layers
		TreeFigure<FigureInterface*, N> *	_figLayers;	// tree figure object for each layer. 


	};










	/************************************************************************************************************************************
	*
	* FIGURE CLASSES
	*
	*************************************************************************************************************************************/



	/**  
	 * Interface class for figure objects. 
	 *
	 * Any Figure object must derived from thispure virtual base class. 
	 **/
	class FigureInterface
	{

	public:


		/** Virtual destructor */
		virtual ~FigureInterface() {}


		/**
		* Draws the figure onto an image with a given range.
		*
		* @param [in,out]	im		    the image to draw onto.
		* @param [in,out]	R		    the range.
		* @param 		  	highQuality (Optional) True when high quality drawing is requested and false
		* 								otherwise.
		*/
		virtual void draw(Image & im, fBox2 & R, bool highQuality = true) = 0;


		/**
		* Return the object's bounding box.
		*
		* @return	A fBox2.
		*/
		virtual fBox2 boundingBox() const = 0;


		/**
		* Print info about the object into an std::string.
		*/
		virtual std::string toString(bool debug = false) const = 0;

		/**
		* Serialize the object.
		*/
		virtual void serialize(OBaseArchive & ar) const = 0;

		/**
		* Deserialize this object.
		*/
		virtual void deserialize(IBaseArchive & ar) = 0;

	};




	/************************************************************************************************************************************
	*
	* CIRCLE
	*
	*************************************************************************************************************************************/

	/**
	 * Circle figure
	 * 
	 * Parameters: 
	 *  - center and radius  
	 *  - outline color
	 *  - thickness  
	 *  - filling color   
	 *  - antialiasing
	 **/
	class FigureCircle : public FigureInterface
	{

	public:

		/** circle parameters **/
		fVec2	center;			// circle center
		double	radius;			// circle radius
		double	thickness;		// circle thickness 0 = no thickness. < 0 = absolute thicknes,  >0 = relative thickness 
		RGBc	color;			// circle color
		RGBc	fillcolor;		// circle interior color (transparent = no filling)


		/**   
		 * Constructor. Simple circle without filling or thickness    
		 **/
		FigureCircle(fVec2 centercircle, double rad, RGBc col) : center(centercircle), radius(rad), thickness(0.0), color(col), fillcolor(RGBc::c_Transparent)
			{
			MTOOLS_ASSERT(rad >= 0);
			}

		/**
		* Constructor. Simple circle with filling color but without thickness
		**/
		FigureCircle(fVec2 centercircle, double rad, RGBc col, RGBc fillcol) : center(centercircle), radius(rad), thickness(0.0), color(col), fillcolor(fillcol)
			{
			MTOOLS_ASSERT(rad >= 0);
			}

		/**
		* Constructor. Thick circle without filling
		**/
		FigureCircle(fVec2 centercircle, double rad, double thick, bool relativethickness, RGBc col) 
		: center(centercircle), radius(rad), thickness(relativethickness ? thick : -thick), color(col), fillcolor(RGBc::c_Transparent)
			{
			MTOOLS_ASSERT(rad >= 0);
			MTOOLS_ASSERT(thick > 0);
			}


		/**
		* Constructor. Thick circle with filling
		**/
		FigureCircle(fVec2 centercircle, double rad, double thick, bool relativethickness, RGBc col, RGBc fillcol)
			: center(centercircle), radius(rad), thickness(relativethickness ? thick : -thick), color(col), fillcolor(fillcol)
			{
			MTOOLS_ASSERT(rad >= 0);
			MTOOLS_ASSERT(thick > 0);
			}


		/**
		* Draws the figure onto an image with a given range.
		*
		* @param [in,out]	im		    the image to draw onto.
		* @param [in,out]	R		    the range.
		* @param 		  	highQuality (Optional) True when high quality drawing is requested and false
		* 								otherwise.
		*/
		virtual void draw(Image & im, fBox2 & R, bool highQuality = true) override
			{
			if (thickness == 0.0)
				{
				if (fillcolor.comp.A == 0)
					{
					im.canvas_draw_circle(R, center, radius, color, highQuality);
					}
				else
					{
					im.canvas_draw_filled_circle(R, center, radius, color, fillcolor, highQuality);
					}
				}
			else
				{
				const bool relative = (thickness > 0);
				const double thick = (relative ? thickness : -thickness);
				if (fillcolor.comp.A == 0)
					{
					im.canvas_draw_thick_circle(R, center, radius, thick, relative, color, highQuality);
					}
				else
					{
					im.canvas_draw_thick_filled_circle(R, center, radius, thick, relative, color, fillcolor, highQuality);
					}
				}
			}


		/**
		* Return the object's bounding box.
		*
		* @return	A fBox2.
		*/
		virtual fBox2 boundingBox() const override
			{
			return fBox2(center.X() - radius, center.X() + radius, center.Y() - radius, center.Y() + radius);
			}


		/**
		* Print info about the object into an std::string.
		*/
		virtual std::string toString(bool debug = false) const override
			{
			std::string str("Circle Figure [");
			str += mtools::toString(center) + " ";
			str += mtools::toString(radius) + " ";
			str += mtools::toString(color);
			if (fillcolor.comp.A != 0) str += std::string(" filled: ") + mtools::toString(fillcolor);
			if (thickness != 0.0)
				{
				if (thickness > 0) str += std::string(" rel. thick: ") + mtools::toString(thickness);
				else str += std::string(" abs. thick: ") + mtools::toString(-thickness);
				}
			return str + "]";
			}


		/** Serialize the object. */
		virtual void serialize(OBaseArchive & ar) const override
			{
			ar & center;
			ar & radius;
			ar & thickness;
			ar & color;
			ar & fillcolor;
			}


		/** Deserialize the object. */
		virtual void deserialize(IBaseArchive & ar) override
			{
			ar & center;
			ar & radius;
			ar & thickness;
			ar & color;
			ar & fillcolor;
			}
	};








	/**
	 * Circle Part figure
	 * 
	 * Parameters: 
	 *  - center and radius  
	 *  - outline color
	 *  - thickness  
	 *  - filling color   
	 *  - antialiasing  
	 *  - part to draw
	 **/
	class FigureCirclePart : public FigureInterface
	{

	public:

		/** circle part parameters **/
		fVec2	center;			// circle center
		double	radius;			// circle radius
		double	thickness;		// circle thickness 0 = no thickness. < 0 = absolute thicknes,  >0 = relative thickness 
		RGBc	color;			// circle color
		RGBc	fillcolor;		// circle interior color (transparent = no filling)
		int		part;			// one of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT

		/**   
		 * Constructor. Simple circle part without filling or thickness    
		 **/
		FigureCirclePart(int circlepart, fVec2 centercircle, double rad, RGBc col) : part(circlepart), center(centercircle), radius(rad), thickness(0.0), color(col), fillcolor(RGBc::c_Transparent)
			{
			MTOOLS_ASSERT((circlepart >= 0) && (circlepart < 4));
			MTOOLS_ASSERT(rad >= 0);
			}

		/**
		* Constructor. Simple circle part with filling color but without thickness
		**/
		FigureCirclePart(int circlepart, fVec2 centercircle, double rad, RGBc col, RGBc fillcol) : part(circlepart), center(centercircle), radius(rad), thickness(0.0), color(col), fillcolor(fillcol)
			{
			MTOOLS_ASSERT((circlepart >= 0) && (circlepart < 4));
			MTOOLS_ASSERT(rad >= 0);
			}

		/**
		* Constructor. Thick circle without filling
		**/
		FigureCirclePart(int circlepart, fVec2 centercircle, double rad, double thick, bool relativethickness, RGBc col)
		: part(circlepart), center(centercircle), radius(rad), thickness(relativethickness ? thick : -thick), color(col), fillcolor(RGBc::c_Transparent)
			{
			MTOOLS_ASSERT((circlepart >= 0) && (circlepart < 4));
			MTOOLS_ASSERT(rad >= 0);
			MTOOLS_ASSERT(thick > 0);
			}


		/**
		* Constructor. Thick circle with filling
		**/
		FigureCirclePart(int circlepart, fVec2 centercircle, double rad, double thick, bool relativethickness, RGBc col, RGBc fillcol)
			: part(circlepart), center(centercircle), radius(rad), thickness(relativethickness ? thick : -thick), color(col), fillcolor(fillcol)
			{
			MTOOLS_ASSERT((circlepart >= 0) && (circlepart < 4));
			MTOOLS_ASSERT(rad >= 0);
			MTOOLS_ASSERT(thick > 0);
			}


		/**
		* Draws the figure onto an image with a given range.
		*
		* @param [in,out]	im		    the image to draw onto.
		* @param [in,out]	R		    the range.
		* @param 		  	highQuality (Optional) True when high quality drawing is requested and false
		* 								otherwise.
		*/
		virtual void draw(Image & im, fBox2 & R, bool highQuality = true) override
			{
			if (thickness == 0.0)
				{
				if (fillcolor.comp.A == 0)
					{
					im.canvas_draw_part_circle(R, part, center, radius, color, highQuality);
					}
				else
					{
					im.canvas_draw_part_filled_circle(R, part, center, radius, color, fillcolor, highQuality);
					}
				}
			else
				{
				const bool relative = (thickness > 0);
				const double thick = (relative ? thickness : -thickness);
				if (fillcolor.comp.A == 0)
					{
					im.canvas_draw_part_thick_circle(R, part, center, radius, thick, relative, color, highQuality);
					}
				else
					{
					im.canvas_draw_part_thick_filled_circle(R, part, center, radius, thick, relative, color, fillcolor, highQuality);
					}
				}
			}


		/**
		* Return the object's bounding box.
		*
		* @return	A fBox2.
		*/
		virtual fBox2 boundingBox() const override
			{
			return fBox2(center.X() - radius, center.X() + radius, center.Y() - radius, center.Y() + radius).get_split(part);
			}


		/**
		* Print info about the object into an std::string.
		*/
		virtual std::string toString(bool debug = false) const override
			{
			std::string str("Circle Part Figure [");
			switch (part)
				{
				case BOX_SPLIT_UP: { str += "UP"; break; }
				case BOX_SPLIT_DOWN: { str += "DOWN"; break; }
				case BOX_SPLIT_LEFT: { str += "LEFT"; break; }
				case BOX_SPLIT_RIGHT: { str += "RIGHT"; break; }
				default: { str += "ERROR_PART"; }
				}
			str += " ";
			str += mtools::toString(center) + " ";
			str += mtools::toString(center) + " ";
			str += mtools::toString(radius) + " ";
			str += mtools::toString(color);
			if (fillcolor.comp.A != 0) str += std::string(" filled: ") + mtools::toString(fillcolor);
			if (thickness != 0.0)
				{
				if (thickness > 0) str += std::string(" rel. thick: ") + mtools::toString(thickness);
				else str += std::string(" abs. thick: ") + mtools::toString(-thickness);
				}
			return str + "]";
			}


		/** Serialize the object. */
		virtual void serialize(OBaseArchive & ar) const override
			{
			ar & part;
			ar & center;
			ar & radius;
			ar & thickness;
			ar & color;
			ar & fillcolor;
			}


		/** Deserialize the object. */
		virtual void deserialize(IBaseArchive & ar) override
			{
			ar & part;
			ar & center;
			ar & radius;
			ar & thickness;
			ar & color;
			ar & fillcolor;
			}
	};



}




/* end of file */

