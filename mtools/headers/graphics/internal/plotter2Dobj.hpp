/** @file plotter2Dobj.hpp */
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

#include "../../maths/vec.hpp"
#include "../../maths/box.hpp"
#include "../customcimg.hpp"
#include "drawable2DInterface.hpp"
#include "rangemanager.hpp"
#include "../../misc/internal/forward_fltk.hpp"

#include <atomic>

namespace mtools
{


    namespace internals_graphics
    {


        /**
         * Base class describing an object that can be inserted into a Plotter2D. Subclass it to create
         * object that can be inserted into a Plotter2D.
         *
         * The derived class must implement
         * - the virtual `inserted()` method.
         * - a virtual destructor which calls `detach()`.
         *
         * It can also optionally implement
         * - the virtual `removed()` method.
         * - the virtual `favouriteRangeX()` `favouriteRangeY()` `hasFavouriteRangeX()` and
         * `hasFavouriteRangeY()` methods.
         *
         * @sa  class Plotter2DObjWithColor
         **/
        class Plotter2DObj
        {

        public:

            /**
             * Constructor. The object is enabled by default. However, it is guaranteed that it will not be
             * accessed until it is inserted hence it is possible to disable via enable(false) before this
             * happens to make sure the underlying drawable object is never used unless we specifically
             * allow it.
             *
             * @param   name    The name of the plot.
             **/
            Plotter2DObj(const std::string & name);


            /**
             * Move Constructor. Detaches the object if it is inserted. Should not be used unless for the
             * factory functions.
             *
             * @param [in,out]  obj The name of the plot.
             **/
            Plotter2DObj(Plotter2DObj && obj);


            /**
             * Virtual destructor. The derived class MUST override the destructor and call `detach()` to
             * make sure that proper destruction occurs when the object is destroyed while still inserted.
             *
             * ! MUST BE OVERRIDEN IN THE DERIVED CLASS !
             **/
            virtual ~Plotter2DObj();


            /**
             * The number associated with this plot. Incremented by one with each new object.
             *
             * @return  the plot number (0 = first plot).
             **/
            int ID() const;


            /**
             * Return the name of the object.
             *
             * @return  A std::string.
             **/
            std::string name() const;


            /**
             * Change the name of the object
             *
             * @param   name    The new name.
             **/
            void name(const std::string & name);


            /**
             * The opacity used for drawing the object.
             *
             * @return  the opacity used between 0.0 (transparent) and 1.0 (opaque).
             **/
            float opacity() const;


            /**
             * Set the opacity used for drawing.
             *
             * @param   op  the opacity used between 0.0 (transparent) and 1.0 (opaque).
             **/
            void opacity(float op);


            /**
             * Check if the object is enabled (an object is enabled by default at constuction but is not
             * accessed anyway until it is inserted).
             *
             * - The worker thread associated to this object (if needed) is on only if the object is
             * inserted AND enabled.
             * - GUARANTEE : A disabled OR not inserted object never call any method from the
             * Drawable2DObject interface (except maybe the needwork() const method)
             *
             * @return  true if enabled and false if disabled.
             **/
            bool enable() const;


            /**
             * Enable/Disable the object. When enabling an object, this also send a refresh signal to the
             * owner (if any) to redraw. This method also enable/disable the option menu associated with the
             * object.
             * 
             * Calling this method override any suspended flag previously set with suspend().              *.
             *
             * @param   status  true to enable and false to disable the object.
             *
             * @sa  suspend
            **/
            void enable(bool status);


            /**
             * Suspend/Resume the object. This method is independent of enable(). After suspend(true)
             * returns, is guaranteed that the underlying drawable object will not be accessed in any way
             * (in particular, quality() and drawOnto() simply return 0). This permits to safely modify an
             * underlying object which cannot be concurrently accessed. Then, call suspend(false) when
             * modification are finished.
             *
             * @param   status  true to suspend the object drawing and false to resume it.
            **/
            void suspend(bool status);


            /**
             * Query whether the object is currently suspended.
             *
             * @return  true if suspended and false otherwise. 
            **/
            bool suspend() const;
            

