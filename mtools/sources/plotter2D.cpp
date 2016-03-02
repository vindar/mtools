/** @file plotter2D.cpp */
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


#include "stdafx_mtools.h"


#include "graphics/customcimg.hpp"
#include "graphics/plotter2D.hpp"
#include "graphics/plotter2Dobj.hpp"
#include "graphics/view2Dwidget.hpp"
#include "io/fltkSupervisor.hpp"
#include "misc/error.hpp"

#include "io/fileio.hpp"

namespace mtools
{



    namespace internals_graphics
    {
        class Plotter2DObj;     // basic class of a plottable object


        /** the plooter window object, everything here happen inside the fltk thread */
        class Plotter2DWindow
        {
            friend class Plotter2D;

            public:

            Plotter2DWindow(bool addAxes, bool addGrid, int X, int Y, int W, int H);

            ~Plotter2DWindow();

            /* start the plot by showing the window */
            void startPlot();

            /* end the plot by hidding the window */
            void endPlot();

            /* update the view */
            void updateView(bool withreset = true);

            /* set the number of channels on the image */
            void fourChannelImage(bool use4);

            static const int OPTION_WIDTH = 290;

            /* set the main CImg image size */
            void setImageSize(int lx, int ly, int channels);

            /* update  View timer callback */
            static void static_updateViewTimer(void* p);
            void updateViewTimer();


            /* axe and grid object insertion */
            void _insertAxesObject(bool status);
            void _insertGridObject(bool status);


            /* callbacks from the View2DWidget */
            static void view2DnotCB_static(void * data, int key);
            void view2DnotCB(int key);
            static bool view2DcrossCB_static(void * data, bool newstatus);
            bool view2DcrossCB(bool newstatus);

            /* callback for the range manager */
            static bool rangeManagerCB_static(void * data, void * data2, bool changedRange, bool changedWinSize, bool changedFixAspectRatio);
            bool rangeManagerCB(bool changedRange, bool changedWinSize, bool changedFixAspectRatio);
            void rangeManagerCB2(fBox2 R, iVec2 winSize, bool fixedAR, bool changedRange, bool changedWinSize, bool changedFixAspectRatio);

            /* callback for the Plotter2Dobj object */
            static void objectCB_static(void * data, void * data2, void * obj, int code);
            void objectCB(void * obj, int code);

            /* window callback called when we try to quit */
            static void windowCB_static(Fl_Widget* W, void* p);
            void windowCB(Fl_Widget* W);

            /* callback for the four channels image check button */
            static void fourchannelsCB_static(Fl_Widget* W, void* p);
            void fourchannelsCB(Fl_Widget* W);

            /* callback for the "show mouse position" check button */
            static void showmouseCB_static(Fl_Widget* W, void* p);
            void showmouseCB(Fl_Widget* W);

            /* callback for the "show mouse position" check button */
            static void fixedratioCB_static(Fl_Widget* W, void* p);
            void fixedratioCB(Fl_Widget* W);

            /* callback for the "reset" button */
            static void resetCB_static(Fl_Widget* W, void* p);
            void resetCB(Fl_Widget* W);

            /* callback for the "1:1" button */
            static void onetooneCB_static(Fl_Widget* W, void* p);
            void onetooneCB(Fl_Widget* W);

            /* callback for the "unit pixel" button */
            static void unitpixelCB_static(Fl_Widget* W, void* p);
            void unitpixelCB(Fl_Widget* W);

            /* callback for the "center" button */
            static void centerCB_static(Fl_Widget* W, void* p);
            void centerCB(Fl_Widget* W);

            /* callback for the "autorange" buttons */
            static void autorangeXCB_static(Fl_Widget* W, void* p);
            void autorangeXCB(Fl_Widget* W);
            static void autorangeYCB_static(Fl_Widget* W, void* p);
            void autorangeYCB(Fl_Widget* W);
            static void autorangeXYCB_static(Fl_Widget* W, void* p);
            void autorangeXYCB(Fl_Widget* W);


            /* callback for the "xmin" input button */
            static void xmin_input_static(Fl_Widget* W, void* p);
            void xmin_inputCB(Fl_Widget* W);

            /* callback for the "xmax" input button */
            static void xmax_input_static(Fl_Widget* W, void* p);
            void xmax_inputCB(Fl_Widget* W);

            /* callback for the "ymin" input button */
            static void ymin_input_static(Fl_Widget* W, void* p);
            void ymin_inputCB(Fl_Widget* W);

            /* callback for the "ymax" input button */
            static void ymax_input_static(Fl_Widget* W, void* p);
            void ymax_inputCB(Fl_Widget* W);

            /* callback for the "apply range" button */
            static void applyrangeCB_static(Fl_Widget* W, void* p);
            void applyrangeCB(Fl_Widget* W);

            /* callback for the "refresh" slider */
            static void refreshscaleCB_static(Fl_Widget* W, void* p);
            void refreshscaleCB(Fl_Widget* W);

            /* callback for the now refresh button */
            static void nowrefreshCB_static(Fl_Widget* W, void* p);
            void nowrefreshCB(Fl_Widget* W);

            /* set the param for the solid color backgorund options */
            void updatesolidback();

            /* callback for the "use solid backgorund" color checkbox */
            static void solidbackCB_static(Fl_Widget* W, void* p);
            void solidbackCB(Fl_Widget* W);

            /* callback for choosing the solid background color button */
            static void solidbackColorCB_static(Fl_Widget* W, void* p);
            void solidbackColorCB(Fl_Widget* W);

            /* callback for choosing the "add axes" check button */
            static void addAxesCB_static(Fl_Widget* W, void* p);
            void addAxesCB(Fl_Widget* W);

            /* callback for choosing the "add grid" check button */
            static void addGridCB_static(Fl_Widget* W, void* p);
            void addGridCB(Fl_Widget* W);

            /* callback for choosing the "add grid" check button */
            static void saveCB_static(Fl_Widget* W, void* p);
            void saveCB(Fl_Widget* W);

            void saveImage();

            /* callback for the zoom factor slider */
            static void zoomFactorSliderCB_static(Fl_Widget* W, void* p);
            void zoomFactorSliderCB(Fl_Widget* W);

            /* set the zoom factor */
            void setZoomFactor(int newzoom);

            /* return the current zoom factor */
            int getZoomFactor() const;

            /* timer for refresh */
            static void refresh_timer_static(void* p);
            void refresh_timer();

            /* blink timeout */
            static void refresh_timer2_static(void* p);
            void refresh_timer2();

            /* set the value of the range in the 4 input control*/
            void setRangeInput(fBox2 R);

            fBox2 getNewRange();

            /* change the range to the union of the obejct prefered ranges*/
            void useCommonRangeX();
            void useCommonRangeY();
            void useCommonRangeXY();

            fBox2 getAutoRangeX(fBox2 CR, bool keepAR);
            fBox2 getAutoRangeY(fBox2 CR, bool keepAR);

            /* change the range to the object preferred range */
            void useRangeX(Plotter2DObj * obj);
            void useRangeY(Plotter2DObj * obj);
            void useRangeXY(Plotter2DObj * obj);

            fBox2 findRangeX(Plotter2DObj * obj, fBox2 CR, bool keepAR);
            fBox2 findRangeY(Plotter2DObj * obj, fBox2 CR, bool keepAR);

            /* convert the windows coordinates */
            void convertWindowCoord(int & W, int & H, int & X, int & Y);

            /* resizing the window */
            void resizeWindow(int W, int H, int X, int Y);
            void setWindowPos(int X, int Y);
            void setDrawingSize(int W, int H);
            void setWindowSize(int W, int H);

            /* set the text "keep aspect ratio : xxx */
            void setRatioTextLabel();

            /* set the refresh rate */
            void setRefreshRate(int newrate);

            /* do a redraw */
            void doRedraw();

            Plotter2DWindow(const Plotter2DWindow &) = delete;              // no copy
            Plotter2DWindow & operator=(const Plotter2DWindow &) = delete;  //

            Img<unsigned char> * _mainImage;           // the view image
            std::atomic<int>      _mainImageQuality; // the quality of this image.

            std::atomic<RangeManager*> _RM;        // the associated range manager. Constructed in ctor and destroyed in dtor

            std::atomic<bool> _shown;          // true if the window is currently shown
            std::atomic<int> _nbchannels;      // number of channels, 3 or 4.

            std::atomic<bool> _usesolidBK;      // true if using a solid backgorund color
            std::atomic<RGBc> _solidBKcolor;    // the solid bk color

            std::atomic<int> _refreshrate;    // the refresh rate (number of refresh per minute).

            std::atomic<unsigned int> _sensibility; // the delta in image quality needed to trigger a redraw

            Fl_Double_Window * _w_mainWin;     // the main window.
            Fl_Group * _w_menuGroup;           // the option window
            Fl_Group * _w_viewGroup;           // the view group
            Fl_Window * _w_objWin;             // the object option window
            Fl_Scroll * _w_scrollWin;          // the scroll associated with the object option window
            Fl_Pack * _w_objGroup;             // the group containing the object option window
            Fl_Input *_w_xmin;                  // widget for the min x value of the range
            Fl_Input *_w_xmax;                  // widget for the max x value of the range
            Fl_Input *_w_ymin;                  // widget for the min y value of the range
            Fl_Input *_w_ymax;                  // widget for the max y value of the range
            Fl_Button *_w_applyrange;           // widget button "Apply Range"
            Fl_Check_Button *_w_fixedratio;     // widget check button "Keep a fixed apsect ratio".
            Fl_Button *_w_reset;                // widget button "Reset Range"
            Fl_Button *_w_onetoone;             // widget button "1:1 aspect ratio"
            Fl_Button *_w_unitpixel;            // widget button "Unit pixel range"
            Fl_Button *_w_center;               // widget button "center"
            Fl_Button *_w_autorangeX;            // widget button "autorange"
            Fl_Button *_w_autorangeY;            // widget button "autorange"
            Fl_Button *_w_autorangeXY;            // widget button "autorange"
            Fl_Check_Button *_w_showmouse;      // widget check button "show the mouse position"
            Fl_Check_Button *_w_fourchannels;   // widget check button "use 4 channel images"
            Fl_Check_Button *_w_solidback;      // widget check button "use a solid background color"
            Fl_Button * _w_solidbackColor;      // wideget button for choosing the solid background color
            Fl_Check_Button *_w_addAxesObj;     // widget "add axes" check button
            Fl_Check_Button *_w_addGridObj;     // widget "add grid" check button
            Fl_Button *_w_save;                 // widget button "Save Image"
            Fl_Counter * _w_zoomfactorslider; // widget zoom factor slider
            Fl_Box * _w_zoomfactortext;            // widget zoom factor text

