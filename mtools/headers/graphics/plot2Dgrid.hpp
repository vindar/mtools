/** @file plot2Dgrid.hpp */
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


#include "maths/vec.hpp"
#include "maths/rect.hpp"
#include "rgbc.hpp"
#include "plotter2Dobj.hpp"
#include "drawable2Dobject.hpp"

#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Check_Button.H>

#include <atomic>

namespace mtools
{
    /**
     * Class used for creating objct that draws a grid. A grid is already included in a Plotter2D
     * object but is disabled by default but can be activated via `Plotter2D::gridOBject()` or via
     * the UI. However, it may be useful to create other grid so that they can be drawn
     * simultaneously on the plot.
     * 
     * @code{.cpp}
     * int main()
     * {
     * auto Grid = makePlot2DGrid();              // create a grid object
     * Grid.color(mtools::RGBc::c_Blue);             // blue color
     * Grid.fitToAxes();                             // the grid shall follow the range and match the axes graduation.
     * Plotter2D PS;                                 // the plotter
     * PS.gridObject(true)->setUnitCells();          // use the grid to draw unit cells
     * PS[Grid];                                     // insert the first grid inside the plotter
     * PS.range().setRange(fRect(-10, 10, -20, 20)); // set the range to be [-10,10]x[-20x20] (for fun)
     * PS.range().setRatio1();                       // rectify to keep a 1:1 aspect ratio (for fun)
     * PS.plot();                                    // make the plot
     * }
     * @endcode.
     **/
    class Plot2DGrid : public internals_graphics::Plotter2DObj, protected internals_graphics::Drawable2DObject
    {

    public:

        /**
         * Constructor.
         *
         * @param   fitToAxes   By default, the parameter of the grid are automatically modifed to fit the
         *                      graduations of the axes. Set to false to draw a unit grid instead.
         * @param   name        The name of the plot (default "Grid").
         **/
        Plot2DGrid(bool fitToAxes = true, std::string name = "Grid");


        /**
         * Move constructor.
         **/
        Plot2DGrid(Plot2DGrid && o);


        /**
         * Destructor. Remove the object if it is still inserted.
         **/
        virtual ~Plot2DGrid();


        /**
         * Set the parameter for drawing a unit grid (space = 1.0, offset = 0.0).
         **/
        void setUnitGrid();


        /**
         * Set the parameter for drawing unit cells (space = 1.0, offset = 0.5).
         **/
        void setUnitCells();


        /**
         * Determines the parameter change dynamically to fit with the axes.
         * @return  true if we adapt and false if the paramters are fixed. 
         **/
        bool fitToAxes() const;


        /**
         * Set/Unset the dynamic "fit to axes" mode.
         *
         * @param   fit true to change the paramter of the grid dynamically with the range to fit to the
         *              axes and false to fix the parameters.
         **/
        void fitToAxes(bool fit);



        /**
         * Return the horizontal spacing
         *
         * @return  the spacing or <= 0 if there is no vertical lines.
         **/
        double horizontalSpacing() const;


        /**
         * Set the horizontal spacing.
         *
         * @param   val The desired spacing or <=0 to remove vertical lines.
         **/
        void horizontalSpacing(double val);


        /**
         * the horizontal offset.
         *
         * @return  the vertical offset for the vertical lines.
         **/
        double horizontalOffset() const;


        /**
         * Set the horizontal offset.
         *
         * @param   offset  The offset for the vertical lines.
         **/
        void horizontalOffset(double offset);


        /**
         * Return the vertical spacing
         *
         * @return  the spacing or <= 0 if there is no horizontal lines.
         **/
        double verticalSpacing() const;


        /**
         * Set the vertical spacing.
         *
         * @param   val The desired spacing or <=0 to remove horizontal lines.
         **/
        void verticalSpacing(double val);


        /**
         * the vertical offset.
         *
         * @return  the vertical offset for the horizontal lines.
         **/
        double verticalOffset() const;


        /**
         * Set the vertical offset.
         *
         * @param   offset  The offset for the horizontal lines.
         **/
        void verticalOffset(double offset);


        /**
         * The drawing color.
         *
         * @return  the color used to draw the grid.
         **/
        RGBc color() const;


        /**
         * Set the drawing color.
         *
         * @param   col The color that should be used when drawing the grid.
         **/
        void color(RGBc col);


        protected:


        /**
         * Override of the setParam method from the Drawable2DObject interface
         **/
        virtual void setParam(mtools::fRect range, mtools::iVec2 imageSize);


        /**
         * Override of the drawOnto() method from the Drawable2DObject interface
         **/
        virtual int drawOnto(cimg_library::CImg<unsigned char> & im, float opacity = 1.0);


        /**
         * Override of the removed method from the Plotter2DObj base class
         **/
        virtual void removed(Fl_Group * optionWin);


        /**
         * Override of the inserted method from the Plotter2DObj base class
         **/
        virtual internals_graphics::Drawable2DObject * inserted(Fl_Group * & optionWin, int reqWidth);


        private:


            /* change the range to fit with the axes graduations */
            void _fitToAxes();

            const float DEFAULT_OPACITY = 0.2f; // the default opacity to use when drawing the grid. 

            /* updating the widget */
            void _updateWidgets();

            /* fix the offset range */
            void _fixOffset();


            /* callbacks for the fltk widgets */
            static void _hspaceCB_static(Fl_Widget * W, void * data);
            void _hspaceCB(Fl_Widget * W);

            static void _vspaceCB_static(Fl_Widget * W, void * data);
            void _vspaceCB(Fl_Widget * W);

            static void _hoffsetCB_static(Fl_Widget * W, void * data);
            void _hoffsetCB(Fl_Widget * W);

            static void _voffsetCB_static(Fl_Widget * W, void * data);
            void _voffsetCB(Fl_Widget * W);

            static void _colorCB_static(Fl_Widget * W, void * data);
            void _colorCB(Fl_Widget * W);

            static void _unitGridCB_static(Fl_Widget * W, void * data);
            void _unitGridCB(Fl_Widget * W);

            static void _unitCellsCB_static(Fl_Widget * W, void * data);
            void _unitCellsCB(Fl_Widget * W);

            static void _fixAxesCB_static(Fl_Widget * W, void * data);
            void _fixAxesCB(Fl_Widget * W);

            Plot2DGrid(const Plot2DGrid &) = delete;                    // no copy
            Plot2DGrid & operator=(const Plot2DGrid &) = delete;        //

            Fl_Group *          _win;
            Fl_Button *         _colorButton;
            Fl_Button *         _unitGridButton;
            Fl_Button *         _unitCellsButton;
            Fl_Check_Button*    _fitAxesCheckbox;
            Fl_Input *          _hspaceInput;
            Fl_Input *          _vspaceInput;
            Fl_Input *          _hoffsetInput;
            Fl_Input *          _voffsetInput;

            std::atomic<RGBc>   _color;
            std::atomic<double> _hspace;
            std::atomic<double> _hoffset;
            std::atomic<double> _vspace;
            std::atomic<double> _voffset;
            std::atomic<bool>   _fittoaxes;

            fRect _range;           // the range we should use to draw the axes
            iVec2 _imageSize;       // the requested size of the image. 
        };


    /**
     * Factory function for creating grid plot objects.
     *
     * @param   fitToAxes   true to fit the grid with the axes graduations.
     * @param   name        The name of the grid plot.
     *
     * @return  The Plot2DGrid object.
     **/
    inline Plot2DGrid makePlot2DGrid(bool fitToAxes = true, std::string name = "Grid")
        {
        return Plot2DGrid(fitToAxes,name);
        }

}


/* end of file */


