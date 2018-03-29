/** @file bseg.hpp */
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

#include "../mtools_config.hpp"
#include "../../misc/error.hpp"
#include "../../misc/misc.hpp"
#include "../../maths/box.hpp"
#include "../../maths/vec.hpp"


namespace mtools
{


namespace internals_bseg
    {




	/** Bresenham segment */
	struct BSeg
		{

		/** Construct segment from P1 to P2 (integer-valued positions) */
		BSeg(const iVec2 & P1, const iVec2 & P2) { init(P1, P2); }


		/** Construct segment from P1 to P2 (real-valued positions) **/
		BSeg(const fVec2 & Pf1, const fVec2 & Pf2) { init(Pf1, Pf2); }


		/** Reverse the segment **/
		inline void reverse()
			{
			const int64 len = _len;
			if (len > 0)
				{ 
				move(len);		// move to the other endpoint
				_len = len;		// reset the lenght
				}
			// reverse
			_stepx *= -1; _stepy *= -1;
			if (_x_major) { _frac = -_dx - 1 - _frac; _frac += 2 * _dy; } else { _frac = -_dy - 1 - _frac; _frac += 2 * _dx; }
			}


		/** Return the reversed segment **/
		MTOOLS_FORCEINLINE BSeg get_reverse() const
			{
			BSeg tmp(*this);
			tmp.reverse();
			return tmp; 
			}


		/** move on the line by one pixel. tmeplate version (for speed) */
		template<bool X_MAJOR> MTOOLS_FORCEINLINE void move()
			{
			_len--;
			if (X_MAJOR)
				{
				if (_frac >= 0) { _y += _stepy; _frac -= _dx; } 
				_x += _stepx; _frac += _dy;
				}
			else
				{
				if (_frac >= 0) { _x += _stepx; _frac -= _dy; }	
				_y += _stepy; _frac += _dx;
				}
			}


		/** Same as above but not templated **/
		MTOOLS_FORCEINLINE void move()
			{
			if (_x_major) move<true>(); else move<false>();
			}


		/**
		* Move the position pos by totlen pixels along a bresenham line.
		*/
		inline void move(int64 totlen)
			{
			_len -= totlen;
			MTOOLS_ASSERT(totlen >= 0);
			int64 len = safeMultB(std::max<int64>(_dx, _dy), totlen);
			while (1)
				{
				if (_x_major)
					{
					if (_dx == 0) return;
					_x += _stepx*len;
					_frac += _dy*len;
					int64 u = _frac / _dx;
					_y += _stepy*u;
					_frac -= u * _dx;
					if (_frac >= _dy) { _frac -= _dx; _y += _stepy; }
					}
				else
					{
					if (_dy == 0) return;
					_y += _stepy*len;
					_frac += _dx*len;
					int64 u = _frac / _dy;
					_x += _stepx*u;
					_frac -= u * _dy;
					if (_frac >= _dx) { _frac -= _dy; _x += _stepx; }
					}
				totlen -= len;
				if (totlen <= 0) { MTOOLS_ASSERT(totlen == 0); return; }
				if (totlen < len) { len = totlen; }
			}
		}


		/**
		* Move the position pos by 1 pixel horizontally along the given bresenham line.
		* return the number of pixel traveled by the bresenham line.
		*
		* [x_major can be deduced from linedir but is given as template paramter for speed optimization]
		*/
		template<bool X_MAJOR> MTOOLS_FORCEINLINE int64 move_x_dir()
			{
			MTOOLS_ASSERT(_x_major == X_MAJOR);  // make sure template argument supplied is correct. 
			MTOOLS_ASSERT(_dx > 0);				 // not a vertical line
			if (X_MAJOR) // compiler optimizes away the template conditionals
				{
				if (_frac >= 0) { _y += _stepy; _frac -= _dx; }
				_x += _stepx; _frac += _dy;
				_len--;
				return 1;
				}
			else
				{
				int64 r = (_frac < ((_dx << 1) - _dy)) ? _rat : ((_dx - _frac) / _dx); // use rat value if just after a step.
				_y += (r*_stepy);
				_frac += (r*_dx);
				if (_frac < _dx) { _y += _stepy; _frac += _dx; r++; }
				MTOOLS_ASSERT((_frac >= _dx) && (_frac < 2 * _dx));
				_frac -= _dy;  _x += _stepx;
				_len -= r;
				return r;
				}
			}


