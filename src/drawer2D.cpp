/** @file drawer2D.cpp */
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


#include "graphics/drawer2D.hpp"



namespace mtools
{



		void Drawer2D::draw(int min_quality)
			{
			static int DRAWER2D_WAITIME = 5;

			if (min_quality > 100) min_quality = 100;
			if (min_quality < 1) min_quality = 1;

			for (int i = (int)_tabobj.size(); i > 0; i--)
				{
				if (_tabobj[i - 1]->enable())
					{
					_tabobj[i - 1]->setParam(_rm.getRange(), _im.dimension()); // set the range and image size
					_tabobj[i - 1]->resetDrawing(); // start redrawing the objet
					}
				}

			for (int i = (int)_tabobj.size(); i > 0; i--)
				{
				if (_tabobj[i - 1]->enable())
					{
					while (_tabobj[i - 1]->quality() < min_quality)
						{
						std::this_thread::sleep_for(std::chrono::milliseconds(DRAWER2D_WAITIME));
						}
					_tabobj[i - 1]->drawOnto(_im);
					}
				}

			if (_disp.isDisplayOn())
				{ // update the display if it is on
				_disp.redrawNow();
				}
			}



		void Drawer2D::_insert(internals_graphics::Plotter2DObj* obj)
			{
			// this method is run inside the fltk thread for safety reasons
			if (!isFltkThread())
				{
				IndirectMemberProc<Drawer2D, internals_graphics::Plotter2DObj*> proxy(*this, &Drawer2D::_insert, obj); // registers the call
				runInFltkThread(proxy); // make the call to the owner indicating we are de-inserting ourselves.
				return;
				}
			if (obj == nullptr)
				{
				MTOOLS_DEBUG("Drawer2D::_insert() with null pointer !");
				return;
				}
			if (_findIndex(obj) >= 0)
				{
				MTOOLS_DEBUG("Drawer2D::_insert(), object already inserted!");
				return;
				}
			// move all the object down and insert the new one on the top 
			_tabobj.resize(_tabobj.size() + 1);
			for (int k = ((int)_tabobj.size()) - 1; k > 0; --k) { _tabobj[k] = _tabobj[k - 1]; }
			_tabobj[0] = obj;
			// call the inserted() method of the object to inform it of its insertion
			obj->_inserted(objectCB_static, &_rm, this, obj, 100);
			}


		void Drawer2D::removeAll()
			{
			// this method is run inside the fltk thread for safety reasons
			if (!isFltkThread())
				{
				IndirectMemberProc<Drawer2D> proxy(*this, &Drawer2D::removeAll); // registers the call
				runInFltkThread(proxy); // make the call to the owner indicating we are de-inserting ourselves.
				return;
				}
			for (auto o : _tabobj)
                {
				o->_removed();
                }
			_tabobj.clear();           
			}


		void Drawer2D::_remove(internals_graphics::Plotter2DObj * obj)
			{
			// this method is run inside the fltk thread for safety reasons
			if (!isFltkThread())
				{
				IndirectMemberProc<Drawer2D, internals_graphics::Plotter2DObj*> proxy(*this, &Drawer2D::_remove, obj); // registers the call
				runInFltkThread(proxy); // make the call to the owner indicating we are de-inserting ourselves.
				return;
				}
            for (int i = 0; i < (int)_tabobj.size(); i++)
                {
                if (_tabobj[i] == obj)
                    {
                    obj->_removed(); // inform than we are removing the object from the plotter
                    while ((i+1) < ((int)_tabobj.size()))
                        {
                        _tabobj[i] = _tabobj[i + 1];
                        i++;
                        }
                    _tabobj.resize(_tabobj.size() - 1); // diminish the vector size by one.
                    return;
                    }
                }
            MTOOLS_DEBUG("Plotter2DWindow::remove(), object not found.");
			}


		void Drawer2D::_moveUp(internals_graphics::Plotter2DObj* obj)
			{
			for (int i = 0; i < (int)_tabobj.size(); i++)
				{
				if (_tabobj[i] == obj)
					{
					if (i == 0) return; // nothing to do if already on top
					auto obj2 = _tabobj[i - 1];
					_tabobj[i - 1] = obj;
					_tabobj[i] = obj2;
					return;
					}
				}
			MTOOLS_DEBUG("Plotter2DWindow::moveUp(), object not found.");
			}


		void Drawer2D::_moveDown(internals_graphics::Plotter2DObj* obj)
			{
			for (int i = 0; i < (int)_tabobj.size(); i++)
				{
				if (_tabobj[i] == obj)
					{
					i++;
					if (i == (int)_tabobj.size()) return; // nothing to do if already at bottom
					_moveUp(_tabobj[i]);
					return;
					}
				}
			MTOOLS_DEBUG("Plotter2DWindow::moveDown(), object not found.");
			}