            /**
             * Move the object up in the object list (the topmost object is drawn last). Does nothing if not
             * inserted.
             **/
            void moveUp();


            /**
             * Move the object down in the object list (the topmost object is drawn last). Does nothing if
             * not inserted.
             **/
            void moveDown();


            /**
             * Move the object at the top of the list : the object is drawn last. Does nothing if not
             * inserted.
             **/
            void moveTop();


            /**
             * Move the object at the bottom of the list : the object is drawn first. Does nothing if not
             * inserted.
             **/
            void moveBottom();


            /**
             * Set the plotter's horizontal range to the object's favourite horizontal range. Does nothing
             * if the object does not have a preferred horizontal range or if the object is not inserted or
             * if the object is not enabled.
             **/
            void autorangeX();


            /**
             * Set the plotter's vertical range to the object's favourite vertical range. Does nothing if
             * the object does not have a preferred horizontal range or if the object is not inserted or if
             * the object is not enabled.
             **/
            void autorangeY();


            /**
             * Set the plotter's X/Y range to the object's favourite X/Y range. Does nothing if the object
             * does not have a preferred horizontal range or if the object is not inserted or if the object
             * is not enabled.
             **/
            void autorangeXY();


            /**
             * Return the current favourite horizontal range for this object (according to a current range).
             * Only the horizontal component of the returned rectangle is considered. It may be horizontally
             * empty if there is no prefered range.
             *
             * CAN BE OVERLOADED. By default, the method return a completelely empty rectangle hence there
             * is no favourite range.
             *
             * When overloading this function, it is possible to use range() to query the current range or
             * window size but NO NON-CONST METHOD of the range manager should be called !
             *
             * @param   R   The current/tentative range that should be considered.
             *
             * @return  the horizontal part of the rectange must contain the prefered horizontal range. This
             *          vertical part is irrelevant.
             **/
            virtual fBox2 favouriteRangeX(fBox2 R);


            /**
             * Return the current favourite vertical range for this object (according to a current range).
             * Only the vertical component of the returned rectangle is considered. It may be vertically
             * empty if there is no prefered range.
             *
             * CAN BE OVERLOADED. By default, the method return a completelely empty rectangle hence there
             * is no favourite range.
             *
             * When overloading this function, it is possible to use range() to query the current range or
             * window size but NO NON-CONST METHOD of the range manager should be called !
             *
             * @param   R   The current/tentative range that should be considered.
             *
             * @return  the vertical part of the rectange must contain the prefered vertical range. The
             *          horizontal part is irrelevant.
             **/
            virtual fBox2 favouriteRangeY(fBox2 R);


            /**
             * Check if the object has a favourite horizontal range. Default implementation return false.
             * Overload this function to return true if the object has one.
             **/
            virtual bool hasFavouriteRangeX();


            /**
             * Check if the object has a favourite vertical range. Default implementation return false.
             * Overload this function to return true if the object has one.
             **/
            virtual bool hasFavouriteRangeY();


            /**
             * Request the plotter to refresh the drawing. Does nothing if not inserted. This method ask the
             * plotter to redraw the screen but it does not reset the drawing. To redraw this object from
             * scratch, use the resetDrawing() method.
             **/
            void refresh();


            /**
             * Return the quality of the current drawing. Return 0 if the drawing is not currently inserted
             * and return 100 if it is not enabled.
             *
             * @return  the quality of the current drawing or 0 if not inserted or 100 if inserted but not
             *          enabled.
             **/
            int quality() const;


            /**
             * Determines if the object uses a worker thread. Return false if the object is not inserted.
             *
             * @return  true if the object is inserted and uses a worker thread. false otherwise.
             **/
            bool needWork() const;


            /**
             * Force a  redraw of this object and then send a refresh signal to the owner if inserted. Does
             * nothing if the object is not inserted or is not enabled.
             *
             * @param   refresh True to refresh the screen (default).
             **/
            void resetDrawing(bool refresh = true);


            /**
             * Query if the object is currently inserted.
             *
             * @return  true if inserted, false if it is not.
             **/
            bool isInserted() const;