            Fl_Value_Slider * _w_refreshscale;  // the refresh rate slider
            Fl_Button *_w_nowRefresh;            // the redraw now button
            View2DWidget * _PW;                 // the 2D widget displaying the graphics

            int _obj_width;                     // the object window width

            Plot2DAxes * _axePlot;              // the axes plot
            Plot2DGrid * _gridPlot;             // the grid plot

            /********************************************************************************************************************************/

            std::vector<Plotter2DObj *> _vecPlot;   // the vector of objects

            /* find the index of an object in the vector, return -1 if not in there */
            int findIndex(Plotter2DObj * obj);

            void add(Plotter2DObj * obj);
            void remove(Plotter2DObj * obj);
            void moveUp(Plotter2DObj * obj);
            void moveDown(Plotter2DObj * obj);
            void moveTop(Plotter2DObj * obj);
            void moveBottom(Plotter2DObj * obj);
            void removeAll();
            void fixObjectWindow();


            /* return the current quality of the complete drawing of all the objects */
            int quality();

            /* reutrn if there is an object that is inserted and suspended at the smae time */
            bool isSuspendedInserted();

        };








        /* find the index of an object in the vector, return -1 if not in there */
        int Plotter2DWindow::findIndex(Plotter2DObj * obj)
            {
            for (int i = 0; i < (int)_vecPlot.size();i++)
                {
                if (obj == _vecPlot[i]) return i;
                }
            return -1;
            }



        /* return the min quality of the active objects. Return 100 if there is no (active) object */
        int Plotter2DWindow::quality()
            {
            int q = 100;
            for (int i = (int)_vecPlot.size(); i > 0; i--)
                {
                if (_vecPlot[i - 1]->enable())
                    {
                    int r = _vecPlot[i - 1]->quality();
                    if (r < q) { q = r; }
                    }
                }
            return q;
            }


        /* return true if there is currently an object that is simultaneously enabled yet suspended, */
        bool Plotter2DWindow::isSuspendedInserted()
            {
            for (int i = (int)_vecPlot.size(); i > 0; i--)
                {
                if ((_vecPlot[i - 1]->enable()) && (_vecPlot[i - 1]->suspend())) return true;
                }
            return false;
            }

        /* add the object in the vector */
        void Plotter2DWindow::add(Plotter2DObj * obj)
            {
            if (obj == nullptr)
                {
                MTOOLS_DEBUG("Plotter2DWindow::add with a nullptr pointer!");
                return;
                }
            if (findIndex(obj) >= 0)
                {
                MTOOLS_DEBUG("Plotter2DWindow::add, object already inserted!");
                return;
                }
            /* move all the object down and insert the new one on the top */
            _vecPlot.resize(_vecPlot.size() + 1);
            for (int k = ((int)_vecPlot.size()) - 1; k > 0; --k) { _vecPlot[k] = _vecPlot[k - 1]; }
            _vecPlot[0] = obj;
            // call the inserted() method of the object to inform it of its insertion
            obj->_inserted(objectCB_static, _RM, this, obj, _obj_width - Fl::scrollbar_size());
            MTOOLS_ASSERT(_vecPlot[0]->_optionWindow() != nullptr);
            _w_scrollWin->add(_vecPlot[0]->_optionWindow()); // insert it into the scroll window
            // move the window at its correct place and move all the other one down
            int l = 0;
            for (int i = 0; i < (int)_vecPlot.size(); i++)
                {
                auto W = _vecPlot[i]->_optionWindow();
                MTOOLS_ASSERT(W != nullptr);
                W->resize(0, l, W->w(), W->h());
                l += W->h() + 10;
                }
            _w_scrollWin->redraw();
            updateView();
            }



        /* remove an object */
        void Plotter2DWindow::remove(Plotter2DObj * obj)
            {
            for (int i = 0; i < (int)_vecPlot.size(); i++)
                {
                if (_vecPlot[i] == obj)
                    {
                    int y = obj->_optionWindow()->y();              // position of the window
                    _w_scrollWin->remove(obj->_optionWindow());     // we remove it from the scroll window
                    obj->_removed();                                // inform than we are removing the object from the plotter
                    while ((i+1) < ((int)_vecPlot.size()))
                        {
                        _vecPlot[i] = _vecPlot[i + 1];
                        _vecPlot[i]->_optionWindow()->resize(0, y, _vecPlot[i]->_optionWindow()->w(), _vecPlot[i]->_optionWindow()->h());
                        y += _vecPlot[i]->_optionWindow()->h() + 10;
                        i++;
                        }
                    _vecPlot.resize(_vecPlot.size() - 1); // diminish the vector size by one.
                    _w_scrollWin->redraw(); // redraw the option window
                    updateView(); // redraw the view
                    return;
                    }
                }
            MTOOLS_DEBUG("Plotter2DWindow::remove(), object not found.");
            }


        /* remove every object in the plotter */
        void Plotter2DWindow::removeAll()
            {
            while (_vecPlot.size() != 0) { remove(_vecPlot[0]); }
            }



        /* move an object up in the vector */
        void Plotter2DWindow::moveUp(Plotter2DObj * obj)
            {
            for (int i = 0; i < (int)_vecPlot.size(); i++)
                {
                if (_vecPlot[i] == obj)
                    {
                    if (i == 0) return; // nothing to do if already on top
                    auto obj2 = _vecPlot[i - 1];
                    _vecPlot[i - 1] = obj;
                    _vecPlot[i] = obj2;
                    int y = obj2->_optionWindow()->y();
                    int y2 = y + obj->_optionWindow()->h() + 10;
                    obj->_optionWindow()->resize(0, y, obj->_optionWindow()->w(), obj->_optionWindow()->h());
                    obj2->_optionWindow()->resize(0, y2, obj2->_optionWindow()->w(), obj2->_optionWindow()->h());
                    _w_scrollWin->redraw(); // redraw the option window
                    updateView(); // redraw the view
                    return;
                    }
                }
            MTOOLS_DEBUG("Plotter2DWindow::moveUp(), object not found.");
            }


        void Plotter2DWindow::moveDown(Plotter2DObj * obj)
            {
            for (int i = 0; i < (int)_vecPlot.size(); i++)
                {
                if (_vecPlot[i] == obj)
                    {
                    i++;
                    if (i == (int)_vecPlot.size()) return; // nothing to do if already at bottom
                    moveUp(_vecPlot[i]);
                    return;
                    }
                }
            MTOOLS_DEBUG("Plotter2DWindow::moveDown(), object not found.");
            }


        void Plotter2DWindow::moveTop(Plotter2DObj * obj)
            {
            for (int i = 0; i < (int)_vecPlot.size(); i++)
                {
                if (_vecPlot[i] == obj)
                    {
                    if (i == 0) return; // nothing to do if already on top
                    for (int k = i; k > 0; k--) { _vecPlot[k] = _vecPlot[k - 1];}
                    _vecPlot[0] = obj;
                    int l = 0;
                    for (int k = 0; k < (int)_vecPlot.size(); k++)
                        {
                        auto w = _vecPlot[k]->_optionWindow();
                        w->resize(0, l, w->w(), w->h());
                        l += w->h() + 10;
                        }
                    _w_scrollWin->redraw(); // redraw the option window
                    updateView(); // redraw the view
                    return;
                    }
                }
            MTOOLS_DEBUG("Plotter2DWindow::moveTop(), object not found.");
            }


        void Plotter2DWindow::moveBottom(Plotter2DObj * obj)
            {
            for (int i = 0; i < (int)_vecPlot.size(); i++)
                {
                if (_vecPlot[i] == obj)
                    {
                    if (i == (((int)_vecPlot.size())-1)) return; // nothing to do if already on bottom
                    for (int k = i+1; k < (int)_vecPlot.size(); k++) { _vecPlot[k - 1] = _vecPlot[k];  }
                    _vecPlot[_vecPlot.size() - 1] = obj;
                    int l = 0;
                    for (int k = 0; k < (int)_vecPlot.size(); k++)
                        {
                        auto w = _vecPlot[k]->_optionWindow();
                        w->resize(0, l, w->w(), w->h());
                        l += w->h() + 10;
                        }
                    _w_scrollWin->redraw(); // redraw the option window
                    updateView(); // redraw the view
                    return;
                    }
                }
            MTOOLS_DEBUG("Plotter2DWindow::moveBottom(), object not found.");
            }


        /* Fix the window with all the object window by resizing them to their right place and then redraw the window */
        void Plotter2DWindow::fixObjectWindow()
            {
            int l = 0;
            for (int i = 0; i < (int)_vecPlot.size(); i++)
                {
                auto w = _vecPlot[i]->_optionWindow();
                w->resize(0, l, w->w(), w->h());
                l += w->h() + 10;
                }
            _w_scrollWin->redraw(); // redraw the option window
            updateView(); // redraw the view
            return;
            }


        /* change the image to the correct size */
        void Plotter2DWindow::setImageSize(int lx, int ly, int ch)
        {
            if (lx*ly*ch == 0) { if (_mainImage != nullptr) { delete _mainImage; _mainImage = nullptr; } return; }
            if (_mainImage == nullptr) { _mainImage = new Img<unsigned char>(lx, ly, 1, ch); } else { _mainImage->resize(lx, ly, 1, ch, -1); }
        }




