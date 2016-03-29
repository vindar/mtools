/** @file planedrawer.hpp */
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





#include "../misc/threadworker.hpp"
#include "drawable2DInterface.hpp"
#include "customcimg.hpp"
#include "rgbc.hpp"
#include "../maths/vec.hpp"
#include "../maths/box.hpp"
#include "../misc/misc.hpp"
#include "../misc/metaprog.hpp"
#include "../randomgen/fastRNG.hpp"


#include <algorithm>
#include <ctime>
#include <mutex>
#include <atomic>


namespace mtools
{



/**
* GetColor Plane method selector.
*
* Detect if a type (or function) contain a method compatible with getColor() used by the PlaneDrawer class.
* The method can be called with call(). (if no method found, return RGBc::c_TransparentWhite).
*
* - fVec2 pos or (int64 x,int64 y) : the coordinate of the point to draw  
* 
* - int32 iter : number of time this pixel was queried. Increased by one after each to the method (until the drawing is resetted, moved...).   
* - 
* - fBox2 box : a box describing the aera covered by the pixel.
*
* - void* & data : reference to an opaque value that identify the thread drawing. Set to nullptr intially, the reference may
*                  be modified by the function and the same value will be returned at the next call
*                  from the same thread.
*
* - return value : - RGBc : the color at the position. The color is blended with the values previously obtained. If you want to overwrite  
* -                         the previous value, use the second return type below.
* - 
*                  - std::pair<RGBc, bool > : The color at the position and a flag to indicate if the color should be reset:  
*                                             false : do not reset and use simple blending like in the RGBc return type.  
*                                             true  : the return color should overwrite the current color (but this does not 
*                                                     reset the iter counter which is still increased by 1 after that call)
*                                             
* The signature below are recognized with the following order:
*
*  std::pair<RGBc,bool> getImage([const] fVec2 [&] pos, [const] fBox2 [&] box, int32 nbiter, void* & data)
*  std::pair<RGBc,bool> getImage([const] fVec2 [&] pos, [const] fBox2 [&] box, int32 nbiter)
*  std::pair<RGBc,bool> getImage([const] fVec2 [&] pos, [const] fBox2 [&] box)
*  std::pair<RGBc,bool> getImage([const] fVec2 [&] pos)
*  std::pair<RGBc,bool> operator()([const] fVec2 [&] pos, [const] fBox2 [&] box, int32 nbiter, void* & data)
*  std::pair<RGBc,bool> operator()([const] fVec2 [&] pos, [const] fBox2 [&] box, int32 nbiter)
*  std::pair<RGBc,bool> operator()([const] fVec2 [&] pos, [const] fBox2 [&] box)
*  std::pair<RGBc,bool> operator()([const] fVec2 [&] pos)
*  std::pair<RGBc,bool> getImage([const] double [&] x, [const] double [&] y)
*  std::pair<RGBc,bool> operator()([const] double [&] x, [const] double [&] y)
*
*  RGBc getImage([const] fVec2 [&] pos, [const] fBox2 [&] box, int32 nbiter, void* & data)
*  RGBc getImage([const] fVec2 [&] pos, [const] fBox2 [&] box, int32 nbiter)
*  RGBc getImage([const] fVec2 [&] pos, [const] fBox2 [&] box)
*  RGBc getImage([const] fVec2 [&] pos)
*  RGBc operator()([const] fVec2 [&] pos, [const] fBox2 [&] box, int32 nbiter, void* & data)
*  RGBc operator()([const] fVec2 [&] pos, [const] fBox2 [&] box, int32 nbiter)
*  RGBc operator()([const] fVec2 [&] pos, [const] fBox2 [&] box)
*  RGBc operator()([const] fVec2 [&] pos)  
*  RGBc getImage([const] double [&] x, [const] double [&] y)
*  RGBc operator()([const] double [&] x, [const] double [&] y)
*  
**/
template<typename T> class GetColorPlaneSelector
    {
    static void * dumptr;
    typedef typename std::decay<mtools::RGBc>::type decayrgb;
    typedef typename std::decay<std::pair<mtools::RGBc, bool> >::type decaypair;

    template<typename U> static decltype((*(U*)(0)).getColor(fVec2(), fBox2(), 0, dumptr)) vers1(int);
    template<typename> static metaprog::no vers1(...);
    static const bool version1 = std::is_same<typename std::decay<decltype(vers1<T>(0))>::type, decaypair>::value;
    static const bool version11 = std::is_same<typename std::decay<decltype(vers1<T>(0))>::type, decayrgb >::value;

    template<typename U> static decltype((*(U*)(0)).getColor(fVec2(), fBox2(), 0)) vers2(int);
    template<typename> static metaprog::no vers2(...);
    static const bool version2 = std::is_same<typename std::decay<decltype(vers2<T>(0))>::type, decaypair >::value;
    static const bool version12 = std::is_same<typename std::decay<decltype(vers2<T>(0))>::type, decayrgb >::value;

    template<typename U> static decltype((*(U*)(0)).getColor(fVec2(), fBox2())) vers3(int);
    template<typename> static metaprog::no vers3(...);
    static const bool version3 = std::is_same<typename std::decay<decltype(vers3<T>(0))>::type, decaypair >::value;
    static const bool version13 = std::is_same<typename std::decay<decltype(vers3<T>(0))>::type, decayrgb >::value;

    template<typename U> static decltype((*(U*)(0)).getColor(fVec2())) vers4(int);
    template<typename> static metaprog::no vers4(...);
    static const bool version4 = std::is_same<typename std::decay<decltype(vers4<T>(0))>::type, decaypair >::value;
    static const bool version14 = std::is_same<typename std::decay<decltype(vers4<T>(0))>::type, decayrgb >::value;

    template<typename U> static decltype((*(U*)(0))(fVec2(), fBox2(), 0, dumptr)) vers5(int);
    template<typename> static metaprog::no vers5(...);
    static const bool version5 = std::is_same<typename std::decay<decltype(vers5<T>(0))>::type, decaypair >::value;
    static const bool version15 = std::is_same<typename std::decay<decltype(vers5<T>(0))>::type, decayrgb >::value;

    template<typename U> static decltype((*(U*)(0))(fVec2(), fBox2(), 0)) vers6(int);
    template<typename> static metaprog::no vers6(...);
    static const bool version6 = std::is_same<typename std::decay<decltype(vers6<T>(0))>::type, decaypair >::value;
    static const bool version16 = std::is_same<typename std::decay<decltype(vers6<T>(0))>::type, decayrgb >::value;

    template<typename U> static decltype((*(U*)(0))(fVec2(), fBox2())) vers7(int);
    template<typename> static metaprog::no vers7(...);
    static const bool version7 = std::is_same<typename std::decay<decltype(vers7<T>(0))>::type, decaypair >::value;
    static const bool version17 = std::is_same<typename std::decay<decltype(vers7<T>(0))>::type, decayrgb >::value;

    template<typename U> static decltype((*(U*)(0))(fVec2())) vers8(int);
    template<typename> static metaprog::no vers8(...);
    static const bool version8 = std::is_same<typename std::decay<decltype(vers8<T>(0))>::type, decaypair >::value;
    static const bool version18 = std::is_same<typename std::decay<decltype(vers8<T>(0))>::type, decayrgb >::value;

    template<typename U> static decltype((*(U*)(0)).getColor(0.0,0.0)) vers9(int);
    template<typename> static metaprog::no vers9(...);
    static const bool version9 = std::is_same<typename std::decay<decltype(vers9<T>(0))>::type, decaypair >::value;
    static const bool version19 = std::is_same<typename std::decay<decltype(vers9<T>(0))>::type, decayrgb >::value;

    template<typename U> static decltype((*(U*)(0))(0.0,0.0)) vers10(int);
    template<typename> static metaprog::no vers10(...);
    static const bool version10 = std::is_same<typename std::decay<decltype(vers10<T>(0))>::type, decaypair >::value;
    static const bool version20 = std::is_same<typename std::decay<decltype(vers10<T>(0))>::type, decayrgb >::value;


    static std::pair<mtools::RGBc, bool> call1(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<true> D) { return obj.getColor(pos, box, nbiter, data); }
    static std::pair<mtools::RGBc, bool> call2(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<true> D) { return obj.getColor(pos, box, nbiter); }
    static std::pair<mtools::RGBc, bool> call3(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<true> D) { return obj.getColor(pos, box); }
    static std::pair<mtools::RGBc, bool> call4(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<true> D) { return obj.getColor(pos); }
    static std::pair<mtools::RGBc, bool> call5(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<true> D) { return obj(pos, box, nbiter, data); }
    static std::pair<mtools::RGBc, bool> call6(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<true> D) { return obj(pos, box, nbiter); }
    static std::pair<mtools::RGBc, bool> call7(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<true> D) { return obj(pos, box); }
    static std::pair<mtools::RGBc, bool> call8(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<true> D) { return obj(pos); }
    static std::pair<mtools::RGBc, bool> call9(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<true> D) { return obj.getColor(pos.X(),pos.Y()); }
    static std::pair<mtools::RGBc, bool> call10(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<true> D) { return obj(pos.X(), pos.Y()); }
    static std::pair<mtools::RGBc, bool> call11(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<true> D) { return std::pair<mtools::RGBc, bool>(obj.getColor(pos, box, nbiter, data), false); }
    static std::pair<mtools::RGBc, bool> call12(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<true> D) { return std::pair<mtools::RGBc, bool>(obj.getColor(pos, box, nbiter), false); }
    static std::pair<mtools::RGBc, bool> call13(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<true> D) { return std::pair<mtools::RGBc, bool>(obj.getColor(pos, box), false); }
    static std::pair<mtools::RGBc, bool> call14(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<true> D) { return std::pair<mtools::RGBc, bool>(obj.getColor(pos), false); }
    static std::pair<mtools::RGBc, bool> call15(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<true> D) { return std::pair<mtools::RGBc, bool>(obj(pos, box, nbiter, data), false); }
    static std::pair<mtools::RGBc, bool> call16(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<true> D) { return std::pair<mtools::RGBc, bool>(obj(pos, box, nbiter), false); }
    static std::pair<mtools::RGBc, bool> call17(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<true> D) { return std::pair<mtools::RGBc, bool>(obj(pos, box), false); }
    static std::pair<mtools::RGBc, bool> call18(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<true> D) { return std::pair<mtools::RGBc, bool>(obj(pos), false); }
    static std::pair<mtools::RGBc, bool> call19(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<true> D) { return std::pair<mtools::RGBc, bool>(obj.getColor(pos.X(), pos.Y()), false); }
    static std::pair<mtools::RGBc, bool> call20(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<true> D) { return std::pair<mtools::RGBc, bool>(obj(pos.X(), pos.Y()), false); }

    static std::pair<mtools::RGBc, bool> call1(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<false> D) { return call2(obj, pos, box, nbiter, data, mtools::metaprog::dummy<version2>()); }
    static std::pair<mtools::RGBc, bool> call2(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<false> D) { return call3(obj, pos, box, nbiter, data, mtools::metaprog::dummy<version3>()); }
    static std::pair<mtools::RGBc, bool> call3(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<false> D) { return call4(obj, pos, box, nbiter, data, mtools::metaprog::dummy<version4>()); }
    static std::pair<mtools::RGBc, bool> call4(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<false> D) { return call5(obj, pos, box, nbiter, data, mtools::metaprog::dummy<version5>()); }
    static std::pair<mtools::RGBc, bool> call5(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<false> D) { return call6(obj, pos, box, nbiter, data, mtools::metaprog::dummy<version6>()); }
    static std::pair<mtools::RGBc, bool> call6(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<false> D) { return call7(obj, pos, box, nbiter, data, mtools::metaprog::dummy<version7>()); }
    static std::pair<mtools::RGBc, bool> call7(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<false> D) { return call8(obj, pos, box, nbiter, data, mtools::metaprog::dummy<version8>()); }
    static std::pair<mtools::RGBc, bool> call8(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<false> D) { return call9(obj, pos, box, nbiter, data, mtools::metaprog::dummy<version9>()); }
    static std::pair<mtools::RGBc, bool> call9(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<false> D) { return call10(obj, pos, box, nbiter, data, mtools::metaprog::dummy<version10>()); }
    static std::pair<mtools::RGBc, bool> call10(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<false> D) { return call11(obj, pos, box, nbiter, data, mtools::metaprog::dummy<version11>()); }
    static std::pair<mtools::RGBc, bool> call11(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<false> D) { return call12(obj, pos, box, nbiter, data, mtools::metaprog::dummy<version12>()); }
    static std::pair<mtools::RGBc, bool> call12(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<false> D) { return call13(obj, pos, box, nbiter, data, mtools::metaprog::dummy<version13>()); }
    static std::pair<mtools::RGBc, bool> call13(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<false> D) { return call14(obj, pos, box, nbiter, data, mtools::metaprog::dummy<version14>()); }
    static std::pair<mtools::RGBc, bool> call14(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<false> D) { return call15(obj, pos, box, nbiter, data, mtools::metaprog::dummy<version15>()); }
    static std::pair<mtools::RGBc, bool> call15(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<false> D) { return call16(obj, pos, box, nbiter, data, mtools::metaprog::dummy<version16>()); }
    static std::pair<mtools::RGBc, bool> call16(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<false> D) { return call17(obj, pos, box, nbiter, data, mtools::metaprog::dummy<version17>()); }
    static std::pair<mtools::RGBc, bool> call17(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<false> D) { return call18(obj, pos, box, nbiter, data, mtools::metaprog::dummy<version18>()); }
    static std::pair<mtools::RGBc, bool> call18(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<false> D) { return call19(obj, pos, box, nbiter, data, mtools::metaprog::dummy<version19>()); }
    static std::pair<mtools::RGBc, bool> call19(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<false> D) { return call20(obj, pos, box, nbiter, data, mtools::metaprog::dummy<version20>()); }
    static std::pair<mtools::RGBc, bool> call20(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<false> D) { MTOOLS_DEBUG("GetColorPlaneSelector: no getColor() found."); return std::pair<mtools::RGBc, bool>(RGBc::c_TransparentWhite,false); }

    public:

        static const bool has_getColor = version1 | version2 | version3 | version4 | version5 | version6 | version7 | version8 | version9 | version10 |
                                         version11 | version12 | version13 | version14 | version15 | version16 | version17 | version18 | version19 | version20;

        static std::pair<mtools::RGBc,bool> call(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data) { return call1(obj, pos, box, nbiter, data, mtools::metaprog::dummy<version1>()); }

    };





