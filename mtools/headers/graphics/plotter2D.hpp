/** @file plotter2D.hpp */
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
#include "misc/error.hpp"
#include "customcimg.hpp"
#include "rangemanager.hpp"

#include "graphics/plot2Daxes.hpp"
#include "graphics/plot2Dgrid.hpp"


namespace mtools
{

    /* forward declaration */
    namespace internals_graphics
    {
        class Plotter2DObj;     // basic class of a plottable object
        class Plotter2DWindow;  // the plotter window
    }


/**
 * The plotter2D class. Take an element of type Plotter2DObj and display it.
 *
 * Use the following global factory method to construct objects that can be insered in the
 * plotter:
 *
 * - FUNCTION:    "auto F = makePlot2DFun(fun,[mindomain],[maxdomain]);"
 * - C-ARRAY:     "auto A = makePlot2DArray(tab,len,[mindomain],[maxdomain],[name]);"
 * - STL VECTOR:  "auto V = makePlot2DVector(vec,[mindomain],[maxdomain],[fixedDomain],[name]);"
 * - LATTICEOBJ:
 *     - "auto L = makePlot2DLattice(obj);"
 *     - "auto L = makePlot2DLattice(LatticeObj<colorFct>::get());"
 *     - "auto L = makePlot2DLattice(LatticeObjImage<colorFct,imageFct>::get());"
 * - AXES:        "auto Axes = makePlot2DAxes();"
 * - GRID:        "auto Axes = makePlot2DGrid();".
**/
class Plotter2D
{
    public:

    static const int POS_TOP = 0;           ///< the top of the screen
    static const int POS_BOTTOM = -1;       ///< the bottom of the screen
    static const int POS_LEFT = 0;          ///< the left of the screen
    static const int POS_RIGHT = -1;        ///< the right of the screen
    static const int POS_CENTER = -2;       ///< the center of the screen

    static const int DEFAULT_W = 1120;      ///< default width of the plotter window
    static const int DEFAULT_H = 820;       ///< default height of the plotter window
    static const int DEFAULT_X = POS_RIGHT; ///< default X position of the plotter window
    static const int DEFAULT_Y = POS_TOP;   ///< default Y position of the plotter window

    static const int MIN_W = 600;           ///< min width of the plotter window
    static const int MIN_H = 600;           ///< min height of the plotter window


    /**
     * Constructor. Create a plotter and insert a plot object.
     *
     * @param [in,out]  obj the object to draw or nullptr if there are none right now (default).
     * @param   addAxes     true to add a Plot2DAxes object on top.
     * @param   addGrid     true to add a Plot2DGrid object on top.
     * @param   W           The width of the plotter window or DEFAULT_W or MIN_W.
     * @param   H           The height of the plotter window or DEFAULT_H or MIN_H.
     * @param   X           The X coordinate of the plotter window or POS_LEFT, POS_RIGHT, POS_CENTER.
     * @param   Y           The Y coordinate  of the plotter window or POS_TOP, POS_BOTTOM,
     *                      POS_CENTER.
     **/
    Plotter2D(internals_graphics::Plotter2DObj & obj, bool addAxes = true, bool addGrid = false, int W = DEFAULT_W, int H = DEFAULT_H, int X = DEFAULT_X, int Y = DEFAULT_Y);


    /**
     * Constructor. Create an initially empty plotter.
     *
     * @param   addAxes true to add a Plot2DAxes object on top.
     * @param   addGrid true to add a Plot2DGrid object on top.
     * @param   W       The width of the plotter window or DEFAULT_W or MIN_W.
     * @param   H       The height of the plotter window or DEFAULT_H or MIN_H.
     * @param   X       The X coordinate of the plotter window or POS_LEFT, POS_RIGHT, POS_CENTER.
     * @param   Y       The Y coordinate  of the plotter window or POS_TOP, POS_BOTTOM, POS_CENTER.
     **/
    Plotter2D(bool addAxes = true, bool addGrid = false, int W = DEFAULT_W, int H = DEFAULT_H, int X = DEFAULT_X, int Y = DEFAULT_Y);


