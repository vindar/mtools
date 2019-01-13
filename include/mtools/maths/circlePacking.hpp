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

#include "../graphics/font.hpp"
#include "../graphics/image.hpp"
#include "../misc/internal/mtools_export.hpp"
#include "../misc/misc.hpp" 
#include "../misc/stringfct.hpp" 
#include "../misc/error.hpp"
#include "../misc/timefct.hpp"
#include "../io/logfile.hpp"
#include "../io/console.hpp"
#include "vec.hpp"
#include "box.hpp"
#include "permutation.hpp"
#include "graph.hpp" 
#include "circle.hpp"
#include "mobius.hpp"
#include "../random/gen_fastRNG.hpp"
#include "../random/classiclaws.hpp"

#include "../extensions/openCL.hpp" // openCL extension
#include "internal/circlePacking.cl.hpp"	    // the openCL program source.





/***********************************************************************************************************************************

CHOSE A FAIRE:
--------------


-> Circle packing euclidien : - methode pour calculer les labels, version CPU          : OK
                              - methode pour calculer les labels, version openCL GPU   : OK

							  - methode pour layout les cercle : A MODIFIER:
							     -> pour l'instant, ne permet pas de placer les cercles qui ne sont pas relie au cercle central via 
								    des sites interieurs...
									(mais au moins ca marche si le bord est de taille 3 pour le calcul du packing maximal par inversion)


-> Circle packing hyperbolic: - methode pour claculer les s-radii CPU : semble OK, � verifier.
                              - methode pour claculer les s-radii openCL : a faire.
							   
							   - methode pour le layout des cercle : A MODIFIER
							   -> pour l'instant, ne permet pas de placer les cercles qui ne sont pas relie au cercle central via
							   des sites interieurs...

   -> dans le cas hyperbolic, faire gaffe au perte de precision pour les s-radii prochent de 1 (ie de rayon hyperbolique petit). 



*************************************************************************************************************************************/




