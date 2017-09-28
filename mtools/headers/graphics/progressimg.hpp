/** @file progressimg.hpp */
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

#include "../misc/internal/mtools_export.hpp"
#include "../misc/misc.hpp"
#include "../misc/error.hpp"
#include "../maths/vec.hpp"
#include "rgbc.hpp"
#include "customcimg.hpp"


namespace mtools
    {


    /**
     * Progress image class.
     * 
     * Simple class which encapsulate a RGBc64 image together with a uint8 buffer that specifies the
     * normalisation for each pixel.
     **/
    class ProgressImg
        {

        public:

            /**
            * Construct an empty image
            **/
            ProgressImg() : _width(0), _height(0), _imData(nullptr), _normData(nullptr) {}


            /**
            * Construct an image with a given size.
            **/
            ProgressImg(size_t LX, size_t LY) : _width(0), _height(0), _imData(nullptr), _normData(nullptr) { resize(LX, LY); }


            /** Destructor. */
            ~ProgressImg()
                {
                delete[] _normData; 
                delete[] _imData;
                }


            /**
             * Copy constructor. Make a deep copy
             **/
            ProgressImg(const ProgressImg & im) : _width(0), _height(0), _imData(nullptr), _normData(nullptr)
                {
                resize(im.width(), im.height());
                const size_t l = _width*_height;
                if (l > 0)
                    {
                    memcpy(_imData, im._imData, l*sizeof(RGBc64));
                    memcpy(_normData, im._normData, l);
                    }
                }


            /**
            * Move constructor.
            **/
            ProgressImg(ProgressImg && im) : _width(im._width), _height(im._height), _imData(im._imData), _normData(im._normData)
                {
                im._width = 0;
                im._height = 0;
                im._imData = nullptr;
                im._normData = nullptr;
                }


            /**
            * Assignement operator. Make a deep copy
            **/
            ProgressImg & operator=(const ProgressImg & im)
                {
                if (&im == this) return *this;
                resize(im.width(), im.height());
                const size_t l = _width*_height;
                if (l > 0)
                    {
                    memcpy(_imData, im._imData, l*sizeof(RGBc64));
                    memcpy(_normData, im._normData, l);
                    }
                return *this;
                }


            /**
            * Move assignement operator.
            **/
            ProgressImg & operator=(ProgressImg && im)
                {
                if (&im == this) return *this;
                resize(0,0);
                _width = im._width;
                _height = im._height;
                _imData = im._imData;
                _normData = im._normData;
                im._width = 0; 
                im._height = 0;
                im._imData = nullptr;
                im._normData = nullptr;
                return *this;
                }


            /**
            * Raw image resizing. Keep the current buffer if the new size is smaller than the previous one,
            * allocate another buffer otherwise.
            **/
            void resize(size_t newLX, size_t newLY)
                {
                if ((newLX <= 0) || (newLY <= 0))
                    {
                    delete[] _normData; _normData = nullptr;
                    delete[] _imData; _imData = nullptr;
                    _width = 0; _height = 0;
                    return;
                    }
                if ((newLX*newLY) > (_width*_height))
                    {
                    size_t l = newLX*newLY;
                    delete[] _normData; _normData = new uint8[l];
                    delete[] _imData; _imData = new RGBc64[l];
                    }
                _width = newLX;
                _height = newLY;
                }


            /**
             * Clears the whole image to a given color. Faster if all color channels are equal. The normalization
             * is set to 1.
             **/
            void clear(RGBc color)
                {
                if (_imData == nullptr) return;
                const size_t l = _width*_height;
                memset(_normData, 0, l);
                if ((color.comp.R == color.comp.G) && (color.comp.R == color.comp.B) && (color.comp.R == color.comp.A))
                    {
                    memset(_imData, color.comp.R, sizeof(RGBc64)*l);
                    }
                else
                    {
                    RGBc64 c64(color);
                    for (size_t z = 0; z < l; z++) _imData[z] = c64;
                    }
                }


            /**
            * Return the normalised pixel color at position (x,y).
            **/
            inline RGBc operator()(size_t x, size_t y) const
                {
                const size_t off = x + _width*y;
                return _imData[off].getRGBc(_normData[off] + 1);
                }


            /**
            * Return the height of the image
            **/
            inline size_t height() const { return _height; }


            /**
            * Return the width of the image.
            **/
            inline size_t width()  const { return _width; }


            /**
            * Return a pointer to the color buffer.
            **/
            inline RGBc64 * imData() { return _imData; }


            /**
            * Return a pointer to the normalization buffer.
            **/
            inline uint8 * normData() { return _normData; }


            /**
             * Normalises a portion of the image so that the multiplying factor for each pixel of the sub
             * image 1.
             *
             * @param   subBox  The sub box describing the portion to normalize. If empty, nothing is done.
             *                  If the box is too large, it is clipped insidde the image.
             **/
            void normalize(iBox2 subBox)
                {
                if (subBox.min[0] < 0) { subBox.min[0] = 0; }
                if (subBox.min[1] < 0) { subBox.min[1] = 0; }
                if (subBox.max[0] > (int64)(_width - 1)) { subBox.min[0] = (int64)(_width - 1); }
                if (subBox.max[1] > (int64)(_height - 1)) { subBox.min[1] = (int64)(_height - 1); }
                if (subBox.isEmpty()) return;
                size_t off = (size_t)(subBox.min[0] + _width*subBox.min[1]);
                const int64 ly = subBox.ly();
                const size_t pa = (size_t)(_width - (subBox.lx() + 1));
                for (int64 y = 0; y <= ly; y++)
                    {
                    for (int64 x = 0; x <= ly; x++)
                        {
                        _imData[off].normalize(_normData[off] + 1);
                        _normData[off] = 0;
                        off++;
                        }
                    off += pa;
                    }
                }


            /** Normalises the whole image. */
            void normalize()
                {
                const size_t l = _width*_height;
                for (size_t i = 0; i<l; i++) { _imData[i].normalize(_normData[i] + 1); _normData[i] = 0; }
                }


            /**
             * Blit the ProgressImg into a Img. Both images must have the same size.
             *
             * @param [in,out]  im  The destination image. Must have the same size as this. Can be a 3 or 4
             *                      channel image. If it exist, the alpha channel is set to full opacity.
             * @param   op          opacity to multiply the progressImg with before blitting.
             * @param   reverse     true to reverse the y axis.
             * @param   blitType    Type of blitting. One of BLIT_CLASSIC, BLIT_REMOVE_TRANSPARENT_WHITE or
             *                      BLIT_REMOVE_TRANSPARENT_BLACK.
             **/
            void blit(Img<unsigned char> & im, float op = 1.0, bool reverse = true, int blitType = BLIT_CLASSIC)
                {
                switch (blitType)
                    {
                    case BLIT_CLASSIC: { blit_classic(im, op, reverse); return; }
                    case BLIT_REMOVE_TRANSPARENT_WHITE: { blit_removeWhite(im, op, reverse); return; }
                    case BLIT_REMOVE_TRANSPARENT_BLACK: { blit_removeBlack(im, op, reverse); return; }
                    }
                MTOOLS_ERROR("Illegal blitType argument...");
                }


        private:


            /* blitting procedure, traditionnal blending version */
            void blit_classic(Img<unsigned char> & im, float op = 1.0, bool reverse = true)
                {
                const uint32 op32 = (uint32)(255.0f*op);
                if (op32 == 0) return;
                const size_t lx = (size_t)im.width();
                const size_t ly = (size_t)im.height();
                if ((lx <= 0) || (ly <= 0)) return;
                MTOOLS_ASSERT(lx == height());
                MTOOLS_ASSERT(ly == width());
                const size_t l = (size_t)lx*ly;
                if (im.spectrum() == 3)
                    {
                    if (!reverse)
                        {
                        unsigned char * p1 = im.data();
                        unsigned char * p2 = im.data() + l;
                        unsigned char * p3 = im.data() + 2 * l;
                        for (size_t z = 0; z < l; z++) { _fastblend(_imData[z], (uint32)_normData[z] + 1, op32, p1[z], p2[z], p3[z]); }
                        return;
                        }
                    else
                        {
                        unsigned char * p1 = im.data() + (l - lx);
                        unsigned char * p2 = im.data() + (2 * l - lx);
                        unsigned char * p3 = im.data() + (3 * l - lx);
                        size_t z = 0;
                        for (size_t j = 0;j < ly; j++)
                            {
                            for (size_t i = 0; i < lx; i++) { _fastblend(_imData[z], (uint32)_normData[z] + 1, op32, p1[i], p2[i], p3[i]); z++; }
                            p1 -= lx; p2 -= lx; p3 -= lx;
                            }
                        }
                    return;
                    }
                if (im.spectrum() == 4)
                    {
                    if (!reverse)
                        {
                        unsigned char * p1 = im.data();
                        unsigned char * p2 = im.data() + l;
                        unsigned char * p3 = im.data() + 2 * l;
                        unsigned char * p4 = im.data() + 3 * l;
                        for (size_t z = 0; z < l; z++) { _fastblend(_imData[z], (uint32)_normData[z] + 1, op32, p1[z], p2[z], p3[z]); p4[z] = 255; }
                        return;
                        }
                    else
                        {
                        unsigned char * p1 = im.data() + (l - lx);
                        unsigned char * p2 = im.data() + (2 * l - lx);
                        unsigned char * p3 = im.data() + (3 * l - lx);
                        unsigned char * p4 = im.data() + (4 * l - lx);
                        size_t z = 0;
                        for (size_t j = 0;j < ly; j++)
                            {
                            for (size_t i = 0; i < lx; i++) { _fastblend(_imData[z], (uint32)_normData[z] + 1, op32, p1[i], p2[i], p3[i]); p4[z] = 255; z++; }
                            p1 -= lx; p2 -= lx; p3 -= lx; p4 -= lx;
                            }
                        }
                    return;
                    }
                MTOOLS_ERROR("incorrect number of channel in the image");
                }


            /* blitting procedure, remove fully transparent white pixel */
            void blit_removeWhite(Img<unsigned char> & im, const float op = 1.0f, bool reverse = true)
                {
                if (op <= 0.0f) return;
                const size_t lx = (size_t)im.width();
                const size_t ly = (size_t)im.height();
                if ((lx <= 0) || (ly <= 0)) return;
                MTOOLS_ASSERT(lx == height());
                MTOOLS_ASSERT(ly == width());
                const size_t l = (size_t)lx*ly;
                if (im.spectrum() == 3)
                    {
                    if (!reverse)
                        {
                        unsigned char * p1 = im.data();
                        unsigned char * p2 = im.data() + l;
                        unsigned char * p3 = im.data() + 2 * l;
                        for (size_t z = 0; z < l; z++) { _blend_removeWhite(_imData[z], (uint32)_normData[z] + 1, op, p1[z], p2[z], p3[z]); }
                        return;
                        }
                    else
                        {
                        unsigned char * p1 = im.data() + (l - lx);
                        unsigned char * p2 = im.data() + (2 * l - lx);
                        unsigned char * p3 = im.data() + (3 * l - lx);
                        size_t z = 0;
                        for (size_t j = 0;j < ly; j++)
                            {
                            for (size_t i = 0; i < lx; i++) { _blend_removeWhite(_imData[z], (uint32)_normData[z] + 1, op, p1[i], p2[i], p3[i]); z++; }
                            p1 -= lx; p2 -= lx; p3 -= lx;
                            }
                        }
                    return;
                    }
                if (im.spectrum() == 4)
                    {
                    if (!reverse)
                        {
                        unsigned char * p1 = im.data();
                        unsigned char * p2 = im.data() + l;
                        unsigned char * p3 = im.data() + 2 * l;
                        unsigned char * p4 = im.data() + 3 * l;
                        for (size_t z = 0; z < l; z++) { _blend_removeWhite(_imData[z], (uint32)_normData[z] + 1, op, p1[z], p2[z], p3[z]); p4[z] = 255; }
                        return;
                        }
                    else
                        {
                        unsigned char * p1 = im.data() + (l - lx);
                        unsigned char * p2 = im.data() + (2 * l - lx);
                        unsigned char * p3 = im.data() + (3 * l - lx);
                        unsigned char * p4 = im.data() + (4 * l - lx);
                        size_t z = 0;
                        for (size_t j = 0;j < ly; j++)
                            {
                            for (size_t i = 0; i < lx; i++) { _blend_removeWhite(_imData[z], (uint32)_normData[z] + 1, op, p1[i], p2[i], p3[i]); p4[z] = 255; z++; }
                            p1 -= lx; p2 -= lx; p3 -= lx; p4 -= lx;
                            }
                        }
                    return;
                    }
                MTOOLS_ERROR("incorrect number of channel in the image");
                }


            /* blitting procedure, remove fully transparent black pixel */
            void blit_removeBlack(Img<unsigned char> & im, const float op = 1.0f, bool reverse = true)
                {
                if (op <= 0.0f) return;
                const size_t lx = (size_t)im.width();
                const size_t ly = (size_t)im.height();
                if ((lx <= 0) || (ly <= 0)) return;
                MTOOLS_ASSERT(lx == height());
                MTOOLS_ASSERT(ly == width());
                const size_t l = (size_t)lx*ly;
                if (im.spectrum() == 3)
                    {
                    if (!reverse)
                        {
                        unsigned char * p1 = im.data();
                        unsigned char * p2 = im.data() + l;
                        unsigned char * p3 = im.data() + 2 * l;
                        for (size_t z = 0; z < l; z++) { _blend_removeBlack(_imData[z], (uint32)_normData[z] + 1, op, p1[z], p2[z], p3[z]); }
                        return;
                        }
                    else
                        {
                        unsigned char * p1 = im.data() + (l - lx);
                        unsigned char * p2 = im.data() + (2 * l - lx);
                        unsigned char * p3 = im.data() + (3 * l - lx);
                        size_t z = 0;
                        for (size_t j = 0;j < ly; j++)
                            {
                            for (size_t i = 0; i < lx; i++) { _blend_removeBlack(_imData[z], (uint32)_normData[z] + 1, op, p1[i], p2[i], p3[i]); z++; }
                            p1 -= lx; p2 -= lx; p3 -= lx;
                            }
                        }
                    return;
                    }
                if (im.spectrum() == 4)
                    {
                    if (!reverse)
                        {
                        unsigned char * p1 = im.data();
                        unsigned char * p2 = im.data() + l;
                        unsigned char * p3 = im.data() + 2 * l;
                        unsigned char * p4 = im.data() + 3 * l;
                        for (size_t z = 0; z < l; z++) { _blend_removeBlack(_imData[z], (uint32)_normData[z] + 1, op, p1[z], p2[z], p3[z]); p4[z] = 255; }
                        return;
                        }
                    else
                        {
                        unsigned char * p1 = im.data() + (l - lx);
                        unsigned char * p2 = im.data() + (2 * l - lx);
                        unsigned char * p3 = im.data() + (3 * l - lx);
                        unsigned char * p4 = im.data() + (4 * l - lx);
                        size_t z = 0;
                        for (size_t j = 0;j < ly; j++)
                            {
                            for (size_t i = 0; i < lx; i++) { _blend_removeBlack(_imData[z], (uint32)_normData[z] + 1, op, p1[i], p2[i], p3[i]); p4[z] = 255; z++; }
                            p1 -= lx; p2 -= lx; p3 -= lx; p4 -= lx;
                            }
                        }
                    return;
                    }
                MTOOLS_ERROR("incorrect number of channel in the image");
                }


                /* blending and artificially removing fully transparent white pixel */
                inline void _blend_removeWhite(const RGBc64 & coul, const uint32 N, float op, uint8 & R, uint8 & G, uint8 & B)
                    {
                    if (coul.comp.A == 0) return;
                    const float g = ((float)(N * 255))/((float)coul.comp.A);
                    const float nR = g*(((float)coul.comp.R / N) - 255) + 255;
                    const float nG = g*(((float)coul.comp.G / N) - 255) + 255;
                    const float nB = g*(((float)coul.comp.B / N) - 255) + 255;                    
                    const float alpha = op/g;
                    const float beta = 1.0f - alpha;
                    R = (uint8)(beta*R + (alpha*nR));
                    G = (uint8)(beta*G + (alpha*nG));
                    B = (uint8)(beta*B + (alpha*nB));
                    }


                /* blending and artificially removing fully transparent black pixel */
                inline void _blend_removeBlack(const RGBc64 & coul, const uint32 N, float op, uint8 & R, uint8 & G, uint8 & B)
                    {
                    if (coul.comp.A == 0) return;
                    const float g = ((float)(N * 255)) / ((float)coul.comp.A);
                    const float nR = g*((float)coul.comp.R / N);
                    const float nG = g*((float)coul.comp.G / N);
                    const float nB = g*((float)coul.comp.B / N);
                    const float alpha = op/g;
                    const float beta = 1.0f - alpha;
                    R = (uint8)(beta*R + (alpha*nR));
                    G = (uint8)(beta*G + (alpha*nG));
                    B = (uint8)(beta*B + (alpha*nB));
                    }


                /* classic blending operation */
                inline void _fastblend(const RGBc64 & coul, const uint32 N, const uint32 op, uint8 & R, uint8 & G, uint8 & B)
                    {
                    const uint32 alpha = (op*coul.comp.A) / N;
                    const uint32 beta = (255 * 255) - alpha;
                    R = (beta*R + (alpha*coul.comp.R) / N) / (255 * 255);
                    G = (beta*G + (alpha*coul.comp.G) / N) / (255 * 255);
                    B = (beta*B + (alpha*coul.comp.B) / N) / (255 * 255);
                    }


                /* fast copy */
                inline void _fastcopy(const RGBc64 & coul, const uint32 N, uint8 & R, uint8 & G, uint8 & B)
                    {
                    R = coul.comp.R / N;
                    G = coul.comp.G / N;
                    B = coul.comp.B / N;
                    }


                /* different types of blitting */
                static const int BLIT_CLASSIC = 0;
                static const int BLIT_REMOVE_TRANSPARENT_WHITE = 1;
                static const int BLIT_REMOVE_TRANSPARENT_BLACK = 2;

                size_t          _width;         // width of the image
                size_t          _height;        // height of the image
                RGBc64 *        _imData;        // image buffer
                uint8 *         _normData;      // normalization buffer (_normData[i] = 0 means already normalized). 

        };




    }


/* end of file */