		/** Same as above but not templated **/
		MTOOLS_FORCEINLINE int64 move_x_dir() 
			{
			return (_x_major ? move_x_dir<true>() : move_x_dir<false>());
			}


		/**
		* Move the position pos by a given number of pixels horizontal along a bresenham line.
		* return the number of pixel traveled by the bresenham line.
		* do nothing and return 0 if lenx <= 0.
		*/
		inline int64 move_x_dir(int64 totlenx)
			{
			if (totlenx <= 0) return 0;
			MTOOLS_ASSERT(_dx > 0); // not a vertical line
			int64 lenx = safeMultB(std::max<int64>(_dx, _dy), totlenx);
			int64 res = 0;
			while (1)
				{
				if (_x_major)
					{
					_x += _stepx*lenx;
					_frac += _dy*lenx;
					int64 u = _frac / _dx;
					_y += _stepy*u;
					_frac -= u * _dx;
					if (_frac >= _dy) { _frac -= _dx; _y += _stepy; }
					res += lenx;
					}
				else
					{
					int64 k = ((lenx - 1)*_dy) / _dx;
					_frac += k * _dx;
					_y += k * _stepy;
					int64 u = _frac / _dy;
					_frac -= u * _dy;
					if (_frac >= _dx) { u++; _frac -= _dy; }
					MTOOLS_ASSERT((u <= lenx) && (u >= lenx - 4));
					_x += u * _stepx;
					while (u != lenx) { k += move_x_dir<false>(); u++; }
					res += k;
					}
				totlenx -= lenx;
				if (totlenx <= 0) { MTOOLS_ASSERT(totlenx == 0); _len -= res; return res; }
				if (totlenx < lenx) { lenx = totlenx; }
				}
			}



		/**
		* Move the position pos by one pixel vertically along the given bresenham line.
		* return the number of pixel traveled by the bresenham line.
		*
		* [x_major can be deduced from linedir but given as template paramter for speed optimization]
		*/
		template<bool X_MAJOR> MTOOLS_FORCEINLINE int64 move_y_dir()
			{
			MTOOLS_ASSERT(_x_major == X_MAJOR);  // make sure tempalte argument supplied is correct. 
			MTOOLS_ASSERT(_dy > 0); // not an horizontal line
			if (X_MAJOR) // compiler optimizes away the template conditionals
				{
				int64 r = (_frac < ((_dy << 1) - _dx)) ? _rat : ((_dy - _frac) / _dy); // use rat value if just after a step.
				_x += (r*_stepx);
				_frac += (r*_dy);
				if (_frac < _dy) { _x += _stepx; _frac += _dy; r++; }
				MTOOLS_ASSERT((_frac >= _dy) && (_frac < 2 * _dy));
				_frac -= _dx;  _y += _stepy;
				_len -= r;
				return r;
				}
			else
				{
				if (_frac >= 0) { _x += _stepx; _frac -= _dy; }
				_y += _stepy; _frac += _dx;
				_len--;
				return 1;
				}
			}


		/** Same as above but not templated **/
		MTOOLS_FORCEINLINE int64 move_y_dir()
			{
			return (_x_major ? move_y_dir<true>() : move_y_dir<false>());
			}


