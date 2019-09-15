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
#include "internal/polyline.hpp"
#include "svgelement.hpp"

#include <type_traits>

#include "tinyxml2.h"


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
	class Line;
	class PolyLine;

	class ThickHorizontalLine;
	class ThickVerticalLine;
	class ThickLine;
	class ThickPolyLine;


	// POLYGON

	class BoxRegion

	class Triangle
	class Quad
	class Polygon
	class Rectangle

	class ThickTriangle
	class ThickQuad
	class ThickPolygon
	class ThickRectangle


	// ELLIPSE / CIRCLES

	class Circle;
	class ThickCircle;

	class CirclePart;
	class ThickCirclePart;

	class Ellipse;
	class ThickEllipse;

	class EllipsePart;
	class ThickEllipsePart;


	// TEXT

	class Text;


	*************** TODO  *****************
	
    // CURVES

	class QuadBezier;
	class CubicBezier;

	class ThickQuadBezier;
	class ThickCubicBezier;

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

		using BBox = Box<double, 2>;	// bounding box structure

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
		 * (IMPLEMENTATION AT BOTTOM OF FILE)
		 */
		template<typename FIGURECLASS> MTOOLS_FORCEINLINE void operator()(const FIGURECLASS & figure, size_t layer = 0);


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



		/**
		* Serialize the canvas with all its figures into an archive.
		**/
		void serialize(OBaseArchive & ar, const int version = 0) const		
			{
			ar & _nbLayers; ar << "number of layers\n";
			for (size_t i = 0; i < _nbLayers; i++)
				{
				ar << "*** Layer " << i << " ***\n";
				ar & _figLayers[i].size();	ar << "number of items in layer " << i << "\n";
				_figLayers[i].iterate_all([&](typename mtools::TreeFigure<typename Figure::internals_figure::FigureInterface *, N, double>::BoundedObject & bo) -> void
					{
					ar & (bo.object)->name();
					ar & (*(bo.object));
					});
				ar << "\n";
				}
			}



		/**
		* Deserialize a canvas
		* 
		* If the canvas is not empty, it must have at least as many layer as canvas archive and the
		* archive is simply added to the current canvas. 
		* 
		* If the canvas is empty, its number of layer is increased if needed to fit the number of layer
		* in the archive.  
		**/
		void deserialize(IBaseArchive & ar)
			{
			// query the number of layers
			size_t nblayers_ar;
			ar & nblayers_ar;

			if (nblayers_ar > _nbLayers)
				{ // increase the number of layer is needed
				MTOOLS_INSURE(size() == 0); // make sure the canvas is empty. 
				delete[] _figLayers;
				_nbLayers = nblayers_ar;
				_figLayers = new TreeFigure<Figure::internals_figure::FigureInterface*, N>[_nbLayers];
				}

			// add all the elements
			for (size_t layer = 0; layer < nblayers_ar; layer++)
				{
				// number of element in this layer
				size_t nbitem;
				ar & nbitem;
				// add all the elements
				for (size_t j = 0; j < nbitem; j++)
					{
					_insertArchiveItem(ar, layer);
					}
				}
			}


		/**
		 * Save the canvas in a file in SVG format.
		 * 
		 * An archive of the canvas is added at the end of the file so it can be reconstructed
		 * exactly later using loadSVG()
		 *
		 * @param	filename	name of the file. 
		 **/
		void saveSVG(const std::string & filename) const
			{
			tinyxml2::XMLDocument xmlDoc;									// create main document
			xmlDoc.InsertEndChild(xmlDoc.NewDeclaration());					// standard xml declaration

			tinyxml2::XMLComment * com = xmlDoc.NewComment("File generated from a mtools::FigureCanvas object.");
			xmlDoc.InsertEndChild(com);

			tinyxml2::XMLUnknown * doc = xmlDoc.NewUnknown("DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\"");
			xmlDoc.InsertEndChild(doc);

			tinyxml2::XMLElement * svg = xmlDoc.NewElement("svg");			// main svg element
			xmlDoc.InsertEndChild(svg);										// standard xml declaration

			BBox bb; // global bounding box

			// iterate over all layer, in order
			for (size_t i = 0; i < _nbLayers; i++)
				{
				// increase the bounding box to contain all elements in this layer. 
				bb.swallowBox(_figLayers[i].mainBoundingBox()); 

				// Insert a group for this layer
				tinyxml2::XMLElement * layer = xmlDoc.NewElement("g"); 
				svg->InsertEndChild(layer);
				layer->SetAttribute("id", (std::string("Figure Layer ") + mtools::toString(i)).c_str());

				// iterate over all element in this layer
				_figLayers[i].iterate_all([&](typename mtools::TreeFigure<typename Figure::internals_figure::FigureInterface *, N, double>::BoundedObject & bo) -> void
					{
					SVGElement el(xmlDoc, layer);												// create an SVGElement for this figure (destroyed out of scope but the tinyxml2::XMLElement survives.

					auto p = typeid(*(bo.object)).name();
					MTOOLS_INSURE(strlen(p) > 14);							// make sure its contains "class mtools::..."
					el.xml->SetAttribute("id", el.getUID(p+14).c_str());	// id  by its type
					bo.object->svg(&el);									// draw on the SVGElement
					});
				}

			// set <svg> attributes
			svg->SetAttribute("version", "1.1");
			svg->SetAttribute("xmlns", "http://www.w3.org/2000/svg");
			svg->SetAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
			svg->SetAttribute("width", bb.lx());
			svg->SetAttribute("height", bb.ly());
			mtools::ostringstream os; 
			os << bb.min[0] << " " << -(bb.max[1]) << " " << bb.lx() << " " << bb.ly();
			svg->SetAttribute("viewBox", os.toString().c_str());

			// add archive at the end of the svg element
			tinyxml2::XMLElement * mtoolsarchive = xmlDoc.NewElement("data-mtools");
			svg->InsertEndChild(mtoolsarchive);
			OStringArchive ar;
			serialize(ar); 
			mtoolsarchive->SetText(ar.get().c_str());

			// and save everything
			xmlDoc.SaveFile(filename.c_str()); 
			}




		/**
		 * Loads a canvas from an SVG file. 
		 * 
		 * THe SVG file must have been created with the saveSVG() method 
		 * otherwise the loading will fail. 
		 * 
		 * Same procedure as deserialize: the canvas must either be empty
		 * or have at least as many layer as the number of layer in the 
		 * SVG file. 
		 * If canvas is not empty, Figure from the SVG file are simply added
		 * to those already present in the canvas. 
		 *
		 * @param	filename	name of the SVG file
		 **/
		void loadSVG(const std::string & filename)
			{
			tinyxml2::XMLDocument xmlDoc;
			if (xmlDoc.LoadFile(filename.c_str()) != 0)
				{
				MTOOLS_ERROR("FigureCanvas::loadSVG(). Cannot load file [" << filename << "].");
				}

			tinyxml2::XMLElement * svg = xmlDoc.FirstChildElement("svg");
			if (svg == nullptr)
				{
				MTOOLS_ERROR("FigureCanvas::loadSVG(). file [" << filename << "] does not contain a 'svg' element.");
				}

			tinyxml2::XMLElement * elar = svg->FirstChildElement("data-mtools"); 
			if (elar == nullptr)
				{
				MTOOLS_ERROR("FigureCanvas::loadSVG(). file [" << filename << "] does not contain a 'data-mtools' element inside the svg element");
				}

			// load the archive 
			mtools::IStringArchive ar(elar->GetText(), strlen(elar->GetText()));

			// deserialize
			deserialize(ar);
			
			return;
			}




	private: 


		/** insert a figure from an archive */
		MTOOLS_FORCEINLINE void _insertArchiveItem(mtools::IBaseArchive & ar, size_t layer);


		/* used by the macro REGISTER_ARCHIVE_FIGURE_CLASS() */
		template <typename FIGURECLASS> MTOOLS_FORCEINLINE void _registerArchiveFigureClass(const std::string & classname, mtools::IBaseArchive & ar, size_t layer, int & match);


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


		/* create a figure from an archive derectly inside the memory pool */
		template<typename FIGURECLASS> MTOOLS_FORCEINLINE Figure::internals_figure::FigureInterface * _archiveInPool(mtools::IBaseArchive & ar)
			{
			void * p = _allocate(sizeof(FIGURECLASS));					// allocate memory in the memory pool for the figure object
			new (p) FIGURECLASS(ar);									// placement new : copy constructor. 
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


		/* delete all allocated memory TODO: REPLACE A BY BETTER VERSION THAN MALLOC/FREE 
		 * (IMPLEMENTATION AT BOTTOM OF FILE)
		 */
		void _deallocateAll();


		std::vector<void *>	_vecallocp;	// vector containing pointers to all allocated figures (TEMP WHILE USING MALLOC/FREE). 

		/*********************************************************************************************************/


		size_t																_nbLayers;	// number of layers
		TreeFigure<Figure::internals_figure::FigureInterface*, N, double> *	_figLayers;	// tree figure object for each layer. 


	};











	/************************************************************************************************************************************
	*
	* FIGURE CLASSES:
	*
	* All the classes located inside the Figure namespace are derived from FigureInterface hence can be inserted in a canvas object. 
	* 
	* To create a new igure class, use the FIGURECLASS_BEGIN and FIGURECLASS_END MACRO ...
	* 
	*
	*************************************************************************************************************************************/


	namespace Figure
	{

		namespace internals_figure
		{

			/**
			* Interface class for figure objects.
			*
			* Any Figure object must derived from this pure virtual base class.
			* 
			* ANY SUBLCASS MUST ALSO IMPLEMENT A CTOR FOR DESERIALIZATION : `FigureClass(IBaseArchive & ar)`
			* 
			* 
			**/
			class FigureInterface
			{

			public:

				/** Virtual destructor */
				virtual ~FigureInterface() {}


				/**
				 * DO NOT IMPLEMENT THIS METHOD WHICH IS CREATED AUTOMATICALLY 
				 * WHEN USING MACROS 'FIGURECLASS_BEGIN' and 'FIGURECLASS_END'
				 **/
				virtual std::string name() const = 0;
				

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
				* Need not be overriden. 
				* 
				* BUT THE OBJECT MUST IMPLEMENT DESERALIZATION VIA THE CTOR `FigureClass(IBaseArchive & ar)`
				* 
				*/
				virtual void deserialize(IBaseArchive & ar)
					{
					MTOOLS_ERROR(typeid(*this).name() << " does not implement the deserialize() method");
					}
	

				/**
				* Write into the SVGElement el to provide a SVG descritpion of the figure
				* which will be used when exporting in SVG format.
				*
				* el.xml give access to the underlying tinyxml::XMLElement
				* 
				* use macro TX(), TY(), TZ() to transform coordinate to SVG coordinate. 
				*/
				virtual void svg(mtools::SVGElement * el) const
				{
					MTOOLS_DEBUG(std::string("Figure::svg() called but not implemented on ") + typeid(*this).name() + ". Item ignored...");
					el->Comment((std::string("MISSING FIGURE\n") + toString()).c_str());
					return;
				}


			private:

			};

		}


		/*********************************************************************************************************************
		* Abusing C preprocessor: macro used for creating Figure classes
		**********************************************************************************************************************/

		// Convenience macros to transform to SVG coordinates. 
		#define TX(Z)	(el->tx( Z ))
		#define TY(Z)	(el->ty( Z ))
		#define TR(Z)	(el->tr( Z ))


		#define FIGURECLASS_BEGIN( CLASSNAME )	class CLASSNAME : public internals_figure::FigureInterface 				\
													{																	\
													public:																\
														static std::string className() { return #CLASSNAME; }			\
													virtual std::string name() const override { return className(); }		


		#define FIGURECLASS_END()					};











		/************************************************************************************************************************************
		*
		* DOTS
		*
		*************************************************************************************************************************************/






		/*********************************************************
		*
		* Circle Dot
		*
		* The radius of a dot is absolute and does not scale with the range.
		*
		**********************************************************/
		FIGURECLASS_BEGIN(CircleDot)

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
				OSS os;
				os << "CircleDot [" << center << ", " << radius << "  outline " << outlinecolor << " interior " << fillcolor << "]";
				return os.str();
			}


			virtual void serialize(OBaseArchive & ar) const override
			{
				ar & center & radius & outlinecolor & fillcolor;
			}


			CircleDot(IBaseArchive & ar)
			{
				ar & center & radius & outlinecolor & fillcolor;
			}


			virtual void svg(mtools::SVGElement * el) const override
			{
				el->Comment("SVG cannot represent Figure::CircleDot correctly !");
				el->SetName("circle");;
				el->setFillColor(fillcolor);
				el->setStrokeColor(outlinecolor);
				el->xml->SetAttribute("cx", TX(center.X()));
				el->xml->SetAttribute("cy", TY(center.Y()));
				el->xml->SetAttribute("r", radius);	// not using TR() since it should not scale. 
				el->xml->SetAttribute("vector-effect", "non-scaling-stroke");
			}

		FIGURECLASS_END()






	
		/*********************************************************
		*
		* Square Dot
		*
		* The radius of a dot is absolute and does not scale with the range.
		*
		**********************************************************/
		FIGURECLASS_BEGIN(SquareDot)


			fVec2	center;			// center
			int32	pw;				// radius
			RGBc	color;			// color


			/**
			 * Construct a square dot : unit pixel size. 
			 *
			 * @param	centerdot position
			 * @param	col		  color
			 */
			SquareDot(fVec2 centerdot, RGBc col) : center(centerdot), pw(1), color(col)
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
				OSS os; 
				os <<"SquareDot [" << center << ", " << pw << ", " << color << "]";
				return os.str();
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & center & pw & color;
				}


			SquareDot(IBaseArchive & ar) 
				{
				ar & center & pw & color;
				}


			virtual void svg(mtools::SVGElement * el) const override
				{
				el->Comment("SVG cannot represent Figure::SquareDot correctly !");
				el->SetName("rect");;
				el->setFillColor(color);
				el->xml->SetAttribute("x", TX(center.X())-pw);
				el->xml->SetAttribute("y", TY(center.Y())-pw);
				el->xml->SetAttribute("width", 2*pw);  // not using TR() since it should not scale
				el->xml->SetAttribute("height", 2*pw); //
				}


		};








		/************************************************************************************************************************************
		*
		* LINES
		*
		*************************************************************************************************************************************/







		/*********************************************************
		* 
		* Horizontal Line
		* 
		**********************************************************/
		FIGURECLASS_BEGIN(HorizontalLine)

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
				im.canvas_draw_horizontal_line(R, y, x1, x2, color, true, true, min_thickness);
				}


			virtual fBox2 boundingBox() const override
				{
				return getBoundingBox(fVec2{ x1, y }, fVec2{ x2, y });
				}


			virtual std::string toString(bool debug = false) const override
				{
				OSS os; 
				os << "HorizontalLine [" << x1 << " - " << x2 << ", " << y << " " << color << "]";
				return os.str();
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & x1 & x2 & y & color;
				}


			HorizontalLine(IBaseArchive & ar)
				{
				ar & x1 & x2 & y & color;
				}


			virtual void svg(mtools::SVGElement * el) const override
				{
				el->SetName("line");;
				el->setStrokeColor(color);
				el->xml->SetAttribute("x1", TX(x1));
				el->xml->SetAttribute("y1", TY(y));
				el->xml->SetAttribute("x2", TX(x2));
				el->xml->SetAttribute("y2", TY(y));
				el->xml->SetAttribute("vector-effect", "non-scaling-stroke");
				}

		FIGURECLASS_END()








		/*********************************************************
		*
		* Vertical Line figure
		*
		**********************************************************/
		FIGURECLASS_BEGIN(VerticalLine)


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
				im.canvas_draw_vertical_line(R, x, y1, y2, color, true, true, min_thickness);
				}
				

			virtual fBox2 boundingBox() const override
				{
				return getBoundingBox(fVec2{ x, y1 }, fVec2{ x, y2 });
				}


			virtual std::string toString(bool debug = false) const override
				{
				OSS os;
				os << "VerticalLine [" << x << ", " << y1 << " - " << y2 << " " << color << "]";
				return os.str();
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & y1 & y2 & x & color;
				}


			VerticalLine(IBaseArchive & ar) 
				{
				ar & y1 & y2 & x & color;
				}


			virtual void svg(mtools::SVGElement * el) const override
				{
				el->SetName("line");
				el->setStrokeColor(color);
				el->xml->SetAttribute("x1", TX(x));
				el->xml->SetAttribute("y1", TY(y1));
				el->xml->SetAttribute("x2", TX(x));
				el->xml->SetAttribute("y2", TY(y2));
				el->xml->SetAttribute("vector-effect", "non-scaling-stroke");
				}

		FIGURECLASS_END()








		/*********************************************************
		*
		* Line
		*
		**********************************************************/
		FIGURECLASS_BEGIN(Line)


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
				im.canvas_draw_line(R, P1, P2, color, true, highQuality, true, pw, min_thickness);
				}


			virtual fBox2 boundingBox() const override
				{
				return fBox2(std::min<double>(P1.X(), P2.X()), std::max<double>(P1.X(), P2.X()), std::min<double>(P1.Y(), P2.Y()), std::max<double>(P1.Y(), P2.Y()));
				}


			virtual std::string toString(bool debug = false) const override
				{
				OSS os; 
				os <<"Line [" << P1 << ", " << P2 << " (" << pw << ") " << color << "]";
				return os.str();
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & P1 & P2 & color & pw;
				}


			Line(IBaseArchive & ar)
				{
				ar & P1 & P2 & color & pw;
				}


			virtual void svg(mtools::SVGElement * el) const override
			{
				el->SetName("line");
				el->setStrokeColor(color);
				el->xml->SetAttribute("x1", TX(P1.X()));
				el->xml->SetAttribute("y1", TY(P1.Y()));
				el->xml->SetAttribute("x2", TX(P2.X()));
				el->xml->SetAttribute("y2", TY(P2.Y()));
				if (pw != 0)
					{
					el->Comment("SVG cannot accurately represent Figure::Line with non zero penwidth, use ThickLine instead.");
					}
				el->xml->SetAttribute("vector-effect", "non-scaling-stroke");
			}

		FIGURECLASS_END()







		/*********************************************************
		*
		* Polyline
		*
		**********************************************************/
		FIGURECLASS_BEGIN(PolyLine)


			std::vector<fVec2> tab;
			RGBc  color;
			int32 pw;

			fBox2 bb; 


			/**
			* Construct a polyline
			*
			* @param	tab_points	list of points, linked by straight lines
			* @param	col		  	color.
			* @param	penwidth   	(Optional) pen width (0 = unit pen).
			**/
			PolyLine(const std::vector<fVec2> & tab_points, RGBc col, int32 penwidth = 0) : tab(tab_points), color(col), pw(penwidth)
			{
				MTOOLS_INSURE(tab_points.size() > 0);
				bb = getBoundingBox(tab);
			}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
			{
				im.canvas_draw_polyline(R, tab, color, true, highQuality, true, pw, min_thickness);
			}


			virtual fBox2 boundingBox() const override
			{
				return bb;
			}


			virtual std::string toString(bool debug = false) const override
			{
				OSS os; 
				os << "PolyLine [" << tab << " - " << color << "(" << pw << ")" << "]";
				return os.str();
			}


			virtual void serialize(OBaseArchive & ar) const override
			{
				ar & tab & color & pw;
			}


			PolyLine(IBaseArchive & ar)
			{
				ar & tab & color & pw;
				bb = getBoundingBox(tab);
			}


			virtual void svg(mtools::SVGElement * el) const override
				{
				el->SetName("polyline");
				el->setStrokeColor(color);				
				el->noFill();
				mtools::ostringstream os;

				for (auto P : tab) { os << TX(P.X()) << "," << TY(P.Y()) << " "; }
				el->xml->SetAttribute("points", os.toString().c_str());

				if (pw != 0)
					{
					el->Comment("SVG cannot accurately represent Figure::PolyLine with non zero penwidth, use ThickPolyLine instead.");
					}
				el->xml->SetAttribute("vector-effect", "non-scaling-stroke");
				}

		FIGURECLASS_END()







		/*********************************************************
		*
		* Thick Horizontal Line
		*
		**********************************************************/
		FIGURECLASS_BEGIN(ThickHorizontalLine)

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
				if (thickness > 0) { const double ht = thickness / 2;  return fBox2(x1, x2, y - ht, y + ht); }
				else { return fBox2(x1, x2, y, y); }
			}


			virtual std::string toString(bool debug = false) const override
			{
				OSS os; 
				os << "ThickHorizontalLine [" << x1 << " - " << x2 << ", " << y << " " << color;
				if (thickness >= 0) { os << " rel. thick: " << thickness; }  else { os << " abs. thick: " << (-thickness); }
				os << "]";
				return os.str();
			}


			virtual void serialize(OBaseArchive & ar) const override
			{
				ar & x1 & x2 & y & color & thickness;
			}


			ThickHorizontalLine(IBaseArchive & ar) 
			{
				ar & x1 & x2 & y & color & thickness;
			}


			virtual void svg(mtools::SVGElement * el) const
				{
				el->SetName("line");
				el->setStrokeColor(color);
				el->xml->SetAttribute("x1", TX(x1));
				el->xml->SetAttribute("y1", TY(y));
				el->xml->SetAttribute("x2", TX(x2));
				el->xml->SetAttribute("y2", TY(y));
				el->xml->SetAttribute("stroke-linecap", "butt");

				if (thickness <= 0)
					{
					el->Comment("SVG cannot accurately represent Figure::ThickHorizontalLine with absolute thickness!");
					el->xml->SetAttribute("vector-effect", "non-scaling-stroke");
					}
				else
					{
					el->xml->SetAttribute("stroke-width", TR(std::abs(thickness)));
					}
				}

		FIGURECLASS_END()








		/*********************************************************
		*
		* Thick Vertical Line
		*
		**********************************************************/
		FIGURECLASS_BEGIN(ThickVerticalLine)


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
				if (thickness > 0) { const double ht = thickness / 2;  return fBox2(x - ht, x + ht, y1, y2); }
				else { return fBox2(x, x, y1, y2); }
			}


			virtual std::string toString(bool debug = false) const override
			{
				OSS os; 
				os << "ThickVerticalLine [" << x << ", " << y1 << " - " << y2 << " " << color;
				if (thickness > 0) { os << " rel. thick: " << thickness; } else { os << " abs. thick: " << (-thickness); }
				os << "]";
				return os.str();
			}


			virtual void serialize(OBaseArchive & ar) const override
			{
				ar & y1 & y2 & x & color & thickness;
			}


			ThickVerticalLine(IBaseArchive & ar) 
			{
				ar & y1 & y2 & x & color & thickness;
			}


			virtual void svg(mtools::SVGElement * el) const
				{
				el->SetName("line");
				el->setStrokeColor(color);
				el->xml->SetAttribute("x1", TX(x));
				el->xml->SetAttribute("y1", TY(y1));
				el->xml->SetAttribute("x2", TX(x));
				el->xml->SetAttribute("y2", TY(y2));
				el->xml->SetAttribute("stroke-linecap", "butt");

				if (thickness <= 0)
					{
					el->Comment("SVG cannot accurately represent Figure::ThickVerticalLine with absolute thickness!");
					el->xml->SetAttribute("vector-effect", "non-scaling-stroke");
					}
				else
					{
					el->xml->SetAttribute("stroke-width", TR(std::abs(thickness)));
					}
				}

		FIGURECLASS_END()







		/*********************************************************
		*
		* Thick Line
		*
		**********************************************************/
		FIGURECLASS_BEGIN(ThickLine)

			fVec2 P1, P2;
			RGBc  color;
			double thick; // positive for relative thickness and negative for absolute thickness

			fBox2 bb; 


			/**
			 * Constructor.
			 *
			 * @param	p1		  first endpoint
			 * @param	p2		  second endpoint
			 * @param	thickness line thickness (relative).
			 * @param	col		  color
			 */
			ThickLine(fVec2 p1, fVec2 p2, double thickness, bool relativethickness, RGBc col) : P1(p1), P2(p2), color(col), thick(thickness)
				{
				MTOOLS_ASSERT(thick >= 0);
				MTOOLS_ASSERT(P1 != P2);
				if (!relativethickness) { thick = -thick; }
				_computeBB();
				}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				const bool rel = (thick >= 0);
				const double tt = (rel ? thick : -thick);
				im.canvas_draw_thick_line(R, P1, P2, tt, rel, color, highQuality, true, min_thickness);
				}


			virtual fBox2 boundingBox() const override
				{
				return bb;
				}


			virtual std::string toString(bool debug = false) const override
				{
				OSS os; 
				os << "ThickLine [" << P1 << ", " << P2 << " - " << ((thick >= 0) ? "rel. thick. (" : "abs. thick. (") << std::abs(thick) << ") " << color << "]";
				return os.str();
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & P1 & P2 & color & thick;
				}


			ThickLine(IBaseArchive & ar)
				{
				ar & P1 & P2 & color & thick;
				_computeBB();
				}


			virtual void svg(mtools::SVGElement * el) const
				{
				el->SetName("line");
				el->setStrokeColor(color);
				el->xml->SetAttribute("x1", TX(P1.X()));
				el->xml->SetAttribute("y1", TY(P1.Y()));
				el->xml->SetAttribute("x2", TX(P2.X()));
				el->xml->SetAttribute("y2", TY(P2.Y()));
				el->xml->SetAttribute("stroke-linecap", "butt");
				if (thick <= 0)
					{
					el->Comment("SVG cannot accurately represent Figure::ThickLine with absolute thickness!");
					el->xml->SetAttribute("vector-effect", "non-scaling-stroke");
					}
				else
					{
					el->xml->SetAttribute("stroke-width", TR(std::abs(thick)));
					}
				}


			private:

			/* precompute the bounding box */
			void _computeBB()
				{
				if (thick >= 0)
					{
					fBox2 R;
					fVec2 H = (P2 - P1).get_rotate90();
					H.normalize();
					H *= (thick*0.5);
					bb = getBoundingBox(P1 + H, P1 - H, P2 + H, P2 - H);
					return;
					}
				bb = getBoundingBox(P1, P2);
				}


		FIGURECLASS_END()







		/*********************************************************
		*
		* Thick PolyLine
		*
		**********************************************************/
		FIGURECLASS_BEGIN(ThickPolyLine)


			std::vector<fVec2>	tab;
			RGBc				color;
			double				thickness; // >= 0 for relative thickness and < 0 for absolute thickness

			fBox2				bb;

			ThickPolyLine(const std::vector<fVec2> & tab_points, double thick, double relativethickness, RGBc col) : tab(tab_points), color(col), thickness(thick)
				{
				MTOOLS_INSURE(tab_points.size() > 0);
				MTOOLS_INSURE(thick >= 0);
				if (!relativethickness) { thickness = -thickness; }
				_constructbb();
				}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				const bool rel = (thickness >= 0);
				const double tt = (rel ? thickness : -thickness);
				im.canvas_draw_thick_polyline(R, tab, tt, rel, color, highQuality, true, min_thickness);
				}


			virtual fBox2 boundingBox() const override
				{
				return bb;
				}


			virtual std::string toString(bool debug = false) const override
				{
				OSS os; 
				os << "ThickPolyLine [" << tab << " - " << ((thickness >= 0) ? "rel. thick. (" : "abs. thick. (") << std::abs(thickness) << ") " << color << "]";
				return os.str();
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & tab & color & thickness;
				}


			ThickPolyLine(IBaseArchive & ar)
				{
				ar & tab & color & thickness;
				_constructbb();
				}


			virtual void svg(mtools::SVGElement * el) const override
				{
				el->SetName("polyline");
				el->setStrokeColor(color);
				el->noFill();
				el->xml->SetAttribute("stroke-linecap","butt");
				el->xml->SetAttribute("stroke-linejoin", "miter");
				mtools::ostringstream os;

				for (auto P : tab) { os << TX(P.X()) << "," << TY(P.Y()) << " "; }
				el->xml->SetAttribute("points", os.toString().c_str());

				if (thickness <= 0)
					{
					el->Comment("SVG cannot accurately represent Figure::ThickLine with absolute thickness!");
					el->xml->SetAttribute("vector-effect", "non-scaling-stroke");
					}
				else
					{
					el->xml->SetAttribute("stroke-width", TR(std::abs(thickness)));
					}

				}


		private:


			/* reconstruct the bounding box */
			void _constructbb()
				{
				if (thickness >= 0)
					{ // relative thickness
					const double tt = thickness / 2;
					std::vector<fVec2> tab2;
					internals_polyline::polylinetoPolygon(tab, tt,tt, tab2);
					bb = getBoundingBox(tab2);
					}
				else
					{ // absolute thickness
					bb = getBoundingBox(tab);
					}
				}


		FIGURECLASS_END()









		/************************************************************************************************************************************
		*
		* POLYGON
		*
		*************************************************************************************************************************************/







		/*********************************************************
		*
		* BoxRegion : filled a boxed region with a single color.
		*
		**********************************************************/
		FIGURECLASS_BEGIN(BoxRegion)


			fBox2 box;
			RGBc  fillcolor;


			/**
			* construct a filled box with a given color
			*/
			BoxRegion(fBox2 & B, RGBc fillcol) : box(B), fillcolor(fillcol)
			{
				MTOOLS_ASSERT(!B.isEmpty());
			}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
			{
				im.canvas_draw_box(R, box, fillcolor, true, min_thickness);
			}


			virtual fBox2 boundingBox() const override
			{
				return box;
			}


			virtual std::string toString(bool debug = false) const override
				{
				OSS os; 
				os << "BoxedRegion [" << box << " - " << fillcolor << "]";
				return os.str();
				}


			virtual void serialize(OBaseArchive & ar) const override
			{
				ar & box & fillcolor;
			}


			BoxRegion(IBaseArchive & ar) 
			{
				ar & box & fillcolor;
			}


			virtual void svg(mtools::SVGElement * el) const override
			{
				el->SetName("rect");
				el->noStroke();
				el->setFillColor(fillcolor);
				el->xml->SetAttribute("x", TX(box.min[0]));
				el->xml->SetAttribute("y", TY(box.min[1]) + TY(box.ly()));
				el->xml->SetAttribute("width", TR(box.lx()));
				el->xml->SetAttribute("height", TR(box.ly()));
			}

		FIGURECLASS_END()







		/*********************************************************
		*
		* Triangle
		*
		**********************************************************/
		FIGURECLASS_BEGIN(Triangle)


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
					im.canvas_draw_triangle(R, P1, P2, P3, color, highQuality, true, 0, min_thickness);
					}
				else
					{
					im.canvas_draw_filled_triangle(R, P1, P2, P3, color, fillcolor, highQuality, true, min_thickness);
					}
				}


			virtual fBox2 boundingBox() const override
				{
				return getBoundingBox(P1,P2,P3);
				}


			virtual std::string toString(bool debug = false) const override
				{
				OSS os; 
				os << "Triangle [" << P1 << ", " << P2 << ", " << P3 << " - " << color;
				if (!fillcolor.isTransparent()) { os << "filled : " << fillcolor; }
				os << "]";
				return os.str();
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & P1 & P2 & P3 & color & fillcolor;
				}


			Triangle(IBaseArchive & ar)
				{
				ar & P1 & P2 & P3 & color & fillcolor;
				}


			virtual void svg(mtools::SVGElement * el) const override
				{
				el->SetName("polygon");
				el->setStrokeColor(color);
				el->setFillColor(fillcolor);
				mtools::ostringstream os; 
				os << TX(P1.X()) << "," << TY(P1.Y()) << " " << TX(P2.X()) << "," << TY(P2.Y()) << " " << TX(P3.X()) << "," << TY(P3.Y());
				el->xml->SetAttribute("points", os.toString().c_str());
				el->xml->SetAttribute("vector-effect", "non-scaling-stroke");
				}


		FIGURECLASS_END()







		/*********************************************************
		*
		* Quad
		*
		**********************************************************/
		FIGURECLASS_BEGIN(Quad)


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
					im.canvas_draw_quad(R, P1, P2, P3, P4, color, highQuality, true, 0, min_thickness);
					}
				else
					{
					im.canvas_draw_filled_quad(R, P1, P2, P3, P4, color, fillcolor, highQuality, true, min_thickness);
					}
				}


			virtual fBox2 boundingBox() const override
				{
				return getBoundingBox(P1,P2,P3,P4);
				}


			virtual std::string toString(bool debug = false) const override
				{
				OSS os; 
				os << "Quad [" <<  P1 << ", " << P2 << ", " << P3 << ", " << P4 << " - " << color;
				if (!fillcolor.isTransparent()) { os << "filled : " << fillcolor; }
				os << "]";
				return os.str();
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & P1 & P2 & P3 & P4 & color & fillcolor;
				}


			Quad(IBaseArchive & ar)
				{
				ar & P1 & P2 & P3 & P4 & color & fillcolor;
				}


			virtual void svg(mtools::SVGElement * el) const override
				{
				el->SetName("polygon");
				el->setStrokeColor(color);
				el->setFillColor(fillcolor);
				mtools::ostringstream os; 
				os << TX(P1.X()) << "," << TY(P1.Y()) << " " << TX(P2.X()) << "," << TY(P2.Y()) << " " << TX(P3.X()) << "," << TY(P3.Y()) << " " << TX(P4.X()) << "," << TY(P4.Y());
				el->xml->SetAttribute("points", os.toString().c_str());
				el->xml->SetAttribute("vector-effect", "non-scaling-stroke");
				}


		FIGURECLASS_END()








		/*********************************************************
		*
		* Polygon
		*
		**********************************************************/
		FIGURECLASS_BEGIN(Polygon)


			std::vector<fVec2> tab;
			RGBc  color, fillcolor;

			fBox2 bb; 

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
				bb = getBoundingBox(tab);
				}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				if (fillcolor.isTransparent())
					{
					im.canvas_draw_polygon(R, tab, color, highQuality, true, 0, min_thickness);
					}
				else
					{
					im.canvas_draw_filled_polygon(R, tab, color, fillcolor, highQuality, true, false, min_thickness);
					}
				}


			virtual fBox2 boundingBox() const override
				{
				return bb;
				}


			virtual std::string toString(bool debug = false) const override
				{
				OSS os; 
				os << "Polygon [" << tab << " - " << color;
				if (!fillcolor.isTransparent()) { os << "filled : " << fillcolor; }
				os << "]";
				return os.str();
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & tab & color & fillcolor;
				}


			Polygon(IBaseArchive & ar) 
				{
				ar & tab & color & fillcolor;
				bb = getBoundingBox(tab);
				}


			virtual void svg(mtools::SVGElement * el) const override
				{
				el->SetName("polygon");
				el->setStrokeColor(color);
				el->setFillColor(fillcolor);
				mtools::ostringstream os; 
				for (auto P : tab)
					{
					os << TX(P.X()) << "," << TY(P.Y()) << " ";
					}
				el->xml->SetAttribute("points", os.toString().c_str());
				el->xml->SetAttribute("vector-effect", "non-scaling-stroke");
				}

		FIGURECLASS_END()









		/*********************************************************
		*
		* Rectangle
		*
		**********************************************************/
		FIGURECLASS_BEGIN(Rectangle)


			fBox2 box;
			RGBc  color, fillcolor;


			/**
			 * Construct a rectangle.
			 *
			 * @param	B	   	(closed) box representing the rectangle
			 * @param	col	   	outline color.
			 * @param	fillcol	(Optional) fill color (default = transparent = no fill)
 			 **/
			Rectangle(const fBox2 & B, RGBc col, RGBc fillcol = RGBc::c_Transparent) : box(B), color(col), fillcolor(fillcol)
				{
				MTOOLS_ASSERT(!B.isEmpty());
				}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				if (fillcolor.isTransparent())
					{
					im.canvas_draw_rectangle(R, box, color, true, min_thickness);
					}
				else
					{
					im.canvas_draw_filled_rectangle(R, box, color, fillcolor, true, min_thickness);
					}
				}


			virtual fBox2 boundingBox() const override
				{
				return box;
				}


			virtual std::string toString(bool debug = false) const override
				{
				OSS os; 
				os << "Rectangle [" << box << " - " << color; 
				if (!fillcolor.isTransparent()) { os << " filled " << fillcolor; }
				os << "]";
				return os.str();
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & box & color & fillcolor;
				}


			Rectangle(IBaseArchive & ar) 
				{
				ar & box & color & fillcolor;
				}


			virtual void svg(mtools::SVGElement * el) const override
				{
				el->SetName("rect");
				el->setStrokeColor(color);
				el->setFillColor(fillcolor);
				el->xml->SetAttribute("x", TX(box.min[0]));
				el->xml->SetAttribute("y", TY(box.min[1]) + TY(box.ly()));
				el->xml->SetAttribute("width", TR(box.lx()));
				el->xml->SetAttribute("height", TR(box.ly()));
				el->xml->SetAttribute("vector-effect", "non-scaling-stroke");
				}

		FIGURECLASS_END()







		/*********************************************************
		*
		* ThickTriangle
		*
		**********************************************************/
		FIGURECLASS_BEGIN(ThickTriangle)


			fVec2 P1, P2, P3;
			double thickness;	// negative for absolute thickness
			RGBc  color, fillcolor;


			/**
			 * Constructor
			 *
			 * @param	p1				   first point
			 * @param	p2				   second point
			 * @param	p3				   third point
			 * @param	thick			   thickness (going inward)
			 * @param	relative_thickness True to use relative thickness.
			 * @param	col				   outline color.
			 * @param	fillcol			   (Optional) interior color.
			 */
			ThickTriangle(fVec2 p1, fVec2 p2, fVec2 p3, double thick,  bool relative_thickness, RGBc col, RGBc fillcol = RGBc::c_Transparent) : P1(p1), P2(p2), P3(p3), thickness(thick), color(col), fillcolor(fillcol)
				{
				MTOOLS_ASSERT(thick >= 0);
				if (!relative_thickness) { thickness = -thickness; }
				}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				const bool rel = (thickness >= 0);
				const double tt = (rel ? thickness : -thickness);
				if (fillcolor.isTransparent())
					{
					im.canvas_draw_thick_triangle(R, P1, P2, P3, tt, rel, color, highQuality, true, min_thickness);
					}
				else
					{
					im.canvas_draw_thick_filled_triangle(R, P1, P2, P3, tt, rel, color, fillcolor, highQuality, true, min_thickness);
					}
				}


			virtual fBox2 boundingBox() const override
				{
				return getBoundingBox(P1, P2, P3);
				}


			virtual std::string toString(bool debug = false) const override
				{
				OSS os; 
				os << "ThickTriangle [" << P1 << ", " << P2 << ", " << P3 << " - " << ((thickness >= 0) ? " rel. thick. (" : "abs. thick. (") << std::abs(thickness) << ") " << color;
				if (!fillcolor.isTransparent()) { os << " filled " << fillcolor; }
				os << "]";
				return os.str();
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar &  P1 & P2 & P3 & thickness & color & fillcolor;
				}


			ThickTriangle(IBaseArchive & ar) 
				{
				ar &  P1 & P2 & P3 & thickness & color & fillcolor;
				}


			virtual void svg(mtools::SVGElement * el) const override
				{
				mtools::ostringstream os;
				os << TX(P1.X()) << "," << TY(P1.Y()) << " " << TX(P2.X()) << "," << TY(P2.Y()) << " " << TX(P3.X()) << "," << TY(P3.Y());

				if (thickness <= 0)
					{
					el->Comment("SVG cannot accurately represent Figure::ThickTriangle with absolute thickness!");
					el->SetName("polygon");
					el->setFillColor(fillcolor);
					el->setStrokeColor(color);
					el->xml->SetAttribute("points", os.toString().c_str());
					el->xml->SetAttribute("vector-effect", "non-scaling-stroke");
					return;
					}

				el->SetName("g");
				const std::string uid = el->getUID("clippath");

				auto clip_el = el->NewChildSVGElement("clipPath"); 
				clip_el->xml->SetAttribute("id", uid.c_str());

				auto path_el = clip_el->NewChildSVGElement("polygon");
				path_el->noFill();
				path_el->noStroke();
				path_el->xml->SetAttribute("points", os.toString().c_str());
				
				auto fig_el = el->NewChildSVGElement("polygon");
				fig_el->xml->SetAttribute("clip-path", (mtools::toString("url(#") + uid + ")").c_str());
				fig_el->setFillColor(fillcolor);
				fig_el->setStrokeColor(color);
				fig_el->xml->SetAttribute("points", os.toString().c_str());
				fig_el->xml->SetAttribute("stroke-width", TR(2*thickness));
				}
	

		FIGURECLASS_END()






		/*********************************************************
		*
		* ThickQuad
		*
		**********************************************************/
		FIGURECLASS_BEGIN(ThickQuad)


			fVec2 P1, P2, P3, P4;
			double thickness;	// negative for absolute thickness
			RGBc  color, fillcolor;


			/**
			* Constructor
			*
			* @param	p1				   first point
			* @param	p2				   second point
			* @param	p3				   third point
			* @param	p4				   fourth point
			* @param	thick			   thickness (going inward)
			* @param	relative_thickness True to use relative thickness.
			* @param	col				   outline color.
			* @param	fillcol			   (Optional) interior color.
			*/
			ThickQuad(fVec2 p1, fVec2 p2, fVec2 p3, fVec2 p4, double thick, bool relative_thickness, RGBc col, RGBc fillcol = RGBc::c_Transparent) : P1(p1), P2(p2), P3(p3), P4(p4), thickness(thick), color(col), fillcolor(fillcol)
				{
				MTOOLS_ASSERT(thick >= 0);
				if (!relative_thickness) { thickness = -thickness; }
				}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				const bool rel = (thickness >= 0);
				const double tt = (rel ? thickness : -thickness);
				if (fillcolor.isTransparent())
					{
					im.canvas_draw_thick_quad(R, P1, P2, P3, P4, tt, rel, color, highQuality, true, min_thickness);
					}
				else
					{
					im.canvas_draw_thick_filled_quad(R, P1, P2, P3, P4, tt, rel, color, fillcolor, highQuality, true, min_thickness);
					}
				}


			virtual fBox2 boundingBox() const override
				{
				return getBoundingBox(P1, P2, P3, P4);
				}


			virtual std::string toString(bool debug = false) const override
				{
				OSS os; 
				os << "ThickQuad [" << P1 << ", " << P2 << ", " << P3 << ", " << P4 << " - " << ((thickness >= 0) ? " rel. thick. (" : "abs. thick. (") << std::abs(thickness) << ") " << color;
				if (!fillcolor.isTransparent()) { os << " filled " << fillcolor; }
				os << "]";
				return os.str();
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar &  P1 & P2 & P3 & P4 & thickness & color & fillcolor;
				}


			ThickQuad(IBaseArchive & ar) 
				{
				ar &  P1 & P2 & P3 & P4 & thickness & color & fillcolor;
				}


			virtual void svg(mtools::SVGElement * el) const override
				{
				mtools::ostringstream os;
				os << TX(P1.X()) << "," << TY(P1.Y()) << " " << TX(P2.X()) << "," << TY(P2.Y()) << " " << TX(P3.X()) << "," << TY(P3.Y()) << " " << TX(P4.X()) << "," << TY(P4.Y());

				if (thickness <= 0)
					{
					el->Comment("SVG cannot accurately represent Figure::ThickQuad with absolute thickness!");
					el->SetName("polygon");
					el->setFillColor(fillcolor);
					el->setStrokeColor(color);
					el->xml->SetAttribute("points", os.toString().c_str());
					el->xml->SetAttribute("vector-effect", "non-scaling-stroke");
					return;
					}

				el->SetName("g");
				const std::string uid = el->getUID("clippath");

				auto clip_el = el->NewChildSVGElement("clipPath"); 
				clip_el->xml->SetAttribute("id", uid.c_str());

				auto path_el = clip_el->NewChildSVGElement("polygon");
				path_el->noFill();
				path_el->noStroke();
				path_el->xml->SetAttribute("points", os.toString().c_str());
				
				auto fig_el = el->NewChildSVGElement("polygon");
				fig_el->xml->SetAttribute("clip-path", (mtools::toString("url(#") + uid + ")").c_str());
				fig_el->setFillColor(fillcolor);
				fig_el->setStrokeColor(color);
				fig_el->xml->SetAttribute("points", os.toString().c_str());
				fig_el->xml->SetAttribute("stroke-width", TR(2*thickness));
				}

		FIGURECLASS_END()







		/*********************************************************
		*
		* ThickPolygon
		*
		**********************************************************/
		FIGURECLASS_BEGIN(ThickPolygon)


			std::vector<fVec2> tab;
			double thickness;	// negative for absolute thickness
			RGBc  color, fillcolor;

			fBox2 bb;


			/**
			 * Constructor
			 *
			 * @param	tab_points		   list of points, in clockwise or anti-clockwise order.
			 * @param	thick			   Thickness (going inward).
			 * @param	relative_thickness True to use relative thickness.
			 * @param	col				   outline color.
			 * @param	fillcol			   (Optional) fill color (default = transparent = no fill)
			 */
			ThickPolygon(const std::vector<fVec2> & tab_points, double thick, bool relative_thickness, RGBc col, RGBc fillcol = RGBc::c_Transparent) : tab(tab_points), thickness(thick), color(col), fillcolor(fillcol)
				{
				MTOOLS_INSURE(tab_points.size() > 0);
				MTOOLS_ASSERT(thickness >= 0); 
				if (!relative_thickness) { thickness = -thickness; }
				bb = getBoundingBox(tab);
				}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				const bool rel = (thickness >= 0);
				const double tt = (rel ? thickness : -thickness);
				if (fillcolor.isTransparent())
					{
					im.canvas_draw_thick_polygon(R, tab,  tt, rel, color, highQuality, true, min_thickness);
					}
				else
					{
					im.canvas_draw_thick_filled_polygon(R, tab, tt, rel, color, fillcolor, highQuality, true, min_thickness);
					}
				}


			virtual fBox2 boundingBox() const override
				{
				return bb;
				}


			virtual std::string toString(bool debug = false) const override
				{
				OSS os; 
				os << "ThickPolygon [" << tab << " - " << ((thickness >= 0) ? " rel. thick. (" : "abs. thick. (") << std::abs(thickness) << ") " << color;
				if (!fillcolor.isTransparent()) { os << " filled " << fillcolor; }
				os << "]";
				return os.str();
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & tab & thickness & color & fillcolor;
				}


			ThickPolygon(IBaseArchive & ar) 
				{
				ar & tab & thickness & color & fillcolor;
				bb = getBoundingBox(tab);
				}


			virtual void svg(mtools::SVGElement * el) const override
				{
				mtools::ostringstream os;
				for (auto P : tab)
					{
					os << TX(P.X()) << "," << TY(P.Y()) << " ";
					}

				if (thickness <= 0)
					{
					el->Comment("SVG cannot accurately represent Figure::ThickPolygon with absolute thickness!");
					el->SetName("polygon");
					el->setFillColor(fillcolor);
					el->setStrokeColor(color);
					el->xml->SetAttribute("points", os.toString().c_str());
					el->xml->SetAttribute("vector-effect", "non-scaling-stroke");
					return;
					}

				el->SetName("g");
				const std::string uid = el->getUID("clippath");

				auto clip_el = el->NewChildSVGElement("clipPath"); 
				clip_el->xml->SetAttribute("id", uid.c_str());

				auto path_el = clip_el->NewChildSVGElement("polygon");
				path_el->noFill();
				path_el->noStroke();
				path_el->xml->SetAttribute("points", os.toString().c_str());
				
				auto fig_el = el->NewChildSVGElement("polygon");
				fig_el->xml->SetAttribute("clip-path", (mtools::toString("url(#") + uid + ")").c_str());
				fig_el->setFillColor(fillcolor);
				fig_el->setStrokeColor(color);
				fig_el->xml->SetAttribute("points", os.toString().c_str());
				fig_el->xml->SetAttribute("stroke-width", TR(2*thickness));
				}


		FIGURECLASS_END()







		/*********************************************************
		*
		* ThickRectangle
		*
		**********************************************************/
		FIGURECLASS_BEGIN(ThickRectangle)


			fBox2 box;
			double thickness_x;	// negative for absolute thickness
			double thickness_y;	//
			RGBc  color, fillcolor;

			/**
			 * Constructor
			 *
			 * @param	B				   (closed) box representing the rectangle
			 * @param	thick_x			   horizontal thickness (going inward)
			 * @param	thick_y			   vertical thickness (going inward)
			 * @param	relative_thickness True to use relative thickness.
			 * @param	col				   outline color
			 * @param	fillcol			   (Optional) interior color
			 */
			ThickRectangle(const fBox2 & B, double thick_x, double thick_y, bool relative_thickness, RGBc col, RGBc fillcol = RGBc::c_Transparent) : box(B), thickness_x(thick_x) , thickness_y(thick_y), color(col), fillcolor(fillcol)
				{
				MTOOLS_ASSERT(!B.isEmpty());
				MTOOLS_ASSERT(thick_x >= 0);
				MTOOLS_ASSERT(thick_y >= 0);
				if (!relative_thickness)
					{
					thickness_x = -thickness_x;
					thickness_y = -thickness_y;
					}
				}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				const bool rel = (thickness_x >= 0);
				const double tx = (rel ? thickness_x : -thickness_x); 
				const double ty = (rel ? thickness_y : -thickness_y);
				if (fillcolor.isTransparent())
					{
					im.canvas_draw_thick_rectangle(R, box, color, tx,ty, rel, true, min_thickness);
					}
				else
					{
					im.canvas_draw_thick_filled_rectangle(R, box, color, fillcolor, tx, ty, rel, true, min_thickness);
					}
				}


			virtual fBox2 boundingBox() const override
				{
				return box;
				}


			virtual std::string toString(bool debug = false) const override
				{
				OSS os; 
				os << "ThickRectangle [" << box <<  ((thickness_x >= 0) ? "rel. thick. (" : "abs. thick. (")  << std::abs(thickness_x) << "," << std::abs(thickness_y) << ") " << color;
				if (!fillcolor.isTransparent()) {os << " filled " << fillcolor; }
				os << "]";
				return os.str();
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar &  box & thickness_x & thickness_y & color & fillcolor;
				}


			ThickRectangle(IBaseArchive & ar)
				{
				ar &  box & thickness_x & thickness_y & color & fillcolor;
				}


			virtual void svg(mtools::SVGElement * el) const override
				{
				if ((thickness_x <= 0)||(thickness_y <= 0))
					{
					el->Comment("SVG cannot accurately represent Figure::ThickRectangle with absolute thickness!");
					el->SetName("rect");
					el->setStrokeColor(color);
					el->setFillColor(fillcolor);
					el->xml->SetAttribute("x", TX(box.min[0]));
					el->xml->SetAttribute("y", TY(box.min[1]) + TY(box.ly()));
					el->xml->SetAttribute("width", TR(box.lx()));
					el->xml->SetAttribute("height", TR(box.ly()));
					el->xml->SetAttribute("vector-effect", "non-scaling-stroke");
					return;
					}

				double thick = thickness_x;
				if (thickness_x != thickness_y)
					{
					thick = (thickness_x + thickness_y) / 2;
					el->Comment((mtools::toString("SVG cannot accurately represent Figure::ThickRectangle with different x/y thickness. Using middle thickness value: ") + mtools::doubleToStringNice(thick)).c_str() );
					}

				double x = box.min[0] + thick/2;
				double y = box.min[1] + thick/2;
				double lx = box.lx() - thick;
				double ly = box.ly() - thick;

				auto el2 = el->NewChildSVGElement("rect");
				el2->setStrokeColor(color);
				el2->noFill();
				el2->xml->SetAttribute("x", TX(x));
				el2->xml->SetAttribute("y", TY(y) + TY(ly));
				el2->xml->SetAttribute("width", TR(lx));
				el2->xml->SetAttribute("height", TR(ly));
				el2->xml->SetAttribute("stroke-width", thick);

				x = box.min[0] + thick;
				y = box.min[1] + thick;
				lx = box.lx() - 2*thick;
				ly = box.ly() - 2*thick;

				auto el3 = el->NewChildSVGElement("rect");
				el3->noStroke();
				el3->setFillColor(fillcolor);
				el3->xml->SetAttribute("x", TX(x));
				el3->xml->SetAttribute("y", TY(y) + TY(ly));
				el3->xml->SetAttribute("width", TR(lx));
				el3->xml->SetAttribute("height", TR(ly));
				}

		FIGURECLASS_END()










		/************************************************************************************************************************************
		*
		* CIRCLE / ELLIPSE
		*
		*************************************************************************************************************************************/









		/*********************************************************
		*
		* Circle
		*
		**********************************************************/
			FIGURECLASS_BEGIN(Circle)


			fVec2	center;
			double	radius;
			RGBc	color;
			RGBc	fillcolor;		// (transparent = no filling)


			/**
			 * Construct a circle without filling.
			 *
			 * @param	centercircle circle center.
			 * @param	rad			 circle radius.
			 * @param	col			 circle (outline) color
			 */
			Circle(fVec2 centercircle, double rad, RGBc col) : center(centercircle), radius(rad), color(col), fillcolor(RGBc::c_Transparent)
				{
				MTOOLS_ASSERT(rad >= 0);
				}


			/**
			 * Construct a filled circle.
			 *
			 * @param	centercircle circle center.
			 * @param	rad			 circle radius.
			 * @param	col			 circle (outline) color
			 * @param	fillcol		 circle fill color
			 */
			Circle(fVec2 centercircle, double rad, RGBc col, RGBc fillcol) : center(centercircle), radius(rad), color(col), fillcolor(fillcol)
				{
				MTOOLS_ASSERT(rad >= 0);
				}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				if (fillcolor.isTransparent()) { im.canvas_draw_circle(R, center, radius, color, highQuality); }
				else { im.canvas_draw_filled_circle(R, center, radius, color, fillcolor, highQuality); }
				}


			virtual fBox2 boundingBox() const override { return fBox2(center.X() - radius, center.X() + radius, center.Y() - radius, center.Y() + radius); }


			virtual std::string toString(bool debug = false) const override
				{
				OSS os; 
				os << "Circle [" << center << " " << radius << " " << color;
				if (fillcolor.comp.A != 0) { os << " filled: " << fillcolor; }
				os << "]";
				return os.str();
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & center & radius & color & fillcolor;
				}


			Circle(IBaseArchive & ar)
				{
				ar & center & radius & color & fillcolor;
				}


			virtual void svg(mtools::SVGElement * el) const override
				{
				el->SetName("circle");
				el->setStrokeColor(color);
				el->setFillColor(fillcolor);
				el->xml->SetAttribute("cx", TX(center.X()));
				el->xml->SetAttribute("cy", TY(center.Y()));
				el->xml->SetAttribute("r", TR(radius));
				el->xml->SetAttribute("vector-effect", "non-scaling-stroke");
				return;
				}

		FIGURECLASS_END()








		/*********************************************************
		*
		* Thick Circle
		*
		**********************************************************/
		FIGURECLASS_BEGIN(ThickCircle)


			fVec2	center;
			double	radius;
			double	thickness;		// circle thickness 0 = no thickness. < 0 = absolute thicknes,  >0 = relative thickness 
			RGBc	color;
			RGBc	fillcolor;		// (transparent = no filling)



			/**
			 * Construct a thick circle (without filling).
			 *
			 * @param	centercircle		circle center.
			 * @param	rad					circle radius.
			 * @param	thick				thickness of the outline (going inside the circle).  
			 * @param	relativethickness	true to use relative thickness.
			 * @param	col					circle (outline) color
			 */
			ThickCircle(fVec2 centercircle, double rad, double thick, bool relativethickness, RGBc col)
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
			ThickCircle(fVec2 centercircle, double rad, double thick, bool relativethickness, RGBc col, RGBc fillcol)
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
				OSS os; 
				os << "ThickCircle [" << center << " " << radius << " " << color;
				if (fillcolor.comp.A != 0) { os << " filled: " << fillcolor; }
				if (thickness != 0.0)
					{
					if (thickness > 0) { os << " rel. thick: " << thickness; } else { os << " abs. thick: " << (-thickness); }
					}
				os << "]";
				return os.str();
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & center & radius & thickness & color & fillcolor;
				}


			ThickCircle(IBaseArchive & ar)
				{
				ar & center & radius & thickness & color & fillcolor;
				}


			virtual void svg(mtools::SVGElement * el) const override
				{
				if (thickness <= 0)
					{
					el->Comment("SVG cannot accurately represent Figure::ThickCircle with absolute thickness!");
					el->SetName("circle");
					el->setStrokeColor(color);
					el->setFillColor(fillcolor);
					el->xml->SetAttribute("cx", TX(center.X()));
					el->xml->SetAttribute("cy", TY(center.Y()));
					el->xml->SetAttribute("r", TR(radius));
					el->xml->SetAttribute("vector-effect", "non-scaling-stroke");
					return;
					}

				
				SVGElement * el2 = el->NewChildSVGElement("circle");
				el2->noStroke();
				el2->setFillColor(fillcolor);
				el2->xml->SetAttribute("cx", TX(center.X()));
				el2->xml->SetAttribute("cy", TY(center.Y()));
				el2->xml->SetAttribute("r", TR(radius - thickness));
				
				SVGElement * el3 = el->NewChildSVGElement("circle");
				el3->setStrokeColor(color);
				el3->noFill();
				el3->xml->SetAttribute("cx", TX(center.X()));
				el3->xml->SetAttribute("cy", TY(center.Y()));
				el3->xml->SetAttribute("r", TR(radius - thickness/2));
				el3->xml->SetAttribute("stroke-width", thickness);
				return;
				}

		FIGURECLASS_END()







		/*********************************************************
		*
		* Circle Part
		*
		**********************************************************/
		FIGURECLASS_BEGIN(CirclePart)


			fVec2	center;
			double	radius;
			RGBc	color;
			RGBc	fillcolor;		// circle interior color (transparent = no filling)
			int		part;			// one of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT,, BOX_SPLIT_DOWN_LEFT, , BOX_SPLIT_DOWN_RIGHT


			/**
			 * Construct part of a circle (no filling).
			 *
			 * @param	circlepart   part to draw, one of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT,
			 * 						 BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT,,
			 * 						 BOX_SPLIT_DOWN_LEFT, , BOX_SPLIT_DOWN_RIGHT.
			 * @param	centercircle center.
			 * @param	rad			 radius.
			 * @param	col			 outline color.
			 */
			CirclePart(int circlepart, fVec2 centercircle, double rad, RGBc col) : center(centercircle), radius(rad), color(col), fillcolor(RGBc::c_Transparent), part(circlepart)
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
			CirclePart(int circlepart, fVec2 centercircle, double rad, RGBc col, RGBc fillcol) : center(centercircle), radius(rad), color(col), fillcolor(fillcol), part(circlepart)
				{
				MTOOLS_ASSERT((circlepart >= 0) && (circlepart < 8));
				MTOOLS_ASSERT(rad >= 0);
				}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				if (fillcolor.isTransparent()) { im.canvas_draw_part_circle(R, part, center, radius, color, highQuality); }
				else { im.canvas_draw_part_filled_circle(R, part, center, radius, color, fillcolor, highQuality); }
				}


			virtual fBox2 boundingBox() const override
				{
				return fBox2(center.X() - radius, center.X() + radius, center.Y() - radius, center.Y() + radius).get_split(part);
				}


			virtual std::string toString(bool debug = false) const override
				{
				OSS os; 
				os << "CirclePart [";
				switch (part)
					{
					case BOX_SPLIT_UP: { os << "HALF UP"; break; }
					case BOX_SPLIT_DOWN: { os << "HALF DOWN"; break; }
					case BOX_SPLIT_LEFT: { os << "HALF LEFT"; break; }
					case BOX_SPLIT_RIGHT: { os << "HALF RIGHT"; break; }
					case BOX_SPLIT_UP_LEFT: { os << "QUARTER UP LEFT"; break; }
					case BOX_SPLIT_UP_RIGHT: { os << "QUARTER UP RIGHT"; break; }
					case BOX_SPLIT_DOWN_LEFT: { os << "QUARTER DOWN LEFT"; break; }
					case BOX_SPLIT_DOWN_RIGHT: { os << "QUARTER DOWN RIGHT"; break; }
					default: { os << "ERROR PART"; }
					}
				os << " " << center << " " << radius << " " << color;
				if (fillcolor.comp.A != 0) { os << " filled: " << fillcolor; }
				os << "]";
				return os.str();
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & part & center & radius & color & fillcolor;
				}


			CirclePart(IBaseArchive & ar)
				{
				ar & part & center & radius & color & fillcolor;
				}


			virtual void svg(mtools::SVGElement * el) const override
				{
				fBox2 B = _clippoly(); 

				el->SetName("g");
				const std::string uid = el->getUID("clippath");

				auto clip_el = el->NewChildSVGElement("clipPath"); 
				clip_el->xml->SetAttribute("id", uid.c_str());

				auto path_el = clip_el->NewChildSVGElement("rect");
				path_el->noFill();
				path_el->noStroke();
				path_el->xml->SetAttribute("x", TX(B.min[0]));
				path_el->xml->SetAttribute("y", TY(B.min[1]) + TY(B.ly()));
				path_el->xml->SetAttribute("width", TR(B.lx()));
				path_el->xml->SetAttribute("height", TR(B.ly()));

				auto fig_el = el->NewChildSVGElement("circle");
				fig_el->setStrokeColor(color);
				fig_el->setFillColor(fillcolor);
				fig_el->xml->SetAttribute("clip-path", (mtools::toString("url(#") + uid + ")").c_str());
				fig_el->xml->SetAttribute("cx", TX(center.X()));
				fig_el->xml->SetAttribute("cy", TY(center.Y()));
				fig_el->xml->SetAttribute("r", TR(radius));
				fig_el->xml->SetAttribute("vector-effect", "non-scaling-stroke");
				return;	
				}


			private:

				/* return the clipping box */
				fBox2 _clippoly() const
				{
					fVec2 P2 = { center.X()      ,  center.Y() + radius };
					fVec2 P3 = { center.X() + radius ,  center.Y() + radius };
					fVec2 P4 = { center.X() - radius ,  center.Y() };
					fVec2 P5 = { center.X()      ,  center.Y() };
					fVec2 P6 = { center.X() + radius ,  center.Y() };
					fVec2 P7 = { center.X() - radius ,  center.Y() - radius };
					fVec2 P8 = { center.X()      ,  center.Y() - radius };
					switch (part)
					{
					case BOX_SPLIT_UP: { return fBox2(P4, P3, true); }
					case BOX_SPLIT_DOWN: { return fBox2(P7, P6, true); }
					case BOX_SPLIT_LEFT: { return fBox2(P7, P2, true); }
					case BOX_SPLIT_RIGHT: { return fBox2(P8, P3, true); }
					case BOX_SPLIT_UP_LEFT: { return fBox2(P4, P2, true); }
					case BOX_SPLIT_UP_RIGHT: { return fBox2(P5, P3, true); }
					case BOX_SPLIT_DOWN_LEFT: { return fBox2(P7, P5, true); }
					case BOX_SPLIT_DOWN_RIGHT: { return fBox2(P8, P6, true); }
					}
					MTOOLS_ERROR("Should not be here, unknown part");
					return fBox2();
				}

		FIGURECLASS_END()





		

		/*********************************************************
		*
		* Thick Circle Part
		*
		**********************************************************/
		FIGURECLASS_BEGIN(ThickCirclePart)


			fVec2	center;
			double	radius;
			double	thickness;		// circle thickness 0 = no thickness. < 0 = absolute thicknes,  >0 = relative thickness 
			RGBc	color;
			RGBc	fillcolor;		// circle interior color (transparent = no filling)
			int		part;			// one of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT,, BOX_SPLIT_DOWN_LEFT, , BOX_SPLIT_DOWN_RIGHT



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
			ThickCirclePart(int circlepart, fVec2 centercircle, double rad, double thick, bool relativethickness, RGBc col)
				:  center(centercircle), radius(rad), thickness(relativethickness ? thick : -thick), color(col), fillcolor(RGBc::c_Transparent), part(circlepart)
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
			ThickCirclePart(int circlepart, fVec2 centercircle, double rad, double thick, bool relativethickness, RGBc col, RGBc fillcol)
				: center(centercircle), radius(rad), thickness(relativethickness ? thick : -thick), color(col), fillcolor(fillcol), part(circlepart)
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
				OSS os;
				os << "ThickCirclePart ["; 
				switch (part)
					{
					case BOX_SPLIT_UP: { os << "HALF UP"; break; }
					case BOX_SPLIT_DOWN: { os << "HALF DOWN"; break; }
					case BOX_SPLIT_LEFT: { os << "HALF LEFT"; break; }
					case BOX_SPLIT_RIGHT: { os << "HALF RIGHT"; break; }
					case BOX_SPLIT_UP_LEFT: { os << "QUARTER UP LEFT"; break; }
					case BOX_SPLIT_UP_RIGHT: { os << "QUARTER UP RIGHT"; break; }
					case BOX_SPLIT_DOWN_LEFT: { os << "QUARTER DOWN LEFT"; break; }
					case BOX_SPLIT_DOWN_RIGHT: { os << "QUARTER DOWN RIGHT"; break; }
					default: { os << "ERROR PART"; }
					}
				os << " " << center << " " << radius << " " << color;
				if (fillcolor.comp.A != 0) { os << " filled: " << fillcolor; }
				if (thickness != 0.0)
					{
					if (thickness > 0) { os << " rel. thick: " << thickness; } else { os << " abs. thick: " << (-thickness); }
					}
				os << "]";
				return os.str();
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & part & center & radius & thickness & color & fillcolor;
				}


			ThickCirclePart(IBaseArchive & ar)
				{
				ar & part & center & radius & thickness & color & fillcolor;
				}


			virtual void svg(mtools::SVGElement * el) const override
				{
				fBox2 B = _clippoly(); 

				el->SetName("g");
				const std::string uid = el->getUID("clippath");

				auto clip_el = el->NewChildSVGElement("clipPath"); 
				clip_el->xml->SetAttribute("id", uid.c_str());

				auto path_el = clip_el->NewChildSVGElement("rect");
				path_el->noFill();
				path_el->noStroke();
				path_el->xml->SetAttribute("x", TX(B.min[0]));
				path_el->xml->SetAttribute("y", TY(B.min[1]) + TY(B.ly()));
				path_el->xml->SetAttribute("width", TR(B.lx()));
				path_el->xml->SetAttribute("height", TR(B.ly()));

				auto fig_el = el->NewChildSVGElement("circle");
				fig_el->setStrokeColor(color);
				fig_el->setFillColor(fillcolor);
				fig_el->xml->SetAttribute("clip-path", (mtools::toString("url(#") + uid + ")").c_str());
				fig_el->xml->SetAttribute("cx", TX(center.X()));
				fig_el->xml->SetAttribute("cy", TY(center.Y()));

				if (thickness <= 0)
					{
					fig_el->Comment("SVG cannot accurately represent Figure::ThickCirclePart with absolute thickness!");
					fig_el->xml->SetAttribute("r", TR(radius));
					fig_el->xml->SetAttribute("vector-effect", "non-scaling-stroke");
					return;
					}

				// TODO : improve the svg code below using a <mask> instead so that the
				// stroke is not drawn over the interior  which cause defects when using transparency 
				fig_el->xml->SetAttribute("r", TR(radius - thickness/2));
				fig_el->xml->SetAttribute("stroke-width", thickness);
				return;
				}


			private:

				/* return the clipping box */
				fBox2 _clippoly() const
				{
					fVec2 P2 = { center.X()      ,  center.Y() + radius };
					fVec2 P3 = { center.X() + radius ,  center.Y() + radius };
					fVec2 P4 = { center.X() - radius ,  center.Y() };
					fVec2 P5 = { center.X()      ,  center.Y() };
					fVec2 P6 = { center.X() + radius ,  center.Y() };
					fVec2 P7 = { center.X() - radius ,  center.Y() - radius };
					fVec2 P8 = { center.X()      ,  center.Y() - radius };
					switch (part)
					{
					case BOX_SPLIT_UP: { return fBox2(P4, P3, true); }
					case BOX_SPLIT_DOWN: { return fBox2(P7, P6, true); }
					case BOX_SPLIT_LEFT: { return fBox2(P7, P2, true); }
					case BOX_SPLIT_RIGHT: { return fBox2(P8, P3, true); }
					case BOX_SPLIT_UP_LEFT: { return fBox2(P4, P2, true); }
					case BOX_SPLIT_UP_RIGHT: { return fBox2(P5, P3, true); }
					case BOX_SPLIT_DOWN_LEFT: { return fBox2(P7, P5, true); }
					case BOX_SPLIT_DOWN_RIGHT: { return fBox2(P8, P6, true); }
					}
					MTOOLS_ERROR("Should not be here, unknown part");
					return fBox2();
				}

		FIGURECLASS_END()







		/*********************************************************
		*
		* Ellipse
		*
		**********************************************************/
		FIGURECLASS_BEGIN(Ellipse)


			fVec2	center;
			double	rx;				// x-radius
			double  ry;				// y-radius
			RGBc	color;			// color
			RGBc	fillcolor;		// interior color (transparent = no filling)



			/**
			 * Construct an ellipse (without filling)
			 *
			 * @param	centerellipse center 
			 * @param	rad_x		  x-radius
			 * @param	rad_y		  y-radius
			 * @param	col			  color
			 */
			Ellipse(fVec2 centerellipse, double rad_x, double rad_y, RGBc col) : center(centerellipse), rx(rad_x), ry(rad_y), color(col), fillcolor(RGBc::c_Transparent)
				{
				MTOOLS_ASSERT(rad_x >= 0);
				MTOOLS_ASSERT(rad_y >= 0);
				}


			/**
			 * Construct an ellipse (without filling) from its bounding box.
			 *
			 * @param	B   bounding box
			 * @param	col color.
			 */
			Ellipse(const fBox2 & B, RGBc col) : center(B.center()), rx(B.l(0) / 2), ry(B.l(1) / 2), color(col), fillcolor(RGBc::c_Transparent)
				{
				MTOOLS_ASSERT(!B.isEmpty());
				}


			/**
			 * Construct a filled ellipse
			 *
			 * @param	centerellipse center.
			 * @param	rad_x		  x-radius.
			 * @param	rad_y		  y-radius.
			 * @param	col			  color.
			 * @param	fillcol		  fillcolor.
			 */
			Ellipse(fVec2 centerellipse, double rad_x, double rad_y, RGBc col, RGBc fillcol) : center(centerellipse), rx(rad_x), ry(rad_y), color(col), fillcolor(fillcol)
				{
				MTOOLS_ASSERT(rad_x >= 0);
				MTOOLS_ASSERT(rad_y >= 0);
				}


			/**
			 * Construct a filled ellipse from its bounding box.
			 *
			 * @param	B	    bounding box.
			 * @param	col	    color.
			 * @param	fillcol fill color
			 */
			Ellipse(const fBox2 & B, RGBc col, RGBc fillcol) : center(B.center()), rx(B.l(0) / 2), ry(B.l(1) / 2), color(col), fillcolor(fillcol)
				{
				MTOOLS_ASSERT(!B.isEmpty());
				}

			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				if (fillcolor.isTransparent()) { im.canvas_draw_ellipse(R, center, rx, ry, color, highQuality); }
				else { im.canvas_draw_filled_ellipse(R, center, rx, ry, color, fillcolor, highQuality); }
				}


			virtual fBox2 boundingBox() const override
				{
				return fBox2(center.X() - rx, center.X() + rx, center.Y() - ry, center.Y() + ry);
				}


			virtual std::string toString(bool debug = false) const override
				{
				OSS os; 
				os << "Ellipse [" << center << " " << rx << " " << ry << " " << color; 
				if (fillcolor.comp.A != 0) { os << " filled: " << fillcolor; }
				os << "]";
				return os.str();
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & center & rx & ry & color & fillcolor; 
				}


			Ellipse(IBaseArchive & ar)
				{
				ar & center & rx & ry & color & fillcolor;
				}


			virtual void svg(mtools::SVGElement * el) const override
				{
				el->SetName("ellipse");
				el->setStrokeColor(color);
				el->setFillColor(fillcolor);
				el->xml->SetAttribute("cx", TX(center.X()));
				el->xml->SetAttribute("cy", TY(center.Y()));
				el->xml->SetAttribute("rx", TR(rx));
				el->xml->SetAttribute("ry", TR(ry));
				el->xml->SetAttribute("vector-effect", "non-scaling-stroke");
				return;
				}


		FIGURECLASS_END()








		/*********************************************************
		*
		* Thick Ellipse
		*
		**********************************************************/
		FIGURECLASS_BEGIN(ThickEllipse)


			fVec2	center;
			double	rx;				// x-radius
			double  ry;				// y-radius
			double	thickness_x;	// 0 = no thickness. < 0 = absolute thicknes,  >0 = relative thickness 
			double	thickness_y;	// 0 = no thickness  < 0 = absolute thicknes,  >0 = relative thickness 
			RGBc	color;			// color
			RGBc	fillcolor;		// interior color (transparent = no filling)




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
			ThickEllipse(fVec2 centerellipse, double rad_x, double rad_y, double thick_x, double thick_y, bool relativethickness, RGBc col)
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
			ThickEllipse(const fBox2 & B, double thick_x, double thick_y, bool relativethickness, RGBc col)
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
			ThickEllipse(fVec2 centerellipse, double rad_x, double rad_y, double thick_x, double thick_y, bool relativethickness, RGBc col, RGBc fillcol)
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
			ThickEllipse(const fBox2 & B, double thick_x, double thick_y, bool relativethickness, RGBc col, RGBc fillcol)
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
				OSS os; 
				os << "ThickEllipse [" << center << " " << rx << " " << ry << " " << color;
				if (fillcolor.comp.A != 0) { os << " filled: " << fillcolor; }
				if ((thickness_x != 0.0) || (thickness_y != 0.0))
					{
					if (thickness_x >= 0) { os << " rel. thick: x " << thickness_x << " y " << thickness_y; } else { os << " abs. thick: " << (-thickness_x) << " y "  << (-thickness_y); }
					}
				os << "]";
				return os.str();
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & center & rx & ry & thickness_x & thickness_y & color & fillcolor; 
				}


			ThickEllipse(IBaseArchive & ar)
				{
				ar & center & rx & ry & thickness_x & thickness_y & color & fillcolor;
				}


			virtual void svg(mtools::SVGElement * el) const override
				{

				if ((thickness_x  <= 0)||((thickness_y <= 0)))
					{
					el->Comment("SVG cannot accurately represent Figure::ThickEllipse with absolute thickness!");
					el->SetName("ellipse");
					el->setStrokeColor(color);
					el->setFillColor(fillcolor);
					el->xml->SetAttribute("cx", TX(center.X()));
					el->xml->SetAttribute("cy", TY(center.Y()));
					el->xml->SetAttribute("rx", TR(rx));
					el->xml->SetAttribute("ry", TR(ry));
					el->xml->SetAttribute("vector-effect", "non-scaling-stroke");
					return;
					}

				double thick = thickness_x;
				if (thickness_x != thickness_y)
					{
					thick = (thickness_x + thickness_y) / 2;
					el->Comment((mtools::toString("SVG cannot accurately represent Figure::ThickEllipse with different x/y thickness. Using middle thickness value: ") + mtools::doubleToStringNice(thick)).c_str());
					}

				// TODO : improve the svg code below using a <mask> instead so that the
				// stroke is not drawn over the interior  which cause defects when using transparency 
				el->SetName("ellipse");
				el->setStrokeColor(color);
				el->setFillColor(fillcolor);
				el->xml->SetAttribute("cx", TX(center.X()));
				el->xml->SetAttribute("cy", TY(center.Y()));
				el->xml->SetAttribute("rx", TR(rx - thick/2));
				el->xml->SetAttribute("ry", TR(ry - thick/2));
				el->xml->SetAttribute("stroke-width", thick);
				return;
				}

		FIGURECLASS_END()







		/*********************************************************
		*
		* Ellipse Part
		*
		**********************************************************/
		FIGURECLASS_BEGIN(EllipsePart)


			/** ellipse parameters **/
			fVec2	center;			// circle center
			double	rx;				// x-radius
			double  ry;				// y-radius
			RGBc	color;			// color
			RGBc	fillcolor;		// interior color (transparent = no filling)
			int		part;			// one of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT,, BOX_SPLIT_DOWN_LEFT, , BOX_SPLIT_DOWN_RIGHT


			/**
			 * Construct part of an ellipse (no filling).
			 *
			 * @param	ellipsepart   part to draw. One of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT,
			 * 						  BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT,,
			 * 						  BOX_SPLIT_DOWN_LEFT, , BOX_SPLIT_DOWN_RIGHT.
			 * @param	centerellipse center.
			 * @param	rad_x		  x-radius.
			 * @param	rad_y		  y-radius.
			 * @param	col			  outline color.
			 */
			EllipsePart(int ellipsepart, fVec2 centerellipse, double rad_x, double rad_y, RGBc col) : center(centerellipse), rx(rad_x), ry(rad_y), color(col), fillcolor(RGBc::c_Transparent), part(ellipsepart)
				{
				MTOOLS_ASSERT((ellipsepart >= 0) && (ellipsepart < 8));
				MTOOLS_ASSERT(rad_x >= 0);
				MTOOLS_ASSERT(rad_y >= 0);
				}


			/**
			 * Construct part of an ellipse (no filling) from its bounding box
			 *
			 * @param	ellipsepart part to draw. One of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT,
			 * 						BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT,,
			 * 						BOX_SPLIT_DOWN_LEFT, , BOX_SPLIT_DOWN_RIGHT.
			 * @param	B		    bounding box (for the whole ellipse).
			 * @param	col		    outline color
			 */
			EllipsePart(int ellipsepart, const fBox2 & B, RGBc col) : center(B.center()), rx(B.l(0) / 2), ry(B.l(1) / 2), color(col), fillcolor(RGBc::c_Transparent), part(ellipsepart)
				{
				MTOOLS_ASSERT((ellipsepart >= 0) && (ellipsepart < 8));
				MTOOLS_ASSERT(!B.isEmpty());
				}


			/**
			 * Construct part of a filled ellipse
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
			EllipsePart(int ellipsepart, fVec2 centerellipse, double rad_x, double rad_y, RGBc col, RGBc fillcol) : center(centerellipse), rx(rad_x), ry(rad_y), color(col), fillcolor(fillcol), part(ellipsepart)
				{
				MTOOLS_ASSERT((ellipsepart >= 0) && (ellipsepart < 8));
				MTOOLS_ASSERT(rad_x >= 0);
				MTOOLS_ASSERT(rad_y >= 0);
				}


			/**
			 * Construct part of a filled ellipse from its bounding box
			 *
			 * @param	ellipsepart part to draw. One of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT,
			 * 						BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT,,
			 * 						BOX_SPLIT_DOWN_LEFT, , BOX_SPLIT_DOWN_RIGHT.
			 * @param	B		    bounding box (for the whole ellipse).
			 * @param	col		    outline color.
			 * @param	fillcol	    fill color.
			 */
			EllipsePart(int ellipsepart, const fBox2 & B, RGBc col, RGBc fillcol) : center(B.center()), rx(B.l(0) / 2), ry(B.l(1) / 2), color(col), fillcolor(fillcol), part(ellipsepart)
				{
				MTOOLS_ASSERT((ellipsepart >= 0) && (ellipsepart < 8));
				MTOOLS_ASSERT(!B.isEmpty());
				}


			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				if (fillcolor.isTransparent()) { im.canvas_draw_part_ellipse(R, part, center, rx, ry, color, highQuality); }
				else { im.canvas_draw_part_filled_ellipse(R, part, center, rx, ry, color, fillcolor, highQuality); }
				}


			virtual fBox2 boundingBox() const override
				{
				return fBox2(center.X() - rx, center.X() + rx, center.Y() - ry, center.Y() + ry).get_split(part);
				}


			virtual std::string toString(bool debug = false) const override
				{
				OSS os;
				os << "EllipsePart [";
				switch (part)
					{
					case BOX_SPLIT_UP: { os << "HALF UP"; break; }
					case BOX_SPLIT_DOWN: { os << "HALF DOWN"; break; }
					case BOX_SPLIT_LEFT: { os << "HALF LEFT"; break; }
					case BOX_SPLIT_RIGHT: { os << "HALF RIGHT"; break; }
					case BOX_SPLIT_UP_LEFT: { os << "QUARTER UP LEFT"; break; }
					case BOX_SPLIT_UP_RIGHT: { os << "QUARTER UP RIGHT"; break; }
					case BOX_SPLIT_DOWN_LEFT: { os << "QUARTER DOWN LEFT"; break; }
					case BOX_SPLIT_DOWN_RIGHT: { os << "QUARTER DOWN RIGHT"; break; }
					default: { os << "ERROR PART"; }
					}
				os << " " << center << " " << rx << " " << ry << " " << color;
				if (fillcolor.comp.A != 0) { os << " filled: " << fillcolor; }
				os << "]";
				return os.str();
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & part & center & rx & ry  & color & fillcolor;
				}


			EllipsePart(IBaseArchive & ar)
				{
				ar & part & center & rx & ry  & color & fillcolor;
				}



			virtual void svg(mtools::SVGElement * el) const override
				{
				fBox2 B = _clippoly(); 

				el->SetName("g");
				const std::string uid = el->getUID("clippath");

				auto clip_el = el->NewChildSVGElement("clipPath"); 
				clip_el->xml->SetAttribute("id", uid.c_str());

				auto path_el = clip_el->NewChildSVGElement("rect");
				path_el->noFill();
				path_el->noStroke();
				path_el->xml->SetAttribute("x", TX(B.min[0]));
				path_el->xml->SetAttribute("y", TY(B.min[1]) + TY(B.ly()));
				path_el->xml->SetAttribute("width", TR(B.lx()));
				path_el->xml->SetAttribute("height", TR(B.ly()));
				
				auto fig_el = el->NewChildSVGElement("ellipse");
				fig_el->setStrokeColor(color);
				fig_el->setFillColor(fillcolor);
				fig_el->xml->SetAttribute("cx", TX(center.X()));
				fig_el->xml->SetAttribute("cy", TY(center.Y()));
				fig_el->xml->SetAttribute("rx", TR(rx));
				fig_el->xml->SetAttribute("ry", TR(ry));
				fig_el->xml->SetAttribute("vector-effect", "non-scaling-stroke");
				fig_el->xml->SetAttribute("clip-path", (mtools::toString("url(#") + uid + ")").c_str());
				}



			private:

		/* return the clipping box */
			fBox2 _clippoly() const
				{	
				fVec2 P2 = { center.X()      ,  center.Y() + ry };
				fVec2 P3 = { center.X() + rx ,  center.Y() + ry };
				fVec2 P4 = { center.X() - rx ,  center.Y() };
				fVec2 P5 = { center.X()      ,  center.Y() };
				fVec2 P6 = { center.X() + rx ,  center.Y() };
				fVec2 P7 = { center.X() - rx ,  center.Y() - ry };
				fVec2 P8 = { center.X()      ,  center.Y() - ry };
				switch (part)
				{
				case BOX_SPLIT_UP: { return fBox2(P4, P3, true); }
				case BOX_SPLIT_DOWN: { return fBox2(P7, P6, true); }
				case BOX_SPLIT_LEFT: { return fBox2(P7, P2, true); }
				case BOX_SPLIT_RIGHT: { return fBox2(P8, P3, true); }
				case BOX_SPLIT_UP_LEFT: { return fBox2(P4, P2, true); }
				case BOX_SPLIT_UP_RIGHT: { return fBox2(P5, P3, true); }
				case BOX_SPLIT_DOWN_LEFT: { return fBox2(P7, P5, true); }
				case BOX_SPLIT_DOWN_RIGHT: { return fBox2(P8, P6, true); }
				}
				MTOOLS_ERROR("Should not be here, unknown part");
				return fBox2();
				}

		FIGURECLASS_END()







		/*********************************************************
		*
		* Thick Ellipse Part
		*
		**********************************************************/
		FIGURECLASS_BEGIN(ThickEllipsePart)


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
			ThickEllipsePart(int ellipsepart, fVec2 centerellipse, double rad_x, double rad_y, double thick_x, double thick_y, bool relativethickness, RGBc col)
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
			ThickEllipsePart(int ellipsepart, const fBox2 & B, double thick_x, double thick_y, bool relativethickness, RGBc col)
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
			ThickEllipsePart(int ellipsepart, fVec2 centerellipse, double rad_x, double rad_y, double thick_x, double thick_y, bool relativethickness, RGBc col, RGBc fillcol)
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
			ThickEllipsePart(int ellipsepart, const fBox2 & B, double thick_x, double thick_y, bool relativethickness, RGBc col, RGBc fillcol)
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
				OSS os; 
				os << "ThickEllipsePart [";
				switch (part)
					{
					case BOX_SPLIT_UP: { os << "HALF UP"; break; }
					case BOX_SPLIT_DOWN: { os << "HALF DOWN"; break; }
					case BOX_SPLIT_LEFT: { os << "HALF LEFT"; break; }
					case BOX_SPLIT_RIGHT: { os << "HALF RIGHT"; break; }
					case BOX_SPLIT_UP_LEFT: { os << "QUARTER UP LEFT"; break; }
					case BOX_SPLIT_UP_RIGHT: { os << "QUARTER UP RIGHT"; break; }
					case BOX_SPLIT_DOWN_LEFT: { os << "QUARTER DOWN LEFT"; break; }
					case BOX_SPLIT_DOWN_RIGHT: { os << "QUARTER DOWN RIGHT"; break; }
					default: { os << "ERROR PART"; }
					}
				os << " " << center << " " << rx << " " << ry << " " << color;
				if (fillcolor.comp.A != 0) { os << " filled: " << fillcolor; }
				if ((thickness_x != 0.0) || (thickness_y != 0.0))
					{
					if (thickness_x >= 0) { os << " rel. thick: x " << thickness_x << " y " << thickness_y; } else { os << " abs. thick: " << (-thickness_x) << " y "  << (-thickness_y); }
					}
				os << "]";
				return os.str();
				}


			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & part & center & rx & ry & thickness_x & thickness_y & color & fillcolor;
				}


			ThickEllipsePart(IBaseArchive & ar)
				{
				ar & part & center & rx & ry & thickness_x & thickness_y & color & fillcolor;
				}






			virtual void svg(mtools::SVGElement * el) const override
				{
				fBox2 B = _clippoly(); 

				el->SetName("g");
				const std::string uid = el->getUID("clippath");

				auto clip_el = el->NewChildSVGElement("clipPath"); 
				clip_el->xml->SetAttribute("id", uid.c_str());

				auto path_el = clip_el->NewChildSVGElement("rect");
				path_el->noFill();
				path_el->noStroke();
				path_el->xml->SetAttribute("x", TX(B.min[0]));
				path_el->xml->SetAttribute("y", TY(B.min[1]) + TY(B.ly()));
				path_el->xml->SetAttribute("width", TR(B.lx()));
				path_el->xml->SetAttribute("height", TR(B.ly()));

				auto fig_el = el->NewChildSVGElement("ellipse");
				fig_el->setStrokeColor(color);
				fig_el->setFillColor(fillcolor);
				fig_el->xml->SetAttribute("clip-path", (mtools::toString("url(#") + uid + ")").c_str());
				fig_el->xml->SetAttribute("cx", TX(center.X()));
				fig_el->xml->SetAttribute("cy", TY(center.Y()));


				if ((thickness_x  <= 0)||((thickness_y <= 0)))
					{
					fig_el->Comment("SVG cannot accurately represent Figure::ThickEllipsePart with absolute thickness!");
					fig_el->xml->SetAttribute("rx", TR(rx));
					fig_el->xml->SetAttribute("ry", TR(ry));
					fig_el->xml->SetAttribute("vector-effect", "non-scaling-stroke");
					return;
					}

				double thick = thickness_x;
				if (thickness_x != thickness_y)
					{
					thick = (thickness_x + thickness_y) / 2;
					fig_el->Comment((mtools::toString("SVG cannot accurately represent Figure::ThickEllipsePart with different x/y thickness. Using middle thickness value: ") + mtools::doubleToStringNice(thick)).c_str());
					}

				// TODO : improve the svg code below using a <mask> instead so that the
				// stroke is not drawn over the interior  which cause defects when using transparency 
				fig_el->xml->SetAttribute("rx", TR(rx - thick/2));
				fig_el->xml->SetAttribute("ry", TR(ry - thick/2));
				fig_el->xml->SetAttribute("stroke-width", thick);
				return;
				}


			private:

				/* return the clipping box */
				fBox2 _clippoly() const
				{
					fVec2 P2 = { center.X()      ,  center.Y() + ry };
					fVec2 P3 = { center.X() + rx ,  center.Y() + ry };
					fVec2 P4 = { center.X() - rx ,  center.Y() };
					fVec2 P5 = { center.X()      ,  center.Y() };
					fVec2 P6 = { center.X() + rx ,  center.Y() };
					fVec2 P7 = { center.X() - rx ,  center.Y() - ry };
					fVec2 P8 = { center.X()      ,  center.Y() - ry };
					switch (part)
					{
					case BOX_SPLIT_UP: { return fBox2(P4, P3, true); }
					case BOX_SPLIT_DOWN: { return fBox2(P7, P6, true); }
					case BOX_SPLIT_LEFT: { return fBox2(P7, P2, true); }
					case BOX_SPLIT_RIGHT: { return fBox2(P8, P3, true); }
					case BOX_SPLIT_UP_LEFT: { return fBox2(P4, P2, true); }
					case BOX_SPLIT_UP_RIGHT: { return fBox2(P5, P3, true); }
					case BOX_SPLIT_DOWN_LEFT: { return fBox2(P7, P5, true); }
					case BOX_SPLIT_DOWN_RIGHT: { return fBox2(P8, P6, true); }
					}
					MTOOLS_ERROR("Should not be here, unknown part");
					return fBox2();
				}

		FIGURECLASS_END()









		/************************************************************************************************************************************
		*
		* OTHER
		*
		*************************************************************************************************************************************/







		/*********************************************************
		*
		* Text
		*
		**********************************************************/
		FIGURECLASS_BEGIN(Text)

			static const int max_font_size = 128;	//maximum font size to use

			std::string		_text;				// text to draw
			fVec2			_pos;				// position in the canvas
			fVec2			_size;				// size of the box (0 if auto adjust in a dimension). 
			int				_text_pos;			// localistion of the anchor point of the text box
			RGBc			_textcolor;			// text color
			RGBc			_bkcolor;			// background color
			float			_op;				// global opacity multiplier

			fBox2			_canvas_bb;			// bounding box on the canvas
			iVec2			_text_max_font_dim;	// size of the text when using max size font

			Image			_im;				// last image used
			int				_im_font_size;		// font size used for the image

			/**
			 * Constructor
			 *
			 * @param	text		  	The text to draw.
			 * @param	pos			  	position of the text in the canvas.
			 * @param	size		  	Size of the bounding box. A negative dimension is adjusted
			 * 							automatically w.r.t. to the other one to respect the text aspect
			 * 							ratio. Both dimensions cannot be simultaneously negative.
			 * @param	txt_pos		  	(Optional) Positioning method which explain which point `pos` refers
			 * 							to w.r.t the text bounding box (combination of MTOOLS_TEXT_XCENTER,
			 * 							MTOOLS_TEXT_LEFT, MTOOLS_TEXT_RIGHT, MTOOLS_TEXT_TOP,
			 * 							MTOOLS_TEXT_BOTTOM, MTOOLS_TEXT_YCENTER)
			 * @param	textcolor	  	the text color.
			 * @param	bkcolor		  	the background color.
			 * @param	op			  	global opacity factor to apply to the object (text + background).
			 **/
			Text(std::string text, fVec2 pos, fVec2 size, int txt_pos = MTOOLS_TEXT_XCENTER | MTOOLS_TEXT_YCENTER, RGBc textcolor = RGBc::c_Black, RGBc bkcolor = RGBc::c_Transparent, float op = 1.0f)
				: _text(text) , _pos(pos) , _size(size) , _text_pos(txt_pos), _textcolor(textcolor), _bkcolor(bkcolor), _op(op)
				{
				MTOOLS_INSURE((_size.X() > 0) || (_size.Y() > 0));	// both size cannot be simultaneously 0

				_text_max_font_dim = gFont(max_font_size, MTOOLS_NATIVE_FONT_ABOVE).textDimension(_text);	// compute text dimensions with max font size				
				if ((_text_max_font_dim.X() == 0) || (_text_max_font_dim.Y() == 0)) { _text_max_font_dim.X() = 0;  MTOOLS_DEBUG("Figure::Text has no text to draw !"); }

				_canvas_bb = _canvas_compute_bb({ (double)_text_max_font_dim.X(), (double)_text_max_font_dim.Y() }); // compute the bounding box of the text in the canvas. 

				_im.empty();		// no image drawn yet
				_im_font_size = 0;  // so no font size
				}


			
			virtual void draw(Image & im, const fBox2 & R, bool highQuality, double min_thickness) override
				{
				if (_text_max_font_dim.X() == 0) return;							// nothing to draw. 
				_recreateImage(im, R);												// recreate the image if needed
				const fBox2 imBox = im.imagefBox();									// get the image box
				const fBox2 rbb = boxTransform(_canvas_bb, R, imBox);				// compute the corresponding subbox to blend into the image
				im.blend_rescaled((highQuality ? 10 : 0), _im, iBox2(rbb), _op);	// blend into im
				}



			/* return the bounding box */
			virtual fBox2 boundingBox() const override
				{
				return _canvas_bb;
				}


			/* print info int a std::string */
			virtual std::string toString(bool debug = false) const override
				{
				OSS os; 
				os << "text [" << _text << "]\n";
				os << "pos " << _pos << "\n";
				os << "text pos ";
				if (_text_pos & MTOOLS_TEXT_XCENTER) os << "MTOOLS_TEXT_XCENTER ";
				if (_text_pos & MTOOLS_TEXT_LEFT) os << "MTOOLS_TEXT_LEFT ";
				if (_text_pos & MTOOLS_TEXT_RIGHT) os << "MTOOLS_TEXT_RIGHT ";
				if (_text_pos & MTOOLS_TEXT_YCENTER) os << "MTOOLS_TEXT_YCENTER ";
				if (_text_pos & MTOOLS_TEXT_TOP) os << "MTOOLS_TEXT_TOP ";
				if (_text_pos & MTOOLS_TEXT_BOTTOM) os << "MTOOLS_TEXT_BOTTOM ";
				os << "\n";
				os << "size "		<< _size << "\n";
				os << "textcolor "	<< _textcolor << "\n";
				os << "bkcolor "	<< _bkcolor << "\n";
				os << "opacity " << _op << "\n";
				os << "\n";
				return os.str();
				}


			/* serialize the figure */ 
			virtual void serialize(OBaseArchive & ar) const override
				{
				ar & _text & _pos & _size & _text_pos & _textcolor & _bkcolor & _op;
				}


			/* ctor for deserialization*/
			Text(IBaseArchive & ar) 
				{
				ar & _text & _pos & _size & _text_pos & _textcolor & _bkcolor & _op;

				_text_max_font_dim = gFont(max_font_size, MTOOLS_NATIVE_FONT_ABOVE).textDimension(_text);	// compute text dimensions with max font size				
				if ((_text_max_font_dim.X() == 0) || (_text_max_font_dim.Y() == 0)) { _text_max_font_dim.X() = 0;  MTOOLS_DEBUG("Figure::Text has no text to draw !"); }

				_canvas_bb = _canvas_compute_bb({ (double)_text_max_font_dim.X(), (double)_text_max_font_dim.Y() }); // compute the bounding box of the text in the canvas. 

				_im.empty();		// no image drawn yet
				_im_font_size = 0;  // so no font size
				}




			virtual void svg(mtools::SVGElement * el) const override
				{
				const double fontsize = 64;				// real font size
				const double vspace = 82;				// vertical spacing to use

				if (_text_max_font_dim.X() == 0)
					{
					el->Comment("Empty text, doing nothing.");
					return;
					}

				fVec2 adim = _textsize_in_Arial_64px(vspace);	// size of the text template in 64px using Arial font with given spacing 
				fBox2 box = _canvas_compute_bb(adim);			// compute the bounding box. 

				// draw the background first if needed
				RGBc bkcol = _bkcolor.getMultOpacity(_op);
				if (bkcol.opacity() > 0)
					{ 
					el->SetName("g");
					auto bk_el = el->NewChildSVGElement("rect");
					bk_el->noStroke();
					bk_el->setFillColor(bkcol);
					bk_el->xml->SetAttribute("x", TX(box.min[0]));
					bk_el->xml->SetAttribute("y", TY(box.min[1]) + TY(box.ly()));
					bk_el->xml->SetAttribute("width", TR(box.lx()));
					bk_el->xml->SetAttribute("height", TR(box.ly()));
					el = el->NewChildSVGElement("text");
					}

				if ((box.lx() <= 0)||(box.ly() <= 0)) return;	// no text to draw

				const double fs = fontsize * box.ly() / adim.Y();				// compute scaled font size
				const double vs = vspace * box.ly() / adim.Y();					// and the scaled vertical spacing
				const double xx = TX(box.min[0]);								// and the position
				const double yy = TY(box.min[1]) + TY(box.ly()) - TY(fs/2);		//
				const double ce = (vspace-fontsize) * box.ly() / (2*adim.Y());	// needed to center when vertical spacing is larger then fontsize.

				// main text element. 
				el->SetName("text");
				el->xml->SetAttribute("x", xx);
				el->xml->SetAttribute("y", yy);
				el->xml->SetAttribute("font-size", fs);
				el->xml->SetAttribute("font-family", "Arial");
				el->xml->SetAttribute("dominant-baseline", "middle");
				el->xml->SetAttribute("text-anchor", "start");
				el->setFillColor(_textcolor.getMultOpacity(_op));

				// draw text line by line
				auto svec = _parseSVGtext();  // parse lines, indent and add non breakable spaces
				for (size_t i=0; i< svec.size(); i++)
					{
					auto tspan_el = el->NewChildSVGElement("tspan");
					tspan_el->xml->SetAttribute("x", xx);
					tspan_el->xml->SetAttribute("dy", ((i == 0) ? ce : vs));
					tspan_el->xml->SetAttribute("white-space", "pre");
					tspan_el->xml->SetText(svec[i].c_str());
					}

				return;
				}



		private:




			/* compute the canvas bounding box when the text has ratio dim (for some font) */
			 fBox2 _canvas_compute_bb(fVec2 dim) const
				{

				 if ((dim.X() == 0) || (dim.Y() == 0)) { dim = { 1.0,1.0 }; } // empty text has unit ratio

				fVec2 rsize = _size;
				if (rsize.X() <= 0.0)
					{ // must adjust x
					MTOOLS_INSURE(dim.Y() > 0);
					rsize.X() = dim.X()*rsize.Y() / dim.Y();
					}
				else if (rsize.Y() <= 0.0)
					{ // must adjust y
					MTOOLS_INSURE(dim.X() > 0);
					rsize.Y() = dim.Y()*rsize.X() / dim.X();
					}

				double ox = -rsize.X()/2;
				if (_text_pos & MTOOLS_TEXT_RIGHT)		ox = -rsize.X();
				if (_text_pos & MTOOLS_TEXT_LEFT)		ox = 0;				

				double oy = -rsize.Y()/2;
				if (_text_pos & MTOOLS_TEXT_TOP)		oy = -rsize.Y();
				if (_text_pos & MTOOLS_TEXT_BOTTOM)		oy = 0;

				return fBox2(_pos.X() + ox, _pos.X() + rsize.X() + ox, _pos.Y() + oy, _pos.Y() + rsize.Y() + oy);
				}


			 /* Compute the real font size that should be on the screen */
			 double _screenFontSize(Image & im, const fBox2 & R) const
				{	
				 const fBox2 imBox = im.imagefBox();								// get the image box
				 const fBox2 rbb = boxTransform(_canvas_bb, R, imBox);				// compute the corresponding subbox to blend into the image
				 return(rbb.ly() * max_font_size / _text_max_font_dim.Y());		
				}




			/* recreate the image only if needed, optimized for a given screen range*/
			void _recreateImage(Image & im, const fBox2 & R)
				{
				double screen_fs = _screenFontSize(im, R);						// find the real (double valued) fontsize needed. 
				if (screen_fs < 4) screen_fs = 4;								// clamp font size value
				if (screen_fs > max_font_size) screen_fs = max_font_size;		//

				int good_fs = gFontNearestSize((int)ceil(screen_fs), MTOOLS_NATIVE_FONT_ABOVE);	// convert to the closest native font	

				if ((_im_font_size >= good_fs) && ((double)_im_font_size <= (2*good_fs))) return; // no need to recreate image

				// ok, we recreate the image with font size good_fs
				auto & font = gFont(good_fs);																// get the font
				iVec2 dim = font.textDimension(_text);														// text dimension
				_im.resizeRaw(dim, true);																	// resize to the good dimension (release previous buffer) 
				_im.clear(_bkcolor);																		// draw the background					
				_im.draw_text(iVec2{ 0,0 }, _text, MTOOLS_TEXT_LEFT | MTOOLS_TEXT_TOP, _textcolor, &font);	// draw the text				
				_im_font_size = good_fs;																	// remember the new font size for the image
				}


			/**   
			 * Return the size of the text when printed using Arial font in 64px height. 			 * 
			 * 
			 * THe width of the Arial font characters obtained using with the javascript below. 
			 * Just run the javascript  script in Firefox, open the console with [CTRL]+[SHIFT]+K 
			 * and copy the result. 
			 * 
			<!DOCTYPE HTML>
			<html>
			<head>
				<style>
				body {
					margin: 0px;
					padding: 0px;
					}
				</style>
			</head>
			<body data-rsssl=1>
				<canvas id="myCanvas" width="578" height="200"></canvas>
				<script>
				var canvas = document.getElementById('myCanvas');
				var context = canvas.getContext('2d');
				context.font = '64px Arial';
				var i;
				var s = "{";
				var j = 0; 
				for (i = 0; i < 256; i++) {
					var text = String.fromCharCode(i)		 
					var metrics = context.measureText(text);
					var width = metrics.width;
					j++; 
					s += width.toString() + ((i != 255) ? "," : "}");
					if (j == 10) {
						s += "\n";			
						j = 0; 
						}
					} 		 		 
				console.log(s); // output result to console. 
				</script>
			</body>
			</html>   
			 **/
			fVec2 _textsize_in_Arial_64px(const double vspace) const
				{
				constexpr static double arial_64px_width[256] = { 0,0,0,0,0,0,0,0,0,17.78333282470703,
							17.78333282470703,17.78333282470703,17.78333282470703,17.78333282470703,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,17.78333282470703,17.78333282470703,
							17.78333282470703,17.78333282470703,17.78333282470703,17.78333282470703,22.71666717529297,35.599998474121094,35.599998474121094,56.900001525878906,42.68333435058594,12.216666221618652,
							21.316667556762695,21.316667556762695,24.899999618530273,37.38333511352539,17.78333282470703,21.316667556762695,17.78333282470703,17.78333282470703,35.599998474121094,35.599998474121094,
							35.599998474121094,35.599998474121094,35.599998474121094,35.599998474121094,35.599998474121094,35.599998474121094,35.599998474121094,35.599998474121094,17.78333282470703,17.78333282470703,
							37.38333511352539,37.38333511352539,37.38333511352539,35.599998474121094,64.96666717529297,42.68333435058594,42.68333435058594,46.21666717529297,46.21666717529297,42.68333435058594,
							39.099998474121094,49.78333282470703,46.21666717529297,17.78333282470703,32,42.68333435058594,35.599998474121094,53.31666564941406,46.21666717529297,49.78333282470703,
							42.68333435058594,49.78333282470703,46.21666717529297,42.68333435058594,39.099998474121094,46.21666717529297,42.68333435058594,60.400001525878906,42.68333435058594,42.68333435058594,
							39.099998474121094,17.78333282470703,17.78333282470703,17.78333282470703,30.03333282470703,35.599998474121094,21.316667556762695,35.599998474121094,35.599998474121094,32,
							35.599998474121094,35.599998474121094,17.78333282470703,35.599998474121094,35.599998474121094,14.216666221618652,14.216666221618652,32,14.216666221618652,53.31666564941406,
							35.599998474121094,35.599998474121094,35.599998474121094,35.599998474121094,21.316667556762695,32,17.78333282470703,35.599998474121094,32,46.21666717529297,
							32,32,32,21.383333206176758,16.633333206176758,21.383333206176758,37.38333511352539,0,0,0,
							0,0,0,17.78333282470703,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,
							17.78333282470703,21.316667556762695,35.599998474121094,35.599998474121094,35.599998474121094,35.599998474121094,16.633333206176758,35.599998474121094,21.316667556762695,47.150001525878906,
							23.683332443237305,35.599998474121094,37.38333511352539,0,47.150001525878906,35.349998474121094,25.600000381469727,35.13333511352539,21.316667556762695,21.316667556762695,
							21.316667556762695,36.88333511352539,34.38333511352539,21.316667556762695,21.316667556762695,21.316667556762695,23.383333206176758,35.599998474121094,53.38333511352539,53.38333511352539,
							53.38333511352539,39.099998474121094,42.68333435058594,42.68333435058594,42.68333435058594,42.68333435058594,42.68333435058594,42.68333435058594,64,46.21666717529297,
							42.68333435058594,42.68333435058594,42.68333435058594,42.68333435058594,17.78333282470703,17.78333282470703,17.78333282470703,17.78333282470703,46.21666717529297,46.21666717529297,
							49.78333282470703,49.78333282470703,49.78333282470703,49.78333282470703,49.78333282470703,37.38333511352539,49.78333282470703,46.21666717529297,46.21666717529297,46.21666717529297,
							46.21666717529297,42.68333435058594,42.68333435058594,39.099998474121094,35.599998474121094,35.599998474121094,35.599998474121094,35.599998474121094,35.599998474121094,35.599998474121094,
							56.900001525878906,32,35.599998474121094,35.599998474121094,35.599998474121094,35.599998474121094,17.78333282470703,17.78333282470703,17.78333282470703,17.78333282470703,
							35.599998474121094,35.599998474121094,35.599998474121094,35.599998474121094,35.599998474121094,35.599998474121094,35.599998474121094,35.13333511352539,39.099998474121094,35.599998474121094,
							35.599998474121094,35.599998474121094,35.599998474121094,32,35.599998474121094,32 };

				if (_text.size() == 0) return fVec2(0.0, 0.0);
				double mx = 0, x = 0;
				double my = vspace;
				for (size_t i = 0; i < _text.size(); i++)
					{
					const char c = _text[i];
					if (c >= 32) { x += arial_64px_width[(size_t)(c)]; }
					else if (c == '\t') { x += 4 * arial_64px_width[' ']; }
					else if (c == '\n') { if (x > mx) { mx = x; } x = 0;  my += vspace; }
					}
				if (x > mx) { mx = x; }
				return fVec2(mx, my);
				}



			/** parse the text before putting it in SVG. */
			std::vector<std::string> _parseSVGtext() const
				{
				std::vector<std::string> svec; 
				mtools::ostringstream os;
				for(size_t i=0; i < _text.size(); i++)
					{
					const char c = _text[i]; 
					if ((c > 32)&&(c < 128)) { os << c; }
					else if (c == '\t') { for (int j = 0; j < 4; j++) { os << (char)(0xC2) << (char)(0xA0); } }
					else if (c == '\n') { svec.push_back(os.str()); os.clear(); }
					else if (c == ' ') { os << (char)(0xC2) << (char)(0xA0); }
					}
				svec.push_back(os.str());
				return svec;
				}





		FIGURECLASS_END()










