/** @file getcolorselector.hpp */
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

#include "customcimg.hpp"
#include "rgbc.hpp"
#include "../maths/vec.hpp"
#include "../maths/box.hpp"
#include "../misc/misc.hpp"
#include "../misc/metaprog.hpp"


namespace mtools
    {


    /**
    * GetColor Plane method selector.
    *
    * Detect if a type (or function) contain a method compatible with getColor() used by the PlaneDrawer class.
    * 
    * The method can be called with call(). (if no method found, return RGBc::c_TransparentWhite).
    *
    * - fVec2 pos or (int64 x,int64 y) : the coordinate of the point to draw
    *
    * - int32 iter : number of time this pixel was queried. Increased by one after each to the method (until the drawing is resetted, moved...).
    * 
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

        template<typename U> static decltype((*(U*)(0)).getColor(0.0, 0.0)) vers9(int);
        template<typename> static metaprog::no vers9(...);
        static const bool version9 = std::is_same<typename std::decay<decltype(vers9<T>(0))>::type, decaypair >::value;
        static const bool version19 = std::is_same<typename std::decay<decltype(vers9<T>(0))>::type, decayrgb >::value;

        template<typename U> static decltype((*(U*)(0))(0.0, 0.0)) vers10(int);
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
        static std::pair<mtools::RGBc, bool> call9(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<true> D) { return obj.getColor(pos.X(), pos.Y()); }
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
        static std::pair<mtools::RGBc, bool> call20(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data, mtools::metaprog::dummy<false> D) { MTOOLS_DEBUG("GetColorPlaneSelector: no getColor() found."); return std::pair<mtools::RGBc, bool>(RGBc::c_TransparentWhite, false); }

        public:

            static const bool has_getColor = version1 | version2 | version3 | version4 | version5 | version6 | version7 | version8 | version9 | version10 |
                version11 | version12 | version13 | version14 | version15 | version16 | version17 | version18 | version19 | version20;

            static std::pair<mtools::RGBc, bool> call(T & obj, const fVec2 & pos, const fBox2 & box, int32 nbiter, void * &data) { return call1(obj, pos, box, nbiter, data, mtools::metaprog::dummy<version1>()); }

        };



        /**
        * GetImage method selector.
        *
        * Detect if a type (or function) contain a method compatible with getImage() used by the SiteDrawer class.
        * 
        * The method can be called with call(). (if no method found, return nullptr).
        *
        * - iVec2 pos or (int64 x,int64 y) : the coordinate of the point to draw
        *
        * - iVec2 imSize or (int64 imLX, int64 imLY) : preferred dimension of the image. Need not be respected but
        *                                              faster if the returned image is of that size
        *
        * - void* & data : reference to an opaque value that identify the thread drawing. Set to nullptr intially, the reference may
        *                  be modified by the getImage()/getColor() function and the same value will be returned at the next call
        *                  from the same thread.
        *
        * The signature below are recognized with the following order:
        *
        *  [const] Im<Tim> * getImage([const]iVec2 [&] pos, const iVec2 [&] imSize, void* & data)
        *  [const] Im<Tim> * getImage([const]iVec2 [&] pos, const iVec2 [&] imSize)
        *  [const] Im<Tim> * getImage([const] int64 [&] x,[const] int64 [&] y,[const] int64 [&] imLX,[const] int64 [&] imLY, void* & data)
        *  [const] Im<Tim> * getImage([const] int64 [&] x,[const] int64 [&] y,[const] int64 [&] imLX,[const] int64 [&] imLY)
        *  [const] Im<Tim> * operator()([const]iVec2 [&] pos, const iVec2 [&] imSize, void* & data)
        *  [const] Im<Tim> * operator()([const]iVec2 [&] pos, const iVec2 [&] imSize)
        *  [const] Im<Tim> * operator()([const] int64 [&] x,[const] int64 [&] y,[const] int64 [&] imLX,[const] int64 [&] imLY, void* & data)
        *  [const] Im<Tim> * operator()([const] int64 [&] x,[const] int64 [&] y,[const] int64 [&] imLX,[const] int64 [&] imLY)
        **/
        template<typename T, typename Tim> class GetImageSelector
            {
            using im = Img<Tim>;
            typedef typename std::decay<im>::type decayim;

            static void * dumptr;

            template<typename U> static decltype(*((*(U*)(0)).getImage(iVec2{ 0,0 }, iVec2{ 0,0 }, dumptr))) vers1(int);
            template<typename> static metaprog::no vers1(...);
            static const bool version1 = std::is_same<typename std::decay<decltype(vers1<T>(0))>::type, decayim >::value;

            template<typename U> static decltype(*((*(U*)(0)).getImage(iVec2{ 0,0 }, iVec2{ 0,0 }))) vers2(int);
            template<typename> static metaprog::no vers2(...);
            static const bool version2 = std::is_same<typename std::decay<decltype(vers2<T>(0))>::type, decayim >::value;

            template<typename U> static decltype(*((*(U*)(0)).getImage(0, 0, 0, 0, dumptr))) vers3(int);
            template<typename> static metaprog::no vers3(...);
            static const bool version3 = std::is_same<typename std::decay<decltype(vers3<T>(0))>::type, decayim >::value;

            template<typename U> static decltype(*((*(U*)(0)).getImage(0, 0, 0, 0))) vers4(int);
            template<typename> static metaprog::no vers4(...);
            static const bool version4 = std::is_same<typename std::decay<decltype(vers4<T>(0))>::type, decayim >::value;

            template<typename U> static decltype(*((*(U*)(0))(iVec2{ 0,0 }, iVec2{ 0,0 }, dumptr))) vers5(int);
            template<typename> static metaprog::no vers5(...);
            static const bool version5 = std::is_same<typename std::decay<decltype(vers5<T>(0))>::type, decayim >::value;

            template<typename U> static decltype(*((*(U*)(0))(iVec2{ 0,0 }, iVec2{ 0,0 }))) vers6(int);
            template<typename> static metaprog::no vers6(...);
            static const bool version6 = std::is_same<typename std::decay<decltype(vers6<T>(0))>::type, decayim >::value;

            template<typename U> static decltype(*((*(U*)(0))(0, 0, 0, 0, dumptr))) vers7(int);
            template<typename> static metaprog::no vers7(...);
            static const bool version7 = std::is_same<typename std::decay<decltype(vers7<T>(0))>::type, decayim >::value;

            template<typename U> static decltype(*((*(U*)(0))(0, 0, 0, 0))) vers8(int);
            template<typename> static metaprog::no vers8(...);
            static const bool version8 = std::is_same<typename std::decay<decltype(vers8<T>(0))>::type, decayim >::value;

            static const im * call1(T & obj, const iVec2 & pos, const iVec2 & imSize, void * &data, mtools::metaprog::dummy<true> D) { return obj.getImage(pos, imSize, data); }
            static const im * call2(T & obj, const iVec2 & pos, const iVec2 & imSize, void * &data, mtools::metaprog::dummy<true> D) { return obj.getImage(pos, imSize); }
            static const im * call3(T & obj, const iVec2 & pos, const iVec2 & imSize, void * &data, mtools::metaprog::dummy<true> D) { return obj.getImage(pos.X(), pos.Y(), imSize.X(), imSize.Y(), data); }
            static const im * call4(T & obj, const iVec2 & pos, const iVec2 & imSize, void * &data, mtools::metaprog::dummy<true> D) { return obj.getImage(pos.X(), pos.Y(), imSize.X(), imSize.Y()); }
            static const im * call5(T & obj, const iVec2 & pos, const iVec2 & imSize, void * &data, mtools::metaprog::dummy<true> D) { return obj(pos, imSize, data); }
            static const im * call6(T & obj, const iVec2 & pos, const iVec2 & imSize, void * &data, mtools::metaprog::dummy<true> D) { return obj(pos, imSize); }
            static const im * call7(T & obj, const iVec2 & pos, const iVec2 & imSize, void * &data, mtools::metaprog::dummy<true> D) { return obj(pos.X(), pos.Y(), imSize.X(), imSize.Y(), data); }
            static const im * call8(T & obj, const iVec2 & pos, const iVec2 & imSize, void * &data, mtools::metaprog::dummy<true> D) { return obj(pos.X(), pos.Y(), imSize.X(), imSize.Y()); }

            static const im * call1(T & obj, const iVec2 & pos, const iVec2 & imSize, void * &data, mtools::metaprog::dummy<false> D) { return call2(obj, pos, imSize, data, mtools::metaprog::dummy<version2>()); }
            static const im * call2(T & obj, const iVec2 & pos, const iVec2 & imSize, void * &data, mtools::metaprog::dummy<false> D) { return call3(obj, pos, imSize, data, mtools::metaprog::dummy<version3>()); }
            static const im * call3(T & obj, const iVec2 & pos, const iVec2 & imSize, void * &data, mtools::metaprog::dummy<false> D) { return call4(obj, pos, imSize, data, mtools::metaprog::dummy<version4>()); }
            static const im * call4(T & obj, const iVec2 & pos, const iVec2 & imSize, void * &data, mtools::metaprog::dummy<false> D) { return call5(obj, pos, imSize, data, mtools::metaprog::dummy<version5>()); }
            static const im * call5(T & obj, const iVec2 & pos, const iVec2 & imSize, void * &data, mtools::metaprog::dummy<false> D) { return call6(obj, pos, imSize, data, mtools::metaprog::dummy<version6>()); }
            static const im * call6(T & obj, const iVec2 & pos, const iVec2 & imSize, void * &data, mtools::metaprog::dummy<false> D) { return call7(obj, pos, imSize, data, mtools::metaprog::dummy<version7>()); }
            static const im * call7(T & obj, const iVec2 & pos, const iVec2 & imSize, void * &data, mtools::metaprog::dummy<false> D) { return call8(obj, pos, imSize, data, mtools::metaprog::dummy<version8>()); }
            static const im * call8(T & obj, const iVec2 & pos, const iVec2 & imSize, void * &data, mtools::metaprog::dummy<false> D) { MTOOLS_DEBUG("GetImageSelector: No getImage() found."); return nullptr; }

            public:

                static const bool has_getImage = version1 | version2 | version3 | version4 | version5 | version6 | version7 | version8;

                static const im * call(T & obj, const iVec2 & pos, const iVec2 & imSize, void * &data) { return call1(obj, pos, imSize, data, mtools::metaprog::dummy<version1>()); }
            };



        /**
        * GetColor method selector.
        *
        * Detect if a type (or function) contain a method compatible with getColor() used by the PixelDrawer class.
        * 
        * The method can be called with call(). (if no method found, return RGBc::c_TransparentWhite).
        *
        * - iVec2 pos or (int64 x,int64 y) : the coordinate of the point to draw
        *
        * - void* & data : reference to an opaque value that identify the thread drawing. Set to nullptr intially, the reference may
        *                  be modified by the getImage()/getColor() function and the same value will be returned at the next call
        *                  from the same thread.
        *
        * The signature below are recognized with the following order:
        *
        *  RGBc getImage([const]iVec2 [&] pos, void* & data)
        *  RGBc getImage([const]iVec2 [&] pos)
        *  RGBc getImage([const] int64 [&] x,[const] int64 [&] y, void* & data)
        *  RGBc getImage([const] int64 [&] x,[const] int64 [&] y)
        *  RGBc operator()([const]iVec2 [&] pos, void* & data)
        *  RGBc operator()([const]iVec2 [&] pos)
        *  RGBc operator()([const] int64 [&] x,[const] int64 [&] y, void* & data)
        *  RGBc operator()([const] int64 [&] x,[const] int64 [&] y)
        *
        *  If none if the method above are found, try to find a getImage() method GetImageSelector<T,unsigned char>
        *  and convert to resulting image to a single color using the Img<unsigned char>::toRGBc() method
        **/
        template<typename T> class GetColorSelector
            {
            static void * dumptr;
            typedef typename std::decay<mtools::RGBc>::type decayrgb;

            template<typename U> static decltype((*(U*)(0)).getColor(iVec2{ 0,0 }, dumptr)) vers1(int);
            template<typename> static metaprog::no vers1(...);
            static const bool version1 = std::is_same<typename std::decay<decltype(vers1<T>(0))>::type, decayrgb>::value;

            template<typename U> static decltype((*(U*)(0)).getColor(iVec2{ 0,0 })) vers2(int);
            template<typename> static metaprog::no vers2(...);
            static const bool version2 = std::is_same<typename std::decay<decltype(vers2<T>(0))>::type, decayrgb >::value;

            template<typename U> static decltype((*(U*)(0)).getColor(0, 0, dumptr)) vers3(int);
            template<typename> static metaprog::no vers3(...);
            static const bool version3 = std::is_same<typename std::decay<decltype(vers3<T>(0))>::type, decayrgb >::value;

            template<typename U> static decltype((*(U*)(0)).getColor(0, 0)) vers4(int);
            template<typename> static metaprog::no vers4(...);
            static const bool version4 = std::is_same<typename std::decay<decltype(vers4<T>(0))>::type, decayrgb >::value;

            template<typename U> static decltype((*(U*)(0))(iVec2{ 0,0 }, dumptr)) vers5(int);
            template<typename> static metaprog::no vers5(...);
            static const bool version5 = std::is_same<typename std::decay<decltype(vers5<T>(0))>::type, decayrgb >::value;

            template<typename U> static decltype((*(U*)(0))(iVec2{ 0,0 })) vers6(int);
            template<typename> static metaprog::no vers6(...);
            static const bool version6 = std::is_same<typename std::decay<decltype(vers6<T>(0))>::type, decayrgb >::value;

            template<typename U> static decltype((*(U*)(0))(0, 0, dumptr)) vers7(int);
            template<typename> static metaprog::no vers7(...);
            static const bool version7 = std::is_same<typename std::decay<decltype(vers7<T>(0))>::type, decayrgb >::value;

            template<typename U> static decltype((*(U*)(0))(0, 0)) vers8(int);
            template<typename> static metaprog::no vers8(...);
            static const bool version8 = std::is_same<typename std::decay<decltype(vers8<T>(0))>::type, decayrgb >::value;

            static mtools::RGBc call1(T & obj, const iVec2 & pos, void * &data, mtools::metaprog::dummy<true> D) { return obj.getColor(pos, data); }
            static mtools::RGBc call2(T & obj, const iVec2 & pos, void * &data, mtools::metaprog::dummy<true> D) { return obj.getColor(pos); }
            static mtools::RGBc call3(T & obj, const iVec2 & pos, void * &data, mtools::metaprog::dummy<true> D) { return obj.getColor(pos.X(), pos.Y(), data); }
            static mtools::RGBc call4(T & obj, const iVec2 & pos, void * &data, mtools::metaprog::dummy<true> D) { return obj.getColor(pos.X(), pos.Y()); }
            static mtools::RGBc call5(T & obj, const iVec2 & pos, void * &data, mtools::metaprog::dummy<true> D) { return obj(pos, data); }
            static mtools::RGBc call6(T & obj, const iVec2 & pos, void * &data, mtools::metaprog::dummy<true> D) { return obj(pos); }
            static mtools::RGBc call7(T & obj, const iVec2 & pos, void * &data, mtools::metaprog::dummy<true> D) { return obj(pos.X(), pos.Y(), data); }
            static mtools::RGBc call8(T & obj, const iVec2 & pos, void * &data, mtools::metaprog::dummy<true> D) { return obj(pos.X(), pos.Y()); }
            static mtools::RGBc call9(T & obj, const iVec2 & pos, void * &data, mtools::metaprog::dummy<true> D) {
                const mtools::Img<unsigned char> * im = GetImageSelector<T, unsigned char>::call(obj, pos, { 1,1 }, data);
                return ((im == nullptr) ? RGBc::c_TransparentWhite : im->toRGBc());
                }

            static mtools::RGBc call1(T & obj, const iVec2 & pos, void * &data, mtools::metaprog::dummy<false> D) { return call2(obj, pos, data, mtools::metaprog::dummy<version2>()); }
            static mtools::RGBc call2(T & obj, const iVec2 & pos, void * &data, mtools::metaprog::dummy<false> D) { return call3(obj, pos, data, mtools::metaprog::dummy<version3>()); }
            static mtools::RGBc call3(T & obj, const iVec2 & pos, void * &data, mtools::metaprog::dummy<false> D) { return call4(obj, pos, data, mtools::metaprog::dummy<version4>()); }
            static mtools::RGBc call4(T & obj, const iVec2 & pos, void * &data, mtools::metaprog::dummy<false> D) { return call5(obj, pos, data, mtools::metaprog::dummy<version5>()); }
            static mtools::RGBc call5(T & obj, const iVec2 & pos, void * &data, mtools::metaprog::dummy<false> D) { return call6(obj, pos, data, mtools::metaprog::dummy<version6>()); }
            static mtools::RGBc call6(T & obj, const iVec2 & pos, void * &data, mtools::metaprog::dummy<false> D) { return call7(obj, pos, data, mtools::metaprog::dummy<version7>()); }
            static mtools::RGBc call7(T & obj, const iVec2 & pos, void * &data, mtools::metaprog::dummy<false> D) { return call8(obj, pos, data, mtools::metaprog::dummy<version8>()); }
            static mtools::RGBc call8(T & obj, const iVec2 & pos, void * &data, mtools::metaprog::dummy<false> D) { return call9(obj, pos, data, mtools::metaprog::dummy<GetImageSelector<T, unsigned char>::has_getImage>()); }
            static mtools::RGBc call9(T & obj, const iVec2 & pos, void * &data, mtools::metaprog::dummy<false> D) { MTOOLS_DEBUG("GetColorSelector: No getImage()/getColor() found."); return RGBc::c_TransparentWhite; }

            public:

                static const bool has_getColor = version1 | version2 | version3 | version4 | version5 | version6 | version7 | version8;

                static RGBc call(T & obj, const iVec2 & pos, void * &data) { return call1(obj, pos, data, mtools::metaprog::dummy<version1>()); }

            };



    }

/* end of file */