		/**
		* Move the position pos along a line by a given number of pixels vertically along a bresenham line.
		* return the number of pixel traveled by the bresenham line.
		* do nothing and return 0 if leny <= 0.
		*/
		inline int64 move_y_dir(int64 totleny)
			{
			if (totleny <= 0) return 0;
			MTOOLS_ASSERT(_dy > 0); // not an horizontal line
			int64 leny = safeMultB(std::max<int64>(_dx, _dy), totleny);
			int64 res = 0;
			while (1)
				{
				if (_x_major) // compiler optimizes away the template conditionals
					{
					int64 k = ((leny - 1)*_dx) / _dy;
					_frac += k * _dy;
					_x += k * _stepx;
					int64 u = _frac / _dx;
					_frac -= u * _dx;
					if (_frac >= _dy) { u++; _frac -= _dx; }
					MTOOLS_ASSERT((u <= leny) && (u >= leny - 4));
					_y += u * _stepy;
					while (u != leny) { k += move_y_dir<true>(); u++; }
					res += k;
					}
				else
					{
					_y += _stepy*leny;
					_frac += _dx*leny;
					int64 u = _frac / _dy;
					_x += _stepx*u;
					_frac -= u * _dy;
					if (_frac >= _dx) { _frac -= _dy; _x += _stepx; }
					res += leny;
					}
				totleny -= leny;
				if (totleny <= 0) { MTOOLS_ASSERT(totleny == 0); _len -= res; return res; }
				if (totleny < leny) { leny = totleny; }
				}
			}


		/**
		* Move the position until it is in the closed box B. Return the number of step performed by the
		* walk or -1 if the line never enters the box. (in this case, pos may be anywhere.)
		**/
		inline int64 move_inside_box(const iBox2 & B)
			{
			if (B.isEmpty()) return -1;
			if (B.isInside({ _x,_y })) return 0;
			int64 tot = 0;
			if (_x < B.min[0])
				{
				if ((_stepx < 0) || (_dx == 0)) { _len = -1; return -1; }
				tot += move_x_dir(B.min[0] - _x);
				}
			else if (_x > B.max[0])
				{
				if ((_stepx > 0) || (_dx == 0)) { _len = -1; return -1; }
				tot += move_x_dir(_x - B.max[0]);
				}
			if (_y < B.min[1])
				{
				if ((_stepy < 0) || (_dy == 0)) { _len = -1; return -1; }
				tot += move_y_dir(B.min[1] - _y);
				}
			else if (_y > B.max[1])
				{
				if ((_stepy > 0) || (_dy == 0)) { _len = -1; return -1; }
				tot += move_y_dir(_y - B.max[1]);
				}
			if (!B.isInside(iVec2{ _x, _y })) { _len = -1; return -1; }
			return tot;
			}


		/**
		* Compute number of pixel of the line before it exits the box B. If the box is empty of if pos
		* is not in it, return 0.
		**/
		inline int64 lenght_inside_box(const iBox2 & B) const
			{
			if (!B.isInside({ _x, _y })) return 0;
			const int64 hx = 1 + ((_stepx > 0) ? (B.max[0] - _x) : (_x - B.min[0])); // number of horizontal step before exit. 
			const int64 hy = 1 + ((_stepy > 0) ? (B.max[1] - _y) : (_y - B.min[1])); // number of vertical step before exit. 
			int64 nx = -1, ny = -1;
			if (_dx != 0) { BSeg tmp(*this); nx = tmp.move_x_dir(hx); }
			if (_dy != 0) { BSeg tmp(*this); ny = tmp.move_y_dir(hy); }
			if (nx == -1) { return ny; }
			if (ny == -1) { return nx; }
			return std::min<int64>(nx, ny);
			}