        /* Constructor : Construct the plotter window but do not show it */
        Plotter2DWindow::Plotter2DWindow(bool addAxes, bool addGrid, int X, int Y, int W, int H) : _mainImage(nullptr), _mainImageQuality(0), _RM(nullptr), _shown(false), _nbchannels(3), _usesolidBK(true), _solidBKcolor(RGBc::c_White), _refreshrate(0), _sensibility(Plotter2D::DEFAULT_SENSIBILITY), _axePlot(nullptr), _gridPlot(nullptr)
        {
            convertWindowCoord(W, H, X, Y);

            _w_mainWin = new Fl_Double_Window(X, Y, W, H, "Plotter 2D"); // the main window

            _w_menuGroup = new Fl_Group(0, 0, 300, H); // the option group

            auto _w_borderrange = new Fl_Box(5, 5, 290, 230);
            _w_borderrange->box(FL_UP_BOX);
            _w_borderrange->labelfont(0);
            _w_borderrange->labelsize(16);
            _w_borderrange->align(Fl_Align(FL_ALIGN_TOP_LEFT));

            _w_ymax = new Fl_Input(110, 30, 80, 17, "Ymax");
            _w_ymax->box(FL_BORDER_BOX);
            _w_ymax->labelfont(0);
            _w_ymax->labelsize(11);
            _w_ymax->textsize(11);
            _w_ymax->align(Fl_Align(FL_ALIGN_TOP));
            _w_ymax->callback(ymax_input_static, this);
            _w_ymax->when(FL_WHEN_ENTER_KEY);

            _w_xmin = new Fl_Input(20, 80, 80, 17, "Xmin");
            _w_xmin->box(FL_BORDER_BOX);
            _w_xmin->labelfont(0);
            _w_xmin->labelsize(11);
            _w_xmin->textsize(11);
            _w_xmin->align(Fl_Align(FL_ALIGN_TOP));
            _w_xmin->callback(xmin_input_static, this);
            _w_xmin->when(FL_WHEN_ENTER_KEY);

            _w_xmax = new Fl_Input(200, 80, 80, 17, "Xmax");
            _w_xmax->box(FL_BORDER_BOX);
            _w_xmax->labelfont(0);
            _w_xmax->labelsize(11);
            _w_xmax->textsize(11);
            _w_xmax->align(Fl_Align(FL_ALIGN_TOP));
            _w_xmax->callback(xmax_input_static, this);
            _w_xmax->when(FL_WHEN_ENTER_KEY);

            _w_ymin = new Fl_Input(110, 133, 80, 17, "Ymin");
            _w_ymin->box(FL_BORDER_BOX);
            _w_ymin->labelfont(0);
            _w_ymin->labelsize(11);
            _w_ymin->textsize(11);
            _w_ymin->align(Fl_Align(FL_ALIGN_TOP));
            _w_ymin->callback(ymin_input_static, this);
            _w_ymin->when(FL_WHEN_ENTER_KEY);

            _w_applyrange = new Fl_Button(125, 71, 50, 34, "Set");
            _w_applyrange->down_box(FL_DOWN_BOX);
            _w_applyrange->labelfont(0);
            _w_applyrange->labelsize(12);
            _w_applyrange->callback(applyrangeCB_static, this);

            _w_fixedratio = new Fl_Check_Button(15, 167, 220, 15, "Maintain a fixed aspect ratio");
            _w_fixedratio->down_box(FL_DOWN_BOX);
            _w_fixedratio->labelfont(0);
            _w_fixedratio->labelsize(11);
            _w_fixedratio->color2(FL_RED);
            _w_fixedratio->callback(fixedratioCB_static, this);
            _w_fixedratio->when(FL_WHEN_CHANGED);

            _w_reset = new Fl_Button(14, 195, 63, 30, "Reset");
            _w_reset->labelsize(12);
            _w_reset->labelfont(0);
            _w_reset->callback(resetCB_static, this);

            _w_onetoone = new Fl_Button(84, 195, 63, 30, "1:1 ratio");
            _w_onetoone->labelsize(12);
            _w_onetoone->labelfont(0);
            _w_onetoone->callback(onetooneCB_static, this);

            _w_unitpixel = new Fl_Button(154, 195, 63, 30, "unit pixel");
            _w_unitpixel->labelsize(12);
            _w_unitpixel->labelfont(0);
            _w_unitpixel->callback(unitpixelCB_static, this);

            _w_center = new Fl_Button(224, 195, 63, 30, "center");
            _w_center->labelsize(12);
            _w_center->labelfont(0);
            _w_center->callback(centerCB_static, this);



            auto _w_borderrange2 = new Fl_Box(5, 240, 290, 175);
            _w_borderrange2->box(FL_UP_BOX);
            _w_borderrange2->labelfont(0);
            _w_borderrange2->labelsize(16);
            _w_borderrange2->align(Fl_Align(FL_ALIGN_TOP_LEFT));


            _w_showmouse = new Fl_Check_Button(15, 246+3, 220, 15, "Show the mouse position");
            _w_showmouse->down_box(FL_DOWN_BOX);
            _w_showmouse->labelfont(0);
            _w_showmouse->labelsize(11);
            _w_showmouse->color2(FL_RED);
            _w_showmouse->callback(showmouseCB_static, this);
            _w_showmouse->when(FL_WHEN_CHANGED);

            _w_fourchannels = new Fl_Check_Button(15, 267 + 3, 200, 15, "Use a 4 channels RGBA image");
            _w_fourchannels->down_box(FL_DOWN_BOX);
            _w_fourchannels->labelfont(0);
            _w_fourchannels->labelsize(11);
            _w_fourchannels->color2(FL_RED);
            _w_fourchannels->callback(fourchannelsCB_static, this);
            _w_fourchannels->when(FL_WHEN_CHANGED);


            _w_solidback = new Fl_Check_Button(15, 288 + 3, 185, 15, "Use a solid background color");
            _w_solidback->down_box(FL_DOWN_BOX);
            _w_solidback->labelfont(0);
            _w_solidback->labelsize(11);
            _w_solidback->color2(FL_RED);
            _w_solidback->callback(solidbackCB_static, this);
            _w_solidback->when(FL_WHEN_CHANGED);
            _w_solidbackColor = new Fl_Button(200, 288 + 3, 15, 15);
            _w_solidbackColor->callback(solidbackColorCB_static, this);

            _w_addAxesObj = new Fl_Check_Button(15, 309 + 3, 185, 15, "Add a Plotter2DAxes object on top");
            _w_addAxesObj->down_box(FL_DOWN_BOX);
            _w_addAxesObj->labelfont(0);
            _w_addAxesObj->labelsize(11);
            _w_addAxesObj->color2(FL_RED);
            _w_addAxesObj->callback(addAxesCB_static, this);
            _w_addAxesObj->when(FL_WHEN_CHANGED);

            _w_addGridObj = new Fl_Check_Button(15, 330 + 3, 185, 15, "Add a Plotter2DGrid object on top");
            _w_addGridObj->down_box(FL_DOWN_BOX);
            _w_addGridObj->labelfont(0);
            _w_addGridObj->labelsize(11);
            _w_addGridObj->color2(FL_RED);
            _w_addGridObj->callback(addGridCB_static, this);
            _w_addGridObj->when(FL_WHEN_CHANGED);

            auto _w_refreshlabel = new Fl_Box(15, 291 + 25 + 37 + 3, 125, 17, "Refresh rate (per min.)");
            _w_refreshlabel->align(Fl_Align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT));
            _w_refreshlabel->labelfont(0);
            _w_refreshlabel->labelsize(11);

            _w_refreshscale = new Fl_Value_Slider(140, 292 + 25 + 37 + 3, 105, 17, nullptr);
            _w_refreshscale->labelfont(0);
            _w_refreshscale->labelsize(11);
            _w_refreshscale->align(Fl_Align(FL_ALIGN_RIGHT));
            _w_refreshscale->box(FL_FLAT_BOX);
            _w_refreshscale->type(FL_HOR_NICE_SLIDER);
            _w_refreshscale->range(0, 600);
            _w_refreshscale->step(1);
            _w_refreshscale->value(0);
            _w_refreshscale->color2(FL_RED);
            _w_refreshscale->callback(refreshscaleCB_static, this);

            _w_nowRefresh = new Fl_Button(250, 290 + 25 + 37 + 3, 35, 21, "now!");
            _w_nowRefresh->labelsize(11);
            _w_nowRefresh->labelfont(0);
            _w_nowRefresh->callback(nowrefreshCB_static, this);

            int L = 83;
            _w_autorangeX = new Fl_Button(16, 380 + 5, L, 21, "range X");
            _w_autorangeX->labelsize(12);
            _w_autorangeX->labelfont(0);
            _w_autorangeX->callback(autorangeXCB_static, this);

            _w_autorangeY = new Fl_Button(26 +L, 380 + 5, L, 21, "range Y");
            _w_autorangeY->labelsize(12);
            _w_autorangeY->labelfont(0);
            _w_autorangeY->callback(autorangeYCB_static, this);

            _w_autorangeXY = new Fl_Button(36 + L*2, 380 + 5, L , 21, "range X/Y");
            _w_autorangeXY->labelsize(12);
            _w_autorangeXY->labelfont(0);
            _w_autorangeXY->callback(autorangeXYCB_static, this);

            _obj_width = 303 - Fl::scrollbar_size();
            int h = H - 360 - 50 - 55 - 50;
            _w_objWin = new Fl_Window(Fl::scrollbar_size(), 360 + 55+10, _obj_width, h);
            _w_scrollWin = new Fl_Scroll(0, 0, _obj_width, h);
            _w_scrollWin->labelfont(0);
            _w_scrollWin->labelsize(16);
            _w_scrollWin->end();

            _w_objWin->end();
            _w_objWin->resizable(_w_scrollWin); // maybe we should resize wrt _w_objGroup ?


            auto _w_borderrange3 = new Fl_Box(5, 360 + 55 + 10+h+20, 290, 65 );
            _w_borderrange3->box(FL_UP_BOX);
            _w_borderrange3->labelfont(0);
            _w_borderrange3->labelsize(16);
            _w_borderrange3->align(Fl_Align(FL_ALIGN_TOP_LEFT));

            auto _w_zoomfactorlabel = new Fl_Box(15, H-60, 100, 17, "Image/View ratio");
            _w_zoomfactorlabel->align(Fl_Align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT));
            _w_zoomfactorlabel->labelfont(0);
            _w_zoomfactorlabel->labelsize(11);

            
            _w_zoomfactorslider = new Fl_Counter(115, H-60, 70, 17, nullptr);
            _w_zoomfactorslider->labelfont(0);
            _w_zoomfactorslider->labelsize(11);
            _w_zoomfactorslider->align(Fl_Align(FL_ALIGN_RIGHT));
            _w_zoomfactorslider->box(FL_FLAT_BOX);
            _w_zoomfactorslider->type(FL_HOR_NICE_SLIDER);
            _w_zoomfactorslider->range(1, 20);
            _w_zoomfactorslider->step(1);
            _w_zoomfactorslider->value(1);
            _w_zoomfactorslider->color2(FL_RED);
            _w_zoomfactorslider->callback(zoomFactorSliderCB_static, this);