		void Drawer2D::_moveTop(internals_graphics::Plotter2DObj* obj)
			{	
            for (int i = 0; i < (int)_tabobj.size(); i++)
                {
                if (_tabobj[i] == obj)
                    {
                    if (i == 0) return; // nothing to do if already on top
                    for (int k = i; k > 0; k--) { _tabobj[k] = _tabobj[k - 1];}
                    _tabobj[0] = obj;
                    return;
                    }
                }
            MTOOLS_DEBUG("Plotter2DWindow::moveTop(), object not found.");
			}


		void Drawer2D::_moveBottom(internals_graphics::Plotter2DObj* obj)
			{
            for (int i = 0; i < (int)_tabobj.size(); i++)
                {
                if (_tabobj[i] == obj)
                    {
                    if (i == (((int)_tabobj.size())-1)) return; // nothing to do if already on bottom
                    for (int k = i+1; k < (int)_tabobj.size(); k++) { _tabobj[k - 1] = _tabobj[k];  }
                    _tabobj[_tabobj.size() - 1] = obj;
                    return;
                    }
                }
            MTOOLS_DEBUG("Plotter2DWindow::moveBottom(), object not found.");
			}


        fBox2 Drawer2D::_findRangeX(internals_graphics::Plotter2DObj * obj, fBox2 CR, bool keepAR)
            {
            fBox2 R = obj->favouriteRangeX(CR);
            if (R.isHorizontallyEmpty()) return fBox2(); // nothing to do in this case
            if (!keepAR)
                {
                R.min[1] = CR.min[1];
                R.max[1] = CR.max[1];
                }
            else
                {
                double c = (CR.min[1] + CR.max[1]) / 2;
                double r = CR.ly()*R.lx() / (2 * CR.lx());
                R.min[1] = c - r;
                R.max[1] = c + r;
                }
            return R;
            }


        fBox2 Drawer2D::_findRangeY(internals_graphics::Plotter2DObj * obj, fBox2 CR, bool keepAR)
            {
            fBox2 R = obj->favouriteRangeY(CR);
            if (R.isVerticallyEmpty()) return  fBox2(); // nothing to do in this case
            R.min[0] = CR.min[0];
            R.max[0] = CR.max[0];
            if (!keepAR) { return R; }
            R = R.fixedRatioEnclosingRect(CR.lx() / CR.ly());
            return R;
            }


		void Drawer2D::_useRangeX(internals_graphics::Plotter2DObj* obj)
			{
			fBox2 CR = _rm.getRange();   // current range
			bool keepAR = _rm.fixedAspectRatio();      // do we keep the aspect ratio
			fBox2 R = _findRangeX(obj, CR, keepAR);
			if (!R.isEmpty()) _rm.setRange(R);
			}


		void Drawer2D::_useRangeY(internals_graphics::Plotter2DObj* obj)
			{
			fBox2 CR = _rm.getRange();   // current range
			bool keepAR = _rm.fixedAspectRatio();      // do we keep the aspect ratio
			fBox2 R = _findRangeY(obj, CR, keepAR);
			if (!R.isEmpty()) _rm.setRange(R);
			}


		void Drawer2D::_useRangeXY(internals_graphics::Plotter2DObj* obj)
			{
			fBox2 CR = _rm.getRange();   // current range
			bool keepAR = _rm.fixedAspectRatio();      // do we keep the aspect ratio
			fBox2 R = _findRangeX(obj, CR, keepAR);
			if (R.isEmpty()) return;
			R = _findRangeY(obj, R, keepAR);
			if (R.isEmpty()) return;
			_rm.setRange(R);
			}


        fBox2 Drawer2D::_getAutoRangeX(fBox2 CR,bool keepAR)
            {
            fBox2 NR;
            for (int i = 0; i < (int)_tabobj.size(); i++)
                {
                if ((_tabobj[i]->enable()) && (_tabobj[i]->hasFavouriteRangeX()))
                    {
                    fBox2 R = _tabobj[i]->favouriteRangeX(CR);
                    if (!R.isHorizontallyEmpty())
                        {
                        if (NR.isHorizontallyEmpty()) { NR = R; } else
                            {
                            if (R.min[0] < NR.min[0]) { NR.min[0] = R.min[0]; }
                            if (NR.max[0] < R.max[0]) { NR.max[0] = R.max[0]; }
                            }
                        }
                    }
                }
            if (NR.isHorizontallyEmpty()) return fBox2();
            if (!keepAR)
                {
                NR.min[1] = CR.min[1];
                NR.max[1] = CR.max[1];
                return NR;
                }
            double c = (CR.min[1] + CR.max[1]) / 2;
            double r = CR.ly()*NR.lx() / (2 * CR.lx());
            NR.min[1] = c - r;
            NR.max[1] = c + r;
            return NR;
            }


