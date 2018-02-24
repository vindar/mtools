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




	class FigureBox;

	class FigureCircle;

	class FigureEllipse;

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
	 * Class that holds figure objects
	 * 
	 * NOT THREADSAFE : do not insert objects while accessing (ie drawing) the canvas. 
	 */
	class FigureCanvas
	{


	public: 


		/**
		 * Constructor: create an empty canvas with a given number of layers. 
		 **/
		FigureCanvas(size_t nbLayers = 1) : _nbLayers(nbLayers)
			{
			MTOOLS_INSURE(nbLayers > 0);
			_figLayers = new TreeFigure<FigureInterface*>(nbLayers);
			}


		 /**
		 * Destructor
		 **/
		~FigureCanvas()
			{
			clear();
			delete _figLayers;
			}


		/**
		 * Insert a figure into the canvas at a given location
		 */
		template<typename FIGURECLASS> MTOOLS_FORCEINLINE void operator()(const FIGURECLASS & figure, size_t layer = 0)
			{
			MTOOLS_ASSERT(layer < _nbLayers);
			FigureInterface * pf = _copyInPool(figure);			// save a copy of the object in the memory pool
			_figLayers[layer].insert(pf->boundingBox(), pf);	// add to the corresponding layer. 
			return;
			}


		/**
		* Return the number of layers
		**/
		MTOOLS_FORCEINLINE size_t nbLayers() const { return _nbLayers; }


		/**   
		 * Empty the canvas. 
		 **/
		void clear()
			{
			// TODO
			}


		/**
		 * Return the number of objects in the canvas. 
		 */
		MTOOLS_FORCEINLINE size_t size() const
			{
			// TODO
			return 0;  
			}


		/** Return a pointer to the TreeFigure object associated with a given layer. */
		MTOOLS_FORCEINLINE TreeFigure<FigureInterface*> * getTreeLayer(size_t layer) const
			{
			MTOOLS_ASSERT(layer < _nbLayers);
			return _figLayers + layer;
			}


	private: 


		/* no copy */
		FigureCanvas(const FigureCanvas &) = delete;
		FigureCanvas & operator=(const FigureCanvas &) = delete;



		/******************** MEMORY POOL **********************/


		/* Make a copy of the figure object inside the memory pool */
		template<typename FIGURECLASS> MTOOLS_FORCEINLINE
		//typename std::enable_if<std::is_base_of<FigureInterface, typename FIGURECLASS>, FigureInterface*>::type
		FigureInterface *
		_copyInPool(const FIGURECLASS & figure)
			{
			void * p = _allocate(sizeof(FIGURECLASS));	// allocate memory in the memory pool for the figure object
			new (p) FIGURECLASS(figure);				// placement new : copy constructor. 
			return ((FigureInterface *)p);				// cast to base class. 
			}


		/* allocate size bytes in the memory pool. */
		MTOOLS_FORCEINLINE void * _allocate(size_t size)
			{ 
			return malloc(size); // TODO : use better memory pool. 
			}


		const size_t					_nbLayers;	// number of layers
		TreeFigure<FigureInterface*> *	_figLayers;	// tree figure object for each layer. 


	};



}




/* end of file */

