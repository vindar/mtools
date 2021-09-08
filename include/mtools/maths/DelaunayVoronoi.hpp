/** @file circle.hpp */
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

#include <vector>

#include "vec.hpp"


namespace mtools
{



    /**
     * Compute the Delaunay Triangulation and the associated Voronoi diagram of a set of point in
     * the plane R^2.
     * 
     * Wrapper class for the Triangle library (https://www.cs.cmu.edu/~quake/triangle.html) that
     * takes care of memory allocation / deallocation
     * 
     * 
     * Example of usage:
     * 
    void testDelaunayVoronoi()
	{
	mtools::MT2004_64 gen(0);
	mtools::DelaunayVoronoi DV;

	// add 100 points uniformly distributed in [0,1]^2
	for (int i = 0; i < 100; i++) DV.DelaunayVertices.push_back(mtools::fVec2(Unif(gen), Unif(gen)));
		
	// compute the Delaunay trianulation and Voronoi diagram
	DV.compute(); 

	// draw the graphs
	mtools::Plotter2D plotter; 
	auto canvas = mtools::makeFigureCanvas(2);

	// draw the Delaunay triangulation
	int nb_D_e = (int)DV.DelaunayEdgesIndices.size();
	for (int k = 0; k < nb_D_e; k++)
		{
		mtools::iVec2 e = DV.DelaunayEdgesIndices[k];
		canvas(mtools::Figure::Line(DV.DelaunayVertices[e.X()], DV.DelaunayVertices[e.Y()], mtools::RGBc::c_Red), 0);
		}

	// draw the Voronoi diagram
	int nb_V_e = (int)DV.VoronoiEdgesIndices.size();
	for (int k = 0; k < nb_V_e; k++)
		{
		auto e = DV.VoronoiEdgesIndices[k];
		mtools::fVec2 P1 = DV.VoronoiVertices[e.X()];
		if (e.Y() == -1)
			{ // semi-infinite ray
			mtools::fVec2 N = DV.VoronoiNormals[e.X()];
			canvas(mtools::Figure::Line(P1, P1 + N, mtools::RGBc::c_Green), 1);
			}
		else
			{ // regular edge
			mtools::fVec2 P2 = DV.VoronoiVertices[e.Y()];
			canvas(mtools::Figure::Line(P1, P2, mtools::RGBc::c_Black), 1);
			}
		}

	// plot
	auto P = mtools::makePlot2DFigure(canvas, 4, "Delaunay Voronoi");
	plotter[P];
	plotter.range().setRange({ 0,1,0,1 });
	plotter.plot();
	}

    *
    **/
    class DelaunayVoronoi
    {

    public:


        /**   
         * ctor. Does nothing
         * Waits for compute() to be called. 
         **/
        DelaunayVoronoi()
            {
            }


        /**
         * Compute the Voronoi diagrams and the Delaunay triangulations of a set of vertices in the
         * plane R^2.
         * 
         * After this constructor, the resulting Delaunay/Voronoi graphs can be queried by examining the
         * vectors below that compose this class.
         *
         * @param   vertices    Set of vertice in the plane to triangulate and compute the Voronoi
         *                      diagrams.
        **/
        DelaunayVoronoi(const std::vector<mtools::fVec2>& vertices) : DelaunayVertices(vertices)
            {
            compute();
            }


        /**
         * Set of vertices in the graph / delaunay triangulation
         *
         * This is the vector used for the input set of vertices zhen compute() is called.
        **/
        std::vector<mtools::fVec2>  DelaunayVertices;


        /**
         * Compute the Voronoi diagram and the Delaunay triangulation of the set of vertices stored in
         * the input vector: DelaunayVertices.
         * 
         * After this constructor, the resulting Delaunay/Voronoi graphs can be queried by examining the
         * vectors below that compose this class.
        **/
        void compute();


        /**
        * Set of vertices in the Voronoi diagram.
         *
         * [This vector is set by compute()]
        **/
        std::vector<mtools::fVec2>  VoronoiVertices;


        /**
         * Edge in the Delaunay triangulation determined by the indexes of the two boundary vertices in
         * the vector DelaunayVertices.
         *
         * [This vector is set by compute()]
        **/
        std::vector<mtools::iVec2>  DelaunayEdgesIndices;


        /**
         * Triangle in the Delaunay triangulation given by the indexes of the three boundary vertices in
         * the input vector DelaunayVertices.
         * 
         * Triangle are given in counter-clockwise order.
         *
         * [This vector is set by compute()]
        **/
        std::vector<mtools::iVec3>  DelaunayTrianglesIndices;


        /**
         * Edge in the Voronoi diagram determined by the indexes of the two boundary vertices in
         * the vector VoronoiVertices
         *
         * Special case:
         *
         * if VoronoiEdgesIndices[i] = fVec2{i,-1}, then this indicates a (boundary) infinite ray emanating
         * from VoronoiVertices[i] and going in the direction VoronoiNormals[i]
         *
         * [This vector is set by compute()]
        **/
        std::vector<mtools::iVec2>  VoronoiEdgesIndices;


        /**
         * Normal vector giving the direction of semi infinite rays emanating from some of the Voronoi
         * vertices.
         *
         * - VoronoiNormals[i] = fVec2{0,0} if i is a 'normal' interior vertex of the voronoi diagram
         * - VoronoiNormals[i] = fVec2{x,y} with x^2+y^2 = 1 if a ray emanate from vertex i in the
         *                                  voronoi diagram and follows direction fVec2{x,y}
         *
         * [This vector is set by compute()]
        **/
        std::vector<mtools::fVec2>  VoronoiNormals;


    private:


        void _copy(std::vector<mtools::fVec2>& vec, const double* data, int len);

        void _copy(std::vector<mtools::iVec2>& vec, const int* data, int len);

        void _copy(std::vector<mtools::iVec3>& vec, const int* data, int len);

        void _freeall(void * p);

    };


}

/** end of file DelaunayVoronoi.hpp */