    /**
     * Destructor. If there is an inserted object, it is removed but it is not deleted.
     **/
    ~Plotter2D();


    /**
     * Insert a Plotter2DObj into the plotter. Remove the current object if there are any. The
     * inserted object can possibly change the range of the plot.
     *
     * @param [in,out]  obj pointer to the object or nullptr to remove the current object.
     **/
    void add(internals_graphics::Plotter2DObj * obj);


    /**
     * Set a Plotter2DObj into the plotter. Remove the current object if there are any. The inserted
     * object can possibly modify the range of the plot.
     *
     * @param [in,out]  obj The object to insert.
     **/
    void add(internals_graphics::Plotter2DObj & obj);


    /**
     * Return whether an Plot2DAxes object is added on top
     *
     * @return  a pointer to the axes Object if there is one and nullptr otherwise.
     **/
    Plot2DAxes * axesObject() const;


    /**
     * Decides whether we add a Plot2DAxes object on top.
     *
     * @param   status  true to add and false to remove.
     *
     * @return  nullptr is status is false, a pointer to the axe object otherwise.
     **/
    Plot2DAxes * axesObject(bool status);


    /**
     * Return whether an Plot2DGrid object is added on top.
     *
     * @return  a pointer to the grid Object if there is one and nullptr otherwise.
     **/
    Plot2DGrid * gridObject() const;


    /**
     * Decides whether we add a Plot2DGrid object on top.
     *
     * @param   status  true to add and false to remove.
     *
     * @return  nullptr is status is false, a pointer to the grid object otherwise.
     **/
    Plot2DGrid * gridObject(bool status);


    /**
     * Set the Plotter2DObj into the plotter. Remove the current object if there are any. Same as
     * `set(obj)`. The inserted object can possibly modify the range of the plotter.
     *
     * @param [in,out]  obj The object to insert.
     *
     * @return  The plotter itself for chaining.
     **/
    Plotter2D & operator[](internals_graphics::Plotter2DObj & obj);


    /**
     * Return the number of object inserted in the plotter.
     *
     * @return  the number of objects in the plotter
     **/
    int nbObject() const;


    /**
     * Get the object at a given position in the plotter (object 0 is on top and drawn last).
     *
     * @param   pos The position between 0 (top object) and nbObject()-1 (bottom object).
     *
     * @return  A pointer to the current object drawn, nullptr if out of bound.
     **/
    internals_graphics::Plotter2DObj * get(int pos) const;


    /**
     * Removes an object from the plotter, does nothing if not inserted.
     *
     * @param [in,out]  obj the object to remove.
     **/
    void remove(internals_graphics::Plotter2DObj * obj);


    /**
     * Removes an object from the plotter, does nothing if not inserted.
     *
     * @param [in,out]  obj the object to remove.
     **/
    void remove(internals_graphics::Plotter2DObj & obj);


    /**
     * Return true is the plotter is using 4 channels images and false if we use only 3 channels.
     **/
    bool fourChannelImage() const;


    /**
     * Set the number of channels to use for the image.
     *
     * @param   use4    true to use 4 channels and false to use only 3 channels (default 3 channels).
     **/
    void fourChannelImage(bool use4);


    /**
     * Decide whether we use a solid backgournd. If not, a default gray checkerboard is drawn as the
     * background.
     *
     * @param   use true to use a solid background.
     **/
    void useSolidBackground(bool use);


    /**
     * Determines if we are using a solid background.
     *
     * @return  true if we use a solid backgond, false if we use the default checkerboard
     **/
    bool useSolidBackground() const;

    /**
     * The solid background color (if used).
     *
     * @param   color   The color.
     **/
    void solidBackGroundColor(mtools::RGBc color);


    /**
     * The current solid background color.
     *
     * @return  the color used for the background (if enabled).
     **/
    mtools::RGBc solidBackGroundColor() const;


    /**
     * Return the current redraw rate.
     *
     * @return  The number of redraw per minute as a number between 0 (no autoredraw) and 600 (max value).
     **/
    unsigned int autoredraw() const;