/*********************************************************************************************************
* Undef macro used for creating figure classes
**********************************************************************************************************/

	#undef TX
	#undef TY
	#undef TR
	#undef FIGURECLASS_BEGIN
	#undef FIGURECLASS_END



	}











	/*****************POSTPONED FROM FigureCanvas ***************/




	/**
	* Insert a figure into the canvas, inside a given layer
	*/
	template<int N> 
	template <typename FIGURECLASS> MTOOLS_FORCEINLINE void FigureCanvas<N>::operator()(const FIGURECLASS & figure, size_t layer)
		{
		MTOOLS_INSURE(layer < _nbLayers);
		Figure::internals_figure::FigureInterface * pf = _copyInPool(figure);			// save a copy of the object in the memory pool
		_figLayers[layer].insert(pf->boundingBox(), pf);	// add to the corresponding layer. 
		return;
		}


	/* delete all allocated memory TODO: REPLACE A BY BETTER VERSION THAN MALLOC/FREE */
	template<int N> void FigureCanvas<N>::_deallocateAll()
		{
		for (void *  p : _vecallocp) 
			{
			((Figure::internals_figure::FigureInterface*)p)->~FigureInterface();	// call dtor
			free(p);																// free memory
			}
		_vecallocp.clear();
		}


	/* used by the macro REGISTER_ARCHIVE_FIGURE_CLASS() */
	template<int N>
	template <typename FIGURECLASS> MTOOLS_FORCEINLINE void FigureCanvas<N>::_registerArchiveFigureClass(const std::string & classname, mtools::IBaseArchive & ar, size_t layer, int & match)
		{
		if (classname == FIGURECLASS::className())
			{
			if (match != 0)
				{
				MTOOLS_ERROR("class name [" << classname << "] is already in use by another class !");
				}
			Figure::internals_figure::FigureInterface * pf = _archiveInPool<FIGURECLASS>(ar); // create the figure directly in the pool
			_figLayers[layer].insert(pf->boundingBox(), pf);								// add to the corresponding layer. 
			match++; 
			}		
		}




	// macro for registering a Figure class so it can be archived
	#define REGISTER_ARCHIVE_FIGURE_CLASS(CLASSNAME)  _registerArchiveFigureClass<CLASSNAME>(classname, ar, layer, match);



	/**
	 * Insert a Figure in the canvas, extracted from an archive. 
	 *
	 * !!!!!!!! EVERY FigureInterface SUBCLASS MUST BE REGISTERED IN THIS METHOD !!!!!!
	 * 
	 **/
	template<int N> MTOOLS_FORCEINLINE void FigureCanvas<N>::_insertArchiveItem(mtools::IBaseArchive & ar, size_t layer)
		{
		MTOOLS_INSURE(layer < _nbLayers);
		std::string classname;
		ar & classname;
		int match = 0; // for sanity check

		//
		// Every Figure class must be registered below
		// 
				
		/************************************************************************************************************************************
		* DOTS
		*************************************************************************************************************************************/

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::CircleDot);

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::SquareDot);

		/************************************************************************************************************************************
		* LINES
		*************************************************************************************************************************************/

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::HorizontalLine);

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::VerticalLine);

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::Line);

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::PolyLine);

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::ThickHorizontalLine);

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::ThickVerticalLine);

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::ThickLine);

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::ThickPolyLine);

		/************************************************************************************************************************************
		* POLYGON
		*************************************************************************************************************************************/

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::BoxRegion);

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::Triangle);

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::Quad);

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::Polygon);

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::Rectangle);

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::ThickTriangle);

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::ThickQuad);

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::ThickPolygon);

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::ThickRectangle);

		/************************************************************************************************************************************
		* CIRCLE / ELLIPSE
		*************************************************************************************************************************************/

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::Circle);

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::ThickCircle);

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::CirclePart);

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::ThickCirclePart);

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::Ellipse);

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::ThickEllipse);

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::EllipsePart);

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::ThickEllipsePart);

		/************************************************************************************************************************************
		* OTHER
		*************************************************************************************************************************************/

		REGISTER_ARCHIVE_FIGURE_CLASS(Figure::Text);

		
		// make sure we had a match, otherwise this means that the classname is not registered
		if (match == 0) { MTOOLS_ERROR("Figure class name [" << classname << "] is not registered with any class !"); }
		}



	#undef REGISTER_ARCHIVE_CLASS

}




/* end of file */