    /**
    * ThreadPlaneDrawer class
    *
    * Use a single thread to draw from a getColor function into a progressImg
    **/
    template<typename ObjType> class ThreadPlaneDrawer : public ThreadWorker
        {


        public:

            /**
            * Constructor. Associate the object. The thread is initially suspended.
            *
            * @param [in,out]  obj     pointer to the object to be drawn. Must implement a method recognized
            *                          by GetColorPlaneSelector
            * @param [in,out]  opaque  (Optional) The opaque data to passed to getColor(), nullptr if not
            *                          specified.
            **/
            ThreadPlaneDrawer(ObjType * obj, void * opaque = nullptr) : ThreadWorker(),
                _obj(obj),
                _opaque(opaque),
                _validParam(false),
                _range(fBox2()),
                _temp_range(fBox2()),
                _im(nullptr),
                _temp_im(nullptr),
                _subBox(iBox2()),
                _temp_subBox(iBox2())
                {
                static_assert(mtools::GetColorPlaneSelector<ObjType>::has_getColor, "The object must be implement one of the getColor() method recognized by GetColorPlaneSelector.");
                }


            /**
            * Destructor. Stop the thread.
            **/
            virtual ~ThreadPlaneDrawer()
                {
                }


            /**
            * Determines if the drawing parameter are valid. Return false if for example, the image is set
            * to nullptr, or the subbox is incorrect, or if the range is too small, or too large, or empty.
            *
            * If it return false, nothing will be drawn and the quality will stay 0.
            *
            * @return  true if the paramter are valid and flase otherwise.
            **/
            inline bool validParam()  const { return (bool)_validParam; }


            /**
            * Sets the drawing parameters.
            *
            * Returns immediately, use sync() to wait for the operation to complete.
            *
            * @param   range       The range to draw.
            * @param [in,out]  im  The image to draw into.
            * @param   subBox      The part of the image to draw (border inclusive). If empty, use the whole
            *                      image.
            **/
            void setParameters(const fBox2 & range, ProgressImg * im, const iBox2 & subBox = iBox2())
                {
                sync();
                _temp_range = range;
                _temp_im = im;
                _temp_subBox = subBox;
                signal(SIGNAL_NEWPARAM);
                }


            /**
            * Force a redraw.
            *
            * Returns immediately, use sync() to wait for the operation to complete.
            *
            * @param   keepPrevious    If true, keep the previous drawing so that quality starts from 1 and
            *                          not 0 if possible.
            **/
            void redraw(bool keepPrevious = true)
                {
                sync();
                signal(SIGNAL_REDRAW);
                }


        private:


            /**
            * Handles the thread messages
            **/
            virtual int message(int64 code) override
                {
                switch (code)
                    {
                    case SIGNAL_NEWPARAM: { return _setNewParam(); }
                    case SIGNAL_REDRAW: { return _setRedraw(); }
                    default: { MTOOLS_ERROR("wtf!"); return 0; }
                    }
                }


            /* set the new parameter */
            int _setNewParam()
                {
                const int MIN_IMAGE_SIZE = 2;
                const double RANGE_MIN_VALUE = std::numeric_limits<double>::min() * 100000;
                const double RANGE_MAX_VALUE = std::numeric_limits<double>::max() / 100000;
                _range = _temp_range;
                _im = _temp_im;
                _subBox = _temp_subBox;
                if ((_im == nullptr) || (_im->width() < MIN_IMAGE_SIZE) || (_im->height() < MIN_IMAGE_SIZE)) { setProgress(0); _validParam = false; return THREAD_RESET_AND_WAIT; }   // make sure im is not nullptr and is big enough.
                if (_subBox.isEmpty()) { _subBox = iBox2(0, _im->width() - 1, 0, _im->height() - 1); } // subbox = whole image if empty. 
                if ((_subBox.min[0] < 0) || (_subBox.max[0] >= (int64)_im->width()) || (_subBox.min[1] < 0) || (_subBox.max[1] >= (int64)_im->height())) { setProgress(0); _validParam = false; return THREAD_RESET_AND_WAIT; } // make sure _subBox is a proper subbox of im
                if ((_subBox.lx() < MIN_IMAGE_SIZE) || (_subBox.ly() < MIN_IMAGE_SIZE)) { setProgress(0); _validParam = false; return THREAD_RESET_AND_WAIT; } // make sure _subBox is a proper subbox of im
                const double rlx = _range.lx();
                const double rly = _range.ly();
                if ((rlx < RANGE_MIN_VALUE) || (rly < RANGE_MIN_VALUE)) { setProgress(0); _validParam = false; return THREAD_RESET_AND_WAIT; } // prevent zooming in too far
                if ((std::abs(_range.min[0]) > RANGE_MAX_VALUE) || (std::abs(_range.max[0]) > RANGE_MAX_VALUE) || (std::abs(_range.min[1]) > RANGE_MAX_VALUE) || (std::abs(_range.max[1]) > RANGE_MAX_VALUE)) { setProgress(0); _validParam = false; return THREAD_RESET_AND_WAIT; } // prevent zooming out too far
                setProgress(0);
                _validParam = true;
                return THREAD_RESET;
                }



            /* trigger a redraw */
            int _setRedraw()
                {
                if (!((bool)_validParam)) return THREAD_RESET_AND_WAIT;
                setProgress(0);
                return THREAD_RESET;
                }



            /**
            * The main 'work' method
            **/
            virtual void work() override
                {
                MTOOLS_INSURE((bool)_validParam);
                _drawFast();
                setProgress(1);
                for (int i = 0; i < 254;i++)
                    {
                    _drawStochastic();
                    setProgress((i * 100) / 255);
                    }
                setProgress(100);
                }



            /* draw by sampling the color at the center of each pixel */
            void _drawFast()
                {
                RGBc64 * imData = _im->imData();
                uint8 * normData = _im->normData();
                const fBox2 r = _range;
                const int64 ilx = _subBox.lx() + 1;
                const int64 ily = _subBox.ly() + 1;
                const double px = r.lx() / ilx;
                const double py = r.ly() / ily;
                const double px2 = px / 2;
                const double py2 = py / 2;
                size_t off = (size_t)(_subBox.min[0] + _im->width()*(_subBox.min[1]));
                const size_t pa = (size_t)(_im->width() - ilx);
                fBox2 cbox(r.min[0], r.min[0] + px, r.min[1], r.min[1] + py);
                for (int64 j = 0; j < ily; j++)
                    {
                    check();
                    for (int64 i = 0; i < ilx; i++)
                        {
                        imData[off] = (mtools::GetColorPlaneSelector<ObjType>::call(*_obj, fVec2{ cbox.min[0] + px2 , cbox.min[1] + py2 }, cbox, 1, _opaque)).first;
                        normData[off] = 0;
                        off++;
                        cbox.min[0] += px;
                        cbox.max[0] += px;
                        }
                    off += pa;
                    cbox.min[1] += py;
                    cbox.max[1] += py;
                    cbox.min[0] = r.min[0];
                    cbox.max[0] = r.min[0] + px;
                    }
                }



            void _drawStochastic()
                {
                RGBc64 * imData = _im->imData();
                uint8 * normData = _im->normData();
                const fBox2 r = _range;
                const int64 ilx = _subBox.lx() + 1;
                const int64 ily = _subBox.ly() + 1;
                const double px = r.lx() / ilx;
                const double py = r.ly() / ily;
                size_t off = (size_t)(_subBox.min[0] + _im->width()*(_subBox.min[1]));
                const size_t pa = (size_t)(_im->width() - ilx);
                fBox2 cbox(r.min[0], r.min[0] + px, r.min[1], r.min[1] + py);
                for (int64 j = 0; j < ily; j++)
                    {
                    check();
                    for (int64 i = 0; i < ilx; i++)
                        {
                        std::pair<RGBc, bool> P = mtools::GetColorPlaneSelector<ObjType>::call(*_obj, fVec2{ cbox.min[0] + _fastgen.unif()*px , cbox.min[1] + _fastgen.unif()*py }, cbox, 1, _opaque);
                        if (P.second)
                            {
                            imData[off] = P.first;
                            normData[off] = 0;
                            }
                        else
                            {
                            imData[off].add(P.first);
                            normData[off]++;
                            }
                        off++;
                        cbox.min[0] += px;
                        cbox.max[0] += px;
                        }
                    off += pa;
                    cbox.min[1] += py;
                    cbox.max[1] += py;
                    cbox.min[0] = r.min[0];
                    cbox.max[0] = r.min[0] + px;
                    }
                }


            // no copy
            ThreadPlaneDrawer(const ThreadPlaneDrawer &) = delete;
            ThreadPlaneDrawer & operator=(const ThreadPlaneDrawer &) = delete;


            static const int SIGNAL_NEWPARAM = 4;
            static const int SIGNAL_REDRAW = 5;

            ObjType * _obj;                         // the object to draw.
            void * _opaque;                         // opaque data passed to _obj;

            std::atomic<bool> _validParam;          // indicate if the parameter are valid.

            fBox2 _range;                           // the range
            std::atomic<fBox2> _temp_range;         // and the temporary use to communicate with the thread.
            ProgressImg* _im;                       // the image to draw onto
            std::atomic<ProgressImg*> _temp_im;     // and the temporary use to communicate with the thread.
            iBox2 _subBox;                          // part of the image to draw
            std::atomic<iBox2> _temp_subBox;        // and the temporary use to communicate with the thread.

            FastRNG _fastgen;                       // fast RNG

        };



