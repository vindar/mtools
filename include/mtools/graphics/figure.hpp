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


namespace mtools
{

	/** Interface class for figure objects. */
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




	class FigureCircle : public FigureInterface
	{

	public:

		/** circle parameters **/

		fVec2	center;			// circle center
		double	radius;			// circle radius
		double	thickness;		// circle thickness
		RGBc	color;			// circle color
		RGBc	fillcolor;		// circle interior color


								/** Constructor. */
		FigureCircle(fVec2 centercircle, double rad, RGBc col) : center(centercircle), radius(rad), color(col)
		{
			//thickness = 0.1;
			fillcolor = RGBc::c_Blue.getMultOpacity(0.5f);
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
			im.canvas_draw_thick_filled_circle(R, center, radius, thickness, false, color, fillcolor, highQuality);
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
			return "todo";
			// TODO
		}

		/**
		* Serialize the object.
		*/
		virtual void serialize(OBaseArchive & ar) const override
		{
			// TODO
		}

		/**
		* Deserialize this object.
		*/
		virtual void deserialize(IBaseArchive & ar) override
		{
			// TODO
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

	class FigureGroup;




	class FigureCanvas
	{


	};


}


/* end of file */

