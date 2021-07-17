/** @file imagedisplay.cpp */
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


#include "graphics/imagedisplay.hpp"




namespace mtools
{



	ImageDisplay::ImageDisplay(int W, int H, int X, int Y, const char* title, bool allow_resizing, bool allow_move, bool allow_close, bool allow_select, bool force_selection_before_closing) 
		{
		mtools::IndirectMemberProc<ImageDisplay,int,int,int,int,const char *,bool,bool,bool,bool,bool> proxy(*this, &ImageDisplay::_createWindow, W, H, X, Y, title, allow_resizing, allow_move, allow_close, allow_select, force_selection_before_closing);
		mtools::runInFltkThread(proxy);
		}


	ImageDisplay::~ImageDisplay()
		{
		mtools::IndirectMemberProc<ImageDisplay> proxy(*this, &ImageDisplay::_destroyWindow);
		mtools::runInFltkThread(proxy);
		}


	void ImageDisplay::_createWindow(int W, int H, int X, int Y, const char* title, bool allow_resizing, bool allow_move, bool allow_close, bool allow_select, bool force_selection_before_closing)
		{
		_iwe = new internals_graphics::ImageWidgetExt(allow_close, allow_resizing, allow_move, allow_select, X, Y, W, H, title);
		_iwe->end();
		forceSelectionBeforeClosing(force_selection_before_closing);
		}


	void ImageDisplay::_destroyWindow()
		{
		delete _iwe;
		}



	void ImageDisplay::allowClosing(bool status)
		{
		_iwe->allowClosing(status);
		}


	void ImageDisplay::forceSelectionBeforeClosing(bool status)
		{
		_iwe->forceSelectionBeforeClosing(status);
		}


	void ImageDisplay::allowUserMove(bool status)
		{
		_iwe->allowUserMove(status);
		}


	void ImageDisplay::allowUserSelection(bool status)
		{
		_iwe->allowUserSelection(status);
		}


	void ImageDisplay::setSelection(iBox2 selectRect)
		{
		if (!mtools::isFltkThread())
			{
			mtools::IndirectMemberProc<ImageDisplay, iBox2> proxy(*this, &ImageDisplay::setSelection, selectRect);
			mtools::runInFltkThread(proxy);
			return;
			}
		_iwe->setSelection(selectRect);
		}


	iBox2 ImageDisplay::getSelection(bool clipWithImage)
		{
		if (!mtools::isFltkThread())
			{
			mtools::IndirectMemberFun<ImageDisplay, iBox2, bool> proxy(*this, &ImageDisplay::getSelection,clipWithImage);
			mtools::runInFltkThread(proxy);
			return (*proxy.result());
			}
		return (_iwe->getSelection(clipWithImage));
		}


	void ImageDisplay::setDefaultRange()
		{
		if (!mtools::isFltkThread())
			{
			mtools::IndirectMemberProc<ImageDisplay> proxy(*this, &ImageDisplay::setDefaultRange);
			mtools::runInFltkThread(proxy);
			return;
			}
		_iwe->setDefaultRange();
		}


	void ImageDisplay::setRange(fBox2 R)
		{
		if (!mtools::isFltkThread())
			{
			mtools::IndirectMemberProc<ImageDisplay, fBox2> proxy(*this, &ImageDisplay::setRange, R);
			mtools::runInFltkThread(proxy);
			return;
			}
		_iwe->setRange(R);
		}


	void ImageDisplay::redrawNow()
		{
		if (!mtools::isFltkThread())
			{
			mtools::IndirectMemberProc<ImageDisplay> proxy(*this, &ImageDisplay::redrawNow);
			mtools::runInFltkThread(proxy);
			return;
			}
		_iwe->redrawNow();
		}


	void ImageDisplay::setImage(const Image* im, bool useDefaultRange)
		{
		if (!mtools::isFltkThread())
			{
			mtools::IndirectMemberProc<ImageDisplay, const Image *, bool> proxy(*this, &ImageDisplay::setImage, im, useDefaultRange);
			mtools::runInFltkThread(proxy);
			return;
			}
		_iwe->set(im,useDefaultRange);
		}


	void ImageDisplay::display()
		{
		allowClosing(true);
		startDisplay();
		while (isDisplayOn())
			{
			std::this_thread::yield();
			}
		}


	void ImageDisplay::startDisplay()
		{
		if (!mtools::isFltkThread())
			{
			mtools::IndirectMemberProc<ImageDisplay> proxy(*this, &ImageDisplay::startDisplay);
			mtools::runInFltkThread(proxy);
			return;
			}
		_iwe->autoredrawflag(true);
		_iwe->show();
		}


	bool ImageDisplay::isDisplayOn()
		{
		return _iwe->shown(); // is it safe to call this outside of the FLTK thread ?
		}


	void ImageDisplay::stopDisplay()
		{
		if (!mtools::isFltkThread())
			{
			mtools::IndirectMemberProc<ImageDisplay> proxy(*this, &ImageDisplay::stopDisplay);
			mtools::runInFltkThread(proxy);
			return;
			}
		_iwe->autoredrawflag(false);
		_iwe->hide();
		}


	void  ImageDisplay::autoredraw(int fps)
		{
		if (!mtools::isFltkThread())
			{
			mtools::IndirectMemberProc<ImageDisplay, int> proxy(*this, &ImageDisplay::autoredraw, fps);
			mtools::runInFltkThread(proxy);
			return;
			}
		_iwe->autoredraw(fps);
		}




}




/** end of file */


