/** @file plot2Dcimg.cpp */
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


#include "graphics/plot2Dcimg.hpp"

#include <atomic>

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Check_Button.H>



namespace mtools
	{



	Plot2DCImg::Plot2DCImg(cimg_library::CImg<unsigned char> * im, int nbthreads, std::string name) : internals_graphics::Plotter2DObj(name), _typepos(TYPEBOTTOMLEFT), _im(im), _PD(nullptr), _proImg(nullptr)
		{
		_PD = new PixelDrawer<Plot2DCImg>(this, nbthreads);
		_proImg = new ProgressImg();
		}


	Plot2DCImg::Plot2DCImg(cimg_library::CImg<unsigned char> & im, int nbthreads, std::string name) : internals_graphics::Plotter2DObj(name), _typepos(TYPEBOTTOMLEFT), _im(&im), _PD(nullptr), _proImg(nullptr)
		{
		_PD = new PixelDrawer<Plot2DCImg>(this, nbthreads);
		_proImg = new ProgressImg();
		}


	Plot2DCImg::Plot2DCImg(Plot2DCImg && o) : internals_graphics::Plotter2DObj(std::move(o)), _typepos((int)o._typepos), _im(o._im), _PD((PixelDrawer<Plot2DCImg>*)o._PD), _proImg(o._proImg), _checkButtonCenter(nullptr), _checkButtonBottomLeft(nullptr)
		{
		o._PD = nullptr;     // so that the plane drawer is not destroyed when the first object goes out of scope.
		o._proImg = nullptr;
		}


	Plot2DCImg::~Plot2DCImg()
		{
		detach(); // detach from its owner if there is still one.
		delete _PD;     // remove the plane drawer
		delete _proImg; // and the progress image
		}


	void Plot2DCImg::image(cimg_library::CImg<unsigned char> * im)
		{
		enable(false);
		_im = im;
		enable(true);
		resetDrawing();
		}


	void Plot2DCImg::image(cimg_library::CImg<unsigned char> & im)
		{
		enable(false);
		_im = &im;
		enable(true);
		resetDrawing();
		}


	cimg_library::CImg<unsigned char> * Plot2DCImg::image() const { return _im; }


	void Plot2DCImg::position(int posType)
		{
		if (((posType != TYPECENTER) && (posType != TYPEBOTTOMLEFT)) || (posType == _typepos)) return; // nothing to do
		_typepos = posType;
		if (isInserted())
			{
			IndirectMemberProc<Plot2DCImg> proxy(*this, &Plot2DCImg::_updatePosTypeInFLTK); // update the status of the button in the fltk thread
			runInFltkThread(proxy); // and also refresh the drawing if needed
			resetDrawing();
			}
		}


	int Plot2DCImg::position() const { return _typepos; }


	fBox2 Plot2DCImg::favouriteRangeX(fBox2 R) { return computeRange(); }


	fBox2 Plot2DCImg::favouriteRangeY(fBox2 R) { return computeRange(); }


	bool Plot2DCImg::hasFavouriteRangeX() { return(!computeRange().isEmpty()); }


	bool Plot2DCImg::hasFavouriteRangeY() { return(!computeRange().isEmpty()); }

	
	void Plot2DCImg::setParam(mtools::fBox2 range, mtools::iVec2 imageSize)
			{
			if ((_proImg->height() != (size_t)imageSize.X()) || (_proImg->width() != (size_t)imageSize.Y()))
				{
				auto npimg = new ProgressImg((size_t)imageSize.X(), (size_t)imageSize.Y());
				_PD->setParameters(range, npimg);
				_PD->sync();
				delete _proImg;
				_proImg = npimg;
				return;
				}
			_PD->setParameters(range, _proImg);
			_PD->sync();
			_PD->enable(_PD->enable());
			}


	void Plot2DCImg::resetDrawing()
			{
			_PD->redraw(false);
			_PD->sync();
			Plotter2DObj::refresh();
			}


	int Plot2DCImg::drawOnto(Image & im, float opacity)
			{
			int q = _PD->progress();
			_proImg->blit(im, opacity, true);
			return q;
			}


	int Plot2DCImg::quality() const { return _PD->progress(); }


	void Plot2DCImg::enableThreads(bool status)  { _PD->enable(status); _PD->sync(); }


	bool Plot2DCImg::enableThreads() const { return _PD->enable(); }


	int Plot2DCImg::nbThreads() const  { return _PD->nbThreads(); }


	void Plot2DCImg::removed(Fl_Group * optionWin)
			{
			Fl::delete_widget(optionWin);
			_checkButtonCenter = nullptr;
			_checkButtonBottomLeft = nullptr;
			_PD->enable(false);
			_PD->sync();
			}


	internals_graphics::Drawable2DInterface * Plot2DCImg::inserted(Fl_Group * & optionWin, int reqWidth)
			{
			/* create the option window */
			optionWin = new Fl_Group(0, 0, reqWidth, 60); // create the option group
			_checkButtonCenter = new Fl_Round_Button(15, 10, reqWidth - 20, 15, "Origin at the center.");
			_checkButtonCenter->labelfont(0);
			_checkButtonCenter->labelsize(11);
			_checkButtonCenter->color2(FL_RED);
			_checkButtonCenter->type(102);
			_checkButtonCenter->callback(_roundButtonCB_static, this);
			_checkButtonCenter->when(FL_WHEN_CHANGED);
			_checkButtonBottomLeft = new Fl_Round_Button(15, 35, reqWidth - 20, 15, "Origin at the bottom left corner.");
			_checkButtonBottomLeft->labelfont(0);
			_checkButtonBottomLeft->labelsize(11);
			_checkButtonBottomLeft->color2(FL_RED);
			_checkButtonBottomLeft->type(102);
			_checkButtonBottomLeft->callback(_roundButtonCB_static, this);
			_checkButtonBottomLeft->when(FL_WHEN_CHANGED);
			if (_typepos == TYPECENTER) { _checkButtonCenter->setonly(); }
			else { _checkButtonBottomLeft->setonly(); }
			optionWin->end();
			return this;
			}


		fBox2 Plot2DCImg::computeRange()
			{
			if (_im == nullptr) return fBox2();
			const int lx = _im->width();
			const int ly = _im->height();
			if (_typepos == TYPEBOTTOMLEFT) return fBox2(-0.5, lx - 0.5, -0.5, ly - 0.5);
			return fBox2(-0.5 - (lx / 2), lx - 0.5 - (lx / 2), -0.5 - (ly / 2), ly - 0.5 - (ly / 2));
			}


		void Plot2DCImg::_updatePosTypeInFLTK()
			{
			if (_typepos == TYPECENTER) { _checkButtonCenter->setonly(); }
			else { _checkButtonBottomLeft->setonly(); }
			}


		void Plot2DCImg::_roundButtonCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DCImg*)data)->_roundButtonCB(W); }


		void Plot2DCImg::_roundButtonCB(Fl_Widget * W)
			{
			if (W == _checkButtonCenter) { _typepos = TYPECENTER; }
			else { _typepos = TYPEBOTTOMLEFT; }
			resetDrawing();
			}

	}
/* end of file */