        fBox2 Drawer2D::_getAutoRangeY(fBox2 CR,bool keepAR)
            {
            fBox2 NR;
            for (int i = 0; i < (int)_tabobj.size(); i++)
                {
                if ((_tabobj[i]->enable()) && (_tabobj[i]->hasFavouriteRangeY()))
                    {
                    fBox2 R = _tabobj[i]->favouriteRangeY(CR);
                    if (!R.isVerticallyEmpty())
                        {
                        if (NR.isVerticallyEmpty()) { NR = R; } else
                            {
                            if (R.min[1] < NR.min[1]) { NR.min[1] = R.min[1]; }
                            if (NR.max[1] < R.max[1]) { NR.max[1] = R.max[1]; }
                            }
                        }
                    }
                }
            if (NR.isHorizontallyEmpty()) return fBox2();
            NR.min[0] = CR.min[0];
            NR.max[0] = CR.max[0];
            if (!keepAR) { return NR; }
            NR = NR.fixedRatioEnclosingRect(CR.lx() / CR.ly());
            return NR;
            }



		void Drawer2D::autorangeX()
			{
			fBox2 CR = _rm.getRange();   // current range
			bool keepAR = _rm.fixedAspectRatio();      // do we keep the aspect ratio
			fBox2 R = _getAutoRangeX(CR, keepAR);
			if (R.isEmpty()) return;
			_rm.setRange(R);
			}


		void Drawer2D::autorangeY()
			{
			fBox2 CR = _rm.getRange();   // current range
			bool keepAR = _rm.fixedAspectRatio();      // do we keep the aspect ratio
			fBox2 R = _getAutoRangeY(CR, keepAR);
			if (R.isEmpty()) return;
			_rm.setRange(R);
			}


		void Drawer2D::autorangeXY()
			{
			fBox2 CR = _rm.getRange();   // current range
			bool keepAR = _rm.fixedAspectRatio();      // do we keep the aspect ratio
			fBox2 R = _getAutoRangeX(CR, keepAR);
			if (R.isEmpty()) return;
			R = _getAutoRangeY(R, keepAR);
			if (R.isEmpty()) return;
			_rm.setRange(R);
			}


		int Drawer2D::_findIndex(internals_graphics::Plotter2DObj* obj)
			{
			for (int i = 0; i < (int)_tabobj.size(); i++)
				{
				if (obj == _tabobj[i]) return i;
				}
			return -1;
			}


        void Drawer2D::objectCB_static(void * data, void * data2, void * obj, int code) { MTOOLS_ASSERT(data != nullptr); ((Drawer2D*)data)->objectCB(obj, code); }

		void Drawer2D::objectCB(void * obj, int code)
            {
            MTOOLS_ASSERT(isFltkThread());
            switch (code)
                {
                case internals_graphics::Plotter2DObj::_REQUEST_DETACH:
                    { // the object is detaching itself
                    _remove((internals_graphics::Plotter2DObj*)obj);
                    return;
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_REFRESH:
                    {
                    return;
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_YIELDFOCUS:
                    {
                    return;
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_UP:
                    {
                    _moveUp((internals_graphics::Plotter2DObj*)obj);
                    return;
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_DOWN:
                    {
                    _moveDown((internals_graphics::Plotter2DObj*)obj);
                    return;
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_TOP:
                    {
                    _moveTop((internals_graphics::Plotter2DObj*)obj);
                    return;
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_BOTTOM:
                    {
                    _moveBottom((internals_graphics::Plotter2DObj*)obj);
                    return;
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_USERANGEX:
                    {
                    _useRangeX((internals_graphics::Plotter2DObj*)obj);
                    return;
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_USERANGEY:
                    {
                    _useRangeY((internals_graphics::Plotter2DObj*)obj);
                    return;
                    }
                case internals_graphics::Plotter2DObj::_REQUEST_USERANGEXY:
                    {
                    _useRangeXY((internals_graphics::Plotter2DObj*)obj);
					return;
					}
                case internals_graphics::Plotter2DObj::_REQUEST_FIXOBJECTWIN:
                    {
                    return;
                    }
                }
            MTOOLS_ERROR("Plotter2DWindow::objectCB, incorrect code!");
            }




}



/* end of file */

