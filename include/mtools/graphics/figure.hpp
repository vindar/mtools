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


	// DONE 
	// 
	class FigureDot;

	class FigureCircle;
	class FigureCirclePart;
	class FigureEllipse;
	class FigureEllipsePart;

	class FigureHorizontalLine;
	class FigureVerticalLine;


	// TODO
	 
	class FigureLine;
	class FigureThickLine;
	class FigurePolyLine;
	class FigureThickPolyLine;

	class FigureBox;
	class FigureTriangle;
	class FigureConvexPolygon;
	class FigureQuadBezier;
	class FigureRatQuadBezier;
	class FigureCubicBezier;

	class FigureText;

	class FigureImage;

	class FigureFill;
	class FigureClip;

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
			for (void *  p : _vecallocp) 
				{
				((FigureInterface*)p)->~FigureInterface();	// call dtor
				free(p);									// free memory
				}
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
		* @param 			R		    the range.
		* @param 		  	highQuality True when high quality drawing is requested and false otherwise
		* @param 		  	min_thick   minimum thickness to use when drawing
		*/
		virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) = 0;


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
	* LINES
	*
	*************************************************************************************************************************************/



	/**
	* 
	* Horizontal Line figure
	*
	**/
	class FigureHorizontalLine : public FigureInterface
	{

	public:

		/** parameters **/  
		double x1, x2, y;		// line 
		double thickness;		// thickness, 0 = no thickness. < 0 = absolute thickness,  >0 = relative thickness
		RGBc	color;			// circle color


		/**
		* Constructor. Horizontal line, no thickness.
		**/
		FigureHorizontalLine(double Y, double X1, double X2, RGBc col) 
		: x1(std::min(X1, X2)), x2(std::max(X1, X2)), y(Y), thickness(0.0), color(col)
			{
			}

		/**
		* Constructor. Horizontal line, with thickness.
		**/
		FigureHorizontalLine(double Y, double X1, double X2, double thick, bool relativethickness, RGBc col)
		: x1(std::min(X1,X2)), x2(std::max(X1, X2)), y(Y), thickness(relativethickness ? thick : -thick), color(col)
			{
			MTOOLS_ASSERT(thick >= 0);
			}


		/** Draw method */
		virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
			{
			if (thickness == 0.0)
				{
				im.canvas_draw_horizontal_line(R, y, x1, x2, color);
				}
			else
				{
				const bool relative = (thickness >= 0);
				const double thick = (relative ? thickness : -thickness);
				im.canvas_draw_thick_horizontal_line(R, y, x1, x2, thick, relative, color,true, true, min_thickness);
				}
			}


		/** Return the object's bounding box. */
		virtual fBox2 boundingBox() const override
			{
			if (thickness > 0)
				{
				return fBox2(x1 , x2 , y - thickness, y + thickness);
				}
			else
				{ 
				return fBox2(x1, x2, y, y);
				}
			}


		/** Print info about the object into an std::string. */
		virtual std::string toString(bool debug = false) const override
			{
			std::string str("Horizontal line [");
			str += mtools::toString(x1) + " - ";
			str += mtools::toString(x2) + ", ";
			str += mtools::toString(y) + " ";
			str += mtools::toString(color);
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
			ar & x1;
			ar & x2;
			ar & y;
			ar & color;
			ar & thickness;
			}


		/** Deserialize the object. */
		virtual void deserialize(IBaseArchive & ar) override
			{
			ar & x1;
			ar & x2;
			ar & y;
			ar & color;
			ar & thickness;
			}
	};



	/**
	*
	* Vertical Line figure
	*
	**/
	class FigureVerticalLine : public FigureInterface
	{

	public:

		/** parameters **/  
		double y1, y2, x;		// line 
		double thickness;		// thickness, 0 = no thickness. < 0 = absolute thickness,  >0 = relative thickness
		RGBc	color;			// circle color


		/**
		* Constructor. Horizontal line, no thickness.
		**/
		FigureVerticalLine(double X, double Y1, double Y2, RGBc col) 
		: y1(std::min(Y1, Y2)), y2(std::max(Y1, Y2)), x(X), thickness(0.0), color(col)
			{
			}

		/**
		* Constructor. Horizontal line, with thickness.
		**/
		FigureVerticalLine(double X, double Y1, double Y2, double thick, bool relativethickness, RGBc col)
		: y1(std::min(Y1,Y2)), y2(std::max(Y1, Y2)), x(X), thickness(relativethickness ? thick : -thick), color(col)
			{
			MTOOLS_ASSERT(thick >= 0);
			}


		/** Draw method */
		virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
			{
			if (thickness == 0.0)
				{
				im.canvas_draw_vertical_line(R, x, y1, y2, color);
				}
			else
				{
				const bool relative = (thickness >= 0);
				const double thick = (relative ? thickness : -thickness);
				im.canvas_draw_thick_vertical_line(R, x, y1, y2, thick, relative, color, true, true, min_thickness);
				}
			}


		/** Return the object's bounding box. */
		virtual fBox2 boundingBox() const override
			{
			if (thickness > 0)
				{
				return fBox2(x - thickness, x + thickness, y1, y2);
				}
			else
				{ 
				return fBox2(x, x, y1, y2);
				}
			}


		/** Print info about the object into an std::string. */
		virtual std::string toString(bool debug = false) const override
			{
			std::string str("Horizontal line [");
			str += mtools::toString(x) + ", ";
			str += mtools::toString(y1) + " - ";
			str += mtools::toString(y2) + " ";
			str += mtools::toString(color);
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
			ar & y1;
			ar & y2;
			ar & x;
			ar & color;
			ar & thickness;
			}


		/** Deserialize the object. */
		virtual void deserialize(IBaseArchive & ar) override
			{
			ar & y1;
			ar & y2;
			ar & x;
			ar & color;
			ar & thickness;
			}
	};



	/**
	*
	* Line figure
	*
	**/
	class FigureLine : public FigureInterface
	{

	public:

		/** parameters **/  
		fVec2 P1, P2;
		RGBc  color;
		int32 thick;


		/**
		* Constructor. Horizontal line, no thickness.
		**/
		FigureLine(fVec2 p1, fVec2 p2, RGBc col, int32 thickness = 0) : P1(p1), P2(p2), color(col), thick(thickness)
		{
		}


		/** Draw method */
		virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
			{
			im.canvas_draw_line(R, P1, P2, color, true, true, highQuality, thick);
			}


		/** Return the object's bounding box. */
		virtual fBox2 boundingBox() const override
			{
			return fBox2(std::min<double>(P1.X(), P2.X()), std::max<double>(P1.X(), P2.X()), std::min<double>(P1.Y(), P2.Y()), std::max<double>(P1.Y(), P2.Y()));
			}


		/** Print info about the object into an std::string. */
		virtual std::string toString(bool debug = false) const override
			{
			std::string str("Line [");
			str += mtools::toString(P1) + ", ";
			str += mtools::toString(P2) + " - ";
			str += mtools::toString(thick) + " ";
			str += mtools::toString(color);
			return str + "]";
			}


		/** Serialize the object. */
		virtual void serialize(OBaseArchive & ar) const override
			{
			ar & P1 & P2 & color & thick;
			}


		/** Deserialize the object. */
		virtual void deserialize(IBaseArchive & ar) override
			{
			ar & P1 & P2 & color & thick;
			}
	};



	/************************************************************************************************************************************
	*
	* CIRCLE / ELLIPSE
	*
	*************************************************************************************************************************************/


	/**
	 * Dot figure : filled (possibly thick) circle with absolute size that
	 * do not scale with the range. 
	 * 
	 * Parameters: 
	 *  - center   
	 *  - radius  
	 *  - outline color
	 *  - filling color   
	 **/
	class FigureDot : public FigureInterface
	{

	public:

		/** circle parameters **/
		fVec2	center;			// circle center
		double	radius;			// circle radius
		RGBc	outlinecolor;	// outline color
		RGBc	fillcolor;		// interior color


		/**
		 * Constructor. unit size color dot.
		 **/
		FigureDot(fVec2 centerdot, RGBc color) : center(centerdot), radius(1.0), outlinecolor(color), fillcolor(color)
			{
			}


		/**   
		 * Constructor. Dot with given size and color.    
		 **/
		FigureDot(fVec2 centerdot, double rad, RGBc color) : center(centerdot), radius(rad), outlinecolor(color), fillcolor(color)
			{
			MTOOLS_ASSERT(rad > 0);
			}


		/**
		* Constructor. Dot with given size and color and outline
		**/
		FigureDot(fVec2 centerdot, double rad, RGBc border_color, RGBc fill_color) : center(centerdot), radius(rad), outlinecolor(border_color), fillcolor(fill_color)
			{
			MTOOLS_ASSERT(rad > 0);
			}


		/**
		* Draws the dot on the image
		**/
		virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
			{
			double r = (radius < min_thickness) ? min_thickness : radius;
			im.canvas_draw_dot(R, center, r, outlinecolor, fillcolor, highQuality, true);
			}


		/**
		* Dot's bounding box : single point. 
		*/
		virtual fBox2 boundingBox() const override
			{
			return fBox2(center.X(), center.X(), center.Y(), center.Y());
			}


		/**
		* Print info about the object into an std::string.
		*/
		virtual std::string toString(bool debug = false) const override
			{
			std::string str("Dot Figure [");
			str += mtools::toString(center) + " ";
			str += mtools::toString(radius) + "  outline ";
			str += mtools::toString(outlinecolor) + " interior " + mtools::toString(fillcolor);
			return str + "]";
			}


		/** Serialize the object. */
		virtual void serialize(OBaseArchive & ar) const override
			{
			ar & center;
			ar & radius;
			ar & outlinecolor;
			ar & fillcolor;
			}


		/** Deserialize the object. */
		virtual void deserialize(IBaseArchive & ar) override
			{
			ar & center;
			ar & radius;
			ar & outlinecolor;
			ar & fillcolor;
			}

	};



	/**
	 * Circle figure
	 * 
	 * Parameters: 
	 *  - center and radius  
	 *  - outline color
	 *  - thickness  
	 *  - filling color   
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
		virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
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
					im.canvas_draw_thick_circle(R, center, radius, thick, relative, color, highQuality, true, min_thickness);
					}
				else
					{
					im.canvas_draw_thick_filled_circle(R, center, radius, thick, relative, color, fillcolor, highQuality, true, min_thickness);
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
		int		part;			// one of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT,, BOX_SPLIT_DOWN_LEFT, , BOX_SPLIT_DOWN_RIGHT


		/**   
		 * Constructor. Simple circle part without filling or thickness    
		 **/
		FigureCirclePart(int circlepart, fVec2 centercircle, double rad, RGBc col) : part(circlepart), center(centercircle), radius(rad), thickness(0.0), color(col), fillcolor(RGBc::c_Transparent)
			{
			MTOOLS_ASSERT((circlepart >= 0) && (circlepart < 8));
			MTOOLS_ASSERT(rad >= 0);
			}

		/**
		* Constructor. Simple circle part with filling color but without thickness
		**/
		FigureCirclePart(int circlepart, fVec2 centercircle, double rad, RGBc col, RGBc fillcol) : part(circlepart), center(centercircle), radius(rad), thickness(0.0), color(col), fillcolor(fillcol)
			{
			MTOOLS_ASSERT((circlepart >= 0) && (circlepart < 8));
			MTOOLS_ASSERT(rad >= 0);
			}

		/**
		* Constructor. Thick circle without filling
		**/
		FigureCirclePart(int circlepart, fVec2 centercircle, double rad, double thick, bool relativethickness, RGBc col)
		: part(circlepart), center(centercircle), radius(rad), thickness(relativethickness ? thick : -thick), color(col), fillcolor(RGBc::c_Transparent)
			{
			MTOOLS_ASSERT((circlepart >= 0) && (circlepart < 8));
			MTOOLS_ASSERT(rad >= 0);
			MTOOLS_ASSERT(thick >= 0);
			}


		/**
		* Constructor. Thick circle with filling
		**/
		FigureCirclePart(int circlepart, fVec2 centercircle, double rad, double thick, bool relativethickness, RGBc col, RGBc fillcol)
			: part(circlepart), center(centercircle), radius(rad), thickness(relativethickness ? thick : -thick), color(col), fillcolor(fillcol)
			{
			MTOOLS_ASSERT((circlepart >= 0) && (circlepart < 8));
			MTOOLS_ASSERT(rad >= 0);
			MTOOLS_ASSERT(thick >= 0);
			}


		/**
		* Draws the figure onto an image with a given range.
		*
		* @param [in,out]	im		    the image to draw onto.
		* @param [in,out]	R		    the range.
		* @param 		  	highQuality (Optional) True when high quality drawing is requested and false
		* 								otherwise.
		*/
		virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
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
					im.canvas_draw_part_thick_circle(R, part, center, radius, thick, relative, color, highQuality, true, min_thickness);
					}
				else
					{
					im.canvas_draw_part_thick_filled_circle(R, part, center, radius, thick, relative, color, fillcolor, highQuality, true, min_thickness);
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
				case BOX_SPLIT_UP: { str += "HALF UP"; break; }
				case BOX_SPLIT_DOWN: { str += "HALF DOWN"; break; }
				case BOX_SPLIT_LEFT: { str += "HALF LEFT"; break; }
				case BOX_SPLIT_RIGHT: { str += "HALF RIGHT"; break; }
				case BOX_SPLIT_UP_LEFT: { str += "QUARTER UP LEFT"; break; }
				case BOX_SPLIT_UP_RIGHT: { str += "QUARTER UP RIGHT"; break; }
				case BOX_SPLIT_DOWN_LEFT: { str += "QUARTER DOWN LEFT"; break; }
				case BOX_SPLIT_DOWN_RIGHT: { str += "QUARTER DOWN RIGHT"; break; }
				default: { str += "ERROR PART"; }
				}
			str += " ";
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




	/**
	 * Ellipse figure
	 * 
	 * Parameters: 
	 *  - center and radii  
	 *  - outline color
	 *  - thickness  
	 *  - filling color   
	 *  - antialiasing
	 **/
	class FigureEllipse : public FigureInterface
	{

	public:

		/** ellipse parameters **/
		fVec2	center;			// circle center
		double	rx;				// x-radius
		double  ry;				// y-radius
		double	thickness_x;	// 0 = no thickness. < 0 = absolute thicknes,  >0 = relative thickness 
		double	thickness_y;	// 0 = no thickness  < 0 = absolute thicknes,  >0 = relative thickness 
		RGBc	color;			// color
		RGBc	fillcolor;		// interior color (transparent = no filling)


		/**   
		 * Constructor. Simple ellipse without filling or thickness    
		 **/
		FigureEllipse(fVec2 centerellipse, double rad_x, double rad_y, RGBc col) : center(centerellipse), rx(rad_x), ry(rad_y), thickness_x(0.0), thickness_y(0.0), color(col), fillcolor(RGBc::c_Transparent)
			{
			MTOOLS_ASSERT(rad_x >= 0);
			MTOOLS_ASSERT(rad_y >= 0);
			}

		/**
		* Constructor. Simple ellipse without filling or thickness, given by its bounding box
		**/
		FigureEllipse(const fBox2 & B, RGBc col) : center(B.center()), rx(B.l(0)/2), ry(B.l(1)/2), thickness_x(0.0), thickness_y(0.0), color(col), fillcolor(RGBc::c_Transparent)
			{
			MTOOLS_ASSERT(! B.isEmpty());
			}


		/**
		* Constructor. Simple ellipse with filling color but without thickness
		**/
		FigureEllipse(fVec2 centerellipse, double rad_x, double rad_y, RGBc col, RGBc fillcol) : center(centerellipse), rx(rad_x), ry(rad_y), thickness_x(0.0), thickness_y(0.0), color(col), fillcolor(fillcol)
			{
			MTOOLS_ASSERT(rad_x >= 0);
			MTOOLS_ASSERT(rad_y >= 0);
			}


		/**
		* Constructor. Simple ellipse with filling but without thickness, given by its bounding box
		**/
		FigureEllipse(const fBox2 & B, RGBc col, RGBc fillcol) : center(B.center()), rx(B.l(0) / 2), ry(B.l(1) / 2), thickness_x(0.0), thickness_y(0.0), color(col), fillcolor(fillcol)
		{
			MTOOLS_ASSERT(!B.isEmpty());
		}


		/**
		* Constructor. Thick ellipse without filling
		**/
		FigureEllipse(fVec2 centerellipse, double rad_x, double rad_y, double thick_x, double thick_y, bool relativethickness, RGBc col)
		: center(centerellipse), rx(rad_x), ry(rad_y), thickness_x(relativethickness ? thick_x : -thick_x), thickness_y(relativethickness ? thick_y : -thick_y), color(col), fillcolor(RGBc::c_Transparent)
			{
			MTOOLS_ASSERT(rad_x >= 0);
			MTOOLS_ASSERT(rad_y >= 0);
			MTOOLS_ASSERT(thick_x >= 0);
			MTOOLS_ASSERT(thick_y >= 0);
			}


		/**
		* Constructor. Thick ellipse without filling, given by its bounding box
		**/
		FigureEllipse(const fBox2 & B, double thick_x, double thick_y, bool relativethickness, RGBc col) 
			: center(B.center()), rx(B.l(0) / 2), ry(B.l(1) / 2), thickness_x(relativethickness ? thick_x : -thick_x), thickness_y(relativethickness ? thick_y : -thick_y), color(col), fillcolor(RGBc::c_Transparent)
			{
			MTOOLS_ASSERT(!B.isEmpty());
			MTOOLS_ASSERT(thick_x >= 0);
			MTOOLS_ASSERT(thick_y >= 0);
			}


		/**
		* Constructor. Thick ellipse with filling
		**/
		FigureEllipse(fVec2 centerellipse, double rad_x, double rad_y, double thick_x, double thick_y, bool relativethickness, RGBc col, RGBc fillcol)
			: center(centerellipse), rx(rad_x), ry(rad_y), thickness_x(relativethickness ? thick_x : -thick_x), thickness_y(relativethickness ? thick_y : -thick_y), color(col), fillcolor(fillcol)
			{
			MTOOLS_ASSERT(rad_x >= 0);
			MTOOLS_ASSERT(rad_y >= 0);
			MTOOLS_ASSERT(thick_x >= 0);
			MTOOLS_ASSERT(thick_y >= 0);
			}


		/**
		* Constructor. Thick ellipse with filling, given by its bounding box
		**/
		FigureEllipse(const fBox2 & B, double thick_x, double thick_y, bool relativethickness, RGBc col, RGBc fillcol)
			: center(B.center()), rx(B.l(0) / 2), ry(B.l(1) / 2), thickness_x(relativethickness ? thick_x : -thick_x), thickness_y(relativethickness ? thick_y : -thick_y), color(col), fillcolor(fillcol)
			{
			MTOOLS_ASSERT(!B.isEmpty());
			MTOOLS_ASSERT(thick_x >= 0);
			MTOOLS_ASSERT(thick_y >= 0);
			}


		/**
		* Draws the figure onto an image with a given range.
		*
		* @param [in,out]	im		    the image to draw onto.
		* @param [in,out]	R		    the range.
		* @param 		  	highQuality (Optional) True when high quality drawing is requested and false
		* 								otherwise.
		*/
		virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
			{
			if ((thickness_x == 0.0)&&(thickness_y == 0.0))
				{
				if (fillcolor.comp.A == 0)
					{
					im.canvas_draw_ellipse(R, center, rx, ry, color, highQuality);
					}
				else
					{
					im.canvas_draw_filled_ellipse(R, center, rx, ry, color, fillcolor, highQuality);
					}
				}
			else
				{
				const bool relative = (thickness_x >= 0);
				const double tx = ((thickness_x < 0) ? (-thickness_x) : thickness_x);
				const double ty = ((thickness_y < 0) ? (-thickness_y) : thickness_y);
				if (fillcolor.comp.A == 0)
					{
					im.canvas_draw_thick_ellipse(R, center, rx, ry, tx, ty, relative, color, highQuality, true, min_thickness);
					}
				else
					{
					im.canvas_draw_thick_filled_ellipse(R, center, rx, ry, tx, ty, relative, color, fillcolor, highQuality, true, min_thickness);
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
			return fBox2(center.X() - rx, center.X() + rx, center.Y() - ry, center.Y() + ry);
			}


		/**
		* Print info about the object into an std::string.
		*/
		virtual std::string toString(bool debug = false) const override
			{
			std::string str("Ellipse Figure [");
			str += mtools::toString(center) + " ";
			str += mtools::toString(rx) + " ";
			str += mtools::toString(ry) + " ";
			str += mtools::toString(color);
			if (fillcolor.comp.A != 0) str += std::string(" filled: ") + mtools::toString(fillcolor);
			if ((thickness_x != 0.0)|| (thickness_y != 0.0))
				{
				if (thickness_x >= 0) str += std::string(" rel. thick: x ") + mtools::toString(thickness_x) + " y " + mtools::toString(thickness_y);
				else str += std::string(" abs. thick: ") + mtools::toString(thickness_x) + " y " + mtools::toString(thickness_y);
				}
			return str + "]";
			}


		/** Serialize the object. */
		virtual void serialize(OBaseArchive & ar) const override
			{
			ar & center;
			ar & rx;
			ar & ry;
			ar & thickness_x;
			ar & thickness_y;
			ar & color;
			ar & fillcolor;
			}


		/** Deserialize the object. */
		virtual void deserialize(IBaseArchive & ar) override
			{
			ar & center;
			ar & rx;
			ar & ry;
			ar & thickness_x;
			ar & thickness_y;
			ar & color;
			ar & fillcolor;
			}
	};




	/**
	 * Ellipse Part figure
	 * 
	 * Parameters: 
	 *  - center and radii  
	 *  - outline color
	 *  - thickness  
	 *  - filling color   
	 *  - antialiasing  
	 *  - part to draw
	 **/
	class FigureEllipsePart : public FigureInterface
	{

	public:

		/** ellipse parameters **/
		fVec2	center;			// circle center
		double	rx;				// x-radius
		double  ry;				// y-radius
		double	thickness_x;	// 0 = no thickness. < 0 = absolute thicknes,  >0 = relative thickness 
		double	thickness_y;	// 0 = no thickness  < 0 = absolute thicknes,  >0 = relative thickness 
		RGBc	color;			// color
		RGBc	fillcolor;		// interior color (transparent = no filling)
		int		part;			// one of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT,, BOX_SPLIT_DOWN_LEFT, , BOX_SPLIT_DOWN_RIGHT


		/**   
		 * Constructor. Simple ellipse without filling or thickness    
		 **/
		FigureEllipsePart(int ellipsepart, fVec2 centerellipse, double rad_x, double rad_y, RGBc col) : center(centerellipse), rx(rad_x), ry(rad_y), thickness_x(0.0), thickness_y(0.0), color(col), fillcolor(RGBc::c_Transparent), part(ellipsepart)
			{
			MTOOLS_ASSERT((ellipsepart >= 0) && (ellipsepart < 8));
			MTOOLS_ASSERT(rad_x >= 0);
			MTOOLS_ASSERT(rad_y >= 0);
			}

		/**
		* Constructor. Simple ellipse without filling or thickness, given by its bounding box  (the bounding box if for the whole ellipse)
		**/
		FigureEllipsePart(int ellipsepart, const fBox2 & B, RGBc col) : center(B.center()), rx(B.l(0)/2), ry(B.l(1)/2), thickness_x(0.0), thickness_y(0.0), color(col), fillcolor(RGBc::c_Transparent), part(ellipsepart)
			{
			MTOOLS_ASSERT((ellipsepart >= 0) && (ellipsepart < 8));
			MTOOLS_ASSERT(! B.isEmpty());
			}


		/**
		* Constructor. Simple ellipse with filling color but without thickness
		**/
		FigureEllipsePart(int ellipsepart, fVec2 centerellipse, double rad_x, double rad_y, RGBc col, RGBc fillcol) : center(centerellipse), rx(rad_x), ry(rad_y), thickness_x(0.0), thickness_y(0.0), color(col), fillcolor(fillcol), part(ellipsepart)
			{
			MTOOLS_ASSERT((ellipsepart >= 0) && (ellipsepart < 8));
			MTOOLS_ASSERT(rad_x >= 0);
			MTOOLS_ASSERT(rad_y >= 0);
			}


		/**
		* Constructor. Simple ellipse with filling but without thickness, given by its bounding box  (the bounding box if for the whole ellipse)
		**/
		FigureEllipsePart(int ellipsepart, const fBox2 & B, RGBc col, RGBc fillcol) : center(B.center()), rx(B.l(0) / 2), ry(B.l(1) / 2), thickness_x(0.0), thickness_y(0.0), color(col), fillcolor(fillcol), part(ellipsepart)
			{
			MTOOLS_ASSERT((ellipsepart >= 0) && (ellipsepart < 8));
			MTOOLS_ASSERT(!B.isEmpty());
			}


		/**
		* Constructor. Thick ellipse without filling
		**/
		FigureEllipsePart(int ellipsepart, fVec2 centerellipse, double rad_x, double rad_y, double thick_x, double thick_y, bool relativethickness, RGBc col)
		: center(centerellipse), rx(rad_x), ry(rad_y), thickness_x(relativethickness ? thick_x : -thick_x), thickness_y(relativethickness ? thick_y : -thick_y), color(col), fillcolor(RGBc::c_Transparent), part(ellipsepart)
			{
			MTOOLS_ASSERT((ellipsepart >= 0) && (ellipsepart < 8));
			MTOOLS_ASSERT(rad_x >= 0);
			MTOOLS_ASSERT(rad_y >= 0);
			MTOOLS_ASSERT(thick_x >= 0);
			MTOOLS_ASSERT(thick_y >= 0);
			}


		/**
		* Constructor. Thick ellipse without filling, given by its bounding box  (the bounding box if for the whole ellipse)
		**/
		FigureEllipsePart(int ellipsepart, const fBox2 & B, double thick_x, double thick_y, bool relativethickness, RGBc col)
			: center(B.center()), rx(B.l(0) / 2), ry(B.l(1) / 2), thickness_x(relativethickness ? thick_x : -thick_x), thickness_y(relativethickness ? thick_y : -thick_y), color(col), fillcolor(RGBc::c_Transparent), part(ellipsepart)
			{
			MTOOLS_ASSERT((ellipsepart >= 0) && (ellipsepart < 8));
			MTOOLS_ASSERT(!B.isEmpty());
			MTOOLS_ASSERT(thick_x >= 0);
			MTOOLS_ASSERT(thick_y >= 0);
			}


		/**
		* Constructor. Thick ellipse with filling
		**/
		FigureEllipsePart(int ellipsepart, fVec2 centerellipse, double rad_x, double rad_y, double thick_x, double thick_y, bool relativethickness, RGBc col, RGBc fillcol)
			: center(centerellipse), rx(rad_x), ry(rad_y), thickness_x(relativethickness ? thick_x : -thick_x), thickness_y(relativethickness ? thick_y : -thick_y), color(col), fillcolor(fillcol), part(ellipsepart)
			{
			MTOOLS_ASSERT((ellipsepart >= 0) && (ellipsepart < 8));
			MTOOLS_ASSERT(rad_x >= 0);
			MTOOLS_ASSERT(rad_y >= 0);
			MTOOLS_ASSERT(thick_x >= 0);
			MTOOLS_ASSERT(thick_y >= 0);
			}


		/**
		* Constructor. Thick ellipse with filling, given by its bounding box (the bounding box if for the whole ellipse)
		**/
		FigureEllipsePart(int ellipsepart, const fBox2 & B, double thick_x, double thick_y, bool relativethickness, RGBc col, RGBc fillcol)
			: center(B.center()), rx(B.l(0) / 2), ry(B.l(1) / 2), thickness_x(relativethickness ? thick_x : -thick_x), thickness_y(relativethickness ? thick_y : -thick_y), color(col), fillcolor(fillcol), part(ellipsepart)
			{
			MTOOLS_ASSERT((ellipsepart >= 0) && (ellipsepart < 8));
			MTOOLS_ASSERT(!B.isEmpty());
			MTOOLS_ASSERT(thick_x >= 0);
			MTOOLS_ASSERT(thick_y >= 0);
			}


		/**
		* Draws the figure onto an image with a given range.
		*
		* @param [in,out]	im		    the image to draw onto.
		* @param [in,out]	R		    the range.
		* @param 		  	highQuality (Optional) True when high quality drawing is requested and false
		* 								otherwise.
		*/
		virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
			{
			if ((thickness_x == 0.0)&&(thickness_y == 0.0))
				{
				if (fillcolor.comp.A == 0)
					{
					im.canvas_draw_part_ellipse(R, part, center, rx, ry, color, highQuality);
					}
				else
					{
					im.canvas_draw_part_filled_ellipse(R, part, center, rx, ry, color, fillcolor, highQuality);
					}
				}
			else
				{
				const bool relative = (thickness_x >= 0);
				const double tx = ((thickness_x < 0) ? (-thickness_x) : thickness_x);
				const double ty = ((thickness_y < 0) ? (-thickness_y) : thickness_y);
				if (fillcolor.comp.A == 0)
					{
					im.canvas_draw_part_thick_ellipse(R, part, center, rx, ry, tx, ty, relative, color, highQuality,true,min_thickness);
					}
				else
					{
					im.canvas_draw_part_thick_filled_ellipse(R, part, center, rx, ry, tx, ty, relative, color, fillcolor, highQuality, true, min_thickness);
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
			return fBox2(center.X() - rx, center.X() + rx, center.Y() - ry, center.Y() + ry).get_split(part);
			}


		/**
		* Print info about the object into an std::string.
		*/
		virtual std::string toString(bool debug = false) const override
			{
			std::string str("Ellipse Part Figure [");
			switch (part)
			{
			case BOX_SPLIT_UP: { str += "HALF UP"; break; }
			case BOX_SPLIT_DOWN: { str += "HALF DOWN"; break; }
			case BOX_SPLIT_LEFT: { str += "HALF LEFT"; break; }
			case BOX_SPLIT_RIGHT: { str += "HALF RIGHT"; break; }
			case BOX_SPLIT_UP_LEFT: { str += "QUARTER UP LEFT"; break; }
			case BOX_SPLIT_UP_RIGHT: { str += "QUARTER UP RIGHT"; break; }
			case BOX_SPLIT_DOWN_LEFT: { str += "QUARTER DOWN LEFT"; break; }
			case BOX_SPLIT_DOWN_RIGHT: { str += "QUARTER DOWN RIGHT"; break; }
			default: { str += "ERROR PART"; }
			}
			str += " " + mtools::toString(center) + " ";
			str += mtools::toString(rx) + " ";
			str += mtools::toString(ry) + " ";
			str += mtools::toString(color);
			if (fillcolor.comp.A != 0) str += std::string(" filled: ") + mtools::toString(fillcolor);
			if ((thickness_x != 0.0)|| (thickness_y != 0.0))
				{
				if (thickness_x >= 0) str += std::string(" rel. thick: x ") + mtools::toString(thickness_x) + " y " + mtools::toString(thickness_y);
				else str += std::string(" abs. thick: ") + mtools::toString(thickness_x) + " y " + mtools::toString(thickness_y);
				}
			return str + "]";
			}


		/** Serialize the object. */
		virtual void serialize(OBaseArchive & ar) const override
			{
			ar & part;
			ar & center;
			ar & rx;
			ar & ry;
			ar & thickness_x;
			ar & thickness_y;
			ar & color;
			ar & fillcolor;
			}


		/** Deserialize the object. */
		virtual void deserialize(IBaseArchive & ar) override
			{
			ar & part;
			ar & center;
			ar & rx;
			ar & ry;
			ar & thickness_x;
			ar & thickness_y;
			ar & color;
			ar & fillcolor;
			}
	};











}




/* end of file */