		/** Initialize with integer-valued positions */
		void init(iVec2 P1, iVec2 P2)
			{
			const int64 EXP = 10;
			if (P1 == P2)
				{ // default horizontal line
				MTOOLS_DEBUG("P1 = P2 : default horizontal line.");
				_x_major = true;
				_dx = 2;
				_dy = 0;
				_stepx = 1;
				_stepy = 1;
				_rat = 0;
				_amul = ((int64)1 << 60) / 2;
				_x = P1.X();
				_y = P1.Y();
				_frac = -2;
				_len = 0;
				return;
				}
			int64 dx = P2.X() - P1.X(); if (dx < 0) { dx = -dx;  _stepx = -1; }
			else { _stepx = 1; } dx <<= EXP; _dx = dx;
			int64 dy = P2.Y() - P1.Y(); if (dy < 0) { dy = -dy;  _stepy = -1; }
			else { _stepy = 1; } dy <<= EXP; _dy = dy;
			if (dx >= dy) { _x_major = true; _rat = (dy == 0) ? 0 : (dx / dy); }
			else { _x_major = false; _rat = (dx == 0) ? 0 : (dy / dx); }
			_x = P1.X(); _y = P1.Y();
			int64 flagdir = (P2.X() > P1.X()) ? 1 : 0; // used to compensante frac so that line [P1,P2] = [P2,P1]. 
			_frac = ((_x_major) ? (dy - (dx >> 1)) : (dx - (dy >> 1))) - flagdir;
			_amul = ((int64)1 << 60) / (_x_major ? _dx : _dy);
			_len = ((_x_major ? dx : dy) >> EXP);
			}


		/** Construct segment from P1 to P2 (real-valued positions) */
		void init(fVec2 Pf1, fVec2 Pf2)
			{
			const int64 PRECISION = 1024 * 16; // 512 * 128;
			bool sw = false;
			if ((Pf1.X() > Pf2.X()) || ((Pf1.X() == Pf2.X()) && (Pf1.Y() > Pf2.Y()))) { sw = true; mtools::swap(Pf1, Pf2); }
			iVec2 P1 = round(Pf1); 
			iVec2 P2 = round(Pf2);
			_x = P1.X();
			_y = P1.Y();
			const int64 adx = abs(P2.X() - P1.X());
			const int64 ady = abs(P2.Y() - P1.Y());			
			const double fdx = (Pf2.X() - Pf1.X());
			const double fdy = (Pf2.Y() - Pf1.Y());
			_len = (adx > ady) ? adx : ady;
			if (adx == ady)
				{ // edge cases, TODO: better. 
				if (sw) { mtools::swap(P1, P2); }
				init(P1, P2);
				return;
				}
			else
				{
				if (adx > ady)
					{ // x major
					_x_major = true;
					const double mul = fdy / fdx;
					double f1 = mul * (P1.X() - Pf1.X()) + Pf1.Y() - P1.Y(); // how much above
					double f2 = mul * (P2.X() - Pf2.X()) + Pf2.Y() - P2.Y(); // how much below
					int64 if1 = (int64)((2 * PRECISION) * f1); if (if1 <= -PRECISION) { if1 = -PRECISION + 1; } else if (if1 >= PRECISION) { if1 = PRECISION - 1; }
					int64 if2 = (int64)((2 * PRECISION) * f2); if (if2 <= -PRECISION) { if2 = -PRECISION + 1; } else if (if2 >= PRECISION) { if2 = PRECISION - 1; }
					if (fdx < 0) { _stepx = -1; } else { _stepx = +1; }
					if (fdy < 0) { _stepy = -1;  if1 = -if1; if2 = -if2; } else { _stepy = +1; }
					_dx = adx * (2 * PRECISION);
					_dy = ady * (2 * PRECISION); _dy += -if1 + if2;
					MTOOLS_ASSERT(_dy >= 0);
					MTOOLS_ASSERT(_dy <= _dx);
					_rat = (_dy == 0) ? 0 : (_dx / _dy);
					_amul = ((int64)1 << 60) / _dx;
					_frac = (if1 - PRECISION)*adx + _dy;
					}
				else
					{ // y major
					_x_major = false;
					const double mul = fdx / fdy;
					double f1 = mul * (P1.Y() - Pf1.Y()) + Pf1.X() - P1.X();
					double f2 = mul * (P2.Y() - Pf2.Y()) + Pf2.X() - P2.X();
					int64 if1 = (int64)((2 * PRECISION) * f1); if (if1 <= -PRECISION) { if1 = -PRECISION + 1; } else if (if1 >= PRECISION) { if1 = PRECISION - 1; }
					int64 if2 = (int64)((2 * PRECISION) * f2); if (if2 <= -PRECISION) { if2 = -PRECISION + 1; } else if (if2 >= PRECISION) { if2 = PRECISION - 1; }
					if (fdx < 0) { _stepx = -1;  if1 = -if1; if2 = -if2; } else { _stepx = +1; }
					if (fdy < 0) { _stepy = -1; } else { _stepy = +1; }
					_dy = ady * (2 * PRECISION);
					_dx = adx * (2 * PRECISION); _dx += -if1 + if2;
					MTOOLS_ASSERT(_dx >= 0);
					MTOOLS_ASSERT(_dx <= _dy);
					_rat = (_dx == 0) ? 0 : (_dy / _dx);
					_amul = ((int64)1 << 60) / _dy;
					_frac = (if1 - PRECISION)*ady + _dx;
					}
				}
			if (sw)
				{
				mtools::swap(P1, P2);
				reverse();
				MTOOLS_ASSERT(_x == P1.X());
				MTOOLS_ASSERT(_y == P1.Y());
				}
			}


