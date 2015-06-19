/** @file rangemanager.cpp */
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


#include "misc/error.hpp"
#include "graphics/rangemanager.hpp"


namespace mtools
{

    namespace internals_graphics
    {

        const double RangeManager::PRECISIONDOUBLE = 1.0e-11;
        const double RangeManager::MAXDOUBLE = 1.0e300;
        const double RangeManager::MINDOUBLE = 1.0e-300;

        const int RangeManager::MAXLOCKTIME = 1000;


        RangeManager::RangeManager(mtools::fRect startRange, mtools::iVec2 winSize, bool fixedAspectRatio, double minValue, double maxValue, double precision) :
        _cbfun(nullptr), _data(nullptr), _data2(nullptr), _startRange(startRange), _range(startRange), _startWin(winSize), _winSize(winSize), _minValue(minValue), _maxValue(maxValue), _precision(precision), _fixedAR(fixedAspectRatio)
            {
                MTOOLS_ASSERT((winSize.X() > 0) && (winSize.Y() > 0));
                MTOOLS_ASSERT(minValue > 0.0);
                MTOOLS_ASSERT((maxValue >= minValue)&&(maxValue <= MAXDOUBLE));
                MTOOLS_ASSERT(precision > 0.0);
                _fixRange();
                if (!_rangeOK(startRange)) { _defaultrange(); }
                _startRange = _range;
                MTOOLS_ASSERT(_rangeOK(_range));
            }


        RangeManager::RangeManager(mtools::iVec2 winSize, bool fixedAspectRatio, double minValue, double maxValue, double precision) :
        _cbfun(nullptr), _data(nullptr), _data2(nullptr), _startRange(-1, 1, -1, 1), _range(-1, 1, -1, 1), _startWin(winSize), _winSize(winSize), _minValue(minValue), _maxValue(maxValue), _precision(precision), _fixedAR(fixedAspectRatio)
            {
                MTOOLS_ASSERT((winSize.X() > 0) && (winSize.Y() > 0));
                MTOOLS_ASSERT(minValue > 0.0);
                MTOOLS_ASSERT((maxValue >= minValue) && (maxValue <= MAXDOUBLE));
                MTOOLS_ASSERT(precision > 0.0);
                _defaultrange();
                _startRange = _range;
                MTOOLS_ASSERT(_rangeOK(_range));
            }


        RangeManager::RangeManager(const RangeManager & R) :
        _cbfun(R._cbfun), _data(R._data), _data2(R._data2), _startRange(R._startRange), _range(R._range), _startWin(R._startWin), _winSize(R._winSize), _minValue(R._minValue), _maxValue(R._maxValue), _precision(R._precision), _fixedAR((bool)R._fixedAR) {}


        RangeManager & RangeManager::operator=(const RangeManager & R)
            {
            std::lock_guard<std::recursive_timed_mutex> lock(_mut);
            if (&R == this) return(*this);
            _cbfun = R._cbfun;
            _data = R._data;
            _data2 = R._data2;
            _startRange = R._startRange;
            _range = R._range;
            _winSize = R._winSize;
            _startWin = R._startWin;
            _minValue = R._minValue;
            _maxValue = R._maxValue;
            _precision = R._precision;
            _fixedAR = (bool)R._fixedAR;
            return(*this);
            }


        RangeManager::~RangeManager() {}


        bool RangeManager::saveAsDefault()
            {
            if (!_mut.try_lock_for(std::chrono::milliseconds(MAXLOCKTIME))) return false;
            _startRange = _range;
            _startWin = _winSize;
            _mut.unlock();
            return true;
            }


        mtools::fRect RangeManager::getRange() const
            {
            return _range;
            }


        mtools::iVec2 RangeManager::getWinSize() const
            {
            return _winSize;
            }


        mtools::fRect RangeManager::getDefaultRange() const
            {
            return _startRange;
            }


        mtools::iVec2 RangeManager::getDefaultWinSize() const
            {
            return _startWin;
            }


        bool RangeManager::up()
            {
            if (!_mut.try_lock_for(std::chrono::milliseconds(MAXLOCKTIME))) return false;
            bool resok = true;
            mtools::fRect oldr = _range;
            double sh = _range.ly() / 20;
            _range.ymin += sh; _range.ymax += sh;
            _fixRange();
            if (!_rangeOK(_range)) { _range = oldr; }
            if (_range != oldr) { if (!rangeNotification(true, false, false)) { _range = oldr;  resok = false; } }
            MTOOLS_ASSERT(_rangeOK(_range));
            _mut.unlock();
            return resok;
            }



