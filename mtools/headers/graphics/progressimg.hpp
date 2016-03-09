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

#include "../misc/misc.hpp"
#include "../misc/error.hpp"
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

        size_t          _width;         // width of the image
        size_t          _height;        // height of the image
        RGBc64 *        _imData;        // image buffer
        uint8 *         _normData;      // normalization buffer (_normData[i] = 0 means already normalized). 

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
                const int64 lx = subBox.lx();
                const int64 ly = subBox.ly();
                const size_t pa = (size_t)(_width - (subBox.lx() + 1));
                for (int64 y = 0; y <= ly; y++)
                    {
                    for (int64 x = 0; x <= ly; x++)
                        {
                        _imData[off].normalize(_normData[off] + 1);
                        _normData[off] = 1;
                        off++;
                        }
                    off += pa;
                    }
                }


            /** Normalises the whole image. */
            void normalize()
                {
                const size_t l = _width*_height;
                for (size_t i = 0; i<l; i++)
                    {
                    _imData[i].normalize(_normData[i] + 1);
                    _normData[i] = 1;
                    }
                }


            /**
             * Raw blitting onto a Img image. 
             * 
             * Fastest method but both images must have the same size.
             **/            
            void blit(Img<unsigned char> & im)
                {
                const size_t l = (size_t)_width*_height;
                MTOOLS_ASSERT(im.width()*im.height() == l);
                unsigned char * p1 = im.data();
                unsigned char * p2 = im.data() + l;
                unsigned char * p3 = im.data() + 2*l;
                for (size_t z = 0; z < l; z++)
                    {
                    RGBc coul = _imData[z].getRGBc(_normData[z] + 1);
                    p1[z] = coul.comp.R;
                    p2[z] = coul.comp.G;
                    p3[z] = coul.comp.B;
                    }
                }


        };




    }


/* end of file */

