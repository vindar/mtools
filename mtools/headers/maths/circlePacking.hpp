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

#include "vec.hpp"
#include "box.hpp"

namespace mtools
	{
	

	/** Defines an alias representing a permutation of {0,1,...,n} */
	typedef std::vector<int> Permutation;


	/**
	* Return the permutation associated with the ordering of labels in non-decreasing order.
	* The label themselves are NOT reordered.
	*
	* @tparam	LABELS	Type of the object to reorder, typically std::vector<int>.
	* 					- Must be accessible via operator[].
	* 					- elements must be comparable with operator<() to allow sorting.
	*
	* @param	labels	The labels used to compute the re-ordering.
	*
	* @return	the reordering permutation. perm[i] = k means that the label initially at
	* 			position k is now at position i after reordering.
	*           Call method permute(labels,perm) to effectively sort the labels.
	**/
	template<typename LABELS> Permutation getSortPermutation(const LABELS & labels)
		{
		Permutation  res;
		const int l = (int)labels.size();
		if (l == 0) return res;
		res.resize(l);
		for (int i = 0; i < l; i++) { res[i] = i; }
		sort(res.begin(), res.end(), [&](const int & x, const int & y) { return labels[x] < labels[y]; });
		return res;
		}


	/**
	* Compute the inverse of a permutation.
	*
	* @param	perm	the permutation. Must be bijection of {0,...,perm.size()-1}.
	*
	* @return	the inverse permutation such that return[perm[k]] = k for all k.
	**/
	inline Permutation invertPermutation(const Permutation & perm)
		{
		Permutation invperm;
		const size_t l = perm.size();
		if (l > 0)
			{
			invperm.resize(l);
			for (size_t i = 0; i < l; i++) { invperm[perm[i]] = (int)i; }
			}
		return invperm;
		}


	/**
	* Re-order the labels according to the permutation perm.
	* (obtained for example from getSortPermutation() )
	*
	* @tparam	VECTOR	Type of the object to reorder, typically std::vector<T>.
	* 					- Must be accessible via operator[].
	*
	* @param	labels	The labels to reorder.
	* @param	perm	the permutation, perm[i] = k means that label at position k must be put at pos i.
	**/
	template<typename VECTOR> VECTOR permute(const VECTOR & labels, const Permutation & perm)
		{
		const size_t l = labels.size();
		MTOOLS_INSURE(perm.size() == l);
		VECTOR res;
		if (l == 0) return res;
		res.resize(l);
		for (size_t i = 0; i < l; i++)
			{
			res[i] = labels[perm[i]];
			}
		return res;
		}


	/**
	* Reorder the vertices of a graph according to a permutation.
	*
	* @tparam	GRAPH   	Type of the graph, typically std::vector< std::list<int> >.
	* 						- The outside container must be accessible via operator[].
	* 						- The inside container must accept be iterable and contain
	*                         elements convertible to size_t (corresponding to the indexes
	*                          the neighour vertices).
	* @param	graph	  	The graph to reorder.
	* @param	perm    	The permutation to apply: perm[i] = k means that the vertex with index k
	*                       must now become the vertex at index i in the new graph.
	* @param	invperm    	The inverse permutation of perm. (use the other permuteGraph() method if
	*						not previously computed).
	*
	* @return  the permuted graph.
	**/
	template<typename GRAPH> GRAPH permuteGraph(const GRAPH & graph, const Permutation  & perm, const Permutation & invperm)
		{
		const size_t l = graph.size();
		MTOOLS_INSURE(perm.size() == l);
		if (l == 0) return GRAPH();
		GRAPH res = permute<GRAPH>(graph, perm);	// permute the order of the vertices. 
		for (size_t i = 0; i < l; i++)
			{
			for (auto it = res[i].begin(); it != res[i].end(); it++)
				{
				(*it) = invperm[*it];
				}
			}
		return res;
		}


	/**
	 * Convert a graph from type A to type B.
	 *
	 **/
	template<typename GRAPH_A, typename GRAPH_B> GRAPH_B convertGraph(const GRAPH_A & graph)
		{
		GRAPH_B res;
		const size_t l = graph.size();
		if (l == 0) return res;
		res.resize(l);
		for (size_t i = 0; i < l; i++)
			{
			auto & lv1 = graph[i];
			auto & lv2 = res[i];
			for (auto it = lv1.begin(); it != lv1.end(); ++it) { lv2.push_back(*it); }
			}
		return res;
		}


	/**
	* Reorder the vertices of a graph according to a permutation.
	* Same as above but also compute the inverse permutation.
	*/
	template<typename GRAPH> GRAPH permuteGraph(const GRAPH & graph, const Permutation & perm)
		{
		return(permuteGraph(graph, perm, invertPermutation(perm)));
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
			* Loads a trinagulation and define the boundary vertices.
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
				MTOOLS_INSURE(_i0 < l);
				}


			/**
			* Sets the initial radii of the circle around each vertex.
			* The radii associated with the boundary vertices are fixed and
			* represent the boundary condition for the packing.
			*
			* @param	rad	The radii. Any values <= 0.0 is set to 1.0.
			*/
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
			 * @param	precision	the required precision, in L2 norm for the total angle sum.
			 *
			 * @return	The number of iterations performed.
			 **/
			int64 computeRadii(double precision = 10e-10)
				{
				precision *= precision;
				int64 iter = 0;
				const size_t i0 = _i0;
				while(1)
					{
					iter++;
					double c = 0.0;
					for (size_t i = 0; i < i0; ++i)
						{
						const double v = _rad[i];
						const double theta = angleSumEuclidian(v, _gr[i]);
						const double k = (double)_gr[i].size();
						const double beta = sin(theta*0.5/k);
						const double tildev = beta*v / (1.0 - beta);
						const double delta = sin(M_PI/k);
						const double u = (1.0 - delta)*tildev / delta;
						const double e = theta - M_2_PI;
						c += e*e;
						_rad[i] = u;
						}
					if (c < precision) return iter;
					}
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
			int64 computeRadiiFast(const double eps = 10e-10,const double delta = 0.1)
				{
				const size_t i0 = _i0;
				int64 iter = 0;
				double c = 1.0, c0;
				double lambda = -1.0, lambda0;
				bool fl = false, fl0;
				std::vector<double> _rad0;
				_rad0.resize(_rad.size());

				while (c > eps)
					{
					iter++;
					c0 = c;
					c = 0;
					lambda0 = lambda;
					fl0 = fl;
					memcpy(_rad0.data(),_rad.data(),i0*sizeof(double));
					for (size_t i = 0; i < i0; ++i)
						{
						const double v = _rad[i];
						const double theta = angleSumEuclidian(v, _gr[i]);
						const double k = (double)_gr[i].size();
						const double beta = sin(theta*0.5 / k);
						const double tildev = beta*v / (1.0 - beta);
						const double delta = sin(M_PI / k);
						const double u = (1.0 - delta)*tildev / delta;
						const double e = theta - M_2_PI;
						c += e*e;
						_rad[i] = u;
						}
					c = sqrt(c); 
					lambda = c / c0;
					lambda *= lambda;
					fl = true;
					
					if ((fl0) && (lambda < 1.0))
						{
						if (abs(lambda - lambda0) < delta) { lambda = lambda / (1.0 - lambda); }
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
						for (size_t i = 0; i < i0; ++i) { _rad[i] += lambda*(_rad[i] - _rad0[i]); }
						fl = 0;
						}
						
					}
				cout << "erreur = " << c << "\n";
				return iter;
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
			fBox2 computeLayout(int i1 = -1, int i2 = -1)
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
				_circle[i1] = fVec2(0.0, 0.0);
				_circle[i2] = fVec2(_rad[i1] + _rad[i2], 0.0);

				fBox2 R;
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
							fVec2 p = layout(index, *pit, *it);
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
			std::vector<fVec2> getLayout() const { return permute(_circle, _invperm); }


			/**
			* Return the rectangle enclosing the circle packing
			*
			* @return	the enclosing rectangle.
			*/
			fBox2 getEnclosingRect() const { return _rect; }


		private:


			/**   
			 * Compute the position of _circle[z] provided that _circle[x]and _circle[y] are already known   
			 * and z is the next vertex after y in the flower of x. 
			 * **/
			inline fVec2 layout(int x, int y, int z)
				{
				const double rx = _rad[x];
				const double ry = _rad[y];
				const double rz = _rad[z];
				double alpha = angleEuclidian(rx, ry, rz);
				fVec2 V = (_circle[y] - _circle[x]);
				V.normalize();
				const double ca = cos(alpha);
				const double sa = sin(alpha);
				fVec2 W = fVec2(ca*V.X() - sa*V.Y(), sa*V.X() + ca*V.Y());
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
			Permutation						_perm;		// the permutation applied to get all the boundary vertices at the end
			Permutation						_invperm;	// the inverse permutation
			std::vector<double>				_rad;		// vertex raduises
			std::vector<fVec2>				_circle;	// circle layout
			size_t							_i0;		// index of the first boundary vertice. 
			fBox2							_rect;		// rectangle enclosing the packing.

		};






	}

/* end of file */