            protected:

            /**
             * Makes the drawing onto im and return the quality of the drawing. This method calls the
             * drawOnto() method of the underlying drawable2DObject with the correct opacity and checking
             * first if the drawing should be drawn. This does nothing and return 0 if the object is not
             * inserted. It does nothing and return 100 if the object is not inserted.
             *
             * @param [in,out]  im  The image to draw onto.
             *
             * @return  the quality of the drawing between 0 (nothing drawn) and 100 (perfect drawing).
             *          Return 100 if we did not draw anything because the object is disabled.
             **/
            int drawOnto(Img<unsigned char> & im);


            /**
             * Sets the parameters of the drawing. Does nothing if not inserted. This method is a forward of
             * the setParam() method of the underlying DRawable2DObject.
             *
             * @param   range       The range to draw.
             * @param   imageSize   The size of the image to draw onto.
             **/
            void setParam(mtools::fBox2 range, mtools::iVec2 imageSize);


            /**
             * This method is called when the object is inserted into a Plotter. After this call and until
             * `removed()` is called, the Drawable object is managed by the owner.
             *
             * This method is always called from the FLTK thread and should perform the following operations:
             *
             * - Create the option window if there is one.
             * - If not yet created, create (and then return) the Drawable2DObject.
             * - Optional : set a favourite range for the object by calling setFavouriteRange().
             *
             * ! MUST BE OVERRIDEN IN DERIVED CLASS !
             *
             * This function must not change the range() nore call any callback to the plotter !
             *
             * @param [in,out]  optionWin   [in,out] Put here a pointer to the option window or nullptr if
             *                              there are none. The widget should be created via new so that
             *                              delete_widget() can be used to delete it during the removed()
             *                              method.
             * @param   reqWidth            The required width that hte created option window should have.
             *
             * @return  a pointer to the Drawable2DObject that is to be drawn.
             **/
            virtual Drawable2DInterface * inserted(Fl_Group * & optionWin, int reqWidth);


            /**
             * This method is called when the owner of the widget removes it. This is the place where the
             * option window should be deleted. The default implementation of this method simply calls
             * "delete_widget(optionWin)" or does nothing if no option window was created and should suffice
             * when using only simple fltk widgets (i.e. no timer or other thing that must be removed before
             * the object goes out of scope).
             * This method is called by the owner from the fltk thread.
             *
             * CAN BE OVERRIDEN IN DERIVED CLASS.
             *
             * @param [in,out]  optionWin   A pointer to the same option window that was given when inserted
             *                              was called.
             **/
            virtual void removed(Fl_Group * optionWin);


            /**
             * Force the object to detach from its current owner. This method does nothing if the object is
             * not currently inserted. If is it, it request and wait for the owner confirmation (hence for
             * the call to the `removed()` virtual method from fltk) before returning. The worker thread is
             * also stopped if it was active.
             *
             * This method is normally called form within the fltk thread but can be called from any thread.
             *
             * In order to prevent segmentation faults when object are not destroyed before their owner, the
             * virtual detructor of this object should always begin by calling `detach()` before cleaning up
             * resources to make sure that that Drawable object is not accessed anymore and that the option
             * window is properly destroyed.
             **/
            void detach();


            /**
             * Call this function to request that the main drawing window take back the focus. Does nothing
             * if not inserted.
             **/
            void yieldFocus();


            /**
             * Return the range manager associated to this object or nullptr if the object is not inserted. Can be ued to query the range but cannot be use to m
             *
             * @return  A pointer to the range manager of the owner or nullptr if there is no owner.
             **/
            const RangeManager * range() const;


            /**
             * Method called when the user pressed on the name button widget.
             *
             * Overloaded in Plotter2DObjWithColor.
             *
             * @param [in,out]  W   The button widget.
             **/
            virtual void colorCB(Fl_Widget * W);


            /**
             * Overloaded in Plotter2DObjWithColor to return the color to use for the name button widget.
             *
             * @return  The name widget color.
             **/
            virtual RGBc nameWidgetColor() const;


