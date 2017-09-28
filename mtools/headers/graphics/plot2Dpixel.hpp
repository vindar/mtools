/** @file plot2Dpixel.hpp */
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


#include "internal/plotter2Dobj.hpp"
#include "internal/drawable2Dobject.hpp"
#include "pixeldrawer.hpp"
#include "internal/rangemanager.hpp"
#include "../misc/indirectcall.hpp"
#include "../misc/internal/forward_fltk.hpp"


#include <iostream>
#include <atomic>

namespace mtools
	{

	template< typename T > class  Plot2DPixel;


	/**
	* Factory function for creating a plot2DPixel (reference version).
	*
	* @tparam  T   The class must implement the PixelDrawer requirements.
	* @param [in,out]  obj The Plane object to plot (reference version)
	* @param   name        The name of the plot.
	*
	* @return  A Plot2DPixel<T>
	**/
	template<typename T> Plot2DPixel<T> makePlot2DPixel(T & obj, int nbthreads = 1, std::string name = "PixelLattice")
		{
		return Plot2DPixel<T>(obj, nbthreads, name);
		}



	/**
	* Factory function for creating a plot2DPixel (pointer version).
	*
	* @tparam  T   The class must implement the PixelDrawer requirements.
	* @param [in,out]  obj The Plane object to plot (reference version)
	* @param   name        The name of the plot.
	*
	* @return  A Plot2DPlane<T>
	**/
	template<typename T> Plot2DPixel<T> makePlot2DPixel(T * obj, int nbthreads = 1, std::string name = "PixelLattice")
		{
		return Plot2DPixel<T>(obj, nbthreads, name);
		}





	/**
	* Plot Object which encapsulate a PixelDrawer object.
	*
	* @tparam  T   Object which fulfills the same requirements as those needed by the PixelDrawer
	*              class.
	**/
	template< typename T > class Plot2DPixel : public internals_graphics::Plotter2DObj, protected internals_graphics::Drawable2DInterface
		{

		public:

			/**
			* Constructor. Pointer version : permit to pass nullptr if the methods are static.
			*
			* @param [in,out]  obj The object that must fullfill the requirement of PixelDrawer. The
			*                      object must survive the plot.
			* @param   name        The name of the plot.
			**/
			Plot2DPixel(T * obj, int nbthread = 1, std::string name = "PixelLattice") : internals_graphics::Plotter2DObj(name), _LD(nullptr), _proImg(nullptr)
				{
				_LD = new PixelDrawer<T>(obj, nbthread);
				_proImg = new ProgressImg();
				}


			/**
			* Constructor. Reference verison
			*
			* @param [in,out]  obj The object that must fullfill the requirement of PixelDrawer. The
			*                      object must survive the plot.
			* @param   name        The name of the plot.
			**/
			Plot2DPixel(T & obj, int nbthread = 1, std::string name = "PixelLattice") : internals_graphics::Plotter2DObj(name), _LD(nullptr), _proImg(nullptr)
				{
				_LD = new PixelDrawer<T>(&obj, nbthread);
				_proImg = new ProgressImg();
				}


			/**
			* Move constructor.
			**/
			Plot2DPixel(Plot2DPixel && o) : internals_graphics::Plotter2DObj(std::move(o)), _LD((PixelDrawer<T>*)o._LD), _proImg(o._proImg)
				{
				o._LD = nullptr;     // so that the pixel drawer is not destroyed when the first object goes out of scope.
				o._proImg = nullptr;
				}


			/**
			* Destructor. Remove the object if it is still inserted.
			**/
			virtual ~Plot2DPixel()
				{
				detach();   // detach from its owner if there is still one.
				delete _LD;     // remove the plane drawer
				delete _proImg; // and the progress image
				}





		protected:

			virtual void setParam(mtools::fBox2 range, mtools::iVec2 imageSize) override
				{
				if ((_proImg->height() != (size_t)imageSize.X()) || (_proImg->width() != (size_t)imageSize.Y()))
					{
					auto npimg = new ProgressImg((size_t)imageSize.X(), (size_t)imageSize.Y());
					_LD->setParameters(range, npimg);
					_LD->sync();
					delete _proImg;
					_proImg = npimg;
					return;
					}
				_LD->setParameters(range, _proImg);
				_LD->sync();
				_LD->enable(_LD->enable());
				}


			virtual void resetDrawing() override
				{
				_LD->redraw(false);
				_LD->sync();
				}


			virtual int drawOnto(Img<unsigned char> & im, float opacity = 1.0) override
				{
				int q = _LD->progress();
				_proImg->blit(im, opacity, true);
				return q;
				}

			virtual int quality() const override { return _LD->progress(); }


			virtual void enableThreads(bool status) override { _LD->enable(status); _LD->sync(); }

			virtual bool enableThreads() const override { return _LD->enable(); }

			virtual int nbThreads() const override { return _LD->nbThreads(); }


			/**
			* Override of the removed method, nothing to do...
			**/
			virtual void removed(Fl_Group * optionWin) override
				{
				_LD->enable(false);
				}


			/**
			* Override of the inserted method. There is no option window for a pixel object...
			**/
			virtual internals_graphics::Drawable2DInterface * inserted(Fl_Group * & optionWin, int reqWidth) override
				{
				optionWin = nullptr;
				return this;
				}


		private:

			PixelDrawer<T> * _LD;
			ProgressImg * _proImg;

		};



	}


/* end of file */