        bool RangeManager::down()
            {
                if (!_mut.try_lock_for(std::chrono::milliseconds(MAXLOCKTIME))) return false;
                bool resok = true;
                mtools::fRect oldr = _range;
                double sh = _range.ly() / 20;
                _range.ymin -= sh; _range.ymax -= sh;
                _fixRange();
                if (!_rangeOK(_range)) { _range = oldr; }
                if (_range != oldr) { if (!rangeNotification(true, false, false)) { _range = oldr;  resok = false; } }
                MTOOLS_ASSERT(_rangeOK(_range));
                _mut.unlock();
                return resok;
            }



        bool RangeManager::left()
            {
                if (!_mut.try_lock_for(std::chrono::milliseconds(MAXLOCKTIME))) return false;
                bool resok = true;
                mtools::fRect oldr = _range;
                double sh = _range.lx() / 20;
                _range.xmin -= sh; _range.xmax -= sh;
                _fixRange();
                if (!_rangeOK(_range)) { _range = oldr; }
                if (_range != oldr) { if (!rangeNotification(true, false, false)) { _range = oldr;  resok = false; } }
                MTOOLS_ASSERT(_rangeOK(_range));
                _mut.unlock();
                return resok;
            }



        bool RangeManager::right()
            {
                if (!_mut.try_lock_for(std::chrono::milliseconds(MAXLOCKTIME))) return false;
                bool resok = true;
                mtools::fRect oldr = _range;
                double sh = _range.lx() / 20;
                _range.xmin += sh; _range.xmax += sh;
                _fixRange();
                if (!_rangeOK(_range)) { _range = oldr; }
                if (_range != oldr) { if (!rangeNotification(true, false, false)) { _range = oldr;  resok = false; } }
                MTOOLS_ASSERT(_rangeOK(_range));
                _mut.unlock();
                return resok;
            }



        bool RangeManager::zoomIn()
            {
                if (!_mut.try_lock_for(std::chrono::milliseconds(MAXLOCKTIME))) return false;
                bool resok = true;
                mtools::fRect oldr = _range;
                double lx = _range.xmax - _range.xmin;
                double ly = _range.ymax - _range.ymin;
                _range.xmin = _range.xmin + (lx / 10.0); _range.xmax = _range.xmax - (lx / 10.0);
                _range.ymin = _range.ymin + (ly / 10.0); _range.ymax = _range.ymax - (ly / 10.0);
                _fixRange();
                if (!_rangeOK(_range)) { _range = oldr; }
                if (_range != oldr) { if (!rangeNotification(true, false, false)) { _range = oldr;  resok = false; } }
                MTOOLS_ASSERT(_rangeOK(_range));
                _mut.unlock();
                return resok;
            }


        bool RangeManager::zoomOut()
            {
                if (!_mut.try_lock_for(std::chrono::milliseconds(MAXLOCKTIME))) return false;
                bool resok = true;
                mtools::fRect oldr = _range;
                double lx = _range.xmax - _range.xmin; double ly = _range.ymax - _range.ymin;
                _range.xmin = _range.xmin - (lx / 8.0); _range.xmax = _range.xmax + (lx / 8.0);
                _range.ymin = _range.ymin - (ly / 8.0); _range.ymax = _range.ymax + (ly / 8.0);
                _fixRange();
                if (!_rangeOK(_range)) { _range = oldr; }
                if (_range != oldr) { if (!rangeNotification(true, false, false)) { _range = oldr;  resok = false; } }
                MTOOLS_ASSERT(_rangeOK(_range));
                _mut.unlock();
                return resok;
            }



        bool RangeManager::winSize(mtools::iVec2 newWinSize)
            {
                if (!_mut.try_lock_for(std::chrono::milliseconds(MAXLOCKTIME))) return false;
                bool resok = true;
                iVec2 oldsize = _winSize;
                mtools::fRect oldr = _range;
                double rx = (_range.lx()*newWinSize.X()) / (_winSize.X()*2);
                double ry = (_range.ly()*newWinSize.Y()) / (_winSize.Y()*2);
                double cx = (_range.xmin + _range.xmax) / 2;
                double cy = (_range.ymin + _range.ymax) / 2;
                _range.xmin = cx - rx;
                _range.xmax = cx + rx;
                _range.ymin = cy - ry;
                _range.ymax = cy + ry;
                _winSize = newWinSize;
                if (!_rangeOK(_range)) { _range = oldr; }
                _fixRange();
                if (!_rangeOK(_range)) { _defaultrange(); }
                bool chwin = ((oldsize != _winSize) ? true : false);
                bool chrange = ((_range != oldr) ? true : false);
                if (!rangeNotification(chrange, chwin, false)) { _range = oldr; _winSize = oldsize;  resok = false; }
                MTOOLS_ASSERT(_rangeOK(_range));
                _mut.unlock();
                return resok;
            }



