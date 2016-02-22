/** @file plot2Daxes.hpp */
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
#include "../maths/box.hpp"
#include "rgbc.hpp"
#include "plotter2Dobj.hpp"
#include "drawable2Dobject.hpp"

#include <FL/Fl_Group.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>

namespace mtools
{


    /**
     * Object of this type can be used to add axes to a plot. NOrmally, it is not needed to create
     * an axe object since one is automatically inseter into the Plotter2D (see the
     * `Plotter2D::axeObject()` methods).
     * 
     * @code{.cpp}
     * int main()
     * { // stupid demo
     * auto Axes = makePlot2DAxes("axes2");       // create an axes object
     * Axes.color(mtools::RGBc::c_Blue);          // blue color
     * mtools::Plotter2D P;                       // the plotter
     * P[Axes];                                   // insert inside the plotter
     * P.axesObject(false);                       // remove the default axes object
     * P.plot();
     * }
     * @endcode.
     **/
    class Plot2DAxes : public internals_graphics::Plotter2DObj, protected internals_graphics::Drawable2DObject
    {

    public:

        /**
         * Constructor.
         **/
        Plot2DAxes(std::string name = "Axes");


        /**
         * Move Constructor.
         **/
        Plot2DAxes(Plot2DAxes && o);


        /**
         * Destructor. Remove the object if it is still inserted.
         **/
        virtual ~Plot2DAxes();


        /**
         * Set the scaling for the axes.
         *
         * @param   scaling The scaling between 0.1 and 5.0 (default 1.0) 
         **/
        void scaling(float scaling);


        /**
         * Define how the graduation are drawn.
         *
         * @param   show    true to show, false to hide.
         * @param   color   The color to use
         **/
        void graduations(bool show, RGBc color);


        /**
         * Set whether the graduation are drawn.
         *
         * @param   show    true to draw the graduations.
         **/
        void graduations(bool show);


        /**
         * Set the graduations color
         *
         * @param   color   The color to use to draw the axes/graduations.
         **/
        void graduations(RGBc color);


        /**
         * Define how the numbers are drawn.
         *
         * @param   show    true to show, false to hide.
         * @param   color   The color to use.
         **/
        void numbers(bool show, RGBc color);


        /**
         * Set whether the numbers are drawn
         *
         * @param   show    true to draw the numbers.
         **/
        void numbers(bool show);


        /**
         * Set the color use to draw the numbers
         *
         * @param   color   The color to use.
         **/
        void numbers(RGBc color);


        /**
         * Return the axe color (for the axe/graduations).
         *
         * @return  The color of the axes (the color of the numbers may differ and can be set via
         *          numbers()).
         **/
        RGBc color() const;


        /**
         * Set the color for both the axes/graduations and the numbers to the same color. Same as
         * calling numbers(coul) and graduations(coul).
         *
         * @param   coul    The color to use when drawing.
         **/
        void color(RGBc coul);




        /* default parameters for drawing the axes */
        static const bool  DEFAULT_GRAD_SHOW;
        static const bool  DEFAULT_NUM_SHOW;
        static const RGBc  DEFAULT_GRAD_COLOR;
        static const RGBc  DEFAULT_NUM_COLOR;
        static const float DEFAULT_SCALING;

    protected:


        /**
         * Override of the setParam method from the Drawable2DObject interface
         **/
        virtual void setParam(mtools::fBox2 range, mtools::iVec2 imageSize) override;


        /**
         * Override of the drawOnto() method from the Drawable2DObject interface
         **/
        virtual int drawOnto(Img<unsigned char> & im, float opacity = 1.0) override;


        /**
         * Override of the removed method from the Plotter2DObj base class
         **/
        virtual void removed(Fl_Group * optionWin) override;


        /**
         * Override of the inserted method from the Plotter2DObj base class
         **/
        virtual internals_graphics::Drawable2DObject * inserted(Fl_Group * & optionWin, int reqWidth) override;


        private:

            /* callbacks for the fltk widgets */
            static void _gradButtonCB_static(Fl_Widget * W, void * data);
            void _gradButtonCB(Fl_Widget * W);

            static void _numButtonCB_static(Fl_Widget * W, void * data);
            void _numButtonCB(Fl_Widget * W);

            static void _gradColorButtonCB_static(Fl_Widget * W, void * data);
            void _gradColorButtonCB(Fl_Widget * W);

            static void _numColorButtonCB_static(Fl_Widget * W, void * data);
            void _numColorButtonCB(Fl_Widget * W);
            
            static void _scaleSliderCB_static(Fl_Widget * W, void * data);
            void _scaleSliderCB(Fl_Widget * W);

            Plot2DAxes(const Plot2DAxes &) = delete;                    // no copy
            Plot2DAxes & operator=(const Plot2DAxes &) = delete;        //

            Fl_Check_Button *   _gradButton;
            Fl_Check_Button *   _numButton;
            Fl_Button *         _gradColorButton;
            Fl_Button *         _numColorButton;
            Fl_Value_Slider *   _scaleSlider;

            bool _gradStatus;       // do we draw the graduations
            bool _numStatus;        // do we draw the numbers
            RGBc _gradColor;        // graduations/axe color
            RGBc _numColor;         // numbers color
            float _scaling;         // scaling

            fBox2 _range;           // the range we should use to draw the axes
            iVec2 _imageSize;       // the requested size of the image. 
        };



    /**
     * Factory function for creating axes plot objects
     *
     * @param   name    The name of the axes plot.
     *
     * @return  The Plot2DAxes object.
     **/
    inline Plot2DAxes makePlot2DAxes(std::string name = "Axes")
        {
        return Plot2DAxes(name);
        }


}


/* end of file */


