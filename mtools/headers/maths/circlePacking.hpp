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
#include "../misc/timefct.hpp"
#include "../io/console.hpp"
#include "vec.hpp"
#include "box.hpp"
#include "permutation.hpp"
#include "graph.hpp" 
#include "circle.hpp"
#include "mobius.hpp"
#include "../random/gen_fastRNG.hpp"
#include "../random/classiclaws.hpp"
#include "../graphics/customcimg.hpp"

#include "../extensions/openCL.hpp"


namespace mtools
	{



	namespace internals_circlepacking
		{


		/** Compute the angle between the two circles of radius y and z that surround the circle of radius x **/
		template<typename FPTYPE> FPTYPE angleEuclidian(const FPTYPE & rx, const  FPTYPE & ry, const  FPTYPE & rz)
			{
			const FPTYPE a = rx + ry;
			const FPTYPE b = rx + rz;
			const FPTYPE c = ry + rz;
			const FPTYPE r = (a*a + b*b - c*c) / (2 * a*b);
			if (r > (FPTYPE)1.0) { return (FPTYPE)0; }
			else if (r < (FPTYPE)(-1.0)) { return acos((FPTYPE)(-1.0)); }
			return acos(r);
			}

		}


			/**
			 * Compute a circle packing layout.
			 *
			 * @param	v0		   	index of the vertex to put at the origin.
			 * @param	graph	   	The graph.
			 * @param	boundary   	The boundary vector.
			 * @param	rad		   	The radii vector.
			 * @param	strictMaths	true to raise an error is the the layout cannot be accuraetly computed.
			 *
			 * @return	The calculated euclidian layout together with it bounding box.
			 **/
			template<typename FPTYPE, typename GRAPH> std::pair< std::vector<Circle<FPTYPE> >, Box<FPTYPE,2> > computeCirclePackLayout(int v0, const GRAPH & graph, const std::vector<int> & boundary, const std::vector<FPTYPE> & rad, bool strictMaths = false)
				{
				MTOOLS_INSURE(graph.size() == rad.size());
				MTOOLS_INSURE(graph.size() == boundary.size());
				MTOOLS_INSURE(boundary[v0] <= 0);
				std::pair< std::vector<Circle<FPTYPE> >, Box<FPTYPE, 2> > res;
				if (rad.size() == 0) return res;
				auto & circle = res.first;
				auto & R = res.second;
				circle.resize(rad.size());
				circle[v0] = Circle<FPTYPE>(complex<FPTYPE>((FPTYPE)0, (FPTYPE)0), rad[v0]);
				int v1 = graph[v0].front();
				circle[v1] = Circle<FPTYPE>(complex<FPTYPE>(rad[v0] + rad[v1], (FPTYPE)0), rad[v1]);
				R.min[0] = -rad[v0];
				R.max[0] = rad[v0] + 2*rad[v1];
				R.max[1] = std::max<FPTYPE>(rad[v0], rad[v1]);
				R.min[1] = -R.max[1];
				std::vector<int> doneCircle(rad.size(),0);
				doneCircle[v0] = 1;
				doneCircle[v1] = 1;
				std::queue<int> st;
				st.push(v0);
				if (boundary[v1] <= 0) st.push(v1);
				while (st.size() != 0)
					{
					int index = st.front(); st.pop();
					auto it = graph[index].begin();
					while (doneCircle[*it] == 0) { ++it; }
					auto sit = it, pit = it;
					++it;
					if (it == graph[index].end()) { it = graph[index].begin(); }
					while (it != sit)
						{
						if (doneCircle[*it] == 0)
							{
							const int x = index;
							const int y = *pit;
							const int z = *it;
							const FPTYPE & rx = rad[x]; if ((strictMaths)&&((rx == (FPTYPE)0.0)||(isnan(rx)))) { MTOOLS_ERROR(std::string("Precision error A. null radius (site ") + mtools::toString(x) + ")"); }
							const FPTYPE & ry = rad[y]; if ((strictMaths)&&((ry == (FPTYPE)0.0)||(isnan(ry)))) { MTOOLS_ERROR(std::string("Precision error B. null radius (site ") + mtools::toString(y) + ")"); }
							const FPTYPE & rz = rad[z]; if ((strictMaths)&&((rz == (FPTYPE)0.0)||(isnan(rz)))) { MTOOLS_ERROR(std::string("Precision error C. null radius (site ") + mtools::toString(z) + ")"); }
							const FPTYPE & alpha = internals_circlepacking::angleEuclidian(rx, ry, rz);
							if ((strictMaths) && (isnan(alpha))) { MTOOLS_ERROR(std::string("Precision error D. null alpha (site ") + mtools::toString(z) + ")"); }
							auto w = circle[y].center - circle[x].center;
							auto rot = complex<FPTYPE>(cos(alpha), sin(alpha));
							w = w*rot;
							const FPTYPE norm = std::abs(w);
							if (norm != (FPTYPE)0.0) { w /= norm; w *= (rx + rz); } else { if (strictMaths) { MTOOLS_ERROR(std::string("Precision error E (site ") + mtools::toString(*it) + ")"); } }
							w += circle[x].center;
							circle[z].center = w;
							if ((circle[z].center == circle[y].center) || (circle[z].center == circle[x].center)) { if (strictMaths) { MTOOLS_ERROR(std::string("Precision error F (site ") + mtools::toString(*it) + ")"); } }
							circle[z].radius = rad[z];
							if (w.real() + rad[z] > R.max[0]) { R.max[0] = w.real() + rad[z]; }
							if (w.real() - rad[z] < R.min[0]) { R.min[0] = w.real() - rad[z]; }
							if (w.imag() + rad[z] > R.max[1]) { R.max[1] = w.imag() + rad[z]; }
							if (w.imag() - rad[z] < R.min[1]) { R.min[1] = w.imag() - rad[z]; }
							doneCircle[z] = 1;
							if (boundary[z] <= 0) { st.push(z); }
							}
						pit = it;
						++it;
						if (it == graph[index].end()) { it = graph[index].begin(); }
						}
					}
				return res;
				}













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
					if ((iter > 1000)&&(iter % 10000 == 0))
						{
						mtools::cout << "iter = " << iter << " error = " << c << "\n";
						}
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
				int LX = (int)(_rect.lx() > _rect.ly()) ? maxImagedimension : (int)(maxImagedimension*_rect.lx()/_rect.ly());
				int LY = (int)(_rect.ly() > _rect.lx()) ? maxImagedimension : (int)(maxImagedimension*_rect.ly() / _rect.lx());
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























		/**
		 * Class used to compute the radii associated with the (euclidian) circle packing
		 * of a triangulation with a boundary.
		 *
		 * The algorithm is taken from Collins and Stephenson (2003).
		 *
		 * NOTE : This class computes a 'packing label' in the euclidian case. It is possible to deduce
		 *        the maximal hyperbolic packing inside the unit disk D in the following way:
		 *        1) Join all boundary vertices to a new vertex v0, creating therefore a triangulation
		 *        without a boundary.
		 *        2) Choose any face (a,b,c) that does not contain W and compute the packing labels
		 *        with (a,b,c) as the outer face with boundary condition (1.0,1.0,1.0).
		 *        3) Construct the layout of of the packing obtained. Center the circle associated
		 *        with the special vertex v0 at the origin and normalize it such that it has unit radius.
		 *        4) Apply the inversion Mobius transformatin z -> 1/z to all circles.
		 *        5) Voila !
		 *
		 * NOTE: If openCL extension is active, the class CirclePackingLabelGPU may be used instead
		 *       increase computation speed.
		 *
		 * @tparam	FPTYPE	Floating type that should be used during calculation.
		 **/
		template<typename FPTYPE = double> class CirclePackingLabel
			{

			public:

			/**
			 * Constructor.
			 *
			 * @param	verbose	true to print info to mtools::cout during packing.
			 */
			CirclePackingLabel(bool verbose = false) : _verbose(verbose), _pi(acos((FPTYPE)(-1.0))) , _twopi(2*acos((FPTYPE)(-1.0)))
				{
				}


			/* dtor, empty object */
			~CirclePackingLabel() {}


			/**
			* Decide whether packing information should be printed to mtools::cout.
			**/
			void verbose(bool verb) { _verbose = verb; }


			/** Clears the object to a blank initial state. */
			void clear()
				{
				_gr.clear();
				_perm.clear();
				_invperm.clear();
				_nb = 0;
				_rad.clear();
				}


			/**
			* Loads a triangulation and define the boundary vertices.
			* All radii are set to 1.0. 
			*
			* @param graph	   The triangulation with boundary.
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
				_nb = l;
				for (size_t i = 0; i < l; i++)
					{
					if (boundary[_perm[i]] > 0.0) { _nb = i; break; }
					}
				MTOOLS_INSURE((_nb > 0)&&(_nb < l-2));
				_rad.resize(l, (FPTYPE)1.0);
				}


			/** Compute current packing 'angle' error in L2 norm. */
			FPTYPE errorL2() const
				{
				FPTYPE e(0.0);
				for (size_t i = 0; i < _nb; ++i)
					{
					const FPTYPE v = _rad[i];
					const FPTYPE c = angleSumEuclidian(v, _gr[i]) - _twopi;
					e += c*c;
					}
				return sqrt(e);
				}


			/** Compute current packing 'angle' error in L1 norm. */
			FPTYPE errorL1() const
				{
				FPTYPE e(0.0);
				for (size_t i = 0; i < _nb; ++i)
					{
					const FPTYPE v = _rad[i];
					const FPTYPE c = angleSumEuclidian(v, _gr[i]) - _twopi;
					e += ((c < (FPTYPE)0.0) ? -c : c);
					}
				return e;
				}


			/**
			 * Sets the radii of the circle around each vertices. 
			 * The radii associated with the boundary vertices are not modified during 
			 * the circle packing algorithm.
			 *
			 * @param	rad	The radii. Any values <= 0.0 is set to 1.0.
			 **/
			void setRadii(const std::vector<FPTYPE> & rad)
				{
				const size_t l = _gr.size();
				MTOOLS_INSURE(rad.size() == l);
				_rad = permute(rad, _perm);
				for (size_t i = 0; i < l; i++)
					{
					if (_rad[i] <= (FPTYPE)0.0) _rad[i] = (FPTYPE)1.0;
					}
				}


			/**
			* Return the list of radii.
			*
			* @return	The radii of the circles around each vertices.
			*/
			std::vector<FPTYPE> getRadii() const { return permute(_rad, _invperm); }


			/**
			 * Sets all radii to r.
			 **/
			void setRadii(FPTYPE r = 1.0)
				{
				MTOOLS_INSURE(r > 0.0);
				const size_t l = _gr.size();
				_rad.resize(l);
				for (size_t i = 0; i < l; i++) { _rad[i] = r; }
				}


			/**
			 * Run the algoritm for computing the value of the radii.
			 *
			 * @param	verbose			true to print progress to mtools::cout.
			 * @param	eps				the required precision, in L2 norm.
			 * @param	delta			parameter that detemrine how super acceleration is performed (slower
			 * 							value = more restrictive condition to perform acceleration).
			 * @param	maxIteration	The maximum number of iteration before stopping. -1 = no limit.
			 * @param	stepIter		number of iterations between printing infos (used only if verbose = true).
			 *
			 * @return	The number of iterations performed.
			 **/
			int64 computeRadii(const FPTYPE eps = 10e-9, const FPTYPE delta = 0.05, const int64 maxIteration = -1, const int64 stepIter = 1000)
				{
				auto totduration = chrono();
				FastRNG gen;					// use to randomize acceleration.
				FPTYPE minc = errorL2();
				if (_verbose)
					{ 
					mtools::cout << "\n  --- Starting Packing Algorithm [CPU] ---\n\n"; 
					mtools::cout << "initial L2 error  = " << minc << "\n";
					mtools::cout << "L2 target         = " << eps << "\n";
					mtools::cout << "max iterations    = " << maxIteration << "\n";
					mtools::cout << "iter between info = " << stepIter << "\n\n";
					}

				//#define DELAY_POST_RADII		// uncomment those line for debugging
				//#define DO_NOT_USE_RNG		//

				const int nb = (int)_nb;
				int64 iter = 0;
				FPTYPE c = 1.0 + eps, c0;
				FPTYPE lambda = -1.0, lambda0;
				bool fl = false, fl0;
				std::vector<FPTYPE> _rad0 = _rad;
				auto duration = chrono();
				while((c > eps)&&(iter != maxIteration))
					{
					iter++;
					c0 = c;
					c = 0.0;
					lambda0 = lambda;
					fl0 = fl;
					#ifndef DELAY_POST_RADII
					_rad0 = _rad;
					#endif
					for (int i = 0; i < nb; i++)
						{
						const FPTYPE v = _rad[i];
						const FPTYPE theta = angleSumEuclidian(v, _gr[i]);
						const FPTYPE k = (FPTYPE)_gr[i].size();
						const FPTYPE beta = sin(theta*0.5 / k);
						const FPTYPE tildev = beta*v / (1.0 - beta);
						const FPTYPE delta = sin(_pi / k);
						const FPTYPE u = (1.0 - delta)*tildev / delta;
						const FPTYPE e = theta - _twopi;
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
					if (c < minc) { minc = c; }
					lambda = c / c0;
					fl = true;					
					if ((fl0) && (lambda < 1.0))
						{
						if (abs(lambda - lambda0) < delta) {  lambda = lambda / (1.0 - lambda); }
						FPTYPE lstar = 3.0*lambda;
						for (size_t i = 0; i < nb; ++i) 
							{
							const FPTYPE d = _rad0[i] - _rad[i];
							if (d > 0.0)
								{
								const FPTYPE d2 = (_rad[i] / d);
								if (d2 < lstar) { lstar = d2; }
								}
							}
						lambda = ((lambda < 0.5*lstar) ? lambda : 0.5*lstar);
						
						#ifdef DO_NOT_USE_RNG
						for (size_t i = 0; i < nb; ++i) { _rad[i] += lambda*(_rad[i] - _rad0[i]); }
						fl = 0;
						#else
						if ((gen() & 1)&&(c > eps)) // do not accelerate if c < eps 
							{
							for (size_t i = 0; i < nb; ++i) { _rad[i] += lambda*(_rad[i] - _rad0[i]); } 
							fl = 0;
							}
						#endif ENDIF
						}
					if ((_verbose)&&((iter % stepIter == 0)||(c < eps)||(iter == maxIteration)))
						{
						mtools::cout << "iteration = " << iter << "\n";
						mtools::cout << "L2 current error  = " << c << "\n";
						mtools::cout << "L2 minimum error  = " << minc << "\n";
						mtools::cout << "L2 target         = " << eps << "\n";
						mtools::cout << ((iter % stepIter == 0) ? stepIter : iter % stepIter) << " interations performed in " << duration << "\n\n";
						duration.reset();
						}
					}
				if (_verbose) 
					{ 
					cout << "\n\nFinal L2 error = " << errorL2() << "\n";
					cout << "Final L1 error = " << errorL1() << "\n\n";
					cout << "Total packing time : " << totduration << "\n\n";
					if (iter == maxIteration) { mtools::cout << "  --- Packing stopped after " << iter << " iterations ---  \n\n"; }
					else { mtools::cout << "  --- Packing complete ---  \n\n"; }
					}
				return iter;

				#undef DO_NOT_USE_RNG
				#undef DELAY_POST_RADII 
				}


			private:


				/** Compute the total angle around vertex index **/
				inline FPTYPE angleSumEuclidian(const FPTYPE rx, const std::vector<int> & neighbour) const
					{
					const size_t l = neighbour.size();
					FPTYPE sum = 0.0;
					FPTYPE ry = _rad[neighbour[l - 1]];
					for (size_t i = 0; i<l; ++i)
						{
						const FPTYPE rz = _rad[neighbour[i]];
						const FPTYPE a = rx + ry;
						const FPTYPE b = rx + rz;
						const FPTYPE c = ry + rz;
						const FPTYPE r = (a*a + b*b - c*c) / (2 * a*b);
						if (r < (FPTYPE)1.0)
							{
							if (r <= (FPTYPE)-1.0) { return sum += _pi; } else { sum += acos(r); }
							}
						ry = rz;
						}
					return sum;
					}

				bool _verbose;		// do we print info on mtools::cout ?

				const FPTYPE					_pi;		// pi
				const FPTYPE					_twopi;		// pi

				std::vector<std::vector<int> >	_gr;		// the graph
				mtools::Permutation				_perm;		// the permutation applied to get all the boundary vertices at the end
				mtools::Permutation				_invperm;	// the inverse permutation
				std::vector<FPTYPE>				_rad;		// vertex raduises
				size_t							_nb;		// number of internal vertices


			};






/**********************************************************************************************************
*
* 
* OPENCL VERSION : only defined if openCL extension is activated.
* 
*
**********************************************************************************************************/


#ifdef MTOOLS_HAS_OPENCL


#include "circlePacking.cl.hpp"	// the openCL program source.


		/**
		 * Same as the class above but use GPU acceleration.
		 * This class is defined only if the openCL extension is activated. 
		 * 
		 * @tparam	FPTYPE	floating type used for calculations. Must be either double or float.
		 **/
		template<typename FPTYPE = double> class CirclePackingLabelGPU
			{

			using UINT_VEC4   = uint32[4];
			using FPTYPE_VEC8 = FPTYPE[8];

			static_assert(std::is_same<FPTYPE, double>::value || std::is_same<FPTYPE, float>::value, "mtools::CirclePackingLabelGPU<FPTYPE> can only be instantiated with FPTYPE= double or float.");

			public:

			/**
			 * Constructor.
			 *
			 * @param	verbose	true print informations to mtools::cout.
			 */
			CirclePackingLabelGPU(bool verbose = false) : _verbose(verbose), _localsize(-1), _nbVertices(0), _maxDegree(0), _clbundle(true, verbose, verbose)
				{
				clear();
				}


			/* dtor, empty object */
			~CirclePackingLabelGPU() 
				{
				}


			/**
			* Decide whether packing information should be printed to mtools::cout.
			**/
			void verbose(bool verb) { _verbose = verb; }


			/** Clears the object to a blank initial state. */
			void clear()
				{
				_gr.clear();
				_perm.clear();
				_invperm.clear();
				_nb = 0;
				_nbdummy = 0;
				_rad.clear();
				}


			/**
			* Loads a triangulation and define the boundary vertices.
			* All radii are set to 1.0. 
			*
			* @param graph	   The triangulation with boundary.
			* @param boundary  The boundary vector. Every index i for which boundary[i] > 0
			* 					is considered to be a boundary vertice.
			*/
			template<typename GRAPH> void setTriangulation(const GRAPH & graph, std::vector<int> boundary)
				{ 
				const size_t l = graph.size();
				MTOOLS_INSURE(l > 4);
				MTOOLS_INSURE(boundary.size() == l);
				clear();
				_nb = 0;
				int lb = -1;
				for (size_t i = 0; i < l; i++) 
					{ 
					if (boundary[i] <= 0.0) { boundary[i] = -(int)graph[i].size() - 2; _nb++; } else { lb = (int)i; }
					}
				MTOOLS_INSURE((_nb > 0) && (_nb < l - 2));
				_gr = convertGraph<GRAPH, std::vector<std::vector<int> > >(graph);
				// add dummy vertice so that the number of inner vertices is a multiple of groupsize
				const int wg = _clbundle.maxWorkGroupSize();
				const int r = ((int)_nb) % wg;
				_nbdummy = ((r == 0) ? 0 : (wg - r));
				_gr.resize(l + _nbdummy);
				boundary.resize(l + _nbdummy);
				for (size_t i = l; i < l + _nbdummy; i++)
					{
					boundary[i] = -1; // not a boundary site.
					_gr[i].clear(); // not a real site (degree = 0)
					}
				_nb += _nbdummy;
				// done.
				_perm = getSortPermutation(boundary);
				_invperm = invertPermutation(_perm);
				_gr = permuteGraph<std::vector<std::vector<int> > >(_gr, _perm, _invperm);
				_rad.resize(_gr.size(), (FPTYPE)1.0);
				}
				

			/** Compute current packing 'angle' error in L2 norm. */
			FPTYPE errorL2() const
				{
				FPTYPE e(0.0);
				for (size_t i = 0; i < _nb; ++i)
					{
					if (_gr[i].size() > 0)
						{
						const FPTYPE v = _rad[i];
						const FPTYPE c = angleSumEuclidian(v, _gr[i]) - (FPTYPE)M_2PI;
						e += c*c;
						}
					}
				return sqrt(e);
				}


			/** Compute current packing 'angle' error in L1 norm. */
			FPTYPE errorL1() const
				{
				FPTYPE e(0.0);
				for (size_t i = 0; i < _nb; ++i)
					{
					if (_gr[i].size() > 0)
						{
						const FPTYPE v = _rad[i];
						const FPTYPE c = angleSumEuclidian(v, _gr[i]) - (FPTYPE)M_2PI;
						e += ((c < (FPTYPE)0.0) ? -c : c);
						}
					}
				return e;
				}


			/**
			 * Sets the radii of the circle around each vertices. 
			 * The radii associated with the boundary vertices are not modified during 
			 * the circle packing algorithm.
			 *
			 * @param	rad	The radii. Any values <= 0.0 is set to 1.0.
			 **/
			void setRadii(std::vector<FPTYPE> rad)
				{
				const size_t l = _gr.size();
				MTOOLS_INSURE(rad.size() == l - _nbdummy);
				rad.resize(l,1.0);
				_rad = permute(rad, _perm);
				for (size_t i = 0; i < l; i++)
					{
					if (_rad[i] <= (FPTYPE)0.0) _rad[i] = (FPTYPE)1.0;
					}
				}


			/**
			* Sets all radii to the same value r.
			**/
			void setRadii(FPTYPE r = 1.0)
				{
				MTOOLS_INSURE(r > 0.0);
				const size_t l = _gr.size();
				_rad.resize(l);
				for (size_t i = 0; i < l; i++) { _rad[i] = r; }
				}


			/**
			* Return the list of radii.
			*
			* @return	The radii of the circles around each vertices.
			*/
			std::vector<FPTYPE> getRadii() const 
				{
				std::vector<FPTYPE> r = permute(_rad, _invperm);
				r.resize(r.size() - _nbdummy);
				return r; 
				}




			/**
			 * Run the algorithm for computing the value of the radii.
			 *
			 * @param	eps				the required precision, in L2 norm.
			 * @param	delta			parameter that determine how super acceleration is performed (slower
			 * 							value = more restrictive condition to perform acceleration).
			 * @param	maxIteration	The maximum number of iteration before stopping. -1 = no limit.
			 * @param	stepIter		number of iterations between each check of the current error 
			 * 							(and printing information if verbose = true)
			 *
			 * @return	The number of iterations performed.
			 **/
			int64 computeRadii(const FPTYPE eps = 10e-9, const FPTYPE delta = 0.05, const int64 maxIteration = -1, const int64 stepIter = 1000)
				{
				auto totduration = chrono();

				// recreate kernels if needed
				_recreateKernels();

				const int nbVerticesPow2 = pow2roundup(_nbVertices); // new power of 2

				// create buffers and set their values
				{
				std::vector<FPTYPE> buff(nbVerticesPow2, (FPTYPE)0);
				_buff_error1.reset(new cl::Buffer(_clbundle.context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(FPTYPE)*nbVerticesPow2, buff.data()));
				_buff_error2.reset(new cl::Buffer(_clbundle.context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(FPTYPE)*nbVerticesPow2, buff.data()));

				for (size_t i = 0; i < nbVerticesPow2; i++) { buff[i] = (FPTYPE)1.0e10; }
				_buff_lambdastar1.reset(new cl::Buffer(_clbundle.context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(FPTYPE)*nbVerticesPow2, buff.data()));
				_buff_lambdastar2.reset(new cl::Buffer(_clbundle.context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(FPTYPE)*nbVerticesPow2, buff.data()));

				_buff_radii1.reset(new cl::Buffer(_clbundle.context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(FPTYPE)*_nbVertices, _rad.data()));
				_buff_radii2.reset(new cl::Buffer(_clbundle.context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(FPTYPE)*_nbVertices, _rad.data()));

				std::vector<int32> degTab(_nbVertices);
				for (int i = 0; i < _nbVertices; i++) { degTab[i] = (int32)_gr[i].size(); }
				_buff_degree.reset(new cl::Buffer(_clbundle.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int32)*_nbVertices, degTab.data()));

				std::vector<int32> neighbourTab(_nbVertices*_maxDegree, 0);
				for (int i = 0; i < _nbVertices; i++) { for (int j = 0; j < (int)(_gr[i].size()); j++) { neighbourTab[j*_nbVertices + i] = (int32)_gr[i][j]; } }
				_buff_neighbour.reset(new cl::Buffer(_clbundle.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int32)*_nbVertices*_maxDegree, neighbourTab.data()));

				FPTYPE_VEC8 paramTab;
				FPTYPE ce = errorL2();
				paramTab[0] = (FPTYPE)(ce);	   	   // error
				paramTab[1] = (FPTYPE)1.0;		   // lambda
				paramTab[2] = (FPTYPE)1.0;		   // flag acceleration
				paramTab[3] = (FPTYPE)eps;         // target value
				paramTab[4] = (FPTYPE)delta;	   // acceleration parameter
				paramTab[5] = (FPTYPE)ce;		   // min error
				_buff_param.reset(new cl::Buffer(_clbundle.context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(paramTab), paramTab));

				UINT_VEC4 rngTab;
				rngTab[0] = 123456789; rngTab[1] = 362436069; rngTab[2] = 521288629; rngTab[3] = 0; // initial seed 
				_buff_rng.reset(new cl::Buffer(_clbundle.context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(rngTab), rngTab));
				}

				// set kernels arguments
				_kernel_updateRadius->setArg(0, *_buff_radii1);
				_kernel_updateRadius->setArg(1, *_buff_radii2);
				_kernel_updateRadius->setArg(2, *_buff_degree);
				_kernel_updateRadius->setArg(3, *_buff_neighbour);
				_kernel_updateRadius->setArg(4, *_buff_error1);
				_kernel_updateRadius->setArg(5, *_buff_lambdastar1);

				_kernel_reduction1->setArg(0, *_buff_error1);
				_kernel_reduction1->setArg(1, *_buff_error2);
				_kernel_reduction1->setArg(2, *_buff_lambdastar1);
				_kernel_reduction1->setArg(3, *_buff_lambdastar2);

				_kernel_reduction2->setArg(0, *_buff_error2);
				_kernel_reduction2->setArg(1, *_buff_error1);
				_kernel_reduction2->setArg(2, *_buff_lambdastar2);
				_kernel_reduction2->setArg(3, *_buff_lambdastar1);

				_kernel_reduction_finale1->setArg(0, *_buff_error1);
				_kernel_reduction_finale1->setArg(1, *_buff_lambdastar1);
				_kernel_reduction_finale1->setArg(2, *_buff_param);
				_kernel_reduction_finale1->setArg(3, *_buff_rng);

				_kernel_reduction_finale2->setArg(0, *_buff_error2);
				_kernel_reduction_finale2->setArg(1, *_buff_lambdastar2);
				_kernel_reduction_finale2->setArg(2, *_buff_param);
				_kernel_reduction_finale2->setArg(3, *_buff_rng);

				_kernel_accelerate->setArg(0, *_buff_radii1);
				_kernel_accelerate->setArg(1, *_buff_radii2);
				_kernel_accelerate->setArg(2, *_buff_param);

				if (_verbose)
					{
					mtools::cout << "\n  --- Starting Packing Algorithm [openCL GPU] ---\n\n";
					mtools::cout << "initial L2 error  = " << errorL2() << "\n";
					mtools::cout << "L2 target         = " << eps << "\n";
					mtools::cout << "max iterations    = " << maxIteration << "\n";
					mtools::cout << "iter between info = " << stepIter << "\n\n";
					}
				// make computation
				int64 iter = 0;
				bool done = false;
				auto duration = chrono();
				while((!done)&&(iter != maxIteration))
					{
					iter++;

					// update radii
					_clbundle.queue.enqueueNDRangeKernel(*_kernel_updateRadius, 0, _nb, cl::NullRange);
					
					// compute the total error
					int globalsize = nbVerticesPow2;
					int flip = 1;
					while (globalsize > _localsize)
						{
						_clbundle.queue.enqueueNDRangeKernel(((flip == 1) ? *_kernel_reduction1 : *_kernel_reduction2), 0, globalsize, _localsize);
						flip = 1 - flip;
						globalsize /= _localsize;
						}					

					// complete the reduction and copute lambda.
					_clbundle.queue.enqueueNDRangeKernel(((flip == 1) ? *_kernel_reduction_finale1 : *_kernel_reduction_finale2), 0, globalsize, globalsize);
					
					// perform acceleration
					_clbundle.queue.enqueueNDRangeKernel(*_kernel_accelerate, 0, _nb, cl::NullRange);

					if (iter % stepIter == 0)
						{
						FPTYPE_VEC8 param;
						_clbundle.queue.finish();
						_clbundle.queue.enqueueReadBuffer(*_buff_param, CL_TRUE, 0, sizeof(param), &param);
						if (param[0] < eps) { done = true; }												
						if (_verbose)
							{
							mtools::cout << "iteration = " << iter << "\n";
							mtools::cout << "L2 current error  = " << param[0] << "\n";
							mtools::cout << "L2 minimum error  = " << param[5] << "\n";
							mtools::cout << "L2 target         = " << param[3] << "\n";
							mtools::cout << stepIter << " interations performed in " << duration << "\n\n";
							duration.reset();
							}													
						}
					}
				// done, read back the result 
				_clbundle.queue.finish();
				_clbundle.queue.enqueueReadBuffer(*_buff_radii1, CL_TRUE, 0, _nbVertices * sizeof(FPTYPE), _rad.data());
				if (_verbose)
					{
					if (done) 
						{ 
						cout << "Total packing time : " << totduration << "\n\n";
						mtools::cout << "  --- Packing complete ---\n\n"; 
						}
					else
						{
						FPTYPE_VEC8 param;
						_clbundle.queue.enqueueReadBuffer(*_buff_param, CL_TRUE, 0, sizeof(param), &param);
						cout << "\nFinal L2 error = " << errorL2() << "\n";
						cout << "Final L1 error = " << errorL1() << "\n\n";
						cout << "Total packing time : " << totduration << "\n\n";
						mtools::cout << "  --- Packing stopped after " << iter << " iterations ---  \n\n";
						}
					}

				return iter;
				}





			private:

				/** Compute the total angle around vertex index **/
				inline FPTYPE angleSumEuclidian(const FPTYPE rx, const std::vector<int> & neighbour) const
					{
					const size_t l = neighbour.size();
					FPTYPE sum(0.0);
					FPTYPE ry = _rad[neighbour[l - 1]];
					for (size_t i = 0; i<l; ++i)
						{
						const FPTYPE rz = _rad[neighbour[i]];
						const FPTYPE a = rx + ry;
						const FPTYPE b = rx + rz;
						const FPTYPE c = ry + rz;
						const FPTYPE r = (a*a + b*b - c*c) / (2 * a*b);
						if (r < (FPTYPE)1.0)
							{
							if (r <= (FPTYPE)-1.0) { return sum += (FPTYPE)M_PI; } else { sum += acos(r); }
							}
						ry = rz;
						}
					return sum;
					}


				/* create the openCL kernels if needed */
				void _recreateKernels()
					{
					const int maxdeg = maxOutDegreeGraph(_gr);
					const int maxgpsize = _clbundle.maxWorkGroupSize();
					const int nbvert = (int)_gr.size();
					if ((_localsize == maxgpsize) && ((int)_gr.size() == _nbVertices) && (maxdeg == _maxDegree)) { return; }
					_localsize = maxgpsize/2;
					_nbVertices = nbvert;
					_maxDegree = maxdeg;

					// compiler options
					std::string options;
					options += std::string(" -DFPTYPE=") + typeid(FPTYPE).name();
					options += std::string(" -DFPTYPE_VEC8=") + typeid(FPTYPE).name() + "8";
					options += " -DNBVERTICES=" + toString(_nbVertices);
					options += " -DMAXDEGREE=" + toString(_maxDegree);
					options += " -DMAXGROUPSIZE=" + toString(_localsize);
					//options += " -cl-nv-verbose"; // debug option. Only available for nvidia openCL

					// build program
					std::string log;
					_prog.reset(new cl::Program(_clbundle.createProgramFromString(internals_circlepacking::circlePacking_openCLprogram,log, options,_verbose))); // create programm

					// create kernels objects
					_kernel_updateRadius.reset(new cl::Kernel(_clbundle.createKernel(*_prog, "updateRadius",_verbose)));			// create kernel rad1 -> rad2
					_kernel_reduction1.reset(new cl::Kernel(_clbundle.createKernel(*_prog, "reduction", _verbose)));				// create kernel error1 -> error2
					_kernel_reduction2.reset(new cl::Kernel(_clbundle.createKernel(*_prog, "reduction", _verbose)));				// create kernel error2 -> error1
					_kernel_reduction_finale1.reset(new cl::Kernel(_clbundle.createKernel(*_prog, "reduction_finale", _verbose)));// create kernel error1 -> param
					_kernel_reduction_finale2.reset(new cl::Kernel(_clbundle.createKernel(*_prog, "reduction_finale", _verbose)));// create kernel error2 -> param
					_kernel_accelerate.reset(new cl::Kernel(_clbundle.createKernel(*_prog, "accelerate", _verbose)));             // create kernel accelerate. 

					return;
					}


				bool _verbose;		// do we print info on mtools::cout ?

				// define in compiler options
				int _localsize;
				int _nbVertices;
				int _maxDegree;

				// openCL bundle
				mtools::OpenCLBundle			_clbundle;

				// kernels
				std::unique_ptr<cl::Program> _prog;
				std::unique_ptr<cl::Kernel>  _kernel_updateRadius;
				std::unique_ptr<cl::Kernel>  _kernel_reduction1;
				std::unique_ptr<cl::Kernel>  _kernel_reduction2;
				std::unique_ptr<cl::Kernel>  _kernel_reduction_finale1;
				std::unique_ptr<cl::Kernel>  _kernel_reduction_finale2;
				std::unique_ptr<cl::Kernel>  _kernel_accelerate;

				// buffer
				std::unique_ptr<cl::Buffer> _buff_error1;
				std::unique_ptr<cl::Buffer> _buff_error2;
				std::unique_ptr<cl::Buffer> _buff_lambdastar1;
				std::unique_ptr<cl::Buffer> _buff_lambdastar2;
				std::unique_ptr<cl::Buffer> _buff_radii1;
				std::unique_ptr<cl::Buffer> _buff_radii2;
				std::unique_ptr<cl::Buffer> _buff_degree;
				std::unique_ptr<cl::Buffer> _buff_neighbour;
				std::unique_ptr<cl::Buffer> _buff_param;
				std::unique_ptr<cl::Buffer> _buff_rng;

				std::vector<std::vector<int> >	_gr;		// the graph
				mtools::Permutation				_perm;		// the permutation applied to get all the boundary vertices at the end
				mtools::Permutation				_invperm;	// the inverse permutation
				std::vector<FPTYPE>				_rad;		// vertex raduises
				size_t							_nb;		// number of internal vertices
				size_t							_nbdummy;   // number of 'dummy' vertice so that the number of acitve vertice

			};

#endif


	}

/* end of file */