            _w_zoomfactortext = new Fl_Box(185, H - 60, 110, 17, "");
            _w_zoomfactortext->align(Fl_Align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT));
            _w_zoomfactortext->labelfont(0);
            _w_zoomfactortext->labelsize(11);
           
            _w_save = new Fl_Button(105, H - 37, 90, 24, "Save Image");
            _w_save->down_box(FL_DOWN_BOX);
            _w_save->labelfont(0);
            _w_save->labelsize(12);
            _w_save->callback(saveCB_static, this);

            _w_menuGroup->end();
            _w_menuGroup->resizable(_w_objWin);

            _w_viewGroup = new Fl_Group(305, 5, W - 310, H - 10);
            _w_viewGroup->box(FL_UP_BOX);

            _PW = new View2DWidget(310, 10, W - 320, H - 20);       // the planar widget
            _PW->setCrossCB(view2DcrossCB_static, this);            // set its callbacks
            _PW->setNotificationCB(view2DnotCB_static, this);       //

            _w_viewGroup->end();
            _w_viewGroup->resizable(_PW);
            _w_mainWin->end();
            _w_mainWin->resizable(_w_viewGroup);

            _w_mainWin->size_range(Plotter2D::MIN_W, Plotter2D::MIN_H);     // min and max size of the plotter window
            _w_mainWin->callback(windowCB_static, this);                    // callback when the user pressed the close button

            _RM = new RangeManager(_PW->viewSizeFactor());                                              // create the range manager
            ((RangeManager*)_RM)->setNotificationCallback(rangeManagerCB_static, this, nullptr);  // and register the callback it should call when the range changes
            _PW->setRangeManager(_RM);                                                            // and attach it to the view ajusting the view size with that in the range manager.

            auto dim = ((RangeManager*)_RM)->getWinSize();      // adjust the _mainImage to have the same size
            setImageSize((int)dim.X(), (int)dim.Y(), _nbchannels);        //

            view2DcrossCB(_PW->crossOn());                      // make sure the checkboxes have the correct initial value
            updatesolidback();                                  //
            _w_fixedratio->value(((RangeManager*)_RM)->fixedAspectRatio() ? 1 : 0); // update the fixed aspect ratio checkbox status

            setRangeInput(((RangeManager*)_RM)->getRange());    // update the range input buttons
            setRatioTextLabel();                                     // update the "keep aspect ratio" text

            setRefreshRate(0);                                  // no refresh by default
            Fl::add_timeout(0.1, static_updateViewTimer, this); // set the timer used for updating the view

            setZoomFactor(1);  // set the view zoom factor to 1
            _w_zoomfactortext->copy_label((std::string("[") + toString(dim.X()) + "x" + toString(dim.Y()) + "]").c_str()); // update the View size text
            _w_zoomfactortext->redraw_label(); // redraw the label.

            _insertAxesObject(addAxes); // create the objects and update the widgets
            _insertGridObject(addGrid); //


        }


        /* destructor, remove all the object, all the timers and then destroy the plotter window*/
        Plotter2DWindow::~Plotter2DWindow()
        {
            endPlot();                                         // hide the window
            Fl::remove_timeout(static_updateViewTimer, this);  // remove the update view timer
            setRefreshRate(0);                                 // remove refresh timer callback
            _insertAxesObject(false); // remove the axes if needed
            _insertGridObject(false); // remove the grid if needed
            removeAll(); // remove all the object still in the plotter

            ((RangeManager*)_RM)->setNotificationCallback(nullptr, nullptr,nullptr); // remove the range manager callback so we do not receive modif event anymore

            _PW->setRangeManager(nullptr);                  // remove the range manager from the view
            _PW->setCrossCB(nullptr, nullptr);              // remove all the callbacks of the view
            _PW->setNotificationCB(nullptr, nullptr);       //

            delete ((RangeManager*)_RM);  // delete the range manager.
            delete _w_mainWin; // and then delete the main window : this delete all the sub widgets.
            setImageSize(0, 0, 0); // finaly, delete the image.
        }


        /* simply show the plotter window (and resize if needed the keep the same view size) */
        void Plotter2DWindow::startPlot()
        {
            ((RangeManager*)_RM)->saveAsDefault();
            if (_gridPlot != nullptr)
                {
                int i = findIndex(_gridPlot);
                if (i >= 0) { moveTop(_gridPlot); }
                }
            if (_axePlot != nullptr)
                {
                int i = findIndex(_axePlot);
                if (i >= 0) { moveTop(_axePlot); }
                }
            _shown = true;
            iVec2 viewSize = _PW->viewSizeFactor();   // save the viewSize (UGLY FIX)
            viewSize /= _PW->zoomFactor(); // get the real size
            _w_mainWin->show();
            setDrawingSize((int)viewSize.X(), (int)viewSize.Y()); // restore the viewSize in case the window manager changed it when we showed the window (UGLY FIX :-) )
            _PW->take_focus();
        }


        /* simply hide the plotter window */
        void Plotter2DWindow::endPlot()
            {
            _w_mainWin->hide();
            _shown = false;
            }


        /* update the view by recreating the whole image.
         * does not update the view if the resulting quality is zero */
        void Plotter2DWindow::updateView(bool withreset)
            {
            int maxretry = (withreset ? 25 : 0);
            int retry = 0;
            if (withreset) _PW->discardImage(); else { _mainImage->checkerboard(); }  // do it now while worker thread continu
            if (isSuspendedInserted()) {maxretry -= 20;} // try only 5 times if there is a suspended object; 
            _mainImageQuality = quality(); // query the current quality
            while ((_mainImageQuality == 0) && (retry < maxretry))
                { // quality is zero, we wait a little and before retry
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                retry++;
                _mainImageQuality = quality();
                }
            if (_mainImageQuality > 0)
                { // ok, there should be something to draw.. (we now interrupt every worker thread)
                if (_usesolidBK) { _mainImage->clear(((RGBc)_solidBKcolor).getOpaque()); } else { _mainImage->checkerboard(); }// draw the background of the image
                int q = 100;
                for (int i = (int)_vecPlot.size(); i > 0; i--)
                    {
                    if (_vecPlot[i - 1]->enable())
                        {
                        int r = 0;
                        if (_vecPlot[i - 1]->quality() > 0) { r = _vecPlot[i - 1]->drawOnto(*_mainImage); }
                        if (r < q) { q = r; }
                        }
                    }
                _mainImageQuality = q;
                if (_mainImageQuality != 0) // make sure the quality is indeed not zero. 
                    {
                    _PW->improveImageFactor(_mainImage); // set the image
                    _PW->redrawView(); // redraw the view.
                    return;
                    }
                }
                // no, still nothing, we draw whatever we can from the previously displayed image  
                _PW->displayMovedImage(RGBc::c_Gray);
            }


        /* timer used to redraw the view when the quality changes. This timer is always on : created a construction and stoped at destruction time
         * call updateView when the current quality is not zero and differs from the quality of the last image drawn */
        void Plotter2DWindow::static_updateViewTimer(void* p) { if (p == nullptr) { return; } ((Plotter2DWindow *)p)->updateViewTimer(); }
        void Plotter2DWindow::updateViewTimer()
            {
            int q = quality();
            if (q != 0)
                {
                if (q != (int)_mainImageQuality)
                    {
                    if ((q == 100) || (q<_mainImageQuality) || (_mainImageQuality == 0) || (q >= _mainImageQuality + (int)_sensibility))
                        {
                        updateView(false);
                        Fl::add_timeout(0.1, static_updateViewTimer, this);
                        return;
                        }
                    }
                if (_PW->zoomFactor() > 1) _PW->improveImageFactor(_mainImage);
                }
            Fl::add_timeout(0.1, static_updateViewTimer, this);
            }



        /* This method is called by the view when there is a key that was pressed */
        void Plotter2DWindow::view2DnotCB_static(void * data, int key) { MTOOLS_ASSERT(data != nullptr); ((Plotter2DWindow *)data)->view2DnotCB(key); }
        void Plotter2DWindow::view2DnotCB(int key)
            {
            if ((key == 'p')||(key == 'P'))
                {
                saveImage();
                return;
                }
            if (key == FL_Home)
                {
                setZoomFactor(getZoomFactor() + 1);
                return;
                }
            if (key == FL_End)
                {
                setZoomFactor(getZoomFactor() - 1);
                return;
                }
            }


        /* this method is called when the status of the cross changes */
        bool Plotter2DWindow::view2DcrossCB_static(void * data, bool newstatus) { MTOOLS_ASSERT(data != nullptr); return ((Plotter2DWindow *)data)->view2DcrossCB(newstatus); }
        bool Plotter2DWindow::view2DcrossCB(bool newstatus)
        {
            _w_showmouse->value(newstatus ? 1 : 0); // update the widget status
            return newstatus; // accept the change
        }



        /* called by the range manager when the range/window size changes
         * Here, we need to be careful to avoid deadlock because this function may be called from outside of the fltk thread
         */
        bool Plotter2DWindow::rangeManagerCB_static(void* data, void * data2, bool changedRange, bool changedWinSize, bool changedFixAspectRatio) { MTOOLS_ASSERT(data != nullptr); return ((Plotter2DWindow *)data)->rangeManagerCB(changedRange, changedWinSize, changedFixAspectRatio); }
        bool Plotter2DWindow::rangeManagerCB(bool changedRange, bool changedWinSize, bool changedFixAspectRatio)
            {
            // here, we may, or may not, be in the fltk thread.
            fBox2 R = ((RangeManager*)_RM)->getRange();             // get the new range
            iVec2 winSize = ((RangeManager*)_RM)->getWinSize();     // get the new window size
            bool fixedAR = ((RangeManager*)_RM)->fixedAspectRatio();  // get the fixed aspect ratio status
            for (int i = 0; i < (int)_vecPlot.size(); i++)
                {
                _vecPlot[i]->setParam(R, winSize); // set the parameters for all the object in the plotter
                }
            // we run rangeManagerCB2 in fltk
            if (isFltkThread()) { rangeManagerCB2(R, winSize, fixedAR, changedRange, changedWinSize, changedFixAspectRatio); }
            else {
                 mtools::IndirectMemberProc<internals_graphics::Plotter2DWindow, fBox2, iVec2, bool, bool, bool, bool> proxy(*this, &Plotter2DWindow::rangeManagerCB2, R, winSize, fixedAR, changedRange, changedWinSize, changedFixAspectRatio);
                 mtools::runInFltkThread(proxy);
                 }
            // back to our original thread.
            return true; // accept the change
            }


        /* this part is run in fltk */
        void Plotter2DWindow::rangeManagerCB2(fBox2 R, iVec2 winSize,bool fixedAR, bool changedRange, bool changedWinSize, bool changedFixAspectRatio)
            {
            setImageSize((int)winSize.X(), (int)winSize.Y(), _nbchannels);    // resize the image if needed
            updateView();                                           // update the view
            setRangeInput(R);                                       // update the range widgets
            setRatioTextLabel();                                    // and the aspect ratio text
            _w_fixedratio->value(fixedAR ? 1 : 0);                  // update the fixed apsect ratio checkbox
            _w_zoomfactortext->copy_label((std::string("[") + toString(winSize.X()) + "x" + toString(winSize.Y()) + "]").c_str()); // update the View size text
            _w_zoomfactortext->redraw_label();
            }



        /* called by the inserted object when it want a redraw or when it detaches itself, always called from FLTK */
        void Plotter2DWindow::objectCB_static(void * data, void * data2, void * obj, int code) { MTOOLS_ASSERT(data != nullptr); ((Plotter2DWindow *)data)->objectCB(obj, code); }
        void Plotter2DWindow::objectCB(void * obj, int code)
            {
            MTOOLS_ASSERT(isFltkThread());
            switch (code)
                {
                case Plotter2DObj::_REQUEST_DETACH:
                    { // the object is detaching itself
                    remove((Plotter2DObj*)obj);
                    return;
                    }
                case Plotter2DObj::_REQUEST_REFRESH:
                    {
                    updateView();
                    return;
                    }
                case Plotter2DObj::_REQUEST_YIELDFOCUS:
                    {
                    _PW->take_focus();
                    return;
                    }
                case Plotter2DObj::_REQUEST_UP:
                    {
                    moveUp((Plotter2DObj*)obj);
                    _PW->take_focus();
                    return;
                    }
                case Plotter2DObj::_REQUEST_DOWN:
                    {
                    moveDown((Plotter2DObj*)obj);
                    _PW->take_focus();
                    return;
                    }
                case Plotter2DObj::_REQUEST_TOP:
                    {
                    moveTop((Plotter2DObj*)obj);
                    _PW->take_focus();
                    return;
                    }
                case Plotter2DObj::_REQUEST_BOTTOM:
                    {
                    moveBottom((Plotter2DObj*)obj);
                    _PW->take_focus();
                    return;
                    }
                case Plotter2DObj::_REQUEST_USERANGEX:
                    {
                    useRangeX((Plotter2DObj*)obj);
                    _PW->take_focus();
                    return;
                    }
                case Plotter2DObj::_REQUEST_USERANGEY:
                    {
                    useRangeY((Plotter2DObj*)obj);
                    _PW->take_focus();
                    return;
                    }
                case Plotter2DObj::_REQUEST_USERANGEXY:
                    {
                    useRangeXY((Plotter2DObj*)obj);
                    _PW->take_focus();
                    return;
                    }
                case Plotter2DObj::_REQUEST_FIXOBJECTWIN:
                    {
                    fixObjectWindow();
                    _PW->take_focus();
                    return;
                    }
                }
            MTOOLS_ERROR("Plotter2DWindow::objectCB, incorrect code!");
            }


        /* called when the user closes the plotter */
        void Plotter2DWindow::windowCB_static(Fl_Widget* W, void* p) { MTOOLS_ASSERT(p != nullptr); ((Plotter2DWindow *)p)->windowCB(W); }
        void Plotter2DWindow::windowCB(Fl_Widget* W)
        {
            endPlot();
            return;
        }



        /* update the widget for the solid bk color */
        void Plotter2DWindow::updatesolidback()
        {
        _w_solidbackColor->color2((Fl_Color)((RGBc)_solidBKcolor));
        _w_solidbackColor->color((Fl_Color)((RGBc)_solidBKcolor));
        if ((bool)_usesolidBK)
            {
            _w_solidback->value(1);
            _w_solidbackColor->activate();
            }
        else
            {
            _w_solidback->value(0);
            _w_solidbackColor->deactivate();
            }
        _w_solidback->redraw();
        _w_solidbackColor->redraw();
        updateView();
        }


        /* called when the "Use a solid background color checkbox" is mofified */
        void Plotter2DWindow::solidbackCB_static(Fl_Widget* W, void* p) { MTOOLS_ASSERT(p != nullptr); ((Plotter2DWindow *)p)->solidbackCB(W); }
        void Plotter2DWindow::solidbackCB(Fl_Widget* W)
        {
        _usesolidBK = (_w_solidback->value()) ? true : false;
        updatesolidback();
        _PW->take_focus();
        }


        /* callback for choosing the solid background color button */
        void Plotter2DWindow::solidbackColorCB_static(Fl_Widget* W, void* p) { MTOOLS_ASSERT(p != nullptr); ((Plotter2DWindow *)p)->solidbackColorCB(W); }
        void Plotter2DWindow::solidbackColorCB(Fl_Widget* W)
        {
        RGBc Coul = ((RGBc)_solidBKcolor);
        if (fl_color_chooser("Axes Color", Coul.comp.R, Coul.comp.G, Coul.comp.B, 1) != 0)
            {
            _solidBKcolor = Coul.getOpaque();
            updatesolidback();
            }
        _PW->take_focus();
        }


        /* callback for the "add axes" check button */
        void Plotter2DWindow::addAxesCB_static(Fl_Widget* W, void* p) { MTOOLS_ASSERT(p != nullptr); ((Plotter2DWindow *)p)->addAxesCB(W); }
        void Plotter2DWindow::addAxesCB(Fl_Widget* W)
            {
            _insertAxesObject((_w_addAxesObj->value() == 0) ? false : true);
            }


        /* create and insert / remove and delete the axe object and update the widget */
        void Plotter2DWindow::_insertAxesObject(bool status)
            {
            if (status)
                {
                if (_axePlot == nullptr)
                    {
                    _axePlot = new Plot2DAxes("Axes");
                    add(_axePlot);
                    }
                if (_w_addAxesObj->value() == 0) { _w_addAxesObj->value(1); }
                return;
                }
            if (_axePlot != nullptr)
                {
                remove(_axePlot);
                delete _axePlot;
                _axePlot = nullptr;
            }
            if (_w_addAxesObj->value() != 0) { _w_addAxesObj->value(0); }
            }


        /* callback for the "add grid" check button */
        void Plotter2DWindow::addGridCB_static(Fl_Widget* W, void* p) { MTOOLS_ASSERT(p != nullptr); ((Plotter2DWindow *)p)->addGridCB(W); }
        void Plotter2DWindow::addGridCB(Fl_Widget* W)
            {
            _insertGridObject((_w_addGridObj->value() == 0) ? false : true);
            }


        /* create and insert / remove and delete the grid object and update the widget */
        void Plotter2DWindow::_insertGridObject(bool status)
            {
            if (status)
                {
                if (_gridPlot == nullptr)
                    {
                    _gridPlot = new Plot2DGrid("Grid");
                    add(_gridPlot);
                    }
                if (_w_addGridObj->value() == 0) { _w_addGridObj->value(1); }
                return;
                }

            if (_gridPlot != nullptr)
                {
                remove(_gridPlot);
                delete _gridPlot;
                _gridPlot = nullptr;
                }
            if (_w_addGridObj->value() != 0) { _w_addGridObj->value(0); }
            }




        /* called when the button "save image" is pressed */
        void Plotter2DWindow::saveCB_static(Fl_Widget* W, void* p) { MTOOLS_ASSERT(p != nullptr); ((Plotter2DWindow *)p)->saveCB(W); }
        void Plotter2DWindow::saveCB(Fl_Widget* W)
            {
            saveImage();
            }


        /* save the image */
        void Plotter2DWindow::saveImage()
        {
         char * res = fl_file_chooser("Image name", "*","image.png",0);
         if (res == nullptr) return;
         if (doFileExist(res))
             {
             if (fl_choice("%s", "YES", "NO", nullptr,(std::string("The file already exist, do you want to overwrite it ?\nFile : [") + res + "]").c_str()) == 0)
                 {
                 if (std::remove(res) != 0)
                     {
                     std::string err("Could not delete the file. The image was NOT saved !\n ");
                     err += std::string("File : [") + res + "]";
                     fl_alert("%s",err.c_str());
                     return;
                     }
                 }
             }
         if (res != nullptr)
            {
            try
                {
                _mainImage->save(res);
                }
            catch (...)
                {
                std::string err = "An error occured while saving [";
                err += res;
                err += "]";
                fl_alert("%s",err.c_str());
                return;
                }
            std::string err = "File [";
            err += res;
            err += "] saved.";
            fl_message("%s",err.c_str());
            return;

            }
         return;

        }


        /* callback for the zoom factor slider */
        void Plotter2DWindow::zoomFactorSliderCB_static(Fl_Widget* W, void* p) { MTOOLS_ASSERT(p != nullptr); ((Plotter2DWindow *)p)->zoomFactorSliderCB(W); }
        void Plotter2DWindow::zoomFactorSliderCB(Fl_Widget* W)
            {
            int newzoom = (int)_w_zoomfactorslider->value();
            setZoomFactor(newzoom);
            return;
            }


        /* set the zoom factor */
        void Plotter2DWindow::setZoomFactor(int newzoom)
            {
            auto oldzoom = _PW->zoomFactor();
            if ((newzoom < 1) || (newzoom > 20) || (newzoom == oldzoom)) return;
            auto zoom = _PW->zoomFactor(newzoom);
            if (oldzoom == zoom) return;
            _w_zoomfactorslider->value(zoom);
            //auto viewSize = _PW->viewSizeFactor();
            }


        /* return the current zoom factor */
        int Plotter2DWindow::getZoomFactor() const { return _PW->zoomFactor(); }




        /* called when the "Use 4 channels images check button value changes */
        void Plotter2DWindow::fourchannelsCB_static(Fl_Widget* W, void* p) { MTOOLS_ASSERT(p != nullptr); ((Plotter2DWindow *)p)->fourchannelsCB(W); }
        void Plotter2DWindow::fourchannelsCB(Fl_Widget* W)
        {
            fourChannelImage(((Fl_Check_Button *)W)->value() != 0); // set the number of chanels accordingly
            _PW->take_focus();
        }


        /* called when the "Show mouse position check button value changes */
        void Plotter2DWindow::showmouseCB_static(Fl_Widget* W, void* p) { MTOOLS_ASSERT(p != nullptr); ((Plotter2DWindow *)p)->showmouseCB(W); }
        void Plotter2DWindow::showmouseCB(Fl_Widget* W)
        {
            _PW->crossOn(((Fl_Check_Button *)W)->value() != 0);
            _PW->take_focus();
        }


        /* called when the "Keep a fixed aspect ratio" check button value changes */
        void Plotter2DWindow::fixedratioCB_static(Fl_Widget* W, void* p) { MTOOLS_ASSERT(p != nullptr); ((Plotter2DWindow *)p)->fixedratioCB(W); }
        void Plotter2DWindow::fixedratioCB(Fl_Widget* W)
        {
            ((RangeManager*)_RM)->fixedAspectRatio(((Fl_Check_Button *)W)->value() != 0);
            _PW->take_focus();
        }


        /* callback for the "reset" button */
        void Plotter2DWindow::resetCB_static(Fl_Widget* W, void* p) { MTOOLS_ASSERT(p != nullptr); ((Plotter2DWindow *)p)->resetCB(W); }
        void Plotter2DWindow::resetCB(Fl_Widget* W)
        {
            ((RangeManager*)_RM)->reset();
            _PW->take_focus();
        }

        /* callback for the "1:1" button */
        void Plotter2DWindow::onetooneCB_static(Fl_Widget* W, void* p) { MTOOLS_ASSERT(p != nullptr); ((Plotter2DWindow *)p)->onetooneCB(W); }
        void Plotter2DWindow::onetooneCB(Fl_Widget* W)
        {
            ((RangeManager*)_RM)->setRatio1();
            ((RangeManager*)_RM)->fixedAspectRatio(true);
            _PW->take_focus();
        }


        /* callback for the "unit pixel" button */
        void Plotter2DWindow::unitpixelCB_static(Fl_Widget* W, void* p) { MTOOLS_ASSERT(p != nullptr); ((Plotter2DWindow *)p)->unitpixelCB(W); }
        void Plotter2DWindow::unitpixelCB(Fl_Widget* W)
        {
            ((RangeManager*)_RM)->set1to1();
            ((RangeManager*)_RM)->fixedAspectRatio(true);
            _PW->take_focus();
        }


        /* callback for the "center" button */
        void Plotter2DWindow::centerCB_static(Fl_Widget* W, void* p) { MTOOLS_ASSERT(p != nullptr); ((Plotter2DWindow *)p)->centerCB(W); }
        void Plotter2DWindow::centerCB(Fl_Widget* W)
        {
            ((RangeManager*)_RM)->center(fVec2(0.0, 0.0));
            _PW->take_focus();
        }


        /* callback for the "xmin" input button */
        void Plotter2DWindow::xmin_input_static(Fl_Widget* W, void* p) { MTOOLS_ASSERT(p != nullptr); ((Plotter2DWindow *)p)->xmin_inputCB(W); }
        void Plotter2DWindow::xmin_inputCB(Fl_Widget* W)
        {
            applyrangeCB(W);
        }


        /* callback for the "xmax" input button */
        void Plotter2DWindow::xmax_input_static(Fl_Widget* W, void* p) { MTOOLS_ASSERT(p != nullptr); ((Plotter2DWindow *)p)->xmax_inputCB(W); }
        void Plotter2DWindow::xmax_inputCB(Fl_Widget* W)
        {
            applyrangeCB(W);
        }


        /* callback for the "ymin" input button */
        void Plotter2DWindow::ymin_input_static(Fl_Widget* W, void* p) { MTOOLS_ASSERT(p != nullptr); ((Plotter2DWindow *)p)->ymin_inputCB(W); }
        void Plotter2DWindow::ymin_inputCB(Fl_Widget* W)
        {
            applyrangeCB(W);
        }


        /* callback for the "ymax" input button */
        void Plotter2DWindow::ymax_input_static(Fl_Widget* W, void* p) { MTOOLS_ASSERT(p != nullptr); ((Plotter2DWindow *)p)->ymax_inputCB(W); }
        void Plotter2DWindow::ymax_inputCB(Fl_Widget* W)
        {
            applyrangeCB(W);
        }


        /* callback for the "apply range" button */
        void Plotter2DWindow::applyrangeCB_static(Fl_Widget* W, void* p) { MTOOLS_ASSERT(p != nullptr); ((Plotter2DWindow *)p)->applyrangeCB(W); }
        void Plotter2DWindow::applyrangeCB(Fl_Widget* W)
        {
            fBox2 R = getNewRange();
            if (R.isEmpty())
                {
                setRangeInput(((RangeManager*)_RM)->getRange());
                _PW->take_focus();
                return;
                }
            ((RangeManager*)_RM)->setRange(R);
            R = ((RangeManager*)_RM)->getRange();
            iVec2 winSize = ((RangeManager*)_RM)->getWinSize();
            for (int i = 0; i < (int)_vecPlot.size(); i++)
                {
                _vecPlot[i]->setParam(R, winSize); // set the parameters for all the object in the plotter
                }
            setImageSize((int)winSize.X(), (int)winSize.Y(), _nbchannels);    // resize if needed
            updateView();
            setRangeInput(R);
            _PW->take_focus();
        }


        /* change the value of the four insert buttons to reflect the current range */
        void Plotter2DWindow::setRangeInput(fBox2 R)
        {
            _w_xmin->value(mtools::doubleToStringNice(R.min[0]).c_str());
            _w_xmax->value(mtools::doubleToStringNice(R.max[0]).c_str());
            _w_ymin->value(mtools::doubleToStringNice(R.min[1]).c_str());
            _w_ymax->value(mtools::doubleToStringNice(R.max[1]).c_str());
            _PW->take_focus();
        }


        /* Compute the new range looking a the value inside the xmin, xmax, ymin ymax widgets.*/
        fBox2 Plotter2DWindow::getNewRange()
        {
            double xmin; mtools::fromString<double>(_w_xmin->value(), xmin);
            double xmax; mtools::fromString<double>(_w_xmax->value(), xmax);
            double ymin; mtools::fromString<double>(_w_ymin->value(), ymin);
            double ymax; mtools::fromString<double>(_w_ymax->value(), ymax);
            fBox2 R(xmin, xmax, ymin, ymax);


            if ((!R.isEmpty()) && (((RangeManager*)_RM)->fixedAspectRatio()))
            {
                fBox2 aR = ((RangeManager*)_RM)->getRange();
                R = R.fixedRatioEnclosingRect(aR.lx() / aR.ly());
            }
            return R;
        }



        fBox2 Plotter2DWindow::findRangeX(Plotter2DObj * obj, fBox2 CR, bool keepAR)
            {
            fBox2 R = obj->favouriteRangeX(CR);
            if (R.isHorizontallyEmpty()) return fBox2(); // nothing to do in this case
            if (!keepAR)
                {
                R.min[1] = CR.min[1];
                R.max[1] = CR.max[1];
                }
            else
                {
                double c = (CR.min[1] + CR.max[1]) / 2;
                double r = CR.ly()*R.lx() / (2 * CR.lx());
                R.min[1] = c - r;
                R.max[1] = c + r;
                }
            return R;
            }


        fBox2 Plotter2DWindow::findRangeY(Plotter2DObj * obj, fBox2 CR, bool keepAR)
            {
            fBox2 R = obj->favouriteRangeY(CR);
            if (R.isVerticallyEmpty()) return  fBox2(); // nothing to do in this case
            R.min[0] = CR.min[0];
            R.max[0] = CR.max[0];
            if (!keepAR) { return R; }
            R = R.fixedRatioEnclosingRect(CR.lx() / CR.ly());
            return R;

            }


        fBox2 Plotter2DWindow::getAutoRangeX(fBox2 CR,bool keepAR)
            {
            fBox2 NR;
            for (int i = 0; i < (int)_vecPlot.size(); i++)
                {
                if ((_vecPlot[i]->enable()) && (_vecPlot[i]->hasFavouriteRangeX()))
                    {
                    fBox2 R = _vecPlot[i]->favouriteRangeX(CR);
                    if (!R.isHorizontallyEmpty())
                        {
                        if (NR.isHorizontallyEmpty()) { NR = R; } else
                            {
                            if (R.min[0] < NR.min[0]) { NR.min[0] = R.min[0]; }
                            if (NR.max[0] < R.max[0]) { NR.max[0] = R.max[0]; }
                            }
                        }
                    }
                }
            if (NR.isHorizontallyEmpty()) return fBox2();
            if (!keepAR)
                {
                NR.min[1] = CR.min[1];
                NR.max[1] = CR.max[1];
                return NR;
                }
            double c = (CR.min[1] + CR.max[1]) / 2;
            double r = CR.ly()*NR.lx() / (2 * CR.lx());
            NR.min[1] = c - r;
            NR.max[1] = c + r;
            return NR;
            }


        fBox2 Plotter2DWindow::getAutoRangeY(fBox2 CR,bool keepAR)
            {
            fBox2 NR;
            for (int i = 0; i < (int)_vecPlot.size(); i++)
                {
                if ((_vecPlot[i]->enable()) && (_vecPlot[i]->hasFavouriteRangeY()))
                    {
                    fBox2 R = _vecPlot[i]->favouriteRangeY(CR);
                    if (!R.isVerticallyEmpty())
                        {
                        if (NR.isVerticallyEmpty()) { NR = R; } else
                            {
                            if (R.min[1] < NR.min[1]) { NR.min[1] = R.min[1]; }
                            if (NR.max[1] < R.max[1]) { NR.max[1] = R.max[1]; }
                            }
                        }
                    }
                }
            if (NR.isHorizontallyEmpty()) return fBox2();
            NR.min[0] = CR.min[0];
            NR.max[0] = CR.max[0];
            if (!keepAR) { return NR; }
            NR = NR.fixedRatioEnclosingRect(CR.lx() / CR.ly());
            return NR;
            }


        void Plotter2DWindow::useCommonRangeX()
            {
            fBox2 CR = ((RangeManager*)_RM)->getRange();                // current range
            bool keepAR = ((RangeManager*)_RM)->fixedAspectRatio();     // do we keep the aspect ratio
            fBox2 R = getAutoRangeX(CR, keepAR);
            if (R.isEmpty()) return;
            ((RangeManager*)_RM)->setRange(R);
            }


        void Plotter2DWindow::useCommonRangeY()
            {
            fBox2 CR = ((RangeManager*)_RM)->getRange();                // current range
            bool keepAR = ((RangeManager*)_RM)->fixedAspectRatio();     // do we keep the aspect ratio
            fBox2 R = getAutoRangeY(CR, keepAR);
            if (R.isEmpty()) return;
            ((RangeManager*)_RM)->setRange(R);
            }


        void Plotter2DWindow::useCommonRangeXY()
            {
            fBox2 CR = ((RangeManager*)_RM)->getRange();                // current range
            bool keepAR = ((RangeManager*)_RM)->fixedAspectRatio();     // do we keep the aspect ratio
            fBox2 R = getAutoRangeX(CR, keepAR);
            if (R.isEmpty()) return;
            R = getAutoRangeY(R, keepAR);
            if (R.isEmpty()) return;
            ((RangeManager*)_RM)->setRange(R);
            }


        void Plotter2DWindow::useRangeX(Plotter2DObj * obj)
            {
            fBox2 CR = ((RangeManager*)_RM)->getRange();                 // current range
            bool keepAR = ((RangeManager*)_RM)->fixedAspectRatio();      // do we keep the aspect ratio
            fBox2 R = findRangeX(obj, CR, keepAR);
            if (!R.isEmpty()) ((RangeManager*)_RM)->setRange(R);
            }


        void Plotter2DWindow::useRangeY(Plotter2DObj * obj)
            {
            fBox2 CR = ((RangeManager*)_RM)->getRange();                 // current range
            bool keepAR = ((RangeManager*)_RM)->fixedAspectRatio();      // do we keep the aspect ratio
            fBox2 R = findRangeY(obj, CR, keepAR);
            if (!R.isEmpty()) ((RangeManager*)_RM)->setRange(R);
            }


        void Plotter2DWindow::useRangeXY(Plotter2DObj * obj)
            {
            fBox2 CR = ((RangeManager*)_RM)->getRange();                 // current range
            bool keepAR = ((RangeManager*)_RM)->fixedAspectRatio();      // do we keep the aspect ratio
            fBox2 R = findRangeX(obj, CR, keepAR);
            if (R.isEmpty()) return;
            R = findRangeY(obj, R, keepAR);
            if (R.isEmpty()) return;
            ((RangeManager*)_RM)->setRange(R);
            }


        void Plotter2DWindow::autorangeXCB_static(Fl_Widget* W, void* p) { MTOOLS_ASSERT(p != nullptr); ((Plotter2DWindow *)p)->autorangeXCB(W); }
        void Plotter2DWindow::autorangeXCB(Fl_Widget* W)
            {
            useCommonRangeX();
            _PW->take_focus();
            }


        void Plotter2DWindow::autorangeYCB_static(Fl_Widget* W, void* p) { MTOOLS_ASSERT(p != nullptr); ((Plotter2DWindow *)p)->autorangeYCB(W); }
        void Plotter2DWindow::autorangeYCB(Fl_Widget* W)
            {
            useCommonRangeY();
            _PW->take_focus();
            }


        void Plotter2DWindow::autorangeXYCB_static(Fl_Widget* W, void* p) { MTOOLS_ASSERT(p != nullptr); ((Plotter2DWindow *)p)->autorangeXYCB(W); }
        void Plotter2DWindow::autorangeXYCB(Fl_Widget* W)
            {
            useCommonRangeXY();
            _PW->take_focus();
            }



        /* callback for the "refresh now" button*/
        void Plotter2DWindow::nowrefreshCB_static(Fl_Widget* W, void* p) { MTOOLS_ASSERT(p != nullptr); ((Plotter2DWindow *)p)->nowrefreshCB(W); }
        void Plotter2DWindow::nowrefreshCB(Fl_Widget* W)
            {
            doRedraw();
            }


        /* callback for the "refresh" slider */
        void Plotter2DWindow::refreshscaleCB_static(Fl_Widget* W, void* p) { MTOOLS_ASSERT(p != nullptr); ((Plotter2DWindow *)p)->refreshscaleCB(W); }
        void Plotter2DWindow::refreshscaleCB(Fl_Widget* W)
            {
            setRefreshRate((int)_w_refreshscale->value());
            }


        /* set the new refresh rate */
        void Plotter2DWindow::setRefreshRate(int newrate)
        {
            if (newrate < 0) { newrate = 0; }
            if (newrate > 600) { newrate = 600; }
            Fl::remove_timeout(refresh_timer_static, this);  // remove the timeouts anyway
            Fl::remove_timeout(refresh_timer2_static, this); //
            if (newrate == 0)
            {
                _w_refreshscale->color2(FL_BLACK);
                _w_refreshscale->value(0);
                _w_refreshscale->color(FL_BACKGROUND_COLOR);
                _refreshrate = 0;
                return;
            }
            _refreshrate = newrate;
            _w_refreshscale->color2(FL_RED);
            _w_refreshscale->color(FL_DARK_GREEN);
            _w_refreshscale->value(newrate);
            Fl::add_timeout(0.1, refresh_timer_static, this); // the first redraw start now !
        }


        /* the refresh timer callback */
        void Plotter2DWindow::refresh_timer_static(void* p) { if (p == nullptr) { return; } ((Plotter2DWindow *)p)->refresh_timer(); }
        void Plotter2DWindow::refresh_timer()
        {
            _w_refreshscale->color(FL_DARK_RED);
            _w_refreshscale->redraw();
            Fl::flush();
            Fl::add_timeout(0.1, refresh_timer2_static, this); // one time timeout to stop the blinking
            for (int i = 0; i < (int)_vecPlot.size(); i++)
                {
                if (_vecPlot[i]->quality() > 0 ) _vecPlot[i]->resetDrawing(false); // soft reset : reset only the drawing with non zero quality
                }
            if (_refreshrate > 0) { Fl::repeat_timeout((60.0 / ((double)_refreshrate)), refresh_timer_static, this); } // repeat the timeout
            //_mainImageQuality = 0; // the display image is outdated so we pretend its quality is zero.
            updateView(); // this is better
            }


        /* the second refresh timeout for blinking */
        void Plotter2DWindow::refresh_timer2_static(void* p) { if (p == nullptr) { return; } ((Plotter2DWindow *)p)->refresh_timer2(); }
        void Plotter2DWindow::refresh_timer2()
            {
            _w_refreshscale->color((_refreshrate > 0) ? FL_DARK_GREEN : FL_BACKGROUND_COLOR);
            _w_refreshscale->redraw();
            Fl::flush();
            }


        /* start a redraw */
        void Plotter2DWindow::doRedraw()
            {
            for (int i = 0; i < (int)_vecPlot.size(); i++)
                {
                _vecPlot[i]->resetDrawing(false); // hard reset : reset every drawing even if they have zero quality
                }
            _w_refreshscale->color(FL_DARK_RED);
            _w_refreshscale->redraw();
            Fl::flush();
            Fl::add_timeout(0.1, refresh_timer2_static, this); // one time timeout to stop the blinking
            //_mainImageQuality = 0; // the display image is outdated so we pretend its quality is zero.
            updateView(); // this is better
            }



        /* convert to real coordinates */
        void Plotter2DWindow::convertWindowCoord(int & W, int & H, int & X, int & Y)
        {
            if (W < Plotter2D::MIN_W) W = Plotter2D::MIN_W;
            if (H < Plotter2D::MIN_H) H = Plotter2D::MIN_H;
            int sx, sy, sw, sh;
            Fl::screen_xywh(sx, sy, sw, sh);
            if (X == Plotter2D::POS_RIGHT) { X = sw - W; }
            else if (X == Plotter2D::POS_CENTER) { X = (sw - W) / 2; }
            if (Y == Plotter2D::POS_BOTTOM) { Y = sh - H; }
            else if (Y == Plotter2D::POS_CENTER) { Y = (sh - H) / 2; }
            if (sx != 0) { X += sx; }
            if (sy != 0) { Y += sy; }
        }


        void Plotter2DWindow::resizeWindow(int W, int H, int X, int Y)
        {
            convertWindowCoord(W, H, X, Y);
            _w_mainWin->resize(X, Y, W, H);
        }


        void Plotter2DWindow::setWindowPos(int X, int Y)
        {
            int W = _w_mainWin->w();
            int H = _w_mainWin->h();
            convertWindowCoord(W, H, X, Y);
            _w_mainWin->resize(X, Y, W, H);
        }


        void Plotter2DWindow::setDrawingSize(int W, int H)
        {
            setWindowSize(W + 320, H + 20); // resize the whole window
            _PW->resize(310, 10, W, H);     // and make sure the view is exactly as we want it to be.
        }


        void Plotter2DWindow::setWindowSize(int W, int H)
        {
            int X = _w_mainWin->x();
            int Y = _w_mainWin->y();
            convertWindowCoord(W, H, X, Y);
            _w_mainWin->resize(X, Y, W, H);
        }


        /* set image with 3 or 4 channels */
        void Plotter2DWindow::fourChannelImage(bool use4)
        {
            int n = (use4 ? 4 : 3);
            if (((int)_nbchannels) == n) { return; }
            _w_fourchannels->value((n == 4) ? 1 : 0);
            _nbchannels = n;
            int W = _mainImage->width();
            int H = _mainImage->height();
            setImageSize(W, H, n);    // resize if needed
            updateView();
        }


        /* set the text "keep aspect ratio : xxx */
        void Plotter2DWindow::setRatioTextLabel()
        {
            if (((RangeManager*)_RM) == nullptr) {_w_fixedratio->copy_label("Maintain the aspect ratio");}
            double ratio = ((RangeManager*)_RM)->ratio();
            auto ratiotxt = std::string("Maintain the aspect ratio : ") + doubleToStringNice(ratio);
            _w_fixedratio->copy_label(ratiotxt.c_str());
        }


    }








    /***************************************************************************************************************************
    * IMPLEMENTATION OF THE Plotter2D class
    ****************************/


    Plotter2D::Plotter2D(internals_graphics::Plotter2DObj & obj, bool addAxes, bool addGrid, int W, int H, int X, int Y)
        {
        _plotterWin = mtools::newInFltkThread<internals_graphics::Plotter2DWindow, bool, bool, int, int, int, int>(std::move(addAxes), std::move(addGrid), std::move(X), std::move(Y), std::move(W), std::move(H)); // create the plotter window if FLTK
        MTOOLS_INSURE(_plotterWin != nullptr);
        add(obj);
        }


    Plotter2D::Plotter2D(bool addAxes, bool addGrid, int W, int H, int X, int Y)
        {
            _plotterWin = mtools::newInFltkThread<internals_graphics::Plotter2DWindow, bool, bool, int, int, int, int>(std::move(addAxes), std::move(addGrid), std::move(X), std::move(Y), std::move(W), std::move(H)); // create the plotter window if FLTK
        MTOOLS_INSURE(_plotterWin != nullptr);
        }


    Plotter2D::~Plotter2D()
        {
        endPlot(); // end the plot just in case.
        mtools::deleteInFltkThread<internals_graphics::Plotter2DWindow>(_plotterWin); // delete the plotter window
        }


    Plot2DAxes * Plotter2D::axesObject() const { return(_plotterWin->_axePlot); }


    Plot2DAxes * Plotter2D::axesObject(bool status)
        {
        mtools::IndirectMemberProc<internals_graphics::Plotter2DWindow, bool> proxy(*_plotterWin, &internals_graphics::Plotter2DWindow::_insertAxesObject, status);
        mtools::runInFltkThread(proxy);
        return axesObject();
        }


    Plot2DGrid * Plotter2D::gridObject() const { return(_plotterWin->_gridPlot); }


    Plot2DGrid * Plotter2D::gridObject(bool status)
        {
        mtools::IndirectMemberProc<internals_graphics::Plotter2DWindow, bool> proxy(*_plotterWin, &internals_graphics::Plotter2DWindow::_insertGridObject, status);
        mtools::runInFltkThread(proxy);
        return gridObject();
        }


    void Plotter2D::add(internals_graphics::Plotter2DObj * obj)
        {
        mtools::IndirectMemberProc<internals_graphics::Plotter2DWindow, internals_graphics::Plotter2DObj *> proxy(*_plotterWin, &internals_graphics::Plotter2DWindow::add, std::move(obj)); // call _plotterWin.insert(obj) in FLTK
        mtools::runInFltkThread(proxy);                                                                                                                                                        //
        }


    void Plotter2D::add(internals_graphics::Plotter2DObj & obj) { add(&obj); }


    Plotter2D & Plotter2D::operator[](internals_graphics::Plotter2DObj & obj) { add(obj); return(*this); }


    int Plotter2D::nbObject() const { return((int)_plotterWin->_vecPlot.size());}


    internals_graphics::Plotter2DObj * Plotter2D::get(int pos) const
        {
        if ((pos < 0) || (pos >= nbObject())) return nullptr;
        return(_plotterWin->_vecPlot[pos]);
        }


    void Plotter2D::remove(internals_graphics::Plotter2DObj * obj)
        {
        mtools::IndirectMemberProc<internals_graphics::Plotter2DWindow, internals_graphics::Plotter2DObj *> proxy(*_plotterWin, &internals_graphics::Plotter2DWindow::remove, std::move(obj)); // call _plotterWin.insert(obj) in FLTK
        mtools::runInFltkThread(proxy);                                                                                                                                                        //
        }


    void Plotter2D::remove(internals_graphics::Plotter2DObj & obj) { remove(&obj); }


    bool Plotter2D::fourChannelImage() const
        {
        return(((int)_plotterWin->_nbchannels) == 4);
        }


    void Plotter2D::fourChannelImage(bool use4)
        {
        mtools::IndirectMemberProc<internals_graphics::Plotter2DWindow, bool> proxy(*_plotterWin, &internals_graphics::Plotter2DWindow::fourChannelImage, use4);
        mtools::runInFltkThread(proxy);
        }


    void Plotter2D::useSolidBackground(bool use)
        {
        _plotterWin->_usesolidBK = use;
        mtools::IndirectMemberProc<internals_graphics::Plotter2DWindow> proxy(*_plotterWin, &internals_graphics::Plotter2DWindow::updatesolidback);
        mtools::runInFltkThread(proxy);
        }


    bool Plotter2D::useSolidBackground() const
        {
        return((bool)_plotterWin->_usesolidBK);
        }


    void Plotter2D::solidBackGroundColor(mtools::RGBc color)
        {
        _plotterWin->_solidBKcolor = color.getOpaque();
        mtools::IndirectMemberProc<internals_graphics::Plotter2DWindow> proxy(*_plotterWin, &internals_graphics::Plotter2DWindow::updatesolidback);
        mtools::runInFltkThread(proxy);
        }


    mtools::RGBc Plotter2D::solidBackGroundColor() const
        {
        return((mtools::RGBc)_plotterWin->_solidBKcolor);
        }


    unsigned int Plotter2D::autoredraw() const { return _plotterWin->_refreshrate; }


    void Plotter2D::autoredraw(unsigned int rate)
        {
        mtools::IndirectMemberProc<internals_graphics::Plotter2DWindow,int> proxy(*_plotterWin, &internals_graphics::Plotter2DWindow::setRefreshRate,rate);
        mtools::runInFltkThread(proxy);
        }


    void Plotter2D::redraw()
        {
        mtools::IndirectMemberProc<internals_graphics::Plotter2DWindow> proxy(*_plotterWin, &internals_graphics::Plotter2DWindow::doRedraw);
        mtools::runInFltkThread(proxy);
        }


    internals_graphics::RangeManager & Plotter2D::range() { return *((internals_graphics::RangeManager*)(_plotterWin->_RM)); }


    unsigned int Plotter2D::sensibility() const { return _plotterWin->_sensibility; }


    void Plotter2D::sensibility(unsigned int delta)
        {
        _plotterWin->_sensibility = (delta < 1) ? 1 : ((delta > 99) ? 99 : delta);
        }



    void Plotter2D::plot()
        {
        if (_plotterWin->_shown == false) { startPlot(); } // if not shown, show the plotter
        while (_plotterWin->_shown == true) { std::this_thread::sleep_for(std::chrono::milliseconds(50)); } // wait until the window is closed
        }


    void Plotter2D::startPlot()
        {
        mtools::IndirectMemberProc<internals_graphics::Plotter2DWindow> proxy(*_plotterWin, &internals_graphics::Plotter2DWindow::startPlot); // call _plotterWin.startPlot() in FLTK
        mtools::runInFltkThread(proxy);                                                                                                       //
        }


    void Plotter2D::endPlot()
        {
        mtools::IndirectMemberProc<internals_graphics::Plotter2DWindow> proxy(*_plotterWin, &internals_graphics::Plotter2DWindow::endPlot); // call _plotterWin.endPlot() in FLTK
        mtools::runInFltkThread(proxy);                                                                                                       //
        }

    bool Plotter2D::shown() const
        {
        return _plotterWin->_shown;
        }


    void Plotter2D::setWindowPos(int X, int Y)
        {
        mtools::IndirectMemberProc<internals_graphics::Plotter2DWindow,int,int> proxy(*_plotterWin, &internals_graphics::Plotter2DWindow::setWindowPos,X,Y);
        mtools::runInFltkThread(proxy);
        }


    void Plotter2D::setDrawingSize(int W, int H)
        {
        mtools::IndirectMemberProc<internals_graphics::Plotter2DWindow, int, int> proxy(*_plotterWin, &internals_graphics::Plotter2DWindow::setDrawingSize, W, H);
        mtools::runInFltkThread(proxy);
        }


    void Plotter2D::setWindowSize(int W, int H)
        {
        mtools::IndirectMemberProc<internals_graphics::Plotter2DWindow, int, int> proxy(*_plotterWin, &internals_graphics::Plotter2DWindow::setWindowSize, W, H);
        mtools::runInFltkThread(proxy);
        }


    void Plotter2D::autorangeX()
        {
        mtools::IndirectMemberProc<internals_graphics::Plotter2DWindow> proxy(*_plotterWin, &internals_graphics::Plotter2DWindow::useCommonRangeX);
        mtools::runInFltkThread(proxy);
        }


    void Plotter2D::autorangeX(bool keepAspectRatio)
        {
        range().fixedAspectRatio(keepAspectRatio);
        autorangeX();
        }


    void Plotter2D::autorangeY()
        {
        mtools::IndirectMemberProc<internals_graphics::Plotter2DWindow> proxy(*_plotterWin, &internals_graphics::Plotter2DWindow::useCommonRangeY);
        mtools::runInFltkThread(proxy);
        }


    void Plotter2D::autorangeY(bool keepAspectRatio)
        {
        range().fixedAspectRatio(keepAspectRatio);
        autorangeY();
        }


    void Plotter2D::autorangeXY()
        {
        mtools::IndirectMemberProc<internals_graphics::Plotter2DWindow> proxy(*_plotterWin, &internals_graphics::Plotter2DWindow::useCommonRangeXY);
        mtools::runInFltkThread(proxy);
        }


    void Plotter2D::autorangeXY(bool keepAspectRatio)
        {
        range().fixedAspectRatio(keepAspectRatio);
        autorangeXY();
        }


    int Plotter2D::viewZoomFactor() const { return(_plotterWin->getZoomFactor()); }


    int Plotter2D::viewZoomFactor(int zoomFactor)
        {
        mtools::IndirectMemberProc<internals_graphics::Plotter2DWindow, int> proxy(*_plotterWin, &internals_graphics::Plotter2DWindow::setZoomFactor, zoomFactor);
        mtools::runInFltkThread(proxy);
        return viewZoomFactor();
        }


}


/* end of file */