            /**
             * Set the color of the name button widget (query the color via the nameWidgetColor() method).
             **/
            void setNameWidgetColor();


        private:


            friend class Plotter2DWindow;
            friend class Plot2DComposer;

            static const int _REQUEST_DETACH = 0;       ///< code when requesting to be detached
            static const int _REQUEST_REFRESH = 1;      ///< code when requesting a refresh of the picture
            static const int _REQUEST_YIELDFOCUS = 2;   ///< request that the focus be given back to the picture view
            static const int _REQUEST_UP = 3;           ///< request to move up
            static const int _REQUEST_DOWN = 4;         ///< request to move down
            static const int _REQUEST_TOP = 5;          ///< request to move top
            static const int _REQUEST_BOTTOM = 6;       ///< request to move bottom
            static const int _REQUEST_FIXOBJECTWIN = 7; ///< request to redraw the object window.
            static const int _REQUEST_USERANGEX = 8;    ///< request to use the object favourite X range
            static const int _REQUEST_USERANGEY = 9;    ///< request to use the object favourite Y range
            static const int _REQUEST_USERANGEXY = 10;  ///< request to use the object favourite X/Y range

            /**
             * Typedef for the callback function.
             **/
            typedef void (*pnot)(void * data, void * data2, void * obj, int code);


            /**
             * The owner must call this method to take ownership of the object.
             *
             * The call must be made from the fltk thread. In return, any call to its callback function will
             * come from the fltk thread.
             *
             * @param   cb                  the callback method to  talk back to the plotter.
             * @param [in,out]  rm          the range manager associated with this object.
             * @param [in,out]  data        The data that should be passed along when using the callback.
             * @param [in,out]  data2       Additonnal data to be passed along when using the callback.
             * @param   hintWidth           Width of the hint.
             *
             * @return  the Drawable2DObject to be drawn.
             **/
            void _inserted(pnot cb, RangeManager * rm, void * data, void * data2, int hintWidth);


            /**
             * The owner of the widget MUST call this method when it releases its ownership ! In particular,
             * the owner is required to call this method when it receive the 'detach' request via its
             * callback method with parameter 0.
             *
             * The call must be made from the fltk thread.
             **/
            void _removed();


            /**
             * The option window of the object. Return nullptr if not inserted.
             *
             * @return  a pointer to the option window widget r nullptr.
             **/
            Fl_Group * _optionWindow() const;


            /**
             * Call the callback function of the owner with a given code.
             *
             * @param   code    The code to send.
             **/
            void _makecallback(int code);


            /* FLTK widget callbacks */
            static void _upButtonCB_static(Fl_Widget * W, void * data);
            void _upButtonCB(Fl_Widget * W);

            static void _downButtonCB_static(Fl_Widget * W, void * data);
            void _downButtonCB(Fl_Widget * W);

            static void _onOffButtonCB_static(Fl_Widget * W, void * data);
            void _onOffButtonCB(Fl_Widget * W);

            static void _useRangeXCB_static(Fl_Widget * W, void * data);
            void _useRangeXCB(Fl_Widget * W);

            static void _useRangeYCB_static(Fl_Widget * W, void * data);
            void _useRangeYCB(Fl_Widget * W);

            static void _useRangeXYCB_static(Fl_Widget * W, void * data);
            void _useRangeXYCB(Fl_Widget * W);

            static void _opacitySliderCB_static(Fl_Widget * W, void * data);
            void _opacitySliderCB(Fl_Widget * W);

            static void _nameColorCB_static(Fl_Widget * W, void * data);
            void _nameColorCB(Fl_Widget * W);

            static void _unrollButtonCB_static(Fl_Widget * W, void * data);
            void _unrollButtonCB(Fl_Widget * W);


            /* timer callback */
            static void _timerCB_static(void * data);
            void _timerCB();

            void _insertOptionWin(bool status);

            bool _unrolled() const;


            Plotter2DObj(const Plotter2DObj &) = delete;                // no copy
            Plotter2DObj & operator=(const Plotter2DObj &) = delete;    //


