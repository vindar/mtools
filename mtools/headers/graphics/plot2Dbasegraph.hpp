/** @file plot2Dbasegraph.hpp */
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



#include "plotter2Dobj.hpp"
#include "drawable2Dobject.hpp"

#include <atomic>
#include <limits>
#include <cmath>

#include <FL/Fl_Group.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Color_Chooser.H> 



namespace mtools
{

    namespace internals_graphics
    {


    /**
     * Base class for function-like plot object. The ctor/dtor and the _function() method must be
     * overriden.
     * @sa Plot2DFun
     **/
    class  Plot2DBaseGraph : public internals_graphics::Plotter2DObjWithColor, protected internals_graphics::Drawable2DObject
        {

        public:

        /**
         * Constructor.
         *
         * @param   minDomain   The minimum of the definition domain.
         * @param   maxDomain   The maximum of the definition domain.
         * @param   name        The name of the plot.
         **/
        Plot2DBaseGraph(double minDomain, double maxDomain, std::string name);


        /**
         * Move Constructor
         **/
        Plot2DBaseGraph(Plot2DBaseGraph &&);


        /**
         * Constructor. The definition domain is the whole line.
         *
         * @param   name    The name of the plot .
         **/
        Plot2DBaseGraph(std::string name);


        /**
         * Destructor. Remove the object if it is still inserted. 
         **/
        virtual ~Plot2DBaseGraph();


        /**
         * Use line to connect the dots of the drawing.
         **/
        void drawLines();


        /**
         * Make the drawing drawing dots with an adaptative precision.
         *
         * @param   quality The quality (i.e. depht of recursion) to use between 0 (worst) and 30 (best).
         **/
        void drawDots(int quality = DEFAULT_PLOT_QUALITY);


        /**
         * Set the tickness of the drawing (between 1 and 20). 
         *
         * @param   tick    The tickness to use.
         **/
        void tickness(int tick);


        /**
         * Set whether we fill the region over the graph with a given color. 
         *
         * @param   status  true to draw over the graph.
         **/
        void epigraph(bool status);


        /**
         * The color used when filling the region over the graph.
         *
         * @param   color   The color to use.
         **/
        void epigraphColor(RGBc color);


        /**
         * The opacity used when filling the region over the graph
         *
         * @param   opacity The opacity (between 0.0 and 1.0).
         **/
        void epigraphOpacity(float opacity);


        /**
         * Set whether we fill the region under the graph with a given color. 
         *
         * @param   status  true to fill under the graph.
         **/
        void hypograph(bool status);


        /**
         * The color used when filling the region under the graph.
         *
         * @param   color   The color to use.
         **/
        void hypographColor(RGBc color);


        /**
         * The opacity used when filling the region under the graph
         *
         * @param   opacity The opacity (between 0.0 and 1.0).
         **/
        void hypographOpacity(float opacity);


        /**
         * Favourite horizontal range. Return the definition domain if it is bounded or an empty range
         * otherwise.
         **/
        virtual fBox2 favouriteRangeX(fBox2 R) override;


        /**
         * Return an estimation of the vertical range of the function over the horizontal range defined
         * by R. If the horizontal range of R is empty and the function has a bounded definition domain,
         * return an estimation of the vertical range of the function over the definition domain. If R
         * is horizontally empty and the definition domain of the function is unbounded, return an empty
         * vertical range.
         **/
        virtual fBox2 favouriteRangeY(fBox2 R) override;


        /**
         * Return true if the function has a bounded definiton domain and false otherwise.
         **/
        virtual bool hasFavouriteRangeX() override;


        /**
         * Return true.
         **/
        virtual bool hasFavouriteRangeY() override;



        static const int DEFAULT_PLOT_QUALITY; ///< default recursion depth when plotting with dots. 
        static const int RANGE_SAMPLE_SIZE;    ///< number of points used when estimating the range




        protected:

           

        /**
         * Override of the setParam method
         **/
        virtual void setParam(mtools::fBox2 range, mtools::iVec2 imageSize) override;
             