namespace mtools
	{



	/*  private stuff */
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
		template<typename FPTYPE, typename GRAPH> FPTYPE errorL2euclidian(const GRAPH & gr, const std::vector<FPTYPE> & rad, const int N)
			{
			const FPTYPE twopi = (2*acos((FPTYPE)-1));
			FPTYPE e = (FPTYPE)0;
			FPTYPE C = (FPTYPE)0;
			for (int i = 0; i < N; ++i) // use Kahan summation algorithm
				{
				if (gr[i].size() > 1)
					{
					const FPTYPE a = (angleSumEuclidian(i, gr, rad) - twopi);
					FPTYPE Y = (a*a) - C;
					FPTYPE T = e + Y;
					C = (T - e) - Y;
					e = T;
					}
				}
			return sqrt(e);
			}


		/** Compute the L1 error for the angle sum for all vertices on the range [0,N-1] **/
		template<typename GRAPH, typename FPTYPE> FPTYPE errorL1euclidian(const GRAPH & gr, const std::vector<FPTYPE> & rad, const int N)
			{
			const FPTYPE twopi = 2 * acos((FPTYPE)-1);
			FPTYPE e = (FPTYPE)0;
			FPTYPE C = (FPTYPE)0;
			for (int i = 0; i < N; ++i) // use Kahan summation algorithm
				{
				if (gr[i].size() > 1)
					{
					const FPTYPE a = (angleSumEuclidian(i, gr, rad) - twopi);
					FPTYPE Y = ((a > (FPTYPE)0) ? a : -a) - C;
					FPTYPE T = e + Y;
					C = (T - e) - Y;
					e = T;
					}
				}
			return e;
			}


		/**
		* Perform an exploration of the graph that can be used for the layout of the circles.
		*
		* @param	gr		  	The graph to explore.
		* @param	v0		  	The start vertex v0.
		* @param	v1		  	The second vertex.
		* @param	v1interior	true to also visit the neighour around v1.
		* @param	fun		  	Function of the form bool f(int x,int y,int iz) called for each new
		* 						vertex visited z.
		*
		* @return	A vector where vec[i] = 1 if circle i was lay out and 0 vec[i] = 0 if it was not encountered. (v0 and v1 are set to 1).
		**/
		template<typename GRAPH> std::vector<int> layoutExplorer(const GRAPH & graph, int v0, int v1, bool explorearoundv1, std::function<bool(int, int, int)> fun)
			{
			std::vector<int> doneCircle(graph.size(), 0);
			doneCircle[v0] = 1;
			doneCircle[v1] = 1;
			size_t tot = 0;
			std::queue<int> st;
			st.push(v0);
			if (explorearoundv1) st.push(v1);
			while (st.size() != 0)
				{
				int index = st.front(); st.pop();
				auto it = graph[index].begin();
				while (doneCircle[*it] == 0) { ++it; }
				auto sit = it, pit = it; ++it;
				if (it == graph[index].end()) { it = graph[index].begin(); }
				while (it != sit)
					{
					if (doneCircle[*it] == 0)
						{
						if (fun(index, *pit, *it)) { st.push(*it); }
						doneCircle[*it] = 1;
						tot++;
						}
					pit = it;
					++it;
					if (it == graph[index].end()) { it = graph[index].begin(); }
					}
				}
			return doneCircle;
			}


		}



	/**
	 * Compute the L2 error for the angle sum of the packing for all inner vertices.
	 *
	 * @param	gr			The graph.
	 * @param	boundary	The boundary: any vertex boundary[v] > 0 belongs to the exterior face thus not counted.
	 * @param	rad			The radiui.
	 **/
	template<typename FPTYPE, typename GRAPH> FPTYPE circlePackErrorL2euclidian(const GRAPH & gr, const std::vector<int> & boundary, const std::vector<FPTYPE> & rad)
		{
		size_t l = (int)gr.size();
		MTOOLS_INSURE((rad.size() == l) && (boundary.size() == l));
		const FPTYPE twopi = (2 * acos((FPTYPE)-1));
		FPTYPE e = (FPTYPE)0;
		FPTYPE C = (FPTYPE)0;
		for (int i = 0; i < l; ++i) // use Kahan summation algorithm
			{
			if (boundary[i] <= 0)
				{
				const FPTYPE a = (internals_circlepacking::angleSumEuclidian((int)i, gr, rad) - twopi);
				FPTYPE Y = (a*a) - C;
				FPTYPE T = e + Y;
				C = (T - e) - Y;
				e = T;
				}
			}
		return sqrt(e);
		}


	/**
	* Other version. Compute the L2 error for the angle sum of the packing for all inner vertices.
	*
	* @param	gr			The graph.
	* @param	boundary	The boundary: any vertex boundary[v] > 0 belongs to the exterior face thus not counted.
	* @param	circles			The circles around each vertex.
	**/
	template<typename FPTYPE, typename GRAPH> FPTYPE circlePackErrorL2euclidian(const GRAPH & gr, const std::vector<int> & boundary, const std::vector<Circle<FPTYPE> > & circles)
		{
		std::vector<FPTYPE> rad(circles.size());
		for (size_t i = 0;i < circles.size(); i++) { rad[i] = circles[i].radius; }
		return circlePackErrorL2euclidian(gr, boundary, rad);
		}



	/**
	* Compute the L1 error for the angle sum of the packing for all inner vertices.
	*
	* @param	gr			The graph.
	* @param	boundary	The boundary: any vertex boundary[v] > 0 belongs to the exterior face thus not counted.
	* @param	rad			The radiui.
	**/
	template<typename GRAPH, typename FPTYPE> FPTYPE circlePackErrorL1euclidian(const GRAPH & gr, const std::vector<int> & boundary, const std::vector<FPTYPE> & rad)
		{
		size_t l = (int)gr.size();
		MTOOLS_INSURE((rad.size() == l) && (boundary.size() == l));
		const FPTYPE twopi = (2 * acos((FPTYPE)-1));
		FPTYPE e = (FPTYPE)0;
		FPTYPE C = (FPTYPE)0;
		for (int i = 0; i < l; ++i) // use Kahan summation algorithm
			{
			if (boundary[i] <= 0)
				{
				const FPTYPE a = (internals_circlepacking::angleSumEuclidian((int)i, gr, rad) - twopi);
				FPTYPE Y = ((a >(FPTYPE)0) ? a : -a) - C;
				FPTYPE T = e + Y;
				C = (T - e) - Y;
				e = T;
				}
			}
		return e;
		}


	/**
	* Other version. Compute the L1 error for the angle sum of the packing for all inner vertices.
	*
	* @param	gr			The graph.
	* @param	boundary	The boundary: any vertex boundary[v] > 0 belongs to the exterior face thus not counted.
	* @param	circles			The circles around each vertex.
	**/
	template<typename FPTYPE, typename GRAPH> FPTYPE circlePackErrorL1euclidian(const GRAPH & gr, const std::vector<int> & boundary, const std::vector<Circle<FPTYPE> > & circles)
		{
		std::vector<FPTYPE> rad(circles.size());
		for (size_t i = 0;i < circles.size(); i++) { rad[i] = circles[i].radius; }
		return circlePackErrorL1euclidian(gr, boundary, rad);
		}




	/**
	 * Saves a circle packing into a file with the .p format of the CirclePack program by Stephenson.
	 * 
	 * The packing is saved in euclidian form. 
	 *
	 * IMPORTANT : for the time being, the value are stored on file using double precision...
	 * 
	 * @param	filename	name of the file.
	 * @param	graph   	the graph.
	 * @param	boundary	The boundary vector: any vertex boundary[v] > 0 belongs to the exterior face.
	 * @param	circles 	The circles.
	 * @param	alpha   	The alpha vertex: the one use to start the layout (must be interior).
	 * @param	beta		The beta vertex:  obsolete, (used to be a boundary vertex).
	 * @param	gamma   	The gamma vertex: indicate that this site should be on the positive imaginary line.
	 */
	template<typename FPTYPE, typename GRAPH> void saveCirclePacking(const std::string & filename, const GRAPH & graph, const std::vector<int> & boundary, const std::vector<Circle<FPTYPE> > & circles, int alpha = -1, int beta = -1, int gamma = -1)
		{
		const size_t l = graph.size();
		MTOOLS_INSURE((l > 0)&&(boundary.size() == l)&&(circles.size() == l));
		std::vector<std::vector<int> > gr = convertGraph<GRAPH , std::vector<std::vector<int> > >(graph);
		rotateGraphNeighbourList(gr, boundary);
		mtools::LogFile F(filename, false, false, false);
		F << "NODECOUNT:  " << l << "\n";
		F << "GEOMETRY: euclidian\n";
		if (alpha < 0) 
			{ // default: first non-boundary vertice
			for (size_t i = 0;i < l; i++) { if (boundary[i] <= 0) { alpha = (int)i; break; } }
			MTOOLS_INSURE(alpha > 0);
			}
		if (gamma == -1) { gamma = gr[alpha].front(); } // default: choose first neighbour of alpha. 
		F << "ALPHA/BETA/GAMMA: " << (alpha + 1) << " " << (beta + 1) << " " << (gamma + 1) << "\n";
		F << "FLOWERS: \n";
		for (size_t i = 0; i < graph.size(); i++)
			{
			F << (i + 1) << " ";
			if (boundary[i] > 0)
				{
				F << gr[i].size() - 1 << "  ";
				for (size_t j = 0;j < gr[i].size(); j++) { F << " " << (gr[i][j] + 1); }
				F << "\n";
				}
			else
				{
				F << gr[i].size() << "  ";
				for (size_t j = 0; j < gr[i].size(); j++) { F << " " << (gr[i][j] + 1); }
				F << " " << (gr[i][0] + 1) << "\n";
				}
			}
		F << "\n\nRADII: \n";
		int count = 0;
		for (size_t i = 0; i < graph.size(); i++)
			{
			count++;
			F << doubleToStringHighPrecision(circles[i].radius,16);
			if (count == 4) { F << "\n"; count = 0; } else { F << "   "; }
			}
		F << "\n\nCENTERS: \n";
		count = 0;
		for (size_t i = 0; i < graph.size(); i++)
			{
			count++;
			F << doubleToStringHighPrecision(circles[i].center.real(),16) << " " << doubleToStringHighPrecision(circles[i].center.imag(),16);
			if (count == 2) { F << "\n"; count = 0; } else { F << "   "; }
			}
		F << "\n\nEND\n\n";
		}


	 /**
	 * Load a circle packing in Stephenson CirclePack .p format.
	 *
	 * RQ: read circle and radius without conversion from a geometry to another. 
	 * 
	 * @param	filename	name of the file.
	 * @param	graph   	the graph.
	 * @param	boundary	The boundary vector (boundar[v] = 1 for boundary vertices and 0 otherwise).
	 * @param	circles 	The circles.
	 * @param	alpha   	The alpha vertex: the one use to start the layout (must be interior).
	 * @param	beta		The beta vertex:  obsolete, (used to be a boundary vertex).
	 * @param	gamma   	The gamma vertex: indicate that this site should be on the positive imaginary line.
	 **/
	template<typename FPTYPE, typename GRAPH> void loadCirclePacking(const std::string & filename, GRAPH & graph, std::vector<int> & boundary, std::vector<Circle<FPTYPE> > & circles, int & alpha, int & beta, int & gamma)
		{
		IFileArchive ar(filename);
		size_t nodecount = 0;
		graph.clear();
		boundary.clear();
		circles.clear();
		alpha = -1; beta = -1; gamma = -1;
		std::string s;
		ar & s; MTOOLS_INSURE(mtools::toLowerCase(s) == std::string("nodecount:"));
		ar & nodecount; MTOOLS_INSURE(nodecount > 0);
		ar & s;
		s = mtools::toLowerCase(s);
		while (1)
			{
			bool treated = false;
			if (s == std::string("end")) { MTOOLS_INSURE(graph.size() > 0); return; }
			MTOOLS_INSURE((s.size() != 0)&&(s[s.size()-1] == ':'));
			if (s == std::string("alpha/beta/gamma:"))
				{
				ar & alpha & beta & gamma; 
				alpha--; beta--; gamma--;
				treated = true;
				}
			if (s == std::string("flowers:"))
				{
				graph.resize(nodecount);
				boundary.resize(nodecount,0);
				circles.resize(nodecount);
				for(size_t i = 0; i < nodecount; i++)
					{
					int index; ar & index; index--;
					MTOOLS_INSURE((index >= 0) && (index < nodecount) && (graph[index].size() == 0));
					size_t nbc; ar & nbc; MTOOLS_INSURE(nbc >= 2);
					graph[index].reserve(nbc+1);
					for (size_t j = 0; j < nbc; j++) 
						{
						int nn; ar & nn; nn--; 
						MTOOLS_INSURE((nn >= 0) && (nn < nodecount));
						graph[index].push_back(nn);
						}
					int ln; ar & ln; ln--;
					MTOOLS_INSURE((ln >= 0) && (ln < nodecount));
					if (ln != graph[index].front()) { graph[index].push_back(ln); boundary[index] = 1; }
					}
				treated = true;
				}
			if (s == std::string("radii:"))
				{
				for (size_t i = 0; i < nodecount; i++) 
					{ 
					FPTYPE r; ar & r;  
					circles[i].radius = r; 
					}
				treated = true;
				}
			if (s == std::string("centers:"))
				{
				for (size_t i = 0; i < nodecount; i++)
					{
					FPTYPE a, b; ar & a & b;
					circles[i].center = mtools::complex<double>(a, b);
					}
				treated = true;
				}
			ar & s;
			s = mtools::toLowerCase(s);
			while (!treated)
				{
				if ((s == std::string("end")) || ((s.size()>0)&&(s[s.size()-1] == ':')) )
					{ 
					treated = true; 
					}
				else 
					{
					ar & s;
					s = mtools::toLowerCase(s);
					}
				}
			}
		}




	/**
	 * Convert distance from the origin from euclidian to hyperbolic
	 */
	template<typename FPTYPE> FPTYPE distRtoH(const FPTYPE & r) { return log((1 + r)/(1 - r)); }


	/**
	* Convert distance from the origin from hyperbolic to euclidian
	*/
	template<typename FPTYPE> FPTYPE distHtoR(const FPTYPE & h) { return (exp(h) - 1)/(exp(h) + 1); }


	/**
	 * convert distance from the s parametrization to real hyperbolic distance. s = exp(-h).
	 */
	template<typename FPTYPE> FPTYPE distHtoS(const FPTYPE & h) { return exp(-h); }


	/**
	* convert real hyperbolic distance to s parametrization. h = -log(s).
	*/
	template<typename FPTYPE> FPTYPE distStoH(const FPTYPE & s) { return -log(s); }


	/**
	 * convert hyperbolic, s_parametrized distance from origin to euclidian distance.
	 * Same as distHtoR(distStoH(s))
	 */
	template<typename FPTYPE> FPTYPE distStoR(const FPTYPE & s) { return (1-s)/(1+s); }


	 /**
	 * convert euclidian distance to hyperbolic, s_parametrized distance from origin.
	 * Same as distHtoS(distRtoH(s))
	 */
	template<typename FPTYPE> FPTYPE distRtoS(const FPTYPE & r) { return (1-r)/(1+r); }



	/**    
	* Let C1 be the circle centered at the origin with euclidian radius r1. 
	* Let C2 be a circle tangent to C1 with hyperbolic s-radius s2.
	* 
	* Returns the euclidian radii r2 of C2.
	* 
	* The computation is obtained in the following way. 
	* 1) Compute the hyperbolic h2 radius from s2 via h2 = -log s2
	* 2) Compute the euclidian raddi rr2 if C2 was centered at the origin rr2 = (exp(h2) - 1)/(exp(h2) + 1);  
	*    -> Combining the two transformations give rr2 = (1 - s2)/(1 + s2)

	* 3 Let M be the mobius transformation with M(-rr2) = r1 while leaving the whole
	*   real line invariant and with M(-1) = -1, M(1) = 1.  
	*   -> M(z) =  ((r1*rr2 + 1)*z + (r1 + rr2)) / ( (r1 + rr2)*z + (r1*rr2 + 1))
	* 
	* 3) The euclidian raddi r2 is given by r2 = (M(rr2) - r1)/2
	*
	* Putting everything together yields:
	* 
	*    r2 = 0.5*( 1 + (r1*s2)^2 - r1^2 -s2^2 )/( 1 + r1 + s2^2 - r1*(s2^2) )
	*   
	* This formula works even if s2 = 0 (ie the circle C2 has infinite radius). 
	**/
	template<typename FPTYPE> FPTYPE tangentCircleStoR(const FPTYPE & r1, const FPTYPE & s2)
		{
		const FPTYPE r1_sqr = r1*r1;
		const FPTYPE s2_sqr = s2*s2;
		return ((FPTYPE)1 + r1_sqr*s2_sqr - r1_sqr - s2_sqr  ) / (((FPTYPE)2)*( (FPTYPE)1 + r1 + s2_sqr - r1*s2_sqr ));
		}



	/**
	 * 
	 *  ********************
	 *  INCOMPLET : NE PLACE PAS TOUT LES RADII. (MANQUE CEUX QUI SONT SEPARE DU SITE CENTRAL PAR DES CERCLE FRONTIERE).
	 *  ********************
	 * 
	 * Compute the layout for an hyperbolic packing label (i.e when the radius are given in term of the hyperbolic
	 * metric instead of the euclidian one).
	 *
	 * @param	graph	   	The graph.
	 * @param	boundary   	The boundary. Any vertex v with boundary[v] > 0 is on the exterior face.
	 * @param	srad	   	The vector of hyperbolic radii given in s-radii format:  s = exp(-h) where h is the real 
	 * 						hyperbolic radius. Thus, s = 0 for infinite radius and s = 1 for null radius
	 * @param	strictMaths	true to raise an error if FPTYPE does not allows sufficient precision for layout. Otherwise, 
	 * 						the algorithm does its best but circles may end up overlapping. 
	 * @param	v0		   	Index of the start vertex to lay out at the origin of the disk or -1 to choose an arbirary one. 
	 *
	 * @return	The positions of the circles for the packing label. This yields a packing inside the unit disk. 
	 */
	template<typename FPTYPE, typename GRAPH> std::vector<Circle<FPTYPE> > computeCirclePackLayoutHyperbolic(const GRAPH & graph, const std::vector<int> & boundary, const std::vector<FPTYPE> & srad, bool strictMaths = false, int v0 = -1)
		{
		MTOOLS_INSURE(graph.size() == srad.size());
		MTOOLS_INSURE(graph.size() == boundary.size());
		if (v0 < 0) { for (size_t i = 0; i < boundary.size(); i++) { if (boundary[i] <= 0) { v0 = (int)i; break; } } }
		MTOOLS_INSURE(boundary[v0] <= 0);
		std::vector<Circle<FPTYPE> > circle(srad.size());		
		circle[v0] = Circle<FPTYPE>(complex<FPTYPE>((FPTYPE)0, (FPTYPE)0), distStoR(srad[v0]));  // lay the circle at the origin		
		int v1 = graph[v0].front();												//
		circle[v1].radius = tangentCircleStoR(circle[v0].radius, srad[v1]);		// lay v1 on the right of v0.
		circle[v1].center = circle[v0].radius + circle[v1].radius;				// 

		auto laidvec = internals_circlepacking::layoutExplorer(graph, v0, v1, (boundary[v1] <= 0), [&](int ix, int iy, int iz)->bool
			{
			auto hypcx = circle[ix].euclidianToHyperbolic().center; // hyperbolic center for C(x)
			mtools::Mobius<FPTYPE> M(hypcx); // Mobius transformation that centers the circle C(x) around 0 (M is an involution)

			FPTYPE rx = distStoR(srad[ix]);					// radius of C(x) when it is centered on 0
			if ((strictMaths) && ((rx == (FPTYPE)0.0))) { MTOOLS_ERROR("Precision error A. null radius (site " << ix << ")"); }
			if ((strictMaths) && ((std::isnan(rx))))    { MTOOLS_ERROR("Precision error A. NaN (site " << ix << ")"); }
			FPTYPE ry = tangentCircleStoR(rx, srad[iy]);	// radius of C(y) when C(x) centered on 0
			if ((strictMaths) && ((ry == (FPTYPE)0.0))) { MTOOLS_ERROR("Precision error B. null radius (site " << iy << ")"); }
			if ((strictMaths) && ((std::isnan(ry))))    { MTOOLS_ERROR("Precision error B. NaN (site " << iy << ")"); }
			FPTYPE rz = tangentCircleStoR(rx, srad[iz]);	// radius of C(z) when C(x) centered on 0
			if ((strictMaths) && ((rz == (FPTYPE)0.0))) { MTOOLS_ERROR("Precision error C. null radius (site " << iz << ")"); }
			if ((strictMaths) && ((std::isnan(rz))))    { MTOOLS_ERROR("Precision error C. NaN (site " << iz << ")"); }

			const FPTYPE & alpha = internals_circlepacking::angleEuclidian(rx, ry, rz); // angle <y,x,z>
			if ((strictMaths) && (isnan(alpha))) { MTOOLS_ERROR("Precision error D. null alpha (site " << iz << ")"); }

			const Circle<FPTYPE> Cy = M*circle[iy];	// position of C(y) after the tranformation M (ie when C(x) is centered at 0)

			auto w = (Cy.center)*complex<FPTYPE>(cos(alpha), sin(alpha)); // apply a rotation of angle alpha to the center of C(y)
			const FPTYPE norm = std::abs(w); // resize the lenght of the vector to be rx + rz.
			if (norm > (FPTYPE)0.0) { w /= norm; w *= (rx + rz); } else { if (strictMaths) { MTOOLS_ERROR("Precision error E (site " << iz << ")"); } }

			const Circle<FPTYPE> Cz(w, rz); // position of C(z) when C(x) is centered.
			circle[iz] = (M*Cz);			// move back to the correct position by applying the inverse tranformation.

			return (boundary[iz] <= 0); // explore also around iz if it is an interior vertex
			});
		// done layout out circle adjacent to an interior circle but there may still be some other one to lay out
		std::queue<int> queue;
		for (int i = 0; i < laidvec.size(); i++) { if (laidvec[i] == 0) { queue.push(i); } } // push all vertices that are not yet lais. 

		//
		// TODO : THE FOLLOWING ASSUME THAT THE REMAINING CIRCLES TO LAY OUT ARE HOROCYCLES (IE WE ARE WORKING WITH
		// A MAXIMAL HYPERBOLIC PACKING
		// -> adapt to deal with the general case.
		/*
		while(queue.size() >0)
			{
			const int iz = queue.front(); queue.pop();
			for (int k = 0; k < graph[iz].size(); k++)
				{
				int ix = graph[iz][k];
				if (laidvec[ix] > 0)
					{
					int iy = graph[iz][(k + 1) % ((int)graph[iz].size())];
					if (laidvec[iy] > 0)
						{ 
						int h = -1; 
						for (int j = 0; j < graph[ix].size(); j++) { if (graph[ix][j] == iy) { h = j; break; } }
						if (h >= 0)
							{ // ok, ix iy iz form a triangle and ix and iy are alreay laid out. 
							if (graph[ix][(h + 1) % ((int)graph[ix].size())] != iz) { int t = ix; ix = iy; iy = t; }
							// ok, we can lay out iz
							const FPTYPE & rx = circle[ix].radius;
							const FPTYPE & ry = circle[iy].radius;
							const FPTYPE rz = 1 / (1 / rx + 1 / ry - 1 + 2 * sqrt(1 / (rx*ry) - 1 / ry - 1 / rx)); // soddy circles theorem :-)
							const FPTYPE & alpha = internals_circlepacking::angleEuclidian(rx, ry, rz);
							if ((strictMaths) && (isnan(alpha))) { MTOOLS_ERROR("Precision error D. null alpha (site " << iz << ")"); }
							auto w = circle[iy].center - circle[ix].center;
							w = w*complex<FPTYPE>(cos(alpha), sin(alpha));
							const FPTYPE norm = std::abs(w);
							if (norm != (FPTYPE)0.0) { w /= norm; w *= (rx + rz); }
							else { if (strictMaths) { MTOOLS_ERROR("Precision error E (site " << iz << ")"); } }
							circle[iz].center = circle[ix].center + w;
							if ((circle[iz].center == circle[iy].center) || (circle[iz].center == circle[ix].center)) { if (strictMaths) { MTOOLS_ERROR("Precision error F (site " << iz << ")"); } }
							circle[iz].radius = rz;
							laidvec[iz] = 1;
							break;
							}
						}
					}
				}
			if (laidvec[iz] == 0) { queue.push(iz); }
			}
			*/
		// all done !
		return circle;
		}


	 /**
	 *  ********************
	 *  INCOMPLET : NE PLACE PAS TOUT LES RADII. (MANQUE CEUX QUI SONT SEPARE DU SITE CENTRAL PAR DES CERCLE FRONTIERE).
	 *  ********************
	 *  
	 * Compute the layout for an euclidian packing label.
	 *
	 * @param	graph	   	The graph.
	 * @param	boundary   	The boundary. Any vertex v with boundary[v] > 0 is on the exterior face.
	 * @param	srad	   	The vector of radii.
	 * @param	strictMaths	true to raise an error if FPTYPE does not allows sufficient precision for layout. Otherwise,
	 * 						the algorithm does its best but circles may end up overlapping anyway.
	 * @param	v0		   	Index of the start vertex to lay out at the origin of the disk or -1 to choose an arbirary one.
	 *
	 * @return	The positions of the circles for the packing label. 
	 */
	template<typename FPTYPE, typename GRAPH> std::vector<Circle<FPTYPE> > computeCirclePackLayout(const GRAPH & graph, const std::vector<int> & boundary, const std::vector<FPTYPE> & rad, bool strictMaths = false, int v0 = -1)
		{
		MTOOLS_INSURE(graph.size() == rad.size());
		MTOOLS_INSURE(graph.size() == boundary.size());
		if (v0 < 0) { for (size_t i = 0; i < boundary.size(); i++) { if (boundary[i] <= 0) { v0 = (int)i; break; } } }
		MTOOLS_INSURE(boundary[v0] <= 0);
		std::vector<Circle<FPTYPE> > circle(rad.size());
		circle[v0] = Circle<FPTYPE>(complex<FPTYPE>((FPTYPE)0, (FPTYPE)0), rad[v0]);
		int v1 = graph[v0].front();
		circle[v1] = Circle<FPTYPE>(complex<FPTYPE>(rad[v0] + rad[v1], (FPTYPE)0), rad[v1]);

		internals_circlepacking::layoutExplorer(graph, v0, v1, (boundary[v1] <= 0), [&](int ix, int iy, int iz)->bool
			{
			const FPTYPE & rx = rad[ix]; if ((strictMaths) && ((rx == (FPTYPE)0.0) || (isnan(rx)))) { MTOOLS_ERROR("Precision error A. null radius (site "<< ix << ")"); }
			const FPTYPE & ry = rad[iy]; if ((strictMaths) && ((ry == (FPTYPE)0.0) || (isnan(ry)))) { MTOOLS_ERROR("Precision error B. null radius (site " << iy << ")"); }
			const FPTYPE & rz = rad[iz]; if ((strictMaths) && ((rz == (FPTYPE)0.0) || (isnan(rz)))) { MTOOLS_ERROR("Precision error C. null radius (site " << iz <<")"); }
			const FPTYPE & alpha = internals_circlepacking::angleEuclidian(rx, ry, rz);
			if ((strictMaths) && (isnan(alpha))) { MTOOLS_ERROR("Precision error D. null alpha (site " << iz << ")"); }
			auto w = circle[iy].center - circle[ix].center;
			w = w*complex<FPTYPE>(cos(alpha), sin(alpha));
			const FPTYPE norm = std::abs(w);
			if (norm != (FPTYPE)0.0) { w /= norm; w *= (rx + rz); } else { if (strictMaths) { MTOOLS_ERROR("Precision error E (site " << iz << ")"); } }
			circle[iz].center = circle[ix].center + w;
			if ((circle[iz].center == circle[iy].center) || (circle[iz].center == circle[ix].center)) { if (strictMaths) { MTOOLS_ERROR("Precision error F (site " << iz << ")"); } }
			circle[iz].radius = rad[iz];
			return (boundary[iz] <= 0);
			});
		return circle;
		}



	/**
	* Draw the graph of the circle packing.
	* -> Draw the circle around each vertex
	*
	* @param [in,out]	img	The image to draw onto. It is not erased first.
	* @param	R		   	The range represented by the image.
	* @param	circles	   	The vector of circles.
	* @param	gr		   	The graph.
	* @parma    filled		true to draw filled circles.						
	* @param	color	   	color for drawing.
	* @param	opacity	   	opacity for drawing.
	* @param	firstIndex 	First index of the sub-graph to draw (included)
	* @param	lastIndex  	Last index of the sub-graph to draw (excluded) or -1 = until the end.
	**/
	template<typename FPTYPE> void drawCirclePacking_Circles(Image & img, const mtools::Box<FPTYPE, 2> & R, const std::vector<Circle<FPTYPE> > circles, const std::vector<std::vector<int> > & gr, bool filled, RGBc color, float opacity = 1.0f, int firstIndex = 0, int lastIndex = -1)
		{
		color.multOpacity(opacity);
		MTOOLS_ASSERT(circles.size() == gr.size());
		if ((lastIndex < 0) || (lastIndex > (int)(gr.size()))) lastIndex = (int)(gr.size());
		for (int i = firstIndex; i < lastIndex; i++)
			{
			if (filled)
				{
				img.canvas_draw_filled_circle(R, circles[i].center, circles[i].radius, color, color,true);
				}
			else
				{
				img.canvas_draw_circle(R, circles[i].center, circles[i].radius, color, true, false);
				}
			}
		}



	/**
	* Draw the graph of the circle packing.
	* -> Draw lines between the position of the centers of each circles.
	*
	* @param [in,out]	img	The image to draw onto. It is not erased first.
	* @param	R		   	The range represented by the image.
	* @param	circles	   	The vector of circles.
	* @param	gr		   	The graph.
	* @param	color	   	color for drawing.
	* @param	opacity	   	opacity for drawing.
	* @param	firstIndex 	First index of the sub-graph to draw (included)
	* @param	lastIndex  	Last index of the sub-graph to draw (excluded) or -1 = until the end.
	**/
	template<typename FPTYPE> void drawCirclePacking_Graph(Image & img, const mtools::Box<FPTYPE, 2> & R, const std::vector<Circle<FPTYPE> > circles, const std::vector<std::vector<int> > & gr, RGBc color, float opacity = 1.0f, int firstIndex = 0, int lastIndex = -1)
		{
		color.multOpacity(opacity);
		MTOOLS_ASSERT(circles.size() == gr.size());
		if ((lastIndex < 0) || (lastIndex > (int)(gr.size()))) lastIndex = (int)(gr.size());
		for (int i = firstIndex; i < lastIndex; i++)
			{
			for (auto it = gr[i].begin(); it != gr[i].end(); ++it)
				{
				if ((*it >= firstIndex) && (*it < lastIndex)) { img.canvas_draw_line(R, circles[i].center, circles[*it].center, color, true); }
				}
			}
		}


	/**
	* Draw the graph of the circle packing.
	* -> Draw the labels around each vertex
	*
	* @param [in,out]	img	The image to draw onto. It is not erased first.
	* @param	R		   	The range represented by the image.
	* @param	circles	   	The vector of circles.
	* @param	gr		   	The graph.
	* @parma    fontsize	size of the font to use
	* @param	color	   	color for drawing.
	* @param	opacity	   	opacity for drawing.
	* @param	firstIndex 	First index of the sub-graph to draw (included)
	* @param	lastIndex  	Last index of the sub-graph to draw (excluded) or -1 = until the end.
	**/
	template<typename FPTYPE> void drawCirclePacking_Labels(Image & img, const mtools::Box<FPTYPE, 2> & R, const std::vector<Circle<FPTYPE> > circles, const std::vector<std::vector<int> > & gr, int fontsize, RGBc color, float opacity = 1.0f, int firstIndex = 0, int lastIndex = -1)
		{
		color.multOpacity(opacity);
		MTOOLS_ASSERT(circles.size() == gr.size());
		if ((lastIndex < 0) || (lastIndex > (int)(gr.size() - 1))) lastIndex = (int)(gr.size());
		for (int i = firstIndex; i < lastIndex; i++)
			{

			img.canvas_draw_text(R, circles[i].center, mtools::toString(i), mtools::MTOOLS_TEXT_CENTER, color, fontsize);
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
				for (size_t i = 0; i < l; i++) 
					{ 
					if (boundary[_perm[(int)i]] > 0) 
						{ 
						_nb = i; break; 
						} 
					}
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
			FPTYPE errorL2() const { return internals_circlepacking::errorL2euclidian(_gr, _rad, (int)_nb); }


			/**
			* Compute the error in the circle radius in L1 norm.
			*/
			FPTYPE errorL1() const { return internals_circlepacking::errorL1euclidian(_gr, _rad, (int)_nb); }


			/**
			 * Run the algorithm for computing the value of the radii.
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
						#endif
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











		/**
		 * Class used to compute the radii associated with the hyperbolic circle packing
		 * of a triangulation with a boundary.
		 *
		 * The algorithm is taken from Collins and Stephenson (2003).
		 *
		 * This class computes a 'packing label' in the hyperbolic case. The radii are
		 * taken and coimputed is the so called s-format where 
		 *                                  
		 *                                    s = exp(-h)
		 *                                    
		 * and h is the true hyperbolic radii. Thus s = 0 means infinite radius and s = 1
		 * means 0 radius. 
		 *
		 * @tparam	FPTYPE	Floating type that should be used during calculation.
		 **/
		template<typename FPTYPE = double> class CirclePackingLabelHyperbolic
			{

			public:

			/**
			 * Constructor.
			 *
			 * @param	verbose	true to print info to mtools::cout during packing.
			 */
			CirclePackingLabelHyperbolic(bool verbose = false) : _verbose(verbose), _pi(acos((FPTYPE)(-1.0))) , _twopi(2*acos((FPTYPE)(-1.0)))
				{
				}


			/* dtor, empty object */
			~CirclePackingLabelHyperbolic() {}


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
			* All inside s-radii are set to 0.5 and all boundary s-radii are set to 0.0 (maximal packing
			* in the disk with infinite radius for boundary circles. 
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
				for (size_t i = 0; i < l; i++)
					{
					if (boundary[_perm[(int)i]] > 0)
						{
						_nb = i; break;
						}
					}
				MTOOLS_INSURE((_nb > 0) && (_nb < l - 2));
				_rad.resize(l, (FPTYPE)0);
				for (size_t i = 0;i < _nb; i++)  { _rad[i] = (FPTYPE)0.5; }
				}


			/**
			* Sets the s-radii of the circle around each vertices.
			* The s-radii associated with the boundary vertices are not modified during
			* the circle packing algorithm and serve as Dirichlet condition. 
			*
			* @param	rad	The s-radii. Any values < 0.0 or > 1.0 is clipped.
			**/
			void setRadii(const std::vector<FPTYPE> & rad)
				{
				const size_t l = _gr.size();
				MTOOLS_INSURE(rad.size() == l);
				_rad = _perm.getPermute(rad);
				for (size_t i = 0; i < l; i++) { if (_rad[i] < (FPTYPE)0.0) _rad[i] = 0.0; else if (_rad[i] > (FPTYPE)1.0) _rad[i] = 1.0; }
				}


			/**
			* Sets all s-radii to r.
			**/
			void setRadii(FPTYPE r = 0.5)
				{
				MTOOLS_INSURE((r >= (FPTYPE)0.0)&&(r <= (FPTYPE)1.0));
				const size_t l = _gr.size();
				_rad.resize(l);
				for (size_t i = 0; i < l; i++) { _rad[i] = r; }
				}


			/**
			* Return the list of s-radii.
			*/
			std::vector<FPTYPE> getRadii() const { return _perm.getAntiPermute(_rad); }


			/**
			*
			*                            TODO
			*
			* Compute the error in the circle radius in L2 norm.
			*/
			FPTYPE errorL2() const 
				{ 
				//   TODO, compute hyperbolic error
				return 1;
				}


			/**
			*
			*                            TODO 
			*
			* Compute the error in the circle radius in L1 norm.
			*/
			FPTYPE errorL1() const 
				{ 
				//   TODO, compute hyperbolic error
				return 1;
				}


			/**
			 * Run the algorithm for computing the value of the radii.
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
				FPTYPE minc = errorL2();  // <- currently, this is wrong since errorL2 is not implemented.
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
					_rad0 = _rad;
					for (int i = 0; i < nb; i++)
						{
						FPTYPE suma = 0;
						const FPTYPE r = _rad[i];
						const FPTYPE sr = sqrt(r);
						const int N = (int)_gr[i].size();
						const FPTYPE twor = 2 * r;
						const FPTYPE r2 = _rad[_gr[i][0]];
						FPTYPE m2 = (r2 > 0) ? (1 - r2) / (1 - r * r2) : (FPTYPE)1;
						for (int k = 1; k < N; k++)
							{
							const FPTYPE r3 = _rad[_gr[i][k]];
							const FPTYPE m3 = (r3 > 0) ? (1 - r3) / (1 - r * r3) : (FPTYPE)1;
							FPTYPE y = 1 - twor * m2 * m3;
							if (y < -1.0) y = -1.0; else if (y > 1.0) y = 1.0;
							suma += acos(y);
							m2 = m3;
							}
						const FPTYPE r3 = _rad[_gr[i][0]];
						const FPTYPE m3 = (r3 > 0) ? (1 - r3) / (1 - r * r3) : (FPTYPE)1;
						FPTYPE y = 1 - twor * m2 * m3;
						if (y < -1.0) y = -1.0; else if (y > 1.0) y = 1.0;
						suma += acos(y);

						const FPTYPE denom = 1.0 / (2.0 * ((FPTYPE)N));
						const FPTYPE del = sin(_twopi * denom);
						const FPTYPE bet = sin(suma * denom);
					    FPTYPE rr2 = (bet - sr) / (bet * r - sr); 
						if (rr2 > 0) 
							{
							const FPTYPE t1 = 1 - rr2;
							const FPTYPE t2 = 2 * del;
							const FPTYPE t3 = t2 / (sqrt(t1 * t1 + t2 * t2 * rr2) + t1);
							rr2 = t3 * t3;
							}
						else { rr2 = del * del; }
						const FPTYPE e = suma - _twopi;
						c += e*e;
						_rad[i] = rr2;
						}
					c = sqrt(c); 
					if (c < minc) { minc = c; }
					lambda = c / c0;
					fl = true;				
					/*

					ACCELERATION NOT WORKING, SHOULD CHANGE SOMETIHNG...

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
							else
								{
								const FPTYPE d2 = -(1 - _rad[i]) / d;
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
					*/
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
				std::vector<FPTYPE>				_rad;		// s-radii
				size_t							_nb;		// number of internal vertices

			};






/**********************************************************************************************************
*
* 
* OPENCL VERSION : only defined if openCL extension is activated.
* 
*
**********************************************************************************************************/


#if (MTOOLS_USE_OPENCL)


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
			* Compute the error in the circle radius in L2 norm.
			*/
			FPTYPE errorL2() const { return internals_circlepacking::errorL2euclidian(_gr, _rad, (int)_nb); }


			/**
			* Compute the error in the circle radius in L1 norm.
			*/
			FPTYPE errorL1() const { return internals_circlepacking::errorL1euclidian(_gr, _rad, (int)_nb); }


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



