/** @file plot2Dlattice.cpp */
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


#include "stdafx_mtools.h"


#include "graphics/internal/plotter2Dobj.hpp"
#include "graphics/internal/drawable2Dobject.hpp"
#include "graphics/latticedrawer.hpp"
#include "misc/indirectcall.hpp"
#include "io/internal/fltkSupervisor.hpp"
#include "misc/internal/forward_fltk.hpp"
#include "graphics/plot2Dlattice.hpp"

#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Value_Slider.H>

#include <atomic>


namespace mtools
	{

	namespace internals_graphics
		{


		Plot2DLatticeBase::Plot2DLatticeBase(std::string name) : internals_graphics::Plotter2DObj(name), _checkButtonImage(nullptr), _checkButtonColor(nullptr), _opacifySlider(nullptr), _checkBlack(nullptr), _checkWhite(nullptr)
			{
			}


		Plot2DLatticeBase::Plot2DLatticeBase(Plot2DLatticeBase && o) : internals_graphics::Plotter2DObj(std::move(o)), _checkButtonImage(nullptr), _checkButtonColor(nullptr), _opacifySlider(nullptr), _checkBlack(nullptr), _checkWhite(nullptr)
			{
			}


		Plot2DLatticeBase::~Plot2DLatticeBase()
			{
			}


		void Plot2DLatticeBase::removed(Fl_Group * optionWin)
			{
			Fl::delete_widget(optionWin);
			_checkButtonColor = nullptr;
			_checkButtonImage = nullptr;
			}



		internals_graphics::Drawable2DInterface * Plot2DLatticeBase::inserted(Fl_Group * & optionWin, int reqWidth)
			{
			MTOOLS_ERROR("should not be called...");
			return nullptr;
			}


		void Plot2DLatticeBase::insertUI(Fl_Group * & optionWin, int reqWidth, int imageType, bool hasImage, float op, int transcolor)
			{
			/* create the option window */
			optionWin = new Fl_Group(0, 0, reqWidth, 110); // create the option group
			Fl_Group * gr1 = new Fl_Group(0, 0, reqWidth, 110); // create the option group
			_checkButtonColor = new Fl_Round_Button(10, 5, reqWidth - 20, 15, "Use the getColor() method.");
			_checkButtonColor->align(Fl_Align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT));
			_checkButtonColor->labelfont(0);
			_checkButtonColor->labelsize(11);
			_checkButtonColor->color2(FL_RED);
			_checkButtonColor->type(102);
			_checkButtonColor->callback(_roundButtonCB_static, this);
			_checkButtonColor->when(FL_WHEN_CHANGED);
			_checkButtonImage = new Fl_Round_Button(10, 85, reqWidth - 20, 15, "Use the getImage() method.");
			_checkButtonImage->align(Fl_Align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT));
			_checkButtonImage->labelfont(0);
			_checkButtonImage->labelsize(11);
			_checkButtonImage->color2(FL_RED);
			_checkButtonImage->type(102);
			_checkButtonImage->callback(_roundButtonCB_static, this);
			_checkButtonImage->when(FL_WHEN_CHANGED);
			if (imageType == TYPEIMAGE) { _checkButtonImage->setonly(); }
			else { _checkButtonColor->setonly(); }
			if (!hasImage) _checkButtonImage->deactivate();
			gr1->end();
			auto label1 = new Fl_Box(30, 25, 50, 15, "Opacify");
			label1->align(Fl_Align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT));
			label1->labelfont(0);
			label1->labelsize(11);
			_opacifySlider = new Fl_Value_Slider(80, 25, reqWidth - 80 - 30, 15, nullptr);
			_opacifySlider->labelfont(0);
			_opacifySlider->labelsize(11);
			_opacifySlider->align(Fl_Align(FL_ALIGN_RIGHT));
			_opacifySlider->box(FL_FLAT_BOX);
			_opacifySlider->type(FL_HOR_NICE_SLIDER);
			_opacifySlider->range(1.0, 4.0);
			_opacifySlider->step(0.05);
			_opacifySlider->value(op);
			_opacifySlider->color2(FL_RED);
			_opacifySlider->callback(_opacifySliderCB_static, this);
			auto label2 = new Fl_Box(30, 45, 145, 15, "Remove transparent pixels :");
			label2->align(Fl_Align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT));
			label2->labelfont(0);
			label2->labelsize(11);
			_checkWhite = new Fl_Check_Button(180, 45, reqWidth - 175 - 30, 15, "white");
			_checkWhite->align(Fl_Align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT));
			_checkWhite->labelfont(0);
			_checkWhite->labelsize(11);
			_checkWhite->value((transcolor == REMOVE_WHITE) ? 1 : 0);
			_checkWhite->callback(_checkWhiteCB_static, this);
			_checkBlack = new Fl_Check_Button(180, 65, reqWidth - 175 - 30, 15, "black");
			_checkBlack->align(Fl_Align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT));
			_checkBlack->labelfont(0);
			_checkBlack->labelsize(11);
			_checkBlack->value((transcolor == REMOVE_BLACK) ? 1 : 0);
			_checkBlack->callback(_checkBlackCB_static, this);
			optionWin->end();
			}


		void Plot2DLatticeBase::updateUI(int imageType, bool hasImage, float op, int transcolor)
			{
			IndirectMemberProc<Plot2DLatticeBase, int, bool, float, int> proxy(*this, &Plot2DLatticeBase::_updateUIinFLTK, imageType, hasImage, op, transcolor); // update the UI in the FLTK thread.
			runInFltkThread(proxy); // and also refresh the drawing if needed
			}


		void Plot2DLatticeBase::_updateUIinFLTK(int imageType, bool hasImage, float op, int transcolor)
			{
			if (imageType == TYPEIMAGE) { _checkButtonImage->setonly(); }
			else { _checkButtonColor->setonly(); }
			_opacifySlider->value(op);
			switch (transcolor)
				{
				case REMOVE_WHITE: { _checkWhite->value(1); _checkBlack->value(0); break; }
				case REMOVE_BLACK: { _checkWhite->value(0); _checkBlack->value(1); break; }
				case REMOVE_NOTHING: { _checkWhite->value(0); _checkBlack->value(0); break; }
				default: MTOOLS_ERROR("wtf...");
				}
			if (!hasImage) _checkButtonImage->deactivate();
			refresh();
			}



		void Plot2DLatticeBase::_roundButtonCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DLatticeBase*)data)->_roundButtonCB(W); }
		void Plot2DLatticeBase::_roundButtonCB(Fl_Widget * W)
			{
			if (W == _checkButtonImage) { _setImageType(TYPEIMAGE); }
			else { _setImageType(TYPEPIXEL); }
			refresh();
			}


		void Plot2DLatticeBase::_opacifySliderCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DLatticeBase*)data)->_opacifySliderCB(W); }
		void Plot2DLatticeBase::_opacifySliderCB(Fl_Widget * W)
			{
			_setOpacity((float)_opacifySlider->value());
			refresh();
			}


		void Plot2DLatticeBase::_checkBlackCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DLatticeBase*)data)->_checkBlackCB(W); }
		void Plot2DLatticeBase::_checkBlackCB(Fl_Widget * W)
			{
			if ((_checkWhite->value() == 0) && (_checkBlack->value() == 0))
				{
				_setTransColor(REMOVE_NOTHING);
				}
			else
				{
				_checkWhite->value(0);
				_checkWhite->redraw();
				_setTransColor(REMOVE_BLACK);
				}
			refresh();
			}


		void Plot2DLatticeBase::_checkWhiteCB_static(Fl_Widget * W, void * data) { MTOOLS_ASSERT(data != nullptr); ((Plot2DLatticeBase*)data)->_checkWhiteCB(W); }
		void Plot2DLatticeBase::_checkWhiteCB(Fl_Widget * W)
			{
			if ((_checkWhite->value() == 0) && (_checkBlack->value() == 0))
				{
				_setTransColor(REMOVE_NOTHING);
				}
			else
				{
				_checkBlack->value(0);
				_checkBlack->redraw();
				_setTransColor(REMOVE_WHITE);
				}
			refresh();
			}

		}

	}
/* end of file */