		/* Compute the aa value on a given side */
		template<bool SIDE, bool X_MAJOR>  MTOOLS_FORCEINLINE int32 AA2() const
			{
			MTOOLS_ASSERT(X_MAJOR == _x_major)
			int64 a;
			if (X_MAJOR)
				{
				a = _dy;
				a = (((a - _frac)*_amul) >> 52);
				if (SIDE) { if (_stepx != _stepy) a = 256 - a; } else { if (_stepx == _stepy) a = 256 - a; }
				}
			else
				{
				a = _dx;
				a = (((a - _frac)*_amul) >> 52);
				if (SIDE) { if (_stepx == _stepy) a = 256 - a; } else { if (_stepx != _stepy) a = 256 - a; }
				}
			a = (a >> 2) + (a >> 1) + 32; // compensate
			MTOOLS_ASSERT((a >= 0) && (a <= 256));
			return (int32)a;
			}


		/* Compute the aa value on a given side */
		template<bool SIDE>  MTOOLS_FORCEINLINE int32 AA1() const
			{
			return ((_x_major) ? AA2<SIDE, true>() : AA2<SIDE, false>());
			}


		/**
		 * Query if the line is x_major
		 */
		MTOOLS_FORCEINLINE bool x_major() const { return _x_major; }


		/**
		* Query the remaining distance to the endpoind
		*/
		MTOOLS_FORCEINLINE bool len() const { return _len; }


		/**
		* Query the current position on the line
		*/
		MTOOLS_FORCEINLINE iVec2 pos() const { return {_x,_y}; }


		int64 _x, _y;			// current pos
		int64 _frac;			// fractional part
		int64 _len;				// number of pixels

		int64 _dx, _dy;			// step size in each direction
		int64 _stepx, _stepy;	// directions (+/-1)
		int64 _rat;				// ratio max(dx,dy)/min(dx,dy) to speed up computations
		int64 _amul;			// multiplication factor to compute aa values. 
		bool  _x_major;			// true if the line is xmajor (ie dx > dy) and false if y major (dy >= dx).
		};


		/**
		* Return the number of pixels that compose the Bressenham segment [P,Q[.
		*
		* closed = true to compute the lenght of [P,Q] and false for [P,Q[.
		**/
		inline int64 length(const iVec2 & P, const iVec2 & Q, bool closed = false)
			{
			return std::max<int64>(abs(P.X() - Q.X()), abs(P.Y() - Q.Y())) + (closed ? 1 : 0);
			}


		/**
		* Compute the position of the next pixel after P on a Bresenham segment [P,Q].
		* if P = Q (no line), return P.
		**/
		inline iVec2 nextPos(const iVec2 & P, const iVec2 & Q)
			{
			if (P == Q) return P;
			BSeg seg(P, Q);
			seg.move();
			return seg.pos();
			}


    }


}


/* end of file */








