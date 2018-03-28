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
	namespace Figure
		{
		namespace internals_figure
			{
			class FigureInterface;		// interface for a figure object
			}
		}


	template<int N> class FigureCanvas;	// main figure canvas class



/*
	// DOTS

	class CircleDot;
	class SquareDot;

	// LINES 

	class HorizontalLine;
	class VerticalLine;

	class ThickHorizontalLine;
	class ThickVerticalLine;

	class Line;
	class ThickLine;

	// CURVES

	class QuadBezier;
	class CubicBezier;

	class ThickQuadBezier;
	class ThickCubicBezier;

	// POLYGON

	class Triangle;
	class Quad;

	// ELLIPSE / CIRCLES

	class Circle;
	class CirclePart;

	class Ellipse;
	class EllipsePart;

	// TEXT
	
	// MISC

*/


	/*
	class PolyLine;
	class ThickPolyLine;
	class Polygon;
	class FigureBox;
	class FigureText;
	class FigureImage;
	class FigureFill;
	class FigureClip;
	template<typename FIGURE1, typename FIGURE2>  class FigurePair;
	template<typename FIGURE1, typename FIGURE2, typename FIGURE3>  class FigureTriplet;
	template<typename FIGURE1, typename FIGURE2, typename FIGURE3, typename FIGURE4>  class FigureQuadruplet;
	template<class... FIGURES> class FigureTuple;
	class FigureGroup;
	*/


	

	/**
	 * Factory function to create an empty Figure canvas with a given number of layers
	 */
	template<int N = 5> FigureCanvas<N> makeFigureCanvas(size_t nbLayers = 1)
		{
		return FigureCanvas<N>(nbLayers);
		}



	/**
	 * Class that holds figure objects.
	 * 
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
			_figLayers = new TreeFigure<Figure::internals_figure::FigureInterface*,N> [nbLayers];
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
			o._figLayers = new TreeFigure<Figure::internals_figure::FigureInterface*, N>[o._nbLayers]; // create empty objects to replace to ones moved.
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
			o._figLayers = new TreeFigure<Figure::internals_figure::FigureInterface*, N>(o._nbLayers); // create empty objects to replace to ones moved.
			return *this;
			}


		/**
		 * Insert a figure into the canvas, inside a given layer
		 */
		template<typename FIGURECLASS> MTOOLS_FORCEINLINE void operator()(const FIGURECLASS & figure, size_t layer = 0)
			{
			MTOOLS_INSURE(layer < _nbLayers);
			Figure::internals_figure::FigureInterface * pf = _copyInPool(figure);			// save a copy of the object in the memory pool
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
		MTOOLS_FORCEINLINE TreeFigure<Figure::internals_figure::FigureInterface*, N> * getTreeLayer(size_t layer) const
			{
			MTOOLS_ASSERT(layer < _nbLayers);
			return _figLayers + layer;
			}


	private: 

		/* no copy */
		FigureCanvas(const FigureCanvas &) = delete;
		FigureCanvas & operator=(const FigureCanvas &) = delete;

		
		/* Make a copy of the figure object inside the memory pool */
		template<typename FIGURECLASS> MTOOLS_FORCEINLINE Figure::internals_figure::FigureInterface * _copyInPool(const FIGURECLASS & figure)
			{
			void * p = _allocate(sizeof(FIGURECLASS));					// allocate memory in the memory pool for the figure object
			new (p) FIGURECLASS(figure);								// placement new : copy constructor. 
			return ((Figure::internals_figure::FigureInterface *)p);	// cast to base class. 
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
				((Figure::internals_figure::FigureInterface*)p)->~FigureInterface();	// call dtor
				free(p);																// free memory
				}
			_vecallocp.clear();
			}

		std::vector<void *>				_vecallocp;	// vector containing pointers to all allocated figures (TEMP WHILE USING MALLOC/FREE). 

		/*********************************************************************************************************/


		size_t															_nbLayers;	// number of layers
		TreeFigure<Figure::internals_figure::FigureInterface*, N> *		_figLayers;	// tree figure object for each layer. 


	};







	/************************************************************************************************************************************
	*
	* FIGURE CLASSES
	* 
	* All the classes located inside the Figure namespace are derived from FigureInterface hence can be inserted in a canvas object. 
	*
	*************************************************************************************************************************************/


	namespace Figure
	{

		namespace internals_figure
		{

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

		}





		/************************************************************************************************************************************
		*
		* DOTS
		*
		*************************************************************************************************************************************/



		/**
		* 
		* Circle Dot
		* 
		* The radius of a dot is absolute and does not scale with the range.
		* 
		**/
		class CircleDot : public internals_figure::FigureInterface
		{

		public:

			fVec2	center;			// center
			double	radius;			// dot radius
			RGBc	outlinecolor;	// outline color
			RGBc	fillcolor;		// interior color


			/**
			 * Construct a circle dot. Unit pixel. 
			 *
			 * @param	centerdot position
			 * @param	color	  color.
			 */
			CircleDot(fVec2 centerdot, RGBc color) : center(centerdot), radius(1.0), outlinecolor(color), fillcolor(color)
				{
				}


			/**
			 * Construct a circle dot with given (aboslute) radius.
			 *
			 * @param	centerdot position.
			 * @param	rad		  radius of the dot.
			 * @param	color	  color.
			 */
			CircleDot(fVec2 centerdot, double rad, RGBc color) : center(centerdot), radius(rad), outlinecolor(color), fillcolor(color)
				{
				MTOOLS_ASSERT(rad >= 0);
				}


			/** Constructor. Dot with given size and color and outline. **/
			CircleDot(fVec2 centerdot, double rad, RGBc border_color, RGBc fill_color) : center(centerdot), radius(rad), outlinecolor(border_color), fillcolor(fill_color)
				{
				MTOOLS_ASSERT(rad >= 0);
				}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				double r = (radius < min_thickness) ? min_thickness : radius;
				im.canvas_draw_circle_dot(R, center, r, outlinecolor, fillcolor, highQuality, true);
				}


			virtual fBox2 boundingBox() const override
				{
				return fBox2(center.X(), center.X(), center.Y(), center.Y());
				}


			virtual std::string toString(bool debug = false) const override
				{
				std::string str("CircleDot [");
				str += mtools::toString(center) + ", ";
				str += mtools::toString(radius) + "  outline ";
				str += mtools::toString(outlinecolor) + " interior " + mtools::toString(fillcolor);
				return str + "]";
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & center & radius & outlinecolor & fillcolor;
				}


			virtual void deserialize(IBaseArchive & ar) override
				{
				ar & center & radius & outlinecolor & fillcolor;
				}

		};




		/**
		* 
		* Square Dot
		* 
		* The radius of a dot is absolute and does not scale with the range.
		* 
		**/
		class SquareDot : public internals_figure::FigureInterface
		{

		public:

			fVec2	center;			// center
			int32	pw;				// radius
			RGBc	color;			// color


			/**
			 * Construct a square dot : unit pixel size. 
			 *
			 * @param	centerdot position
			 * @param	col		  color
			 */
			SquareDot(fVec2 centerdot, RGBc col) : center(centerdot), pw(0), color(col)
				{
				}


			/**
			 * Construct a square dot with given radius
			 *
			 * @param	centerdot position.
			 * @param	col		  color.
			 * @param	penwidth  radius (in pixels) of the dot. 
			 */
			SquareDot(fVec2 centerdot, RGBc col, int32 penwidth) : center(centerdot), pw(penwidth), color(col)
				{
				MTOOLS_ASSERT(penwidth >= 0);
				}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				im.canvas_draw_square_dot(R, center, color, true, pw);
				}


			virtual fBox2 boundingBox() const override
				{
				return fBox2(center.X(), center.X(), center.Y(), center.Y());
				}


			virtual std::string toString(bool debug = false) const override
				{
				std::string str("SquareDot [");
				str += mtools::toString(center) + ", ";
				str += mtools::toString(pw) + ", ";
				str += mtools::toString(color);
				return str + "]";
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & center & pw & color;
				}


			virtual void deserialize(IBaseArchive & ar) override
				{
				ar & center & pw & color;
				}

		};




		/************************************************************************************************************************************
		*
		* LINES
		*
		*************************************************************************************************************************************/


		/**
		*
		* Horizontal Line
		*
		**/
		class HorizontalLine : public internals_figure::FigureInterface
		{

		public:

			double x1, x2, y;
			RGBc	color;


			/**
			 * Construct an horizontal line.
			 *
			 * @param	Y   Y coordinate of the line.
			 * @param	X1  X coordinate of the first endpoint.
			 * @param	X2  X coordinate of the second endpoint.
			 * @param	col color to use.
			 */
			HorizontalLine(double Y, double X1, double X2, RGBc col) : x1(std::min(X1, X2)), x2(std::max(X1, X2)), y(Y), color(col)
				{
				}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				im.canvas_draw_horizontal_line(R, y, x1, x2, color);
				}


			virtual fBox2 boundingBox() const override
				{
				return fBox2(x1, x2, y, y);
				}


			virtual std::string toString(bool debug = false) const override
				{
				std::string str("HorizontalLine [");
				str += mtools::toString(x1) + " - ";
				str += mtools::toString(x2) + ", ";
				str += mtools::toString(y) + " ";
				str += mtools::toString(color);
				return str + "]";
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & x1 & x2 & y & color;
				}


			virtual void deserialize(IBaseArchive & ar) override
				{
				ar & x1 & x2 & y & color;
				}
		};



		/**
		*
		* Vertical Line figure
		*
		**/
		class VerticalLine : public internals_figure::FigureInterface
		{

		public:

			double y1, y2, x;
			RGBc	color;


			/**
			 * Construct a vertical line.
			 *
			 * @param	X   X coordinate of the line.
			 * @param	Y1  Y coordinate of the first endpoint.
			 * @param	Y2  Y coordinate of the second endpoint.
			 * @param	col color to use.
			 */
			VerticalLine(double X, double Y1, double Y2, RGBc col) : y1(std::min(Y1, Y2)), y2(std::max(Y1, Y2)), x(X), color(col)
				{
				}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				im.canvas_draw_vertical_line(R, x, y1, y2, color);
				}
				

			virtual fBox2 boundingBox() const override
				{
				return fBox2(x, x, y1, y2);
				}


			virtual std::string toString(bool debug = false) const override
			{
				std::string str("VerticalLine [");
				str += mtools::toString(x) + ", ";
				str += mtools::toString(y1) + " - ";
				str += mtools::toString(y2) + " ";
				str += mtools::toString(color);
				return str + "]";
			}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & y1 & y2 & x & color;
				}


			virtual void deserialize(IBaseArchive & ar) override
				{
				ar & y1 & y2 & x & color;
				}

		};



		/**
		*
		* Thick Horizontal Line
		*
		**/
		class ThickHorizontalLine : public internals_figure::FigureInterface
		{

		public:

			double x1, x2, y;
			double thickness; // positive for relative thickness and negative for absolute thickness
			RGBc	color;


			/**
			 * Construct a thick horizontal line
			 *
			 * @param	Y				  Y coord of the line.
			 * @param	X1				  first X coord endpoint.
			 * @param	X2				  second X coord endpoint.
			 * @param	col				  color to use.
			 * @param	thick			  thickness (should be positve)
			 * @param	relativethickness (Optional) True to scale thickness with range and false for fixed
			 * 							  thickness.
			 */
			ThickHorizontalLine(double Y, double X1, double X2, RGBc col, double thick, bool relativethickness = true) : x1(std::min(X1, X2)), x2(std::max(X1, X2)), y(Y), thickness(relativethickness ? thick : -thick), color(col)
				{
				MTOOLS_ASSERT(thick >= 0);
				}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				const bool relative = (thickness >= 0);
				const double thick = (relative ? thickness : -thickness);
				im.canvas_draw_thick_horizontal_line(R, y, x1, x2, thick, relative, color, true, true, min_thickness);
				}


			virtual fBox2 boundingBox() const override
				{				
				if (thickness > 0) { const double ht = thickness / 2;  return fBox2(x1, x2, y - ht, y + ht); } else { return fBox2(x1, x2, y, y); }
				}


			virtual std::string toString(bool debug = false) const override
				{
				std::string str("ThickHorizontalLine [");
				str += mtools::toString(x1) + " - ";
				str += mtools::toString(x2) + ", ";
				str += mtools::toString(y) + " ";
				str += mtools::toString(color);
				if (thickness >= 0) str += std::string(" rel. thick: ") + mtools::toString(thickness); else str += std::string(" abs. thick: ") + mtools::toString(-thickness);
				return str + "]";
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & x1 & x2 & y & color & thickness;
				}


			virtual void deserialize(IBaseArchive & ar) override
				{
				ar & x1 & x2 & y & color & thickness;
				}

		};


		/**
		*
		* Thick Vertical Line 
		*
		**/
		class ThickVerticalLine : public internals_figure::FigureInterface
		{

		public:

			
			double y1, y2, x;
			double thickness; // positive for relative thickness and negative for absolute thickness
			RGBc color;


			/**
			 * Construct a thick vertical line
			 *
			 * @param	X				  x coord of the line.
			 * @param	Y1				  first Y coord endpoint.
			 * @param	Y2				  second Y coord endpoint.
			 * @param	col				  color to use.
			 * @param	thick			  thickness (should be positve)
			 * @param	relativethickness (Optional) True to scale thickness with range and false for fixed
			 * 							  thickness.
			 */
			ThickVerticalLine(double X, double Y1, double Y2, RGBc col, double thick, bool relativethickness = true) : y1(std::min(Y1, Y2)), y2(std::max(Y1, Y2)), x(X), thickness(relativethickness ? thick : -thick), color(col)
				{
				MTOOLS_ASSERT(thick >= 0);
				}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				const bool relative = (thickness >= 0);
				const double thick = (relative ? thickness : -thickness);
				im.canvas_draw_thick_vertical_line(R, x, y1, y2, thick, relative, color, true, true, min_thickness);
				}


			virtual fBox2 boundingBox() const override
				{
				if (thickness > 0) { const double ht = thickness / 2;  return fBox2(x - ht, x + ht, y1, y2); } else { return fBox2(x, x, y1, y2); }
				}


			virtual std::string toString(bool debug = false) const override
				{
				std::string str("ThickVerticalLine [");
				str += mtools::toString(x) + ", ";
				str += mtools::toString(y1) + " - ";
				str += mtools::toString(y2) + " ";
				str += mtools::toString(color);
				if (thickness > 0) str += std::string(" rel. thick: ") + mtools::toString(thickness); else str += std::string(" abs. thick: ") + mtools::toString(-thickness);
				return str + "]";
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & y1 & y2 & x & color & thickness;
				}


			virtual void deserialize(IBaseArchive & ar) override
				{
				ar & y1 & y2 & x & color & thickness;
				}

		};


		/**
		*
		* Line
		*
		**/
		class Line : public internals_figure::FigureInterface
		{

		public:

			fVec2 P1, P2;
			RGBc  color;
			int32 pw;


			/**
			 * Constructor
			 *
			 * @param	p1		 first endpoint
			 * @param	p2		 second endpoint
			 * @param	col		 color
			 * @param	penwidth (Optional) penwidth (0 = unit pixel line). 
			 */
			Line(fVec2 p1, fVec2 p2, RGBc col, int32 penwidth = 0) : P1(p1), P2(p2), color(col), pw(penwidth)
				{
				}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				im.canvas_draw_line(R, P1, P2, color, true, highQuality, true, pw);
				}


			virtual fBox2 boundingBox() const override
				{
				return fBox2(std::min<double>(P1.X(), P2.X()), std::max<double>(P1.X(), P2.X()), std::min<double>(P1.Y(), P2.Y()), std::max<double>(P1.Y(), P2.Y()));
				}


			virtual std::string toString(bool debug = false) const override
				{
				std::string str("Line [");
				str += mtools::toString(P1) + ", ";
				str += mtools::toString(P2) + " (";
				str += mtools::toString(pw) + ") ";
				str += mtools::toString(color);
				return str + "]";
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & P1 & P2 & color & pw;
				}


			virtual void deserialize(IBaseArchive & ar) override
				{
				ar & P1 & P2 & color & pw;
				}
		};



		/**
		*
		* Thick Line
		*
		**/
		class ThickLine : public internals_figure::FigureInterface
		{

		public:

			fVec2 P1, P2;
			RGBc  color;
			double thick;


			/**
			 * Constructor.
			 *
			 * @param	p1		  first endpoint
			 * @param	p2		  second endpoint
			 * @param	thickness line thickness (relative).
			 * @param	col		  color
			 */
			ThickLine(fVec2 p1, fVec2 p2, double thickness, RGBc col) : P1(p1), P2(p2), color(col), thick(thickness)
				{
				MTOOLS_ASSERT(thick >= 0);
				MTOOLS_ASSERT(P1 != P2);
				}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				im.canvas_draw_thick_line(R, P1, P2, thick, color, highQuality, true, min_thickness);
				}


			virtual fBox2 boundingBox() const override
				{
				fBox2 R;
				fVec2 H = (P2 - P1).get_rotate90();
				H.normalize();
				H *= (thick*0.5);
				R.swallowPoint(P1 + H);
				R.swallowPoint(P1 - H);
				R.swallowPoint(P2 + H);
				R.swallowPoint(P2 - H);
				return R;
				}


			virtual std::string toString(bool debug = false) const override
				{
				std::string str("ThickLine [");
				str += mtools::toString(P1) + ", ";
				str += mtools::toString(P2) + " - ";
				str += mtools::toString(thick) + " ";
				str += mtools::toString(color);
				return str + "]";
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & P1 & P2 & color & thick;
				}


			virtual void deserialize(IBaseArchive & ar) override
				{
				ar & P1 & P2 & color & thick;
				}

		};



		/************************************************************************************************************************************
		*
		* POLYGON
		*
		*************************************************************************************************************************************/


		/**
		*
		* Triangle
		*
		**/
		class Triangle : public internals_figure::FigureInterface
		{

		public:

			fVec2 P1, P2, P3;
			RGBc  color, fillcolor;


			/**
			 * construct a triangle
			 *
			 * @param	p1	    first point.
			 * @param	p2	    second point.
			 * @param	p3	    third point.
			 * @param	col	    outline color.
			 * @param	fillcol (Optional) fill color (default = transparent = no fill)
			 */
			Triangle(fVec2 p1, fVec2 p2, fVec2 p3, RGBc col, RGBc fillcol = RGBc::c_Transparent) : P1(p1), P2(p2), P3(p3), color(col), fillcolor(fillcol)
				{
				}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				if (fillcolor.isTransparent())
					{
					im.canvas_draw_triangle(R, P1, P2, P3, color, highQuality, true);
					}
				else
					{
					im.canvas_draw_filled_triangle(R, P1, P2, P3, color, fillcolor, highQuality, true);
					}
				}


			virtual fBox2 boundingBox() const override
				{
				fBox2 R;
				R.swallowPoint(P1);
				R.swallowPoint(P2);
				R.swallowPoint(P3);
				return R;
				}


			virtual std::string toString(bool debug = false) const override
				{
				std::string str("Triangle [");
				str += mtools::toString(P1) + ", ";
				str += mtools::toString(P2) + ", ";
				str += mtools::toString(P3) + " - ";
				str += mtools::toString(color);
				if (!fillcolor.isTransparent())
					{
					str += " filled : ";
					str += mtools::toString(fillcolor);
					}
				return str + "]";
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & P1 & P2 & P3 & color & fillcolor;
				}


			virtual void deserialize(IBaseArchive & ar) override
				{
				ar & P1 & P2 & P3 & color & fillcolor;
				}

		};



		/**
		*
		* Quad
		*
		**/
		class Quad : public internals_figure::FigureInterface
		{

		public:

			
			fVec2 P1, P2, P3, P4;
			RGBc  color, fillcolor;


			/**
			 * Construct a quad.
			 *
			 * @param	p1	    first point.
			 * @param	p2	    second point.
			 * @param	p3	    third point.
			 * @param	p4	    fourth point.
			 * @param	col	    outline color
			 * @param	fillcol (Optional) fill color (default = transparent = no fill)
			 */
			Quad(fVec2 p1, fVec2 p2, fVec2 p3, fVec2 p4, RGBc col, RGBc fillcol = RGBc::c_Transparent) : P1(p1), P2(p2), P3(p3), P4(p4), color(col), fillcolor(fillcol)
				{
				}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				if (fillcolor.isTransparent())
					{
					im.canvas_draw_quad(R, P1, P2, P3, P4, color, highQuality, true);
					}
				else
					{
					im.canvas_draw_filled_quad(R, P1, P2, P3, P4, color, fillcolor, highQuality, true);
					}
				}


			virtual fBox2 boundingBox() const override
				{
				fBox2 R;
				R.swallowPoint(P1);
				R.swallowPoint(P2);
				R.swallowPoint(P3);
				R.swallowPoint(P4);
				return R;
				}


			virtual std::string toString(bool debug = false) const override
				{
				std::string str("Quad [");
				str += mtools::toString(P1) + ", ";
				str += mtools::toString(P2) + ", ";
				str += mtools::toString(P3) + ", ";
				str += mtools::toString(P4) + " - ";
				str += mtools::toString(color);
				if (!fillcolor.isTransparent())
					{
					str += "filled : ";
					str += mtools::toString(fillcolor);
					}
				return str + "]";
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & P1 & P2 & P3 & P4 & color & fillcolor;
				}


			virtual void deserialize(IBaseArchive & ar) override
				{
				ar & P1 & P2 & P3 & P4 & color & fillcolor;
				}

		};



		/**
		*
		* Polygon
		*
		**/
		class Polygon : public internals_figure::FigureInterface
		{

		public:

			std::vector<fVec2> tab;
			RGBc  color, fillcolor;


			/**
			 * Construct a polygon
			 *
			 * @param	tab_points	list of points, in clockwise or anti-clockwise order.
			 * @param	col		  	outline color.
			 * @param	fillcol   	(Optional) fill color (default = transparent = no fill)
			 **/
			Polygon(const std::vector<fVec2> & tab_points, RGBc col, RGBc fillcol = RGBc::c_Transparent) : tab(tab_points), color(col), fillcolor(fillcol)
				{
				MTOOLS_INSURE(tab_points.size() > 0);
				}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				if (fillcolor.isTransparent())
					{
					im.canvas_draw_polygon(R, tab, color, highQuality, true);
					}
				else
					{
					im.canvas_draw_filled_polygon(R, tab, color, fillcolor, highQuality, true);
					}
				}


			virtual fBox2 boundingBox() const override
				{
				fBox2 R;
				for (size_t i = 0; i < tab.size(); i++) { R.swallowPoint(tab[i]); }
				return R;
				}


			virtual std::string toString(bool debug = false) const override
				{
				std::string str("Polygon [");
				str += mtools::toString(tab) + " - ";
				str += mtools::toString(color);
				if (!fillcolor.isTransparent())
					{
					str += "filled : ";
					str += mtools::toString(fillcolor);
					}
				return str + "]";
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & tab & color & fillcolor;
				}


			virtual void deserialize(IBaseArchive & ar) override
				{
				ar & tab & color & fillcolor;
				}

		};


		/************************************************************************************************************************************
		*
		* CIRCLE / ELLIPSE
		*
		*************************************************************************************************************************************/


		/**
		*
		* Circle figure
		*
		**/
		class Circle : public internals_figure::FigureInterface
		{

		public:

			fVec2	center;
			double	radius;
			double	thickness;		// circle thickness 0 = no thickness. < 0 = absolute thicknes,  >0 = relative thickness 
			RGBc	color;
			RGBc	fillcolor;		// (transparent = no filling)


			/**
			 * Construct a circle with unit thickness and without filling.
			 *
			 * @param	centercircle circle center.
			 * @param	rad			 circle radius.
			 * @param	col			 circle (outline) color
			 */
			Circle(fVec2 centercircle, double rad, RGBc col) : center(centercircle), radius(rad), thickness(0.0), color(col), fillcolor(RGBc::c_Transparent)
				{
				MTOOLS_ASSERT(rad >= 0);
				}


			/**
			 * Construct a filled circle with unit thickness.
			 *
			 * @param	centercircle circle center.
			 * @param	rad			 circle radius.
			 * @param	col			 circle (outline) color
			 * @param	fillcol		 circle fill color
			 */
			Circle(fVec2 centercircle, double rad, RGBc col, RGBc fillcol) : center(centercircle), radius(rad), thickness(0.0), color(col), fillcolor(fillcol)
				{
				MTOOLS_ASSERT(rad >= 0);
				}


			/**
			 * Construct a thick circle (without filling).
			 *
			 * @param	centercircle		circle center.
			 * @param	rad					circle radius.
			 * @param	thick				thickness of the outline (going inside the circle).  
			 * @param	relativethickness	true to use relative thickness.
			 * @param	col					circle (outline) color
			 */
			Circle(fVec2 centercircle, double rad, double thick, bool relativethickness, RGBc col)
				: center(centercircle), radius(rad), thickness(relativethickness ? thick : -thick), color(col), fillcolor(RGBc::c_Transparent)
				{
				MTOOLS_ASSERT(rad >= 0);
				MTOOLS_ASSERT(thick > 0);
				}


			/**
			 * Construct a thick filled circle
			 *
			 * @param	centercircle		circle center.
			 * @param	rad					circle radius.
			 * @param	thick				thickness of the outline (going inside the circle).
			 * @param	relativethickness	true to use relative thickness.
			 * @param	col					circle (outline) color
			 * @param	fillcol				filling color
			 */
			Circle(fVec2 centercircle, double rad, double thick, bool relativethickness, RGBc col, RGBc fillcol)
				: center(centercircle), radius(rad), thickness(relativethickness ? thick : -thick), color(col), fillcolor(fillcol)
				{
				MTOOLS_ASSERT(rad >= 0);
				MTOOLS_ASSERT(thick > 0);
				}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				if (thickness == 0.0)
					{
					if (fillcolor.isTransparent()) { im.canvas_draw_circle(R, center, radius, color, highQuality); }
					else { im.canvas_draw_filled_circle(R, center, radius, color, fillcolor, highQuality); }
					}
				else
					{
					const bool relative = (thickness > 0);
					const double thick = (relative ? thickness : -thickness);
					if (fillcolor.isTransparent()) { im.canvas_draw_thick_circle(R, center, radius, thick, relative, color, highQuality, true, min_thickness); }
					else { im.canvas_draw_thick_filled_circle(R, center, radius, thick, relative, color, fillcolor, highQuality, true, min_thickness); }
					}
				}


			virtual fBox2 boundingBox() const override { return fBox2(center.X() - radius, center.X() + radius, center.Y() - radius, center.Y() + radius); }


			virtual std::string toString(bool debug = false) const override
				{
				std::string str("Circle [");
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


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & center & radius & thickness & color & fillcolor;
				}


			virtual void deserialize(IBaseArchive & ar) override
				{
				ar & center & radius & thickness & color & fillcolor;
				}

		};





		/**
		* 
		* Circle Part
		*
		**/
		class CirclePart : public internals_figure::FigureInterface
		{

		public:

			fVec2	center;
			double	radius;
			double	thickness;		// circle thickness 0 = no thickness. < 0 = absolute thicknes,  >0 = relative thickness 
			RGBc	color;
			RGBc	fillcolor;		// circle interior color (transparent = no filling)
			int		part;			// one of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT,, BOX_SPLIT_DOWN_LEFT, , BOX_SPLIT_DOWN_RIGHT


			/**
			 * Construct part of a circle (no filling, no thickness).
			 *
			 * @param	circlepart   part to draw, one of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT,
			 * 						 BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT,,
			 * 						 BOX_SPLIT_DOWN_LEFT, , BOX_SPLIT_DOWN_RIGHT.
			 * @param	centercircle center.
			 * @param	rad			 radius.
			 * @param	col			 outline color.
			 */
			CirclePart(int circlepart, fVec2 centercircle, double rad, RGBc col) : part(circlepart), center(centercircle), radius(rad), thickness(0.0), color(col), fillcolor(RGBc::c_Transparent)
				{
				MTOOLS_ASSERT((circlepart >= 0) && (circlepart < 8));
				MTOOLS_ASSERT(rad >= 0);
				}


			/**
			 * Construct part of a circle with filling (no thickness).
			 *
			 * @param	circlepart   part to draw, one of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT,
			 * 						 BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT,,
			 * 						 BOX_SPLIT_DOWN_LEFT, , BOX_SPLIT_DOWN_RIGHT.
			 * @param	centercircle center.
			 * @param	rad			 radius.
			 * @param	col			 outline color.
			 * @param	fillcol		 fill color.
			 */
			CirclePart(int circlepart, fVec2 centercircle, double rad, RGBc col, RGBc fillcol) : part(circlepart), center(centercircle), radius(rad), thickness(0.0), color(col), fillcolor(fillcol)
				{
				MTOOLS_ASSERT((circlepart >= 0) && (circlepart < 8));
				MTOOLS_ASSERT(rad >= 0);
				}


			/**
			 * Construct part of a circle with thickness (no filling).
			 *
			 * @param	circlepart		  part to draw, one of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT,
			 * 							  BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT,,
			 * 							  BOX_SPLIT_DOWN_LEFT, , BOX_SPLIT_DOWN_RIGHT.
			 * @param	centercircle	  center.
			 * @param	rad				  radius.
			 * @param	thick			  thickness of the outline (going inside the circle).
			 * @param	relativethickness true to scale thickness with range.
			 * @param	col				  outline color.
			 */
			CirclePart(int circlepart, fVec2 centercircle, double rad, double thick, bool relativethickness, RGBc col)
				: part(circlepart), center(centercircle), radius(rad), thickness(relativethickness ? thick : -thick), color(col), fillcolor(RGBc::c_Transparent)
				{
				MTOOLS_ASSERT((circlepart >= 0) && (circlepart < 8));
				MTOOLS_ASSERT(rad >= 0);
				MTOOLS_ASSERT(thick >= 0);
				}


			/**
			 * Construct part of a circle with thickness and filling
			 *
			 * @param	circlepart		  part to draw, one of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT,
			 * 							  BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT,,
			 * 							  BOX_SPLIT_DOWN_LEFT, , BOX_SPLIT_DOWN_RIGHT.
			 * @param	centercircle	  center.
			 * @param	rad				  radius.
			 * @param	thick			  thickness of the outline (going inside the circle).
			 * @param	relativethickness true to scale thickness with range.
			 * @param	col				  outline color.
			 * @param	fillcol			  fill color.
			 */
			CirclePart(int circlepart, fVec2 centercircle, double rad, double thick, bool relativethickness, RGBc col, RGBc fillcol)
				: part(circlepart), center(centercircle), radius(rad), thickness(relativethickness ? thick : -thick), color(col), fillcolor(fillcol)
				{
				MTOOLS_ASSERT((circlepart >= 0) && (circlepart < 8));
				MTOOLS_ASSERT(rad >= 0);
				MTOOLS_ASSERT(thick >= 0);
				}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				if (thickness == 0.0)
					{
					if (fillcolor.isTransparent()) { im.canvas_draw_part_circle(R, part, center, radius, color, highQuality); }
					else { im.canvas_draw_part_filled_circle(R, part, center, radius, color, fillcolor, highQuality); }
					}
				else
					{
					const bool relative = (thickness > 0);
					const double thick = (relative ? thickness : -thickness);
					if (fillcolor.isTransparent()) { im.canvas_draw_part_thick_circle(R, part, center, radius, thick, relative, color, highQuality, true, min_thickness); }
					else { im.canvas_draw_part_thick_filled_circle(R, part, center, radius, thick, relative, color, fillcolor, highQuality, true, min_thickness); }
					}
				}


			virtual fBox2 boundingBox() const override
				{
				return fBox2(center.X() - radius, center.X() + radius, center.Y() - radius, center.Y() + radius).get_split(part);
				}


			virtual std::string toString(bool debug = false) const override
				{
				std::string str("CirclePart [");
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


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & part & center & radius & thickness & color & fillcolor;
				}


			virtual void deserialize(IBaseArchive & ar) override
				{
				ar & part & center & radius & thickness & color & fillcolor;
				}

		};



		/**
		* 
		* Ellipse
		*
		**/
		class Ellipse : public internals_figure::FigureInterface
		{

		public:

			fVec2	center;
			double	rx;				// x-radius
			double  ry;				// y-radius
			double	thickness_x;	// 0 = no thickness. < 0 = absolute thicknes,  >0 = relative thickness 
			double	thickness_y;	// 0 = no thickness  < 0 = absolute thicknes,  >0 = relative thickness 
			RGBc	color;			// color
			RGBc	fillcolor;		// interior color (transparent = no filling)



			/**
			 * Construct an ellipse (without filling nor thickness)
			 *
			 * @param	centerellipse center 
			 * @param	rad_x		  x-radius
			 * @param	rad_y		  y-radius
			 * @param	col			  color
			 */
			Ellipse(fVec2 centerellipse, double rad_x, double rad_y, RGBc col) : center(centerellipse), rx(rad_x), ry(rad_y), thickness_x(0.0), thickness_y(0.0), color(col), fillcolor(RGBc::c_Transparent)
				{
				MTOOLS_ASSERT(rad_x >= 0);
				MTOOLS_ASSERT(rad_y >= 0);
				}


			/**
			 * Construct an ellipse (without filling nor thickness) from its bounding box.
			 *
			 * @param	B   bounding box
			 * @param	col color.
			 */
			Ellipse(const fBox2 & B, RGBc col) : center(B.center()), rx(B.l(0) / 2), ry(B.l(1) / 2), thickness_x(0.0), thickness_y(0.0), color(col), fillcolor(RGBc::c_Transparent)
				{
				MTOOLS_ASSERT(!B.isEmpty());
				}


			/**
			 * Construct a filled ellipse (without thickness)
			 *
			 * @param	centerellipse center.
			 * @param	rad_x		  x-radius.
			 * @param	rad_y		  y-radius.
			 * @param	col			  color.
			 * @param	fillcol		  fillcolor.
			 */
			Ellipse(fVec2 centerellipse, double rad_x, double rad_y, RGBc col, RGBc fillcol) : center(centerellipse), rx(rad_x), ry(rad_y), thickness_x(0.0), thickness_y(0.0), color(col), fillcolor(fillcol)
				{
				MTOOLS_ASSERT(rad_x >= 0);
				MTOOLS_ASSERT(rad_y >= 0);
				}


			/**
			 * Construct a filled ellipse (without thickness) from its bounding box.
			 *
			 * @param	B	    bounding box.
			 * @param	col	    color.
			 * @param	fillcol fill color
			 */
			Ellipse(const fBox2 & B, RGBc col, RGBc fillcol) : center(B.center()), rx(B.l(0) / 2), ry(B.l(1) / 2), thickness_x(0.0), thickness_y(0.0), color(col), fillcolor(fillcol)
				{
				MTOOLS_ASSERT(!B.isEmpty());
				}


			/**
			 * Construct a thick ellipse (without filling)
			 *
			 * @param	centerellipse	  center.
			 * @param	rad_x			  x-radius.
			 * @param	rad_y			  y-radius.
			 * @param	thick_x			  thickness on the x axis (going inside)
			 * @param	thick_y			  thickness on the y axis (going inside)
			 * @param	relativethickness true to use relative thickness.
			 * @param	col				  color.
			 */
			Ellipse(fVec2 centerellipse, double rad_x, double rad_y, double thick_x, double thick_y, bool relativethickness, RGBc col)
				: center(centerellipse), rx(rad_x), ry(rad_y), thickness_x(relativethickness ? thick_x : -thick_x), thickness_y(relativethickness ? thick_y : -thick_y), color(col), fillcolor(RGBc::c_Transparent)
				{
				MTOOLS_ASSERT(rad_x >= 0);
				MTOOLS_ASSERT(rad_y >= 0);
				MTOOLS_ASSERT(thick_x >= 0);
				MTOOLS_ASSERT(thick_y >= 0);
				}


			/**
			* Construct a thick ellipse (without filling) from its bounding box
			*
			 * @param	B				  bounding box
			 * @param	thick_x			  thickness on the x axis (going inside)
			 * @param	thick_y			  thickness on the y axis (going inside)
			 * @param	relativethickness true to use relative thickness.
			 * @param	col				  color.
			 */
			Ellipse(const fBox2 & B, double thick_x, double thick_y, bool relativethickness, RGBc col)
				: center(B.center()), rx(B.l(0) / 2), ry(B.l(1) / 2), thickness_x(relativethickness ? thick_x : -thick_x), thickness_y(relativethickness ? thick_y : -thick_y), color(col), fillcolor(RGBc::c_Transparent)
				{
				MTOOLS_ASSERT(!B.isEmpty());
				MTOOLS_ASSERT(thick_x >= 0);
				MTOOLS_ASSERT(thick_y >= 0);
				}


			/**
			 * Construct a thick filled ellipse
			 *
			 * @param	centerellipse	  center.
			 * @param	rad_x			  x-radius.
			 * @param	rad_y			  y-radius.
			 * @param	thick_x			  thickness on the x axis (going inside)
			 * @param	thick_y			  thickness on the y axis (going inside)
			 * @param	relativethickness true to use relative thickness.
			 * @param	col				  color.
			 * @param	fillcol			  fill color
			 */
			Ellipse(fVec2 centerellipse, double rad_x, double rad_y, double thick_x, double thick_y, bool relativethickness, RGBc col, RGBc fillcol)
				: center(centerellipse), rx(rad_x), ry(rad_y), thickness_x(relativethickness ? thick_x : -thick_x), thickness_y(relativethickness ? thick_y : -thick_y), color(col), fillcolor(fillcol)
				{
				MTOOLS_ASSERT(rad_x >= 0);
				MTOOLS_ASSERT(rad_y >= 0);
				MTOOLS_ASSERT(thick_x >= 0);
				MTOOLS_ASSERT(thick_y >= 0);
				}


			/**
			 * Construct a thick filled ellipse from its bounding box
			 *
			 * @param	B				  bounding box.
			 * @param	thick_x			  thickness on the x axis (going inside)
			 * @param	thick_y			  thickness on the y axis (going inside)
			 * @param	relativethickness true to use relative thickness.
			 * @param	col				  color.
			 * @param	fillcol			  fill color
			 */
			Ellipse(const fBox2 & B, double thick_x, double thick_y, bool relativethickness, RGBc col, RGBc fillcol)
				: center(B.center()), rx(B.l(0) / 2), ry(B.l(1) / 2), thickness_x(relativethickness ? thick_x : -thick_x), thickness_y(relativethickness ? thick_y : -thick_y), color(col), fillcolor(fillcol)
				{
				MTOOLS_ASSERT(!B.isEmpty());
				MTOOLS_ASSERT(thick_x >= 0);
				MTOOLS_ASSERT(thick_y >= 0);
				}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				if ((thickness_x == 0.0) && (thickness_y == 0.0))
					{
					if (fillcolor.isTransparent()) { im.canvas_draw_ellipse(R, center, rx, ry, color, highQuality); }
					else { im.canvas_draw_filled_ellipse(R, center, rx, ry, color, fillcolor, highQuality); }
					}
				else
					{
					const bool relative = (thickness_x >= 0);
					const double tx = ((thickness_x < 0) ? (-thickness_x) : thickness_x);
					const double ty = ((thickness_y < 0) ? (-thickness_y) : thickness_y);
					if (fillcolor.isTransparent()) { im.canvas_draw_thick_ellipse(R, center, rx, ry, tx, ty, relative, color, highQuality, true, min_thickness); }
					else { im.canvas_draw_thick_filled_ellipse(R, center, rx, ry, tx, ty, relative, color, fillcolor, highQuality, true, min_thickness); }
					}
				}


			virtual fBox2 boundingBox() const override
				{
				return fBox2(center.X() - rx, center.X() + rx, center.Y() - ry, center.Y() + ry);
				}


			virtual std::string toString(bool debug = false) const override
				{
				std::string str("Ellipse [");
				str += mtools::toString(center) + " ";
				str += mtools::toString(rx) + " ";
				str += mtools::toString(ry) + " ";
				str += mtools::toString(color);
				if (fillcolor.comp.A != 0) str += std::string(" filled: ") + mtools::toString(fillcolor);
				if ((thickness_x != 0.0) || (thickness_y != 0.0))
					{
					if (thickness_x >= 0) str += std::string(" rel. thick: x ") + mtools::toString(thickness_x) + " y " + mtools::toString(thickness_y);
					else str += std::string(" abs. thick: ") + mtools::toString(thickness_x) + " y " + mtools::toString(thickness_y);
					}
				return str + "]";
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & center & rx & ry & thickness_x & thickness_y & color & fillcolor; 
				}


			virtual void deserialize(IBaseArchive & ar) override
				{
				ar & center & rx & ry & thickness_x & thickness_y & color & fillcolor;
				}

		};



		/**
		* 
		* Ellipse Part figure
		*
		**/
		class EllipsePart : public internals_figure::FigureInterface
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
			 * Construct part of an ellipse (no filling, no thickness).
			 *
			 * @param	ellipsepart   part to draw. One of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT,
			 * 						  BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT,,
			 * 						  BOX_SPLIT_DOWN_LEFT, , BOX_SPLIT_DOWN_RIGHT.
			 * @param	centerellipse center.
			 * @param	rad_x		  x-radius.
			 * @param	rad_y		  y-radius.
			 * @param	col			  outline color.
			 */
			EllipsePart(int ellipsepart, fVec2 centerellipse, double rad_x, double rad_y, RGBc col) : center(centerellipse), rx(rad_x), ry(rad_y), thickness_x(0.0), thickness_y(0.0), color(col), fillcolor(RGBc::c_Transparent), part(ellipsepart)
				{
				MTOOLS_ASSERT((ellipsepart >= 0) && (ellipsepart < 8));
				MTOOLS_ASSERT(rad_x >= 0);
				MTOOLS_ASSERT(rad_y >= 0);
				}


			/**
			 * Construct part of an ellipse (no filling, no thickness) from its bounding box
			 *
			 * @param	ellipsepart part to draw. One of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT,
			 * 						BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT,,
			 * 						BOX_SPLIT_DOWN_LEFT, , BOX_SPLIT_DOWN_RIGHT.
			 * @param	B		    bounding box (for the whole ellipse).
			 * @param	col		    outline color
			 */
			EllipsePart(int ellipsepart, const fBox2 & B, RGBc col) : center(B.center()), rx(B.l(0) / 2), ry(B.l(1) / 2), thickness_x(0.0), thickness_y(0.0), color(col), fillcolor(RGBc::c_Transparent), part(ellipsepart)
				{
				MTOOLS_ASSERT((ellipsepart >= 0) && (ellipsepart < 8));
				MTOOLS_ASSERT(!B.isEmpty());
				}


			/**
			 * Construct part of a filled ellipse (no thickness)
			 *
			 * @param	ellipsepart   part to draw. One of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT,
			 * 						  BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT,,
			 * 						  BOX_SPLIT_DOWN_LEFT, , BOX_SPLIT_DOWN_RIGHT.
			 * @param	centerellipse center.
			 * @param	rad_x		  x-radius.
			 * @param	rad_y		  y-radius.
			 * @param	col			  outline color.
			 * @param	fillcol		  fill color.
			 */
			EllipsePart(int ellipsepart, fVec2 centerellipse, double rad_x, double rad_y, RGBc col, RGBc fillcol) : center(centerellipse), rx(rad_x), ry(rad_y), thickness_x(0.0), thickness_y(0.0), color(col), fillcolor(fillcol), part(ellipsepart)
				{
				MTOOLS_ASSERT((ellipsepart >= 0) && (ellipsepart < 8));
				MTOOLS_ASSERT(rad_x >= 0);
				MTOOLS_ASSERT(rad_y >= 0);
				}


			/**
			 * Construct part of a filled ellipse (no thickness) from its bounding box
			 *
			 * @param	ellipsepart part to draw. One of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT,
			 * 						BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT,,
			 * 						BOX_SPLIT_DOWN_LEFT, , BOX_SPLIT_DOWN_RIGHT.
			 * @param	B		    bounding box (for the whole ellipse).
			 * @param	col		    outline color.
			 * @param	fillcol	    fill color.
			 */
			EllipsePart(int ellipsepart, const fBox2 & B, RGBc col, RGBc fillcol) : center(B.center()), rx(B.l(0) / 2), ry(B.l(1) / 2), thickness_x(0.0), thickness_y(0.0), color(col), fillcolor(fillcol), part(ellipsepart)
				{
				MTOOLS_ASSERT((ellipsepart >= 0) && (ellipsepart < 8));
				MTOOLS_ASSERT(!B.isEmpty());
				}


			/**
			 * Construct part of a thick ellipse (no filling).
			 *
			 * @param	ellipsepart		  part to draw. One of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT,
			 * 							  BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT,,
			 * 							  BOX_SPLIT_DOWN_LEFT, , BOX_SPLIT_DOWN_RIGHT.
			 * @param	centerellipse	  center.
			 * @param	rad_x			  x-radius.
			 * @param	rad_y			  y-radius.
			 * @param	thick_x			  thickness on the x-axis (going inside)
			 * @param	thick_y			  thickness on the y-axis (going inside)
			 * @param	relativethickness true to use relative thickness.
			 * @param	col				  outline color.
			 */
			EllipsePart(int ellipsepart, fVec2 centerellipse, double rad_x, double rad_y, double thick_x, double thick_y, bool relativethickness, RGBc col)
				: center(centerellipse), rx(rad_x), ry(rad_y), thickness_x(relativethickness ? thick_x : -thick_x), thickness_y(relativethickness ? thick_y : -thick_y), color(col), fillcolor(RGBc::c_Transparent), part(ellipsepart)
				{
				MTOOLS_ASSERT((ellipsepart >= 0) && (ellipsepart < 8));
				MTOOLS_ASSERT(rad_x >= 0);
				MTOOLS_ASSERT(rad_y >= 0);
				MTOOLS_ASSERT(thick_x >= 0);
				MTOOLS_ASSERT(thick_y >= 0);
				}


			/**
			 * Construct part of a thick ellipse (no filling) from its bounding box
			 *
			 * @param	ellipsepart		  part to draw. One of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT,
			 * 							  BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT,,
			 * 							  BOX_SPLIT_DOWN_LEFT, , BOX_SPLIT_DOWN_RIGHT.
			 * @param	B				  bounding box (for the whole ellipse).
			 * @param	thick_x			  thickness on the x-axis (going inside)
			 * @param	thick_y			  thickness on the y-axis (going inside)
			 * @param	relativethickness true to use relative thickness.
			 * @param	col				  outline color.
			 */
			EllipsePart(int ellipsepart, const fBox2 & B, double thick_x, double thick_y, bool relativethickness, RGBc col)
				: center(B.center()), rx(B.l(0) / 2), ry(B.l(1) / 2), thickness_x(relativethickness ? thick_x : -thick_x), thickness_y(relativethickness ? thick_y : -thick_y), color(col), fillcolor(RGBc::c_Transparent), part(ellipsepart)
				{
				MTOOLS_ASSERT((ellipsepart >= 0) && (ellipsepart < 8));
				MTOOLS_ASSERT(!B.isEmpty());
				MTOOLS_ASSERT(thick_x >= 0);
				MTOOLS_ASSERT(thick_y >= 0);
				}


			/**
			 * Construct part of a thick filled ellipse.
			 *
			 * @param	ellipsepart		  part to draw. One of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT,
			 * 							  BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT,,
			 * 							  BOX_SPLIT_DOWN_LEFT, , BOX_SPLIT_DOWN_RIGHT.
			 * @param	centerellipse	  center.
			 * @param	rad_x			  x-radius.
			 * @param	rad_y			  y-radius.
			 * @param	thick_x			  thickness on the x-axis (going inside)
			 * @param	thick_y			  thickness on the y-axis (going inside)
			 * @param	relativethickness true to use relative thickness.
			 * @param	col				  outline color.
			 * @param	fillcol			  fill color.
			 */
			EllipsePart(int ellipsepart, fVec2 centerellipse, double rad_x, double rad_y, double thick_x, double thick_y, bool relativethickness, RGBc col, RGBc fillcol)
				: center(centerellipse), rx(rad_x), ry(rad_y), thickness_x(relativethickness ? thick_x : -thick_x), thickness_y(relativethickness ? thick_y : -thick_y), color(col), fillcolor(fillcol), part(ellipsepart)
				{
				MTOOLS_ASSERT((ellipsepart >= 0) && (ellipsepart < 8));
				MTOOLS_ASSERT(rad_x >= 0);
				MTOOLS_ASSERT(rad_y >= 0);
				MTOOLS_ASSERT(thick_x >= 0);
				MTOOLS_ASSERT(thick_y >= 0);
				}


			/**
			 * Construct part of a thick filled ellipse from its bounding box
			 *
			 * @param	ellipsepart		  part to draw. One of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT,
			 * 							  BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT,,
			 * 							  BOX_SPLIT_DOWN_LEFT, , BOX_SPLIT_DOWN_RIGHT.
			 * @param	B				  bounding box (for the whole ellipse).
			 * @param	thick_x			  thickness on the x-axis (going inside)
			 * @param	thick_y			  thickness on the y-axis (going inside)
			 * @param	relativethickness true to use relative thickness.
			 * @param	col				  outline color.
			 * @param	fillcol			  fill color.
			 */
			EllipsePart(int ellipsepart, const fBox2 & B, double thick_x, double thick_y, bool relativethickness, RGBc col, RGBc fillcol)
				: center(B.center()), rx(B.l(0) / 2), ry(B.l(1) / 2), thickness_x(relativethickness ? thick_x : -thick_x), thickness_y(relativethickness ? thick_y : -thick_y), color(col), fillcolor(fillcol), part(ellipsepart)
				{
				MTOOLS_ASSERT((ellipsepart >= 0) && (ellipsepart < 8));
				MTOOLS_ASSERT(!B.isEmpty());
				MTOOLS_ASSERT(thick_x >= 0);
				MTOOLS_ASSERT(thick_y >= 0);
				}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				if ((thickness_x == 0.0) && (thickness_y == 0.0))
					{
					if (fillcolor.isTransparent()) { im.canvas_draw_part_ellipse(R, part, center, rx, ry, color, highQuality); }
					else { im.canvas_draw_part_filled_ellipse(R, part, center, rx, ry, color, fillcolor, highQuality); }
					}
				else
					{
					const bool relative = (thickness_x >= 0);
					const double tx = ((thickness_x < 0) ? (-thickness_x) : thickness_x);
					const double ty = ((thickness_y < 0) ? (-thickness_y) : thickness_y);
					if (fillcolor.isTransparent()) { im.canvas_draw_part_thick_ellipse(R, part, center, rx, ry, tx, ty, relative, color, highQuality, true, min_thickness); }
					else { im.canvas_draw_part_thick_filled_ellipse(R, part, center, rx, ry, tx, ty, relative, color, fillcolor, highQuality, true, min_thickness); }
					}
				}


			virtual fBox2 boundingBox() const override
				{
				return fBox2(center.X() - rx, center.X() + rx, center.Y() - ry, center.Y() + ry).get_split(part);
				}


			virtual std::string toString(bool debug = false) const override
				{
				std::string str("EllipsePart [");
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
				if ((thickness_x != 0.0) || (thickness_y != 0.0))
					{
					if (thickness_x >= 0) str += std::string(" rel. thick: x ") + mtools::toString(thickness_x) + " y " + mtools::toString(thickness_y);
					else str += std::string(" abs. thick: ") + mtools::toString(thickness_x) + " y " + mtools::toString(thickness_y);
					}
				return str + "]";
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & part & center & rx & ry & thickness_x & thickness_y & color & fillcolor;
				}


			virtual void deserialize(IBaseArchive & ar) override
				{
				ar & part & center & rx & ry & thickness_x & thickness_y & color & fillcolor;
				}

		};




	}



}




/* end of file */

