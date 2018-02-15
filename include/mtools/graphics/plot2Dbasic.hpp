/** @file plot2Dbasic.hpp */
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
#include "../maths/box.hpp"
#include "image.hpp"
#include "internal/plotter2Dobj.hpp"
#include "internal/drawable2Dobject.hpp"

namespace mtools
{



	/**   
	 * Plot2DBasic class    
	 *
	 * This class is used to create simple plot object that do not need work and without options. 
	 * 
	 * There are two ways of using the class. 
	 * 
	 * 1) Create a derived class and override the virtual method:
	 *                     'void draw(const fBox2 & R, Image & im, float opacity)'  
	 *
	 * 2) Give the ctor of the PlotBasic object an external function with signature
	 *                     'void [drawfun](const fBox2 & R, Image & im, float opacity)'
	 * 
	 * 
	 * Example using approach (1)
	 * 
	 * struct Plot2DTest : public Plot2DBasic
     *     {
     *     Plot2DTest() : Plot2DBasic() {}	// ctor
	 *     virtual void draw(const fBox2 & R, Image & im, float op) override
	 *         { // draw a circle
	 *         im.canvas_draw_thick_filled_circle(R, { 100.0,100.0 }, 100.0, 10.0, true, RGBc::c_Red.getMultOpacity(op), RGBc::c_Green.getMultOpacity(op));
	 *         }
	 *     };
	 **/
	class Plot2DBasic : public internals_graphics::Plotter2DObj, protected internals_graphics::Drawable2DInterface
	{

		typedef void(*p_drawfun)(const fBox2 & R, Image & im, float opacity);		///< signature of the draw function

	public:

		/**
		* Constructor
		*
		* @param	drawfun (Optional) drawing function with signature 'void drawfun(const fBox2 & range, Image & image, float opacity)'
		* @param	name    (Optional) name of the plot.
		*/
		Plot2DBasic(p_drawfun drawfun = nullptr, const std::string & name = "Plot2DBasic") : internals_graphics::Plotter2DObj(name), _drawfun(drawfun), _range()
			{
			}


		/**
		* Move constructor
		*/
		Plot2DBasic(Plot2DBasic && o) : internals_graphics::Plotter2DObj(std::move(o)), _drawfun(o._drawfun)
			{
			}


		/** Destructor */
		virtual ~Plot2DBasic()
			{
			detach();
			}


		/**
		* Draw function. This function must be overloaded in derived classes
		* or an external drawfun function must be provided in the constructor of the class.
		*
		* @param 		  	R	    the range
		* @param [in,out]	im	    The image to draw onto
		* @param 		  	opacity The opacity to use (0.0 = fully transparent, 1.0 = fully opaque).
		*/
		virtual void draw(const fBox2 & R, Image & im, float opacity)
			{
			MTOOLS_INSURE(_drawfun != nullptr);
			_drawfun(R, im, opacity); 		// default draw method call the external draw function.
			}


	protected:


		/**
		* Override of the setParam method from the Drawable2DObject interface
		**/
		virtual void setParam(mtools::fBox2 range, mtools::iVec2 imageSize)
			{
			_range = range;
			}


		/**
		* Override of the drawOnto() method from the Drawable2DObject interface
		**/
		virtual int drawOnto(Image & im, float opacity = 1.0)
			{
			draw(_range, im, opacity);
			return 100;
			}



		/**
		* Override of the removed method from the Plotter2DObj base class
		**/
		virtual void removed(Fl_Group * optionWin)
			{
			return;
			}


		/**
		* Override of the inserted method from the Plotter2DObj base class
		**/
		virtual internals_graphics::Drawable2DInterface * inserted(Fl_Group * & optionWin, int reqWidth)
			{
			return this;
			}


	private:

		p_drawfun  _drawfun;
		fBox2	   _range;

	};




}


/* end of file */