        bool RangeManager::setRange(mtools::fRect newRange)
            {
                if (!_mut.try_lock_for(std::chrono::milliseconds(MAXLOCKTIME))) return false;
                bool resok = true;
                mtools::fRect oldr = _range;
                if (_fixedAR)
                    {
                    _range = newRange.fixedRatioEnclosingRect(_range.lx() / _range.ly());
                    }
                else
                    {
                    _range = newRange;
                    }
                _fixRange();
                if (!_rangeOK(_range)) { _range = oldr; }
                if (_range != oldr) { if (!rangeNotification(true, false, false)) { _range = oldr;  resok = false; } }
                MTOOLS_ASSERT(_rangeOK(_range));
                _mut.unlock();
                return resok;
            }


        bool RangeManager::setRangeSilently(mtools::fRect newRange, bool keepAspectRatio)
            {
                if (!_mut.try_lock_for(std::chrono::milliseconds(MAXLOCKTIME))) return false;
                mtools::fRect oldr = _range;
                if (keepAspectRatio)
                    {
                    _range = newRange.fixedRatioEnclosingRect(_range.lx() / _range.ly());
                    }
                else
                    {
                    _range = newRange;
                    }
                _fixRange();
                if (!_rangeOK(_range)) { _range = oldr; }
                MTOOLS_ASSERT(_rangeOK(_range));
                _mut.unlock();
                return true;
            }


        bool RangeManager::center(mtools::fVec2 center)
            {
                if (!_mut.try_lock_for(std::chrono::milliseconds(MAXLOCKTIME))) return false;
                bool resok = true;
                mtools::fRect oldr = _range;
                double lx = _range.lx();
                double ly = _range.ly();
                _range.xmin = center.X() - lx / 2.0; _range.xmax = center.X() + lx / 2.0;
                _range.ymin = center.Y() - ly / 2.0; _range.ymax = center.Y() + ly / 2.0;
                _fixRange();
                if (!_rangeOK(_range)) { _range = oldr; }
                if (_range != oldr) { if (!rangeNotification(true, false, false)) { _range = oldr;  resok = false; } }
                MTOOLS_ASSERT(_rangeOK(_range));
                _mut.unlock();
                return resok;
            }


        double RangeManager::ratio() const
            {
            MTOOLS_ASSERT(_range.lx()*_winSize.Y() > 0.0);
            MTOOLS_ASSERT(_range.ly()*_winSize.X() > 0.0);
            return (_range.lx()*_winSize.Y()) / (_range.ly()*_winSize.X());
            }


        bool RangeManager::fixedAspectRatio() const { return _fixedAR; }


        bool RangeManager::fixedAspectRatio(bool fix)
            {
            if (!_mut.try_lock_for(std::chrono::milliseconds(MAXLOCKTIME))) return false;
            bool resok = true;
            bool oldAR = _fixedAR;
            _fixedAR = fix;
            if (!rangeNotification(false, false, true)) { _fixedAR = oldAR;  resok = false; }
            _mut.unlock();
            return resok;
            }


        bool RangeManager::set1to1()
            {
                if (!_mut.try_lock_for(std::chrono::milliseconds(MAXLOCKTIME))) return false;
                bool resok = true;
                mtools::fRect oldr = _range;
                double xc = floor((_range.xmin + _range.xmax) / 2) + ((_winSize.X() % 2 == 0) ? 0.5 : 0.0);
                double yc = floor((_range.ymin + _range.ymax) / 2) + ((_winSize.Y() % 2 == 0) ? 0.5 : 0.0);
                double lx = ((double)_winSize.X());
                double ly = ((double)_winSize.Y());
                _range.xmin = xc - lx / 2.0; _range.xmax = xc + lx / 2.0;
                _range.ymin = yc - ly / 2.0; _range.ymax = yc + ly / 2.0;
                if (!_rangeOK(_range)) { _range = oldr; }
                if (_range != oldr) { if (!rangeNotification(true, false, false)) { _range = oldr;  resok = false; } }
                MTOOLS_ASSERT(_rangeOK(_range));
                _mut.unlock();
                return resok;
            }