    /**
    * PlaneDrawer class
    *
    * Use several threads to draw from a getColor function into a progressImg.
    *
    * @tparam  ObjType Type of the object type.
    **/

    template<typename ObjType> class PlaneDrawer
        {

        public:

            /**
            * Constructor.
            *
            * @param [in,out]  obj The object to draw
            **/
            PlaneDrawer(ObjType * obj, int nbthread = 1) :  _obj(obj), _vecThread()
                {
                static_assert(mtools::GetColorPlaneSelector<ObjType>::has_getColor, "The object must be implement one of the getColor() methods recognized by GetColorPlaneSelector.");
                if (nbthread < 1) nbthread = 1;
                nbThreads(nbthread);
                }


            /** Destructor. */
            ~PlaneDrawer()
                {
                _deleteAllThread();
                }


            /**
            * Return the current number of threads.
            **/
            int nbThreads() const { return (int)_vecThread.size(); }


            /**
            * Change the number of thread.
            *
            * All threads are disabled and setParam must be called again to set the parameters.
            **/
            void nbThreads(int nb)
                {
                if (nb < 1) nb = 1;
                if (nb == nbThreads()) return;
                _deleteAllThread();
                _vecThread.resize(nb);
                for (size_t i = 0; i < nb; i++) { _vecThread[i] = new ThreadPlaneDrawer<ObjType>(_obj); }
                }


            /**
            * Determines if the drawing parameters are valid.
            **/
            inline bool validParam()  const
                {
                if (_vecThread.size() == 0) return false;
                sync();
                for (size_t i = 0; i < _vecThread.size(); i++) { if (!((_vecThread[i])->validParam())) return false; }
                return true;
                }


            /** Synchronises all threads */
            void sync()
                {
                for (size_t i = 0; i < _vecThread.size(); i++) 
                    { 
                    (_vecThread[i])->sync();
                    }
                }


            /**
            * Get the current progress value (which is the min of the progress of all the threads).
            **/
            inline int progress() const
                {
                if (_vecThread.size() == 0) return 0;
                int pr = (_vecThread[0])->progress();
                for (size_t i = 1; i < _vecThread.size(); i++)
                    {
                    const int q = (_vecThread[i])->progress();
                    if (q < pr) { pr = q; }
                    }
                return pr;
                }

            /**
            * Enables/Disable all the threads.
            **/
            void enable(bool newstatus)
                {
                if (_vecThread.size() == 0) return;
                sync();
                if (newstatus == _vecThread[0]->enable()) return;
                for (size_t i = 0; i < _vecThread.size(); i++) { _vecThread[i]->enable(newstatus); }
                }


            /**
            * Query if the threads are currently enabled.
            **/
            inline bool enable()
                {
                if (_vecThread.size() == 0) return false;
                sync();
                return _vecThread[0]->enable();
                }


            /**
            * Sets the drawing parameters.
            *
            * Returns immediately, use sync() to wait for the operation to complete.
            *
            * @param   range       The range to draw.
            * @param [in,out]  im  The image to draw into.
            * @param   subBox      The part of the image to draw (border inclusive). If empty, use the whole
            *                      image.
            **/
            void setParameters(const fBox2 & range, ProgressImg * im, iBox2 subBox = iBox2())
                {
                if (subBox.isEmpty()) { subBox = iBox2(0, im->width() - 1, 0, im->height() - 1); }
                const size_t nt = _vecThread.size();
                const int64 H = subBox.ly() + 1;
                if ((nt == 0) || (H < (int64)(3 * nt))) return;
                int64 h = H / nt;
                size_t m = (size_t)(H % nt);
                iBox2 cbox(subBox.min[0], subBox.max[0], subBox.min[1], subBox.min[1] + h - 1);
                for (size_t i = 0; i < (nt - m); i++)
                    {
                    _vecThread[i]->setParameters(_computeRange(range, subBox, cbox), im, cbox);
                    cbox.min[1] += h; cbox.max[1] += h;
                    }
                cbox.max[1]++;
                for (size_t i = (nt - m); i < nt; i++)
                    {
                    _vecThread[i]->setParameters(_computeRange(range, subBox, cbox), im, cbox);
                    cbox.min[1] += (h + 1); cbox.max[1] += (h + 1);
                    }
                MTOOLS_INSURE(cbox.min[1] == 1 + subBox.max[1]);
                }


            fBox2 _computeRange(fBox2 range, iBox2 subBox, iBox2 cBox)
                {
                const double px = range.lx() / (subBox.lx() + 1);
                const double py = range.ly() / (subBox.ly() + 1);
                const double xmin = range.min[0] + px*(cBox.min[0] - subBox.min[0]);
                const double xmax = range.max[0] - px*(subBox.max[0] - cBox.max[0]);
                const double ymin = range.min[1] + py*(cBox.min[1] - subBox.min[1]);
                const double ymax = range.max[1] - py*(subBox.max[1] - cBox.max[1]);
                return fBox2(xmin, xmax, ymin, ymax);
                }


            /**
            * Force a redraw.
            *
            * Returns immediately, use sync() to wait for the operation to complete.
            **/
            void redraw()
                {
                for (size_t i = 0; i < _vecThread.size(); i++) { (_vecThread[i])->redraw(); }
                }


        private:


            /* delete all worker thread */
            void _deleteAllThread()
                {
                for (size_t i = 0; i < _vecThread.size(); i++) { delete(_vecThread[i]); }
                _vecThread.clear();
                }


            // no copy
            PlaneDrawer(const PlaneDrawer &) = delete;
            PlaneDrawer & operator=(const PlaneDrawer &) = delete;


            ObjType * _obj;                                             // the object to draw.
            std::vector< ThreadPlaneDrawer<ObjType>*  > _vecThread;     // vector of all the threads. 


        };


























}

/* end of file */

