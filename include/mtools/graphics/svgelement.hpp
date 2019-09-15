/** @file svgelement.hpp */
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
#include "rgbc.hpp"
#include "../misc/stringfct.hpp"
#include "../misc/misc.hpp"

#include "tinyxml2.h"

#include <atomic>


namespace mtools
{


	static const char * SVGElement_DEFAULT_NAME = "g";  // by default, we create an empty group (which is a valid svg object)


	/**
	 * Class that extend tinyxml::XMLElement to provide useful method when writing SVG documents.
	 *
	 * TODO : find a way to derive from tinyxml::XMLElement (not possible before ctor/dtor are private).
	 **/
	class SVGElement
	{


		template<int NN> friend class FigureCanvas; // friend class can create objects. 
	

		/**
		 * Constructor. 
		 * If el is not null, then the object constructed is added as last child of el
		 * otherwise it is added as last child of xmlDoc
		 **/
		SVGElement(tinyxml2::XMLDocument & xmlDoc, tinyxml2::XMLElement * father) : xml(xmlDoc.NewElement(SVGElement_DEFAULT_NAME)), _xmlDoc(&xmlDoc), _children()
			{
			father->InsertEndChild(xml);
			}


	public:

		/* dtor, delete also all the children*/
		~SVGElement()
			{
			for (SVGElement * el : _children) { delete el; }
			}


		/**
		 * Creates a new SGVElement. Ownership still belong to this object which will take care
		 * of destroying it when it is itself destroyed.  
		 **/
		SVGElement * NewChildSVGElement(const char * name = SVGElement_DEFAULT_NAME)
			{
			SVGElement * el = new SVGElement(*_xmlDoc, xml);
			_children.push_back(el); // we keep ownership: register for deletion when this object is destoyed. 
			el->SetName(name);
			return el;
			}


		/**
		 * Return a unique id at each call that can be used as id tag in the SVG file.
		 **/
		std::string getUID(const std::string & name) const
			{			
			return name + "-" + mtools::toString((int64)(_id++));
			}


		/**
		 * Add a comment as the last child of this element.
		 **/
		void Comment(const char * str)
			{
			xml->LinkEndChild(_xmlDoc->NewComment(str));
			}


		/* Set the element name*/
		void SetName(const char * name)
			{
			xml->SetName(name);
			}


		/* set the stroke color together with its opacity*/
		void setStrokeColor(mtools::RGBc color)
			{
			xml->SetAttribute("stroke", (mtools::toString("rgb(") + mtools::toString(color.comp.R) + "," + mtools::toString(color.comp.G) + "," + mtools::toString(color.comp.B) + ")").c_str());
			xml->SetAttribute("stroke-opacity", color.opacity());
			}


		/* set the fill color together with its opacity*/
		void setFillColor(mtools::RGBc color)
			{
			xml->SetAttribute("fill", (mtools::toString("rgb(") + mtools::toString(color.comp.R) + "," + mtools::toString(color.comp.G) + "," + mtools::toString(color.comp.B) + ")").c_str());
			xml->SetAttribute("fill-opacity", color.opacity());
			}


		/* no stroke */
		void noStroke()
			{
			xml->SetAttribute("stroke", "none");
			xml->SetAttribute("stroke-opacity", 0);
			xml->SetAttribute("vector-effect", "non-scaling-stroke");
			}

		/* no stroke */
		void noFill()
			{
			xml->SetAttribute("fill", "none");
			xml->SetAttribute("fill-opacity", 0);
			}

		/** give acces to the underlying xml element */
		tinyxml2::XMLElement * xml;


		/** Coordinate transform to apply on the x-axis */
		double tx(double x) const { return x; } 
		int64 tx(int64 x) const { return x; }


		/** Coordinate transform to apply on the y-axis */
		double ty(double y) const { return -y; }
		int64 ty(int64 y) const { return -y; }


		/** Transform to apply to lenght/radius etc...*/
		double tr(double r) const { return r; }
		int64 tr(int64 r) const { return r; }



	private:

		tinyxml2::XMLDocument * _xmlDoc;			// Associated XMLDocument
		std::vector<SVGElement*> _children;			// vector of all the children that should be deleted when this object is deleted. 
		static std::atomic<int64> _id;
	};


}



/* end of file */

