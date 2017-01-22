/** @file circlePacking.hpp */
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

#include "../misc/misc.hpp" 
#include "../misc/stringfct.hpp" 
#include "../misc/error.hpp"
#include "vec.hpp"
#include "box.hpp"
#include "permutation.hpp"
#include "../random/gen_fastRNG.hpp"
#include "../random/classiclaws.hpp"
#include "../graphics/customcimg.hpp"

namespace mtools
	{
	

	/**
	* Class used for computing the circle packing of a triangulation.
	*
	* @tparam	GRAPH   Type of the graph, typically std::vector< std::vector<int> >.
	* 					- The outside container must be accessible via operator[].
	* 					- The inside container must accept be iterable and contain
	*                    elements convertible to size_t (corresponding to the indexes
	*                    the neighour vertices).
	*/
	class CirclePacking
		{

		public:

			/* ctor, empty object */
			CirclePacking() {}


			/* dtor, empty object */
			~CirclePacking() {}


			/** Clears the object to a blank initial state. */
			void clear()
				{
				_gr.clear();
				_perm.clear();
				_invperm.clear();
				_i0 = 0;
				_rad.clear();
				_circle.clear();
				_rect.clear();
				}


			/**
			* Loads a triangulation and define the boundary vertices.
			* This clear all previous computation.
			*
			* @param graph		The triangulation with boundary (must be 3 connected).
			* @param boundary  The boundary vector. Every index i for which boundary[i] > 0
			* 					is considered to be a boundary vertice.
			*/
			template<typename GRAPH> void setTriangulation(const GRAPH & graph, const std::vector<int> & boundary)
				{ 
				clear();
				const size_t l = graph.size();
				MTOOLS_INSURE(boundary.size() == l);
				_perm = getSortPermutation(boundary);
				_invperm = invertPermutation(_perm);
				_gr = permuteGraph<std::vector<std::vector<int> > >(convertGraph<GRAPH, std::vector<std::vector<int> > >(graph), _perm, _invperm);
				_i0 = l;
				for (size_t i = 0; i < l; i++)
					{
					if (boundary[_perm[i]] > 0.0) { _i0 = i; break; }
					}
				MTOOLS_INSURE((_i0 > 0)&&(_i0 < l-2));
				}


			/** Compute the error in the circle radius in L2 norm. */
			double errorL2() const
				{
				double e = 0.0;
				for (size_t i = 0; i < _i0; ++i)
					{
					const double v = _rad[i];
					const double c = angleSumEuclidian(v, _gr[i]) - M_2PI;
					e += c*c;
					}
				return sqrt(e);
				}

			/** Compute the error in the circle radius in L1 norm. */
			double errorL1() const
				{
				double e = 0.0;
				for (size_t i = 0; i < _i0; ++i)
					{
					const double v = _rad[i];
					const double c = angleSumEuclidian(v, _gr[i]) - M_2PI;
					e += fabs(c);
					}
				return e;
				}


			/**
			 * Sets the radii of the circle around each vertices. The radii associated with the boundary
			 * vertices are not modified during the circle packing algorithm.
			 *
			 * @param	rad	The radii. Any values < 0 is set to 1.0.
			 **/
			void setRadii(const std::vector<double> & rad)
				{
				const size_t l = _gr.size();
				MTOOLS_INSURE(rad.size() == l);
				_rad = permute(rad, _perm);
				for (size_t i = 0; i < l; i++)
					{
					if (_rad[i] <= 0.0) _rad[i] = 1.0;
					}
				}


			/** Set all radii to r. **/
			void setRadii(double r = 1.0)
				{
				MTOOLS_INSURE(r > 0.0);
				const size_t l = _gr.size();
				_rad.resize(l);
				for (size_t i = 0; i < l; i++) { _rad[i] = r; }
				}


			/**
			* Compute the radii for circle packing the triangulation. After the method returns, the radii
			* may be queried via getRadii().
			* 
			* Version with acceleration.
			*
			* @param	precision	the required precision, in L2 norm for the total angle sum.
			*
			* @return	The number of iterations performed.
			**/
			int64 computeRadii(const double eps = 10e-9,const double delta = 0.05)
				{

				FastRNG gen;

				#define DELAY_POST_RADII		// uncomment those line for debugging
				//#define DO_NOT_USE_RNG		//

				const size_t i0 = _i0;
				int64 iter = 0;
				double c = 1.0, c0;
				double lambda = -1.0, lambda0;
				bool fl = false, fl0;
				std::vector<double> _rad0 = _rad;
				while (c > eps)
					{
					iter++;
					c0 = c;
					c = 0.0;
					lambda0 = lambda;
					fl0 = fl;
					#ifndef DELAY_POST_RADII
					memcpy(_rad0.data(), _rad.data(), _rad.size()*sizeof(double));	
					#endif
					for (size_t i = 0; i < i0; ++i)
						{
						const double v = _rad[i];
						const double theta = angleSumEuclidian(v, _gr[i]);
						const double k = (double)_gr[i].size();
						const double beta = sin(theta*0.5 / k);
						const double tildev = beta*v / (1.0 - beta);
						const double delta = sin(M_PI / k);
						const double u = (1.0 - delta)*tildev / delta;
						const double e = theta - M_2PI;
						c += e*e;
						#ifdef DELAY_POST_RADII
						_rad0[i] = u;
						#else
						_rad[i] = u;
						#endif
						}
					#ifdef DELAY_POST_RADII
					_rad.swap(_rad0);
					#endif
					c = sqrt(c); 
					lambda = c / c0;
					fl = true;					
					if ((fl0) && (lambda < 1.0))
						{
						if (abs(lambda - lambda0) < delta) 
							{ 
							lambda = lambda / (1.0 - lambda); 
							}
						double lstar = 3*lambda;
						for (size_t i = 0; i < i0; ++i) 
							{
							double d = _rad0[i] - _rad[i];
							if (d > 0.0)
								{
								double d2 = (_rad[i] / d);
								if (d2 < lstar) { lstar = d2; }
								}
							}

						lambda = std::min<double>(lambda, 0.5*lstar);
						
						#ifdef DO_NOT_USE_RNG
						for (size_t i = 0; i < i0; ++i) { _rad[i] += lambda*(_rad[i] - _rad0[i]); }
						fl = 0;
						#else
						if (Unif(gen) > 0.5) 
							{
							for (size_t i = 0; i < i0; ++i) { _rad[i] += lambda*(_rad[i] - _rad0[i]); } 
							fl = 0;
							}
						#endif ENDIF

						}
					/*
					if (iter <= 4)
						{
						cout << "*************************************************************************************************************************************\n";
						cout << iter - 1 << "\n";
						cout << "*************************************************************************************************************************************\n";

						for (int i = 0;i < _rad.size(); i++)
							{
							cout << i << "   " << _rad[i] << "\n";
							}

						}
					*/

					}
				return iter;

				#undef DO_NOT_USE_RNG
				#undef DELAY_POST_RADII 
				}


			/**
			* Return the list of radii.
			*
			* @return	The radii of the circles around each vertices.
			*/
			std::vector<double> getRadii() const { return permute(_rad, _invperm); }


			/**
			 * Compute the positions of the circles.
			 *
			 * @param	i1	index of the vertex at (0,0). Must be an interior vertice. Set to <0 value for
			 * 				arbitrary vertex.
			 * @param	i2	index of the vertex at (r,0) on the right of the origin circle. Must be adjacent
			 * 				to i1. set to <0 value for arbitrary vertex.
			 *
			 * @return	The enclosing rectangle of the circle packing
			 **/
			mtools::fBox2 computeLayout(int i1 = -1, int i2 = -1)
				{
				if (i1 < 0)
					{
					i1 = 0;						// the first one, it is not a boundary vertice
					i2 = _gr[i1].front();		// its first neighour
					}
				else
					{
					i1 = _invperm[i1];			// apply the permutation
					MTOOLS_INSURE(i1 < _i0);	// check it is not a obundary vertex
					if (i2 < 0)
						{
						i2 = _gr[i1].front();	// choose i1 first neighour. 
						}
					else
						{
						i2 = _invperm[i2];
						bool fl = false;
						for (auto it = _gr[i1].begin(); it != _gr[i1].end(); ++it)
							{
							if (*it == i2) { fl = true; break; }
							}
						if (!fl) { MTOOLS_ERROR("i2 is not a neighour of i1"); }
						}
					}
				// i1 and i2 are good, we can start the layout
				_circle.resize(_rad.size());
				_circle[i1] = mtools::fVec2(0.0, 0.0);
				_circle[i2] = mtools::fVec2(_rad[i1] + _rad[i2], 0.0);

				mtools::fBox2 R;
				R.min[0] = -_rad[i1];
				R.max[0] = _rad[i1] + 2 * _rad[i2];
				R.max[1] = std::max<double>(_rad[i1], _rad[i2]);
				R.min[1] = -R.max[1];

				std::vector<int> doneCircle;
				doneCircle.resize(_rad.size());
				doneCircle[i1] = 1;
				doneCircle[i2] = 1;
				std::queue<int> st;
				st.push(i1);
				if (i2 < _i0) st.push(i2);

				while (st.size() != 0)
					{
					int index = st.front(); st.pop();
					auto it = _gr[index].begin();
					while (doneCircle[*it] == 0) { ++it; }
					auto sit = it;
					auto pit = it;
					++it;
					if (it == _gr[index].end()) { it = _gr[index].begin(); }
					while(it != sit)
						{
						if (doneCircle[*it] == 0)
							{
							mtools::fVec2 p = layout(index, *pit, *it);
							_circle[*it] = p;
							double rad = _rad[*it];
							if (p.X() + rad > R.max[0]) { R.max[0] = p.X() + rad; }
							if (p.X() - rad < R.min[0]) { R.min[0] = p.X() - rad; }
							if (p.Y() + rad > R.max[1]) { R.max[1] = p.Y() + rad; }
							if (p.Y() - rad < R.min[1]) { R.min[1] = p.Y() - rad; }
							doneCircle[*it] = 1;
							if (*it < _i0) { st.push(*it); }
							}
						pit = it;
						++it;
						if (it == _gr[index].end()) { it = _gr[index].begin(); }
						}
					}
				_rect = R;
				return R;
				}


			/**
			* Return the position of the circles
			*
			* @return	The layout.
			*/
			std::vector<mtools::fVec2> getLayout() const { return permute(_circle, _invperm); }


			/**
			* Return the rectangle enclosing the circle packing
			*
			* @return	the enclosing rectangle.
			*/
			mtools::fBox2 getEnclosingRect() const { return _rect; }


			/**
			 * Draw circle packing into an image
			 *
			 * @param	maxImagedimension	The maximum image dimension in any direction.
			 * @param	drawCircles		 	true to draw circles.
			 * @param	drawLines		 	true to draw lines.
			 * @param	colorCircles	 	The color of the circles.
			 * @param	colorLines		 	The color of the lines.
			 *
			 * @return	A mtools::Img<unsigned char>
			 **/
			mtools::Img<unsigned char> drawCirclePacking(int maxImagedimension, bool drawCircles = true, bool drawLines = true, RGBc colorCircles = RGBc::c_Red, RGBc colorLines = RGBc::c_Black)
				{
				MTOOLS_INSURE(!_rect.isEmpty());
				int LX = (_rect.lx() > _rect.ly()) ? maxImagedimension : (maxImagedimension*_rect.lx()/_rect.ly());
				int LY = (_rect.ly() > _rect.lx()) ? maxImagedimension : (maxImagedimension*_rect.ly() / _rect.lx());
				mtools::Img<unsigned char> Im(LX, LY, 1, 4);
				if (drawCircles)
					{
					for (int i = 0;i < _circle.size(); i++)
						{
						Im.fBox2_draw_circle(_rect, _circle[i], _rad[i], colorCircles, 0.5f);
						//Im.fBox2_drawText(R, toString(i + 1), circles[i], 'c', 'c', 10, false, RGBc::c_Blue);
						}
					}
				if (drawLines)
					{
					for (int i = 0;i < _circle.size(); i++)
						{
						for (int j = 0;j < _gr[i].size(); j++)
							{
							Im.fBox2_drawLine(_rect, _circle[i], _circle[_gr[i][j]], colorLines);
							}
						}
					}
				return Im;
				}


		private:


			/**   
			 * Compute the position of _circle[z] provided that _circle[x]and _circle[y] are already known   
			 * and z is the next vertex after y in the flower of x. 
			 * **/
			inline mtools::fVec2 layout(int x, int y, int z)
				{
				const double rx = _rad[x];
				const double ry = _rad[y];
				const double rz = _rad[z];
				double alpha = angleEuclidian(rx, ry, rz);
				mtools::fVec2 V = (_circle[y] - _circle[x]);
				V.normalize();
				const double ca = cos(alpha);
				const double sa = sin(alpha);
				mtools::fVec2 W = mtools::fVec2(ca*V.X() - sa*V.Y(), sa*V.X() + ca*V.Y());
				W.normalize();
				W *= (rx + rz);
				W += _circle[x];
				return W;
				}


			/** Compute the angle between the two circles of radius y and z that surround the circle of radius x **/
			inline double angleEuclidian(double rx, double ry, double rz) const 
				{
				const double a = rx + ry;
				const double b = rx + rz;
				const double c = ry + rz;
				const double r = (a*a + b*b - c*c) / (2 * a*b);
				if (r > 1.0) { return 0.0; } else if (r < -1.0) { return M_PI; }
				return acos(r);
				}



			/** Compute the total angle around vertex index **/
			inline double angleSumEuclidian(const double rx, const std::vector<int> & neighbour) const
				{
				const size_t l = neighbour.size();
				double sum = 0.0;
				double ry = _rad[neighbour[l-1]];
				for (size_t i=0; i<l; ++i)
					{
					const double rz = _rad[neighbour[i]];
					sum += angleEuclidian(rx, ry, rz);
					ry = rz;
					}
				return sum;
				}


			std::vector<std::vector<int> >	_gr;		// the graph
			mtools::Permutation				_perm;		// the permutation applied to get all the boundary vertices at the end
			mtools::Permutation				_invperm;	// the inverse permutation
			std::vector<double>				_rad;		// vertex raduises
			std::vector<mtools::fVec2>		_circle;	// circle layout
			size_t							_i0;		// index of the first boundary vertice. 
			mtools::fBox2					_rect;		// rectangle enclosing the packing.

		};






	}

/* end of file */

