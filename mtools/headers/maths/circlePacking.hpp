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

#include "../extensions/openCL.hpp" // openCL extension
#include "circlePacking.cl.hpp"	    // the openCL program source.


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


		/* compute the sum of the angle around a given vertex */
		template<typename FPTYPE, typename GRAPH> FPTYPE angleSumEuclidian(const int index, const GRAPH & gr, const std::vector<FPTYPE> & rad)
			{
			FPTYPE theta = 0.0;
			if (gr[index].size() < 2) return theta;
			FPTYPE v = rad[index];
			auto it = gr[index].begin();
			const FPTYPE firstR =rad[*it];
			FPTYPE prevR = firstR;
			it++;
			FPTYPE C = 0.0;
			for(; it != gr[index].end(); ++it) // use Kahan summation algorithm
				{
				FPTYPE nextR = rad[*it];
				FPTYPE Y = angleEuclidian(v, prevR, nextR) - C;
				FPTYPE T = theta + Y;
				C = (T - theta) - Y;
				theta = T;
				prevR = nextR;
				}
			theta += (angleEuclidian(v, prevR, firstR) - C);
			return theta;
			}


		/** Compute the L2 error for the angle sum for all vertices on the range [0,N-1] **/
		template<typename FPTYPE, typename GRAPH> FPTYPE errorL2(const GRAPH & gr, const std::vector<FPTYPE> & rad, const int N)
			{
			CONST FPTYPE twopi = (2*acos((FPTYPE)-1));
			FPTYPE e = (FPTYPE)0;
			FPTYPE C = (FPTYPE)0;
			for (int i = 0; i < N; ++i) // use Kahan summation algorithm
				{
				const FPTYPE a = (angleSumEuclidian(i,gr,rad) - twopi);
				FPTYPE Y = (a*a) - C;
				FPTYPE T = e + Y;
				C = (T - e) - Y;
				e = T;
				}
			return sqrt(e);
			}


		/** Compute the L2 error for the angle sum for all vertices on the range [0,N-1] **/
		template<typename GRAPH, typename FPTYPE> FPTYPE errorL1(const GRAPH & gr, const std::vector<FPTYPE> & rad, const int N)
			{
			CONST FPTYPE twopi = 2 * acos((FPTYPE)-1);
			FPTYPE e = (FPTYPE)0;
			FPTYPE C = (FPTYPE)0;
			for (int i = 0; i < N; ++i) // use Kahan summation algorithm
				{
				const FPTYPE a = (angleSumEuclidian(i, gr, rad) - twopi);
				FPTYPE Y = ((a > (FPTYPE)0) ? a : -a) - C;
				FPTYPE T = e + Y;
				C = (T - e) - Y;
				e = T;
				}
			return e;
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
			 * Draw circle packing into an image.
			 *
			 * @param [in,out]	img	The image to draw onto. It is not erased first.
			 * @param	R		   	The range represented by the image.
			 * @param	circles	   	The vector of circles.
			 * @param	gr		   	The graph.
			 * @param	drawCircles	true to draw the circles.
			 * @param	filled	   	true to fill the circles if they are drawn.
			 * @param	drawLines  	true to draw the graph lines.
			 * @param	color	   	color for drawing.
			 * @param	opacity	   	opacity for drawing.
			 * @param	firstIndex 	First index of the sub-graph to draw (inclusive)
			 * @param	lastIndex  	Last index of the sub-graph to draw (inclusive) -1 = until the end.
			 **/
			template<typename FPTYPE> void drawCirclePacking(mtools::Img<unsigned char> & img, const mtools::Box<FPTYPE, 2> & R, const std::vector<Circle<FPTYPE> > circles, const std::vector<std::vector<int> > & gr,
				                                             bool drawCircles, bool filled, bool drawLines, RGBc color, float opacity = 1.0f, int firstIndex = 0, int lastIndex = -1)
				{
				MTOOLS_ASSERT(circles.size() == gr.size());
				if ((lastIndex < 0) || (lastIndex >= (int)(gr.size() - 1))) lastIndex = (int)(gr.size() - 1);
				if (drawCircles)
					{
					for (int i = firstIndex; i <= lastIndex; i++) 
						{ 
						img.fBox2_draw_circle(R, circles[i].center, circles[i].radius, color, opacity, filled);
						}
					}
				if (drawLines)
					{
					for (int i = firstIndex; i <= lastIndex; i++)
						{
						for (auto it = gr[i].begin(); it != gr[i].end(); ++it)
							{
							if ((*it >= firstIndex) && (*it <= lastIndex)) { img.fBox2_drawLine(R, circles[i].center, circles[*it].center,color,opacity); }
							}
						}
					}
				}

	



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
				_perm.setSortPermutation(boundary);
				_gr = permuteGraph<std::vector<std::vector<int> > >(convertGraph<GRAPH, std::vector<std::vector<int> > >(graph), _perm);
				_nb = l;
				for (size_t i = 0; i < l; i++) { if (boundary[_perm[i]] > 0) { _nb = (int)i; break; } }
				MTOOLS_INSURE((_nb > 0)&&(_nb < l-2));
				_rad.resize(l, (FPTYPE)1.0);
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
				_rad = _perm.getPermute(rad);
				for (size_t i = 0; i < l; i++)
					{
					if (_rad[i] <= (FPTYPE)0.0) _rad[i] = (FPTYPE)1.0;
					}
				}


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
			* Return the list of radii.
			*/
			std::vector<FPTYPE> getRadii() const { return _perm.getAntiPermute(_rad); }


			/**
			* Compute the error in the circle radius in L2 norm.
			*/
			FPTYPE errorL2() const { return internals_circlepacking::errorL2(_gr, _rad, (int)_nb); }


			/**
			* Compute the error in the circle radius in L1 norm.
			*/
			FPTYPE errorL1() const { return internals_circlepacking::errorL1(_gr, _rad, (int)_nb); }


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
						const FPTYPE theta = internals_circlepacking::angleSumEuclidian(i, _gr, _rad);
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


				bool _verbose;	// do we print info on mtools::cout ?

				const FPTYPE					_pi;		// pi
				const FPTYPE					_twopi;		// pi

				std::vector<std::vector<int> >	_gr;		// the graph
				mtools::Permutation				_perm;		// the permutation applied to get all the boundary vertices at the end
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
			CirclePackingLabelGPU(bool verbose = false) : _verbose(verbose), _localsize(-1), _nbVertices(0), _clbundle(true, verbose, verbose)
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
				_perm.setSortPermutation(boundary);
				_gr = permuteGraph<std::vector<std::vector<int> > >(_gr, _perm);
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
				_rad = _perm.getPermute(rad);
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
				std::vector<FPTYPE> r = _perm.getAntiPermute(_rad);
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
				std::vector<int32> neighbourTabOff(_nbVertices);
				std::vector<int32> neighbourTabList; neighbourTabList.reserve(_nbVertices*3);
				int offset = 0;
				for (int i = 0; i < _nbVertices; i++) 
					{ 
					const int l = (int)(_gr[i].size());
					degTab[i] = l;
					neighbourTabOff[i] = offset;
					for (int j = 0; j < l; j++) 
						{ 
						neighbourTabList.push_back((int32)_gr[i][j]); 
						offset++;
						}
					}

				_buff_degree.reset(new cl::Buffer(_clbundle.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int32)*_nbVertices, degTab.data()));
				_buff_neighbourOff.reset(new cl::Buffer(_clbundle.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int32)*_nbVertices, neighbourTabOff.data()));
				_buff_neighbourList.reset(new cl::Buffer(_clbundle.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int32)*offset, neighbourTabList.data()));

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
				_kernel_updateRadius->setArg(3, *_buff_neighbourOff);
				_kernel_updateRadius->setArg(4, *_buff_neighbourList);
				_kernel_updateRadius->setArg(5, *_buff_error1);
				_kernel_updateRadius->setArg(6, *_buff_lambdastar1);

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
					const int maxgpsize = _clbundle.maxWorkGroupSize();
					const int nbvert = (int)_gr.size();
					if ((maxgpsize == _localsize) && (nbvert == _nbVertices)) { return; }
					_localsize = maxgpsize;
					_nbVertices = nbvert;

					// compiler options
					std::string options;
					options += std::string(" -DFPTYPE=") + typeid(FPTYPE).name();
					options += std::string(" -DFPTYPE_VEC8=") + typeid(FPTYPE).name() + "8";
					options += " -DNBVERTICES=" + toString(_nbVertices);
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
				std::unique_ptr<cl::Buffer> _buff_neighbourOff;
				std::unique_ptr<cl::Buffer> _buff_neighbourList;
				std::unique_ptr<cl::Buffer> _buff_param;
				std::unique_ptr<cl::Buffer> _buff_rng;

				std::vector<std::vector<int> >	_gr;		// the graph
				mtools::Permutation				_perm;		// the permutation applied to get all the boundary vertices at the end
				std::vector<FPTYPE>				_rad;		// vertex raduises
				size_t							_nb;		// number of internal vertices
				size_t							_nbdummy;   // number of 'dummy' vertice so that the number of acitve vertice

			};

#endif


	}

/* end of file */