    /**
     * Set the auto-redraw rate
     *
     * @param   rate    The number of redraw per minute between 0 (no autoredraw) and 600 (max value).
     **/
    void autoredraw(unsigned int rate);


    /**
     * Request that the plotter start a redraw of the image.
     **/
    void redraw();


    /**
     * Return the Range Manager associated with the plotter. This object may in turn be used to set
     * the range of the drawing.
     *
     * @return  the range manager.
     **/
    internals_graphics::RangeManager & range();


    /**
     * Start the plotter and block until the window is closed.
     **/
    void plot();


    /**
     * Starts the plotter and return immediately.
     **/
    void startPlot();


    /**
     * Close the plotter window. Does nothing if it is already closed.
     **/
    void endPlot();


    /**
     * Check if the plotter is currently shown.
     *
     * @return  true if shown and flse if hidden.
     **/
    bool shown() const;


    /**
     * Sets the position of the plotter window. Does not change its size.
     *
     * @param   X   The X coordinate of the top left corner. Can also be POS_LEFT, POS_RIGHT, POS_CENTER
     * @param   Y   The Y coordinate of the top left corner. Can also be POS_TOP, POS_BOTTOM, POS_CENTER
     **/
    void setWindowPos(int X,int Y);


    /**
     * Sets the size of the plotter window. This is the size of the whole window and not just the
     * drawing aera of the image.
     *
     * @param   W   The width of the plotter window. Can be DEFAULT_W or MIN_W.
     * @param   H   The height of the plotter window. Can be DEFAULT_H or MIN_H.
     **/
    void setWindowSize(int W, int H);


    /**
     * Sets the size of the plotter window.
     *
     * @param   W   Yhe width of the view.
     * @param   H   The height of the view
     **/
    void setDrawingSize(int W, int H);


    /**
     * Try to find the best range horizontal range according to the current range and the prefered
     * horizontal range of the objects that are currently enabled. We keep (or not) the aspect ratio
     * depending on whether the flag is currently set in the range() object.
     **/
    void autorangeX();

    /**
     *  Same as autorangeX() but we first set/unset the "keep aspect ratio" flag.
     **/
    void autorangeX(bool keepAspectRatio);


    /**
     * Try to find the best vertical range according to the horizontal range and the prefered range
     * of the objects that are currently enabled. We keep (or not) the aspect ratio depending on
     * whether the flag is currently set in the range() object.
     **/
    void autorangeY();


    /**
     *  Same as autorangeY() but we first set/unset the "keep aspect ratio" flag.
     **/
    void autorangeY(bool keepAspectRatio);


    /**
     * Try to find the best range (horizontal and vertical) according to the current range and the
     * prefered range of the object that are currently enabled.We keep (or not) the aspect ratio
     * depending on whether the flag is currently set in the range() object.
     **/
    void autorangeXY();


    /**
     *  Same as autorangeXY() but we first set/unset the "keep aspect ratio" flag.
     **/
    void autorangeXY(bool keepAspectRatio);


    /**
     * Query the view zoom factor: this is the ratio of the view size w.r.t to the size of the image
     * really drawn (i.e. the size of the image that is saved in memory /on file). By default, the
     * ratio is 1 but can be set higher for purposes of creating high defniton image file on disk.
     *
     * @return  The view zoom factor between 1 and 20.
     **/
    int viewZoomFactor() const;


    /**
     * Set the view zoom factor: this is the ratio of the view size w.r.t to the size of the image
     * really drawn (i.e. the size of the image that is saved in memory /on file). By default, the
     * ratio is 1 but can be set higher for purposes of creating high defniton image file on disk.
     *
     * @param   zoomFactor  The desired zoom factor (between 1 and 20).
     *
     * @return  The new zoom factor (may differ from the resquested one if it would lead to an
     *          incorrect range).
     **/
    int viewZoomFactor(int zoomFactor);



    private:

    Plotter2D(const Plotter2D &) = delete;                  // no copy
    Plotter2D & operator=(const Plotter2D &) = delete;      //

    internals_graphics::Plotter2DWindow * _plotterWin;        // the plotter window.


};


}


/* end of file */
