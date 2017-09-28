/** @file plot2Dmap.hpp */
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


#include "../maths/vec.hpp"
#include "internal/plotter2Dobj.hpp"
#include "internal/drawable2Dobject.hpp"
#include "internal/rangemanager.hpp"
#include "interpolation.hpp"
#include "internal/plot2Dbasegraph.hpp"

#include <map>


namespace mtools
	{


	template<typename T1, typename T2, typename Compare, typename Alloc> class Plot2DMap;


	/**
	 * Factory function for creating Plot of std::map<TFloat,Tfloat> object.
	 *
	 * @param [in,out]	map	The map.
	 * @param   name        The name of the plot.
	 *
	 * @return	A Plot2DMap object encapsulating the map.
	 */
	template<typename T1,typename T2, typename Compare, typename Alloc>  Plot2DMap<T1,T2,Compare,Alloc>  makePlot2DMap(std::map<T1,T2,Compare,Alloc> & map, std::string name = "Map")
		{
		return Plot2DMap<T1, T2, Compare, Alloc>(map, name);
		}



	/**
	* Plot object for std::map objects.
	*
	**/
	template<typename T1, typename T2, typename Compare, typename Alloc> class  Plot2DMap : public internals_graphics::Plot2DBaseGraphWithInterpolation
		{

		public:


			/**
			 * Constructor.
			 *
			 * @param [in,out]	map	The map to plot.
			 * @param	name	   	The name of the map.
			 */
			Plot2DMap(std::map<T1, T2, Compare, Alloc> & map, std::string name = "Map")  : Plot2DBaseGraphWithInterpolation(name), _pmap(&map)
				{
				_minDomain = +std::numeric_limits<double>::infinity();
				_maxDomain = -std::numeric_limits<double>::infinity();
				if (_pmap->size() > 0)
					{
					_minDomain = (_pmap->begin())->first;
					_maxDomain = (_pmap->rbegin())->first;
					}
				}


			/**
			* Move Constructor.
			**/
			Plot2DMap(Plot2DMap && obj) : Plot2DBaseGraphWithInterpolation(std::move(obj)), _pmap(obj._pmap)
				{
				}


			/**
			* Destructor. Remove the object if it is still inserted.
			**/
			virtual ~Plot2DMap()
				{
				detach(); // detach from its owner if there is still one. 
				}


		protected:

			/**
			* Get the value of the function at x, return a quiet NAN if x is not in the definiton domain.
			**/
			virtual double _function(double x) const override
				{
				if (_pmap->size() == 0)	{ return  std::numeric_limits<double>::quiet_NaN(); }
				_minDomain = (_pmap->begin())->first;
				_maxDomain = (_pmap->rbegin())->first;
				if ((x < _minDomain)||(x > _maxDomain)) { return  std::numeric_limits<double>::quiet_NaN(); }
				
				auto it2 = _pmap->lower_bound(x);
				if (it2 == _pmap->end()) { return  std::numeric_limits<double>::quiet_NaN(); }
				if (it2 == _pmap->begin()) { if (x < (double)(it2->first)) { return  std::numeric_limits<double>::quiet_NaN(); } else { return it2->second; } }
				
				double x2 = (double)it2->first;
				double y2 = (double)it2->second;
				auto it1 = it2; it1--;
				double x1 = (double)it1->first;
				double y1 = (double)it1->second;

				int t = interpolationMethod();
				if (t == INTERPOLATION_NONE) { return y1; }
				if (t == INTERPOLATION_LINEAR) { return linearInterpolation(x, fVec2(x1, y1), fVec2(x2, y2)); }

				double x0 = x1 - 1.0;
				double x3 = x2 + 1.0;
				double y0 = std::numeric_limits<double>::quiet_NaN();
				double y3 = std::numeric_limits<double>::quiet_NaN();
				if (it1 != _pmap->begin())
					{
					it1--;
					double x0 = (double)it1->first;
					double y0 = (double)it1->second;
					}
				it2++;
				if (it2 != _pmap->end())
					{
					double x3 = (double)it2->first;
					double y3 = (double)it2->second;
					}
				if (t == INTERPOLATION_CUBIC) return cubicInterpolation(x, fVec2(x0, y0), fVec2(x1, y1), fVec2(x2, y2), fVec2(x3, y3));
				return monotoneCubicInterpolation(x, fVec2(x0, y0), fVec2(x1, y1), fVec2(x2, y2), fVec2(x3, y3));

				/*
				if ((_tab == nullptr) || (_len == 0)) return std::numeric_limits<double>::quiet_NaN();
				if (!((x >= _minDomain) && (x <= _maxDomain))) return std::numeric_limits<double>::quiet_NaN();
				double _e = (_maxDomain - _minDomain) / _len;
				if (!((_e >= DBL_MIN * 2) && (_e <= DBL_MAX / 2.0))) return std::numeric_limits<double>::quiet_NaN();
				int t = interpolationMethod();
				if (t == INTERPOLATION_NONE)
					{
					size_t n = (size_t)((x - _minDomain) / _e);
					if (n >= _len) return std::numeric_limits<double>::quiet_NaN();
					double y;
					try { y = (double)_tab[n]; }
					catch (...) { y = std::numeric_limits<double>::quiet_NaN(); }
					return y;
					}
				if (t == INTERPOLATION_LINEAR)
					{
					size_t n = (size_t)((x - _minDomain) / _e);
					if (n >= _len) std::numeric_limits<double>::quiet_NaN();
					double x1 = _minDomain + n*_e;
					double x2 = x1 + _e;
					double y1 = std::numeric_limits<double>::quiet_NaN();
					double y2 = std::numeric_limits<double>::quiet_NaN();
					try { y1 = (double)_tab[n]; }
					catch (...) { y1 = std::numeric_limits<double>::quiet_NaN(); }
					try { y2 = (n + 1 >= _len) ? (std::numeric_limits<double>::quiet_NaN()) : ((double)_tab[n + 1]); }
					catch (...) { y2 = std::numeric_limits<double>::quiet_NaN(); }
					return linearInterpolation(x, fVec2(x1, y1), fVec2(x2, y2));
					}
				size_t n = (size_t)((x - _minDomain) / _e);
				if (n >= _len) std::numeric_limits<double>::quiet_NaN();
				double x1 = _minDomain + n*_e;
				double x0 = x1 - _e;
				double x2 = x1 + _e;
				double x3 = x2 + _e;
				double y0 = std::numeric_limits<double>::quiet_NaN();
				double y1 = std::numeric_limits<double>::quiet_NaN();
				double y2 = std::numeric_limits<double>::quiet_NaN();
				double y3 = std::numeric_limits<double>::quiet_NaN();
				try { y0 = (n == 0) ? (std::numeric_limits<double>::quiet_NaN()) : ((double)_tab[n - 1]); }
				catch (...) { y0 = std::numeric_limits<double>::quiet_NaN(); }
				try { y1 = (double)_tab[n]; }
				catch (...) { y1 = std::numeric_limits<double>::quiet_NaN(); }
				try { y2 = (n + 1 >= _len) ? (std::numeric_limits<double>::quiet_NaN()) : ((double)_tab[n + 1]); }
				catch (...) { y2 = std::numeric_limits<double>::quiet_NaN(); }
				try { y3 = (n + 2 >= _len) ? (std::numeric_limits<double>::quiet_NaN()) : ((double)_tab[n + 2]); }
				catch (...) { y3 = std::numeric_limits<double>::quiet_NaN(); }
				if (t == INTERPOLATION_CUBIC) return cubicInterpolation(x, fVec2(x0, y0), fVec2(x1, y1), fVec2(x2, y2), fVec2(x3, y3));
				return monotoneCubicInterpolation(x, fVec2(x0, y0), fVec2(x1, y1), fVec2(x2, y2), fVec2(x3, y3));
				*/
				}


		protected:

			mutable std::map<T1, T2, Compare, Alloc> * _pmap;

		};



	}


/* end of file */