        /**
         * Override of the drawOnto method
         **/
        virtual int drawOnto(Img<unsigned char> & im, float opacity = 1.0) override;


            /***** Override of methods from Plotter2DObject ******/


        /**
         * Override of the inserted method
         **/
        virtual internals_graphics::Drawable2DObject * inserted(Fl_Group * & optionWin, int reqWidth) override;


        /**
         * Override of the removed method
         **/
        virtual void removed(Fl_Group * optionWin) override;


        /**
         * The pure virtual function to override in derived classes.
         **/
        virtual double _function(double x) const = 0;


        private:


            /* Estimate the range of the function by sampling over 10000 points and return the min/max value found.
            modify R.min[1] and R.max[1]. The range returned is vertically empty if no point could be sampled. */
            void estimateYRange(fBox2 & R) const;
           


            /* callbacks */

            static void _roundButtonCB_static(Fl_Widget * W, void * data);
            void _roundButtonCB(Fl_Widget * W);

            static void _dichoQualitySliderCB_static(Fl_Widget * W, void * data);
            void _dichoQualitySliderCB(Fl_Widget * W);

            static void _overButtonCB_static(Fl_Widget * W, void * data);
            void _overButtonCB(Fl_Widget * W);

            static void _underButtonCB_static(Fl_Widget * W, void * data);
            void _underButtonCB(Fl_Widget * W);

            static void _overColorButtonCB_static(Fl_Widget * W, void * data);
            void _overColorButtonCB(Fl_Widget * W);

            static void _underColorButtonCB_static(Fl_Widget * W, void * data);
            void _underColorButtonCB(Fl_Widget * W);

            static void _underSliderCB_static(Fl_Widget * W, void * data);
            void _underSliderCB(Fl_Widget * W);

            static void _overSliderCB_static(Fl_Widget * W, void * data);
            void _overSliderCB(Fl_Widget * W);

            static void _ticknessSliderCB_static(Fl_Widget * W, void * data);
            void _ticknessSliderCB(Fl_Widget * W);


            /* utility methods used for the drawing*/
            void _drawPoint(int i, int j, Img<unsigned char> & im, const RGBc & coul, const float opacity, const int tickness);
            void _drawLine(int i, int j1, int j2, Img<unsigned char> & im, const RGBc & coul, const float opacity, const int tickness);
            void _dicho(int j0, int i1, int i2, int j3, double x0, double x3, int depth, Img<unsigned char> & im, const fBox2 & R, const RGBc & coul, const float & opacity, const int & tickness);

            /* make the drawing with dichotomy */
            void _drawWithDicho(int depth, Img<unsigned char> & im, const fBox2 & R, const RGBc coul, const float opacity, const int tickness);

            /* make the drawing with linear interpolation */
            void _drawWithInterpolation(int depth, Img<unsigned char> & im, const fBox2 & R, const RGBc coul, const float opacity, const int tickness);

            /* fill below or over */
             void _drawOverOrBelow(bool over, Img<unsigned char> & im, const fBox2 & R, RGBc coul, const float opacity);
            
             /* update all widget */
             void _updateWidgets();


        protected : 


            /**
             * CAN BE OVERRIDEN : Optional pannel to add on top (use to add the interpolation option in
             * Plot2DBaseGraphWithInterpolation.
             **/
            virtual Fl_Group * optionalPanel(int reqWidth);


            /**
             * CAN BE OVERRIDEN : Called when we remove the object (this is thep lace to destroy the opt
             * group)
             **/
            virtual void optionalPanelRemoved(Fl_Group * opt);


            fBox2 _range;               
            iVec2 _imageSize;           

            mutable double _minDomain;          // the definition domain
            mutable  double _maxDomain;          //

        private:

            bool _drawMethod;               // the drawing method, tur for linear interpolation
            int  _dichoQuality;             // the dichotomy quality i.e. max number of recursion
            int  _tickness;                 // the tickness of the drawing
            bool _drawOver;                 // true to draw over the plot
            bool _drawUnder;                // true to draw under the plot
            RGBc _drawOverColor;            // color of the drawing over the plot
            RGBc _drawUnderColor;           // color of the drawing under the plot
            float _drawOverOpacity;         // opacity of the drawing over the plot
            float _drawUnderOpacity;        // opacity of the drawing under the plot

            Fl_Round_Button * _interpolateCheck;    // widgets
            Fl_Round_Button * _dichoCheck;          //
            Fl_Value_Slider * _dichoQualitySlider;  //
            Fl_Check_Button * _overButton;          //
            Fl_Check_Button * _underButton;         //
            Fl_Button *       _overColorButton;     //
            Fl_Button *       _underColorButton;    //
            Fl_Value_Slider * _overSlider;          //
            Fl_Value_Slider * _underSlider;         //
            Fl_Value_Slider * _ticknessSlider;      //
            Fl_Group *        _optGroup;            //

        };


    /**
     * Base class for array-like Plot object (same as function-like but with and additonnal
     * interpolation mehod menu). The ctor/dtor `_function()` metohd must be overriden in the derive
     * class.
     *
     * @sa  Plot2DArray, Plot2DVector
     **/
    class  Plot2DBaseGraphWithInterpolation : public internals_graphics::Plot2DBaseGraph
    {

        public:

        /**
         * Constructor.
         *
         * @param   minDomain   The minimum of the definition domain.
         * @param   maxDomain   The maximum of the definition domain.
         * @param   name        The name of the plot.
         **/
        Plot2DBaseGraphWithInterpolation(double minDomain, double maxDomain, std::string name);


        /**
         * Move Constructor.
         **/
        Plot2DBaseGraphWithInterpolation(Plot2DBaseGraphWithInterpolation &&);

        Plot2DBaseGraphWithInterpolation(const Plot2DBaseGraphWithInterpolation &) = delete;


        /**
         * Destructor. Remove the object if it is still inserted. 
         **/
        virtual ~Plot2DBaseGraphWithInterpolation();


        /**
         * Set the interpolation method to use.
         *
         * @param   type    The method to use Either INTERPOLATION_NONE, INTERPOLATION_LINEAR,
         *                  INTERPOLATION_CUBIC, INTERPOLATION_MONOTONE_CUBIC.
         **/
        void interpolationMethod(int type);


        /**
         * Return the interpolation method used for drawing. Either INTERPOLATION_NONE,
         * INTERPOLATION_LINEAR, INTERPOLATION_CUBIC, INTERPOLATION_MONOTONE_CUBIC.
         *
         * @return  The interpolation method in use.
         **/
        int interpolationMethod() const;


        /** Do not use interpolation : draw flat intervals. */
        void interpolationNone();


        /** Use linear interpolation  */
        void interpolationLinear();


        /** Use cubic interpolation  */
        void interpolationCubic();


        /** Use monotone cubic interpolation  */
        void interpolationMonotoneCubic();


        static const int INTERPOLATION_NONE;                ///< no interpolation
        static const int INTERPOLATION_LINEAR;              ///< linear interpolation
        static const int INTERPOLATION_CUBIC;               ///< cubic interpolation
        static const int INTERPOLATION_MONOTONE_CUBIC;      ///< monotone cubic interpolation


        protected:

        virtual Fl_Group * optionalPanel(int reqWidth) override;

        virtual void optionalPanelRemoved(Fl_Group * opt) override;

        private:

        /* the buttons cb */
        static void _interButtonCB_static(Fl_Widget * W, void * data);
        void _interButtonCB(Fl_Widget * W);

        /* set the button */
        void _setInterpolationButtons();


        std::atomic<int> _interpolationType; // the type of interpolation


        Fl_Group * _optGroup;               // the interpolation pannel
        Fl_Round_Button * _interNone;       // widgets for the interpolation pannel
        Fl_Round_Button * _interLinear;     //
        Fl_Round_Button * _interCubic;      //
        Fl_Round_Button * _interCubicMono;  //

    };


    }

}


/* end of file */