        bool RangeManager::setRatio1()
            {
                if (!_mut.try_lock_for(std::chrono::milliseconds(MAXLOCKTIME))) return false;
                bool resok = true;
                mtools::fRect oldr = _range;
                mtools::fRect newr = _range.fixedRatioEnclosingRect(((double)_winSize.X()) / ((double)_winSize.Y()));
                if (_rangeOK(newr)) _range = newr;
                if (_range != oldr) { if (!rangeNotification(true, false, false)) { _range = oldr;  resok = false; } }
                MTOOLS_ASSERT(_rangeOK(_range));
                _mut.unlock();
                return resok;
            }



        bool RangeManager::reset()
            {
                if (!_mut.try_lock_for(std::chrono::milliseconds(MAXLOCKTIME))) return false;
                bool resok = true;
                mtools::fRect oldr = _range;
                _range = _startRange;
                double rx = (_range.lx()*_winSize.X()) / _startWin.X();
                double ry = (_range.ly()*_winSize.Y()) / _startWin.Y();
                _range.xmax = _range.xmin + rx;
                _range.ymin = _range.ymax - ry;
                _fixRange();
                if (!_rangeOK(_range)) { _range = oldr; }
                if (_range != oldr) { if (!rangeNotification(true, false, false)) { _range = oldr; resok = false; } }
                MTOOLS_ASSERT(_rangeOK(_range));
                _mut.unlock();
                return resok;
            }



        bool RangeManager::canonicalRange()
            {
                if (!_mut.try_lock_for(std::chrono::milliseconds(MAXLOCKTIME))) return false;
                bool resok = true;
                mtools::fRect oldr = _range;
                _defaultrange();
                if (_range != oldr) { if (!rangeNotification(true, false, false)) { _range = oldr; resok = false; } }
                MTOOLS_ASSERT(_rangeOK(_range));
                _mut.unlock();
                return resok;
            }



        fVec2 RangeManager::pixelToAbs(iVec2 pixpos) const { return _range.pixelToAbs(pixpos, _winSize); }


        iVec2 RangeManager::absToPix(fVec2 abspos) const { return _range.absToPixel(abspos, _winSize); }


        bool RangeManager::_rangeOK(mtools::fRect r)
            {
            if ((std::isnan(r.xmin)) || (std::isnan(r.xmax)) || (std::isnan(r.ymin)) || (std::isnan(r.ymax))) return false;
            if ((r.xmin <= -_maxValue) || (r.ymin <= -_maxValue) || (r.xmax >= _maxValue) || (r.ymax >= _maxValue)) return false;
            if ((r.lx() <= _minValue) || (r.lx() >= _maxValue)) return false;
            if ((r.ly() <= _minValue) || (r.ly() >= _maxValue)) return false;
            double vx = std::abs(r.xmin) + std::abs(r.xmax);
            if ((r.lx() / vx) < _precision) return false;
            double vy = std::abs(r.ymin) + std::abs(r.ymax);
            if ((r.ly() / vy) < _precision) return false;
            return true;
            }


        void RangeManager::_fixRange()
            {
            double ratio = (_range.lx()*_winSize.Y()) / (_range.ly()*_winSize.X());
            if ((ratio == 1.0) || (ratio<0.99) || (ratio>1.01)) return;
            mtools::fRect newr = _range.fixedRatioEnclosingRect(((double)_winSize.X()) / ((double)_winSize.Y()));
            if (_rangeOK(newr)) _range = newr;
            }


        void RangeManager::_defaultrange()
            {
            mtools::fRect oldr = _range;
            double xc = ((_winSize.X() % 2 == 0) ? 0.5 : 0.0);
            double yc = ((_winSize.Y() % 2 == 0) ? 0.5 : 0.0);
            double lx = ((double)_winSize.X());
            double ly = ((double)_winSize.Y());
            _range.xmin = xc - lx / 2.0; _range.xmax = xc + lx / 2.0;
            _range.ymin = yc - ly / 2.0; _range.ymax = yc + ly / 2.0;
            if (!_rangeOK(_range)) { _range = oldr; }
            }


        bool RangeManager::rangeNotification(bool changedRange, bool changedWinSize, bool changedFixAspectRatio)
            {
            if (_cbfun != nullptr) return _cbfun(_data, _data2, changedRange, changedWinSize, changedFixAspectRatio);
            return true;
            }


        void RangeManager::setNotificationCallback(pnotif cb, void * data, void * data2)
            {
             _cbfun = cb;
             _data = data;
             _data2 = data2;
            }


    }
}

/* end of file */