            std::atomic<fBox2> _crange;                     // the last range given via setParam
            std::atomic<iVec2> _cwinSize;                   // the last window size given by setParam
            std::atomic<bool>  _missedSetParam;             // true if we did not inform the drawableObject the last time around.

            std::atomic<pnot>  _ownercb;                    // callback to the owner function. The object is inserted if and only if the callback is not nullptr
            std::atomic<void*> _data;                       // data supplied by the owner to pass along when using the callback.
            std::atomic<void*> _data2;                      // additional data.
            std::atomic<RangeManager *> _rm;                // the range manager of the owner or nullptr if not yet inserted.
            std::atomic<Drawable2DInterface*> _di;          // the auto drawable 2D object
            std::atomic<float> _opacity;                    // the opacity for drawing
            std::atomic<bool>  _drawOn;                     // is the object enabled.
            std::atomic<bool>  _suspended;                  // is the object suspended
            std::string _name;                              // the object name
            int _progVal;                                   // the last value of the progress bar, -1 if thread stopped
            int _nbth;                                      // last number of thread queried.
            Fl_Button *             _nameBox;               // the box with the name of the drawing
            Fl_Group *              _optionWin;             // the option window
            Fl_Group *              _extOptionWin;          // the additionnal option window
            Fl_Progress *           _progBar;               // the working thread progress bar
            Fl_Button  *            _upButton;              // the up button
            Fl_Button  *            _downButton;            // the down button
            Fl_Light_Button *       _onOffButton;           // the on / off button
            Fl_Button *             _useRangeX;             // the use range button
            Fl_Button *             _useRangeY;             // the use range button
            Fl_Button *             _useRangeXY;            // the use range button
            Fl_Value_Slider *       _opacitySlider;         // the opacity slider
            Fl_Box *                _titleBox;              // the title box
            Fl_Button *             _unrollButton;          // the unroll button
            Fl_Box *                _nbthl;                 // number of thread
            const std::atomic<int>  _plotNB;                // the plot Number
            static std::atomic<int> _totPlotNB;             // the total number of plot object created.
        };





        /**
         * Base class describing an object that can be inserted into a Plotter2D. This class also
         * enables to select a main color by clicking on the name of the plot (or using the public
         * method `color()`).
         *
         * The derived class must implement
         * - the virtual `inserted()` method.
         * - a virtual destructor which calls `detach()`.
         *
         * It can also optionally implement
         * - the virtual `removed()` method.
         * - the virtual `favouriteRangeX()` `favouriteRangeY()` `hasFavouriteRangeX()` and
         * `hasFavouriteRangeY()` methods.
         *
         * @sa  class Plotter2DObjWithColor
         **/
        class Plotter2DObjWithColor : public Plotter2DObj
        {

            public:

            /**
             * Constructor. The plot is automaticcally given a default color which depend on the number of
             * Plotter2DObjWithColor object constructed.
             *
             * @param   name    The name of the plot.
             **/
            Plotter2DObjWithColor(const std::string & name);


            /**
             * Dtor, detach the object if inserted.
             **/
            ~Plotter2DObjWithColor();


            /**
             * Move Constructor. Detaches the object if it is inserted.
             *
             * @param [in,out]  obj the object to move.
             **/
            Plotter2DObjWithColor(Plotter2DObjWithColor && obj);


            /**
             * Return the main color associated with this drawing.
             *
             * @return  The main color associated with the drawing.
             **/
            RGBc color() const;


            /**
             * Set the main color associated with this drawing.
             *
             * @param   coul    The color.
             **/
            void color(RGBc coul);


            protected:


            /* overload : callback called when the user presse the name button of the plot */
            virtual void colorCB(Fl_Widget * W) override;

            /* overload : return the current color of the plot */
            virtual RGBc nameWidgetColor() const override;

            private:

            std::atomic<RGBc> _color;               // the main color associated with the drawing
            std::atomic<int> _no;                   // number of the plot (use to decide the initial color).
            static std::atomic<int> _noColorPlot;   // number of color plot created.

        };


    }



}


/* end of file */
