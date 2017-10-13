/** @file image.hpp */
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

#include "../mtools_config.hpp"
#include "../misc/misc.hpp"
#include "../misc/error.hpp"
#include "../maths/vec.hpp"
#include "../maths/box.hpp"
#include "rgbc.hpp"
#include "../io/serialization.hpp"
#include "../random/gen_fastRNG.hpp"
#include "../random/classiclaws.hpp"
#include <cairo.h>


namespace mtools
	{


	/**
	 * Class representing a true color image  
	 * 
	 * Each pixel stores a 32bit integer in RGBc format: the color are ordered: G R B A (yes...
	 * because that is what cairo want).
	 * 
	 * The pixels buffer is in row major order, starting from the upper left corner, going right and
	 * down. The image can have an optional padding at the end of each row which means that the image 
	 * stride may be larger than its width (this simplifies the management of shared sub-image).
	 * 
	 * For example, an image with dimension lx = 4 , ly = 3 and padding = 2 has a stride = 6 and the 
	 * data buffer uses 16 uint32 (ie 64 bytes). 
	 * 
	 *           image                 padding
	 *      | [ 0] [ 1] [ 2] [ 3]  |  [ 4] [ 5]
	 *      | [ 6] [ 7] [ 8] [ 9]  |  [10] [11]
	 *      | [12] [13] [14] [15]  |  
	 * 
	 * here [XX] represent the uint32 at position XX in the buffer. Note that the last line padding
	 * is optionnal and so should never be accessed. 
	 * 
	 * Just as in the CImg library, an image may be shared or not. A shared image does not manage
	 * its pixel buffer (in particular, it does not delete it when the image is deleted). Thus
	 * writing on a shared image modifies the parent image. Shared image are useful for selecting
	 * sub-images without having to allocate a new memory buffer.
	 *
	 * By default, copy and assignement operator are shallow. Thus means that the source and destination 
	 * object share the same pixel buffer.
	 *   
	 *  ************************************* SSE optimizations ************************************
	 * define MTOOLS_USE_SSE in the preprocessor options to activate these optimizations.   
	 * 
	 **/
	class Image
		{

		public:

			/**
			 * Default constructor.
			 * 
			 * 	Construct an empty image.
			 */
			Image() :	_lx(0), _ly(0), _stride(0),
						_deletepointer(nullptr), _data(nullptr),
						_pcairo_surface(nullptr), _pcairo_context(nullptr)
				{}


			/**
			 * Create a image from a file.
			 *
			 * @param	filename Name of the file (must have extension "png" or "jpg"). If the operation fails,
			 * 					 the image is empty.
			 */
			Image(const std::string & filename) : Image()
				{
				load_png(filename);
				}


			/**
			 * Constructor.
			 * 
			 * Create a new image. The pixel buffer contain undefined color.
			 *
			 * @param	lx	    width of the image.
			 * @param	ly	    height of the image.
			 * @param	padding padding at the end of lines (default = no padding).
			 */
			Image(int64 lx, int64 ly, int64 padding = 0) :  _lx(lx), _ly(ly), _stride(lx + ((padding < 0) ? 0 : padding)),
				                                            _deletepointer(nullptr), _data(nullptr), 
				                                            _pcairo_surface(nullptr), _pcairo_context(nullptr)
				{
				if ((_lx <= 0) || (_ly <= 0)) { empty(); return; }
				_allocate(_ly, _stride, nullptr);
				}


			/**
			 * Constructor.
			 * 
			 * Create a new image. The pixel buffer contain undefined color.
			 *
			 * @param	dim	   	dimensions of the image.
			 * @param	padding	padding at the end of lines (default = no padding).
			 **/
			Image(const iVec2 & dim, int64 padding = 0) : Image(dim.X(), dim.Y(),padding)
				{
				}


			/**
			 * Constructor.
			 * 
			 * Create a new image and set the background color.
			 *
			 * @param	lx	    width of the image.
			 * @param	ly	    height of the image.
			 * @param	bkColor background color.
			 * @param	padding padding at the end of lines (default = no padding).
			 */
			Image(int64 lx, int64 ly, RGBc bkColor, int64 padding = 0) : Image(lx, ly, padding)
				{
				clear(bkColor);
				}


			/**
			 * Constructor.
			 * 
			 * Create a new image and set the background color.
			 *
			 * @param	dim	   	dimensions of the image.
			 * @param	bkColor	background color.
			 * @param	padding	padding at the end of lines (default = no padding).
			 **/
			Image(const iVec2 & dim, RGBc bkColor, int64 padding = 0) : Image(dim.X(), dim.Y(), bkColor, padding)
				{
				}



			/**
			 * Constructor from a given buffer
			 * 
			 * Create an image using a given pixel buffer. If shallow = true, then THE BUFFER MUST REMAIN
			 * VALID UNTIL THE IMAGE IS DESTROYED OR REASSIGNED TO ANOTHER BUFFER. If shallow = false, the
			 * image creates a copy of the supplied buffer which may be deleted once the method returns.
			 *
			 * @param [in,out]	data	the pixel buffer.
			 * @param	lx				width of the image.
			 * @param	ly				height of the image.
			 * @param	shallow			true to use the supplied buffer directly and false to make an
			 * 							internal copy.
			 * @param	padding			padding at the end of each lines (default = no padding).
			 **/
			Image(RGBc * data, int64 lx, int64 ly, bool shallow, int64 padding = 0) : _lx(lx), _ly(ly), _stride(lx + padding),
																				      _deletepointer(nullptr), _data(nullptr), 
																		              _pcairo_surface(nullptr), _pcairo_context(nullptr)
				{
				MTOOLS_INSURE(data != nullptr);
				MTOOLS_INSURE(_lx > 0);
				MTOOLS_INSURE(_ly > 0);
				MTOOLS_INSURE(padding >= 0);
				if (shallow)
					{
					_allocate(_ly, _stride, data);
					}
				else
					{
					_allocate(_ly, _stride, nullptr);
					_blitRegion(_data, _stride, data, _stride, lx, ly);
					}
				}


			/**
			 * Constructor from a given buffer
			 * 
			 * Create an image using a given pixel buffer. If shallow = true, then THE BUFFER MUST REMAIN
			 * VALID UNTIL THE IMAGE IS DESTROYED OR REASSIGNED TO ANOTHER BUFFER. If shallow = false, the
			 * image creates a copy of the supplied buffer which may be deleted once the method returns.
			 *
			 * @param [in,out]	data	the pixel buffer.
			 * @param	dim				dimensions of the image.
			 * @param	shallow			true to use the supplied buffer directly and false to make an
			 * 							internal copy.
			 * @param	padding			padding at the end of each lines (default = no padding).
			 **/
			Image(RGBc * data, const iVec2 & dim, bool shallow, int64 padding = 0) : Image(data, dim.X(), dim.Y(), shallow, padding)
				{
				}



			/** Destructor. */
			virtual ~Image()
				{
				empty();
				}


			/**
			 * Move Constructor.
			 **/
			Image(Image && source) : _lx(source._lx), _ly(source._ly), _stride(source._stride),
									 _deletepointer(source._deletepointer), _data(source._data),
									 _pcairo_surface(source._pcairo_surface), _pcairo_context(source._pcairo_context)				
				{
				source._lx = 0;
				source._ly = 0;
				source._stride = 0;
				source._deletepointer = nullptr;
				source._data = nullptr;
				source._pcairo_surface = nullptr;
				source._pcairo_context = nullptr;
				}


			/**
			 * Shallow copy constructor: the pixel buffer is shared with the source image. 
			 * 
			 * To create an independent image, use the extended copy ctor or the get_standalone() method. 
			 *
			 * @param	source Source image.
			 */
			Image(const Image & source) : Image(source, 0, 0, source._lx, source._ly, true)
				{
				}


			/**
			 * Copy Constructor, either deep or shallow.
			 *
			 * @param	source  Source image.
			 * @param	shallow true to make a shallow copy of the object that shares the same pixel buffer
			 * 					and false to make a deep copy with its own pixel buffer.
			 * @param	padding padding of the new image (ignored if shallow is true since the padding must
			 * 					be the same as the father image).
			 */
			Image(const Image & source, bool shallow, int64 padding = 0) : Image(source, 0, 0, source._lx, source._ly, shallow, padding)
				{
				}


			/**
			 * Create a sub-image, either deep or shallow.
			 *
			 * @param	source  Source image.
			 * @param	x0	    x-coord of the upper left point.
			 * @param	y0	    y-coord of the upper left point.
			 * @param	newlx   width of the sub-image.
			 * @param	newly   heigth of the sub-image.
			 * @param	shallow true to make a shallow copy that shares the same pixel buffer and false to
			 * 					make an independent image with its own pixel buffer.
			 * @param	padding padding of the sub-image (ignored if shallow is true since the padding it then
			 * 					constrained by the source padding and the size of the sub-image).
			 */
			Image(const Image & source, int64 x0, int64 y0, int64 newlx, int64 newly, bool shallow, int64 padding = 0) :
								_lx(newlx), _ly(newly), _stride(shallow ? source._stride : (newlx + ((padding >= 0) ? padding : 0))),
								_deletepointer(nullptr), _data(nullptr),
								_pcairo_surface(nullptr), _pcairo_context(nullptr)
				{
				MTOOLS_INSURE((newlx >= 0) && (newly >= 0));
				if((newlx*newly == 0) || (source._data == nullptr)) { empty(); return; }
				MTOOLS_INSURE((x0 >= 0) && (x0 + newlx <= source._lx));
				MTOOLS_INSURE((y0 >= 0) && (y0 + newly <= source._ly));				
				RGBc * p = source._data + source._stride*y0 + x0;
				if (shallow)
					{
					_shallow_copy(source._deletepointer, p);
					}
				else
					{
					_allocate(_ly, _stride, nullptr);
					_blitRegion(_data, _stride, p, source._stride, _lx, _ly);
					}
				}


			/**
			 * Create a sub-image, either deep or shallow.
			 *
			 * @param	source 	Source image.
			 * @param	B	   	The (closed) portion of the image to use.
			 * @param	shallow	true to make a shallow copy that shares the same pixel buffer and false to
			 * 					make an independent image with its own pixel buffer.
			 * @param	padding	padding of the sub-image (ignored if shallow is true since the padding it
			 * 					then constrained by the source padding and the size of the sub-image).
			 **/
			Image(const Image & source, const iBox2 & B, bool shallow, int64 padding = 0) : Image(source, B.min[0], B.min[1], B.max[0] - B.min[0] + 1, B.max[1] - B.min[1] + 1, shallow, padding)
				{
				}


			/**
			 * Make the image standalone by recreating the pixel buffer if need be. After this method, no
			 * other image share the same pixel buffer with this one.
			 *
			 * @param	padding The new padding (only used if the pixel buffer is really re-created, if the
			 * 					image was already standalone, the current padding is kept).
			 *
			 * @return	true if the buffer was re-created and false if the image was already standalone.
			 */
			bool standalone(int64 padding = 0)
				{
				if (!isShared()) return false;
				*this = get_standalone(padding); // use move assignement operator. 
				return true;
				}


			/**
			 * Return a deep copy of the object with its own pixel buffer.
			 *
			 * @param	padding padding for the returned image.
			 *
			 * @return	A copy of the image which does not share its pixel buffer with anyone.
			 */
			Image get_standalone(int64 padding = 0) const
				{
				return Image(*this,false,padding); // ctor + move operator or in place contruction. 
				}


			/**
			 * Move assignment operator.
			 **/
			Image & operator=(Image && source)
				{
				if (this != &source)
					{
					empty();
					_lx = source._lx;
					_ly = source._ly;
					_stride = source._stride;
					_data = source._data;
					_deletepointer = source._deletepointer;
					_pcairo_surface = source._pcairo_surface;
					_pcairo_context = source._pcairo_context;
					source._lx = 0;
					source._ly = 0;
					source._stride = 0;
					source._data = nullptr;
					source._deletepointer = nullptr;
					source._pcairo_context = nullptr;
					source._pcairo_surface = nullptr;
					}
				return *this;
				}


			/**
			 * Shallow assignment operator. Make a copy of the image that shares the same pixel buffer as the source.
			 */
			Image & operator=(const Image & source)
				{
				if (this != &source)
					{
					empty();
					if (source._data == nullptr) return (*this);
					_lx = source._lx;
					_ly = source._ly;
					_stride = source._stride;
					_shallow_copy(source._deletepointer, source._data);
					}
				return *this;
				}


			/**
			 * Equality operator.
			 *
			 * @param	im The image to compare against.
			 *
			 * @return	true if the images are visually equivalent: they have the same size (lx, ly) and the
			 * 			same pixel color (but the padding may differ).
			 */
			bool operator==(const Image & im) const
				{
				if ((_lx != im._lx) || (_ly != im._ly)) return false;
				if ((_data == nullptr)|| (_data == im._data)) return true;
				for (int64 j = 0; j < _ly; j++)
					{
					if (memcmp(_data + j*_stride, im._data + j*im._stride, _lx * 4) != 0) return false;
					}
				return true;
				}


			/**
			 * Crop the image.
			 * 
			 * Note: The method is fast when shallow is true (but this will change the image padding).
			 *       When shallow is false, a new pixel buffer is created.
			 *
			 * @param	x0	    x-coord of the upper left point.
			 * @param	y0	    y-coord of the upper left point.
			 * @param	newlx   new width.
			 * @param	newly   new heigth.
			 * @param	shallow True to keep the same pixel buffer (with a different padding) and false to
			 * 					create a new pixel buffer with the correct size.
			 * @param	padding new padding of the image (ignored if shallow = true).
			 */
			void crop(int64 x0, int64 y0, int64 newlx, int64 newly, bool shallow, int64 padding = 0)
				{
				if ((newlx <= 0)||(newly <= 0)) { empty(); return; }
				MTOOLS_INSURE((x0 >= 0) && (x0 + newlx <= _lx));
				MTOOLS_INSURE((y0 >= 0) && (y0 + newly <= _ly));
				if (shallow)
					{
					_removecairo();
					_lx = newlx;
					_ly = newly;
					_data +=  (_stride*y0) + x0;
					return;
					}
				(*this) = get_crop(x0, y0, newlx, newly, false, padding);
				}


			/**
			 * Crop the image.
			 * 
			 * Note: The method is fast when shallow is true (but this will change the image padding).
			 *       When shallow is false, a new pixel buffer is created.
			 *
			 * @param	B	   	The (closed) portion of the image to keep.
			 * @param	shallow	True to keep the same pixel buffer (with a different padding) and false to
			 * 					create a new pixel buffer with the correct size.
			 * @param	padding	new padding of the image (ignored if shallow = true).
			 **/
			void crop(const iBox2 & B, bool shallow, int64 padding = 0)
				{
				crop(B.min[0], B.min[1], B.max[0] - B.min[0] + 1, B.max[1] - B.min[1] + 1, shallow, padding);
				}


			/**
			 * Create a sub-image (deep or shallow).
			 * 
			 *    Image im2 = im1.get_crop(x,y,lx,ly,shallow,padding);
			 * 
			 * is equivalent to
			 * 
			 *    Image im2(im1,x,y,lx,ly,shallow,padding);
			 *
			 * @param	x0	    x-coord of the upper left point.
			 * @param	y0	    y-coord of the upper left point.
			 * @param	newlx   width of the sub-image.
			 * @param	newly   heigth of the sub-image.
			 * @param	shallow true to keep the same pixel buffer (with a different padding) and false to
			 * 					create a new pixel buffer.
			 * @param	padding padding of the resulting image (ignored if shallow is true since the padding
			 * 					is constrained).
			 *
			 * @return	a sub image.
			 */
			Image get_crop(int64 x0, int64 y0, int64 newlx, int64 newly, bool shallow, int64 padding = 0) const
				{
				return Image(*this, x0, y0, newlx, newly, shallow, padding);
				}


			/**
			 * Create a sub-image (deep or shallow).
			 * 
			 *    Image im2 = im1.get_crop(B,shallow,padding);
			 * 
			 * is equivalent to
			 * 
			 *    Image im2(im1,B,shallow,padding);
			 *
			 * @param	B	   	The (closed) portion of the image to keep.
			 * @param	shallow	true to keep the same pixel buffer (with a different padding) and false to
			 * 					create a new pixel buffer.
			 * @param	padding	padding of the resulting image (ignored if shallow is true since the padding
			 * 					is constrained).
			 *
			 * @return	a sub image.
			 **/
			Image get_crop(const iBox2 & B, bool shallow, int64 padding = 0) const
				{
				return Image(*this, B, shallow, padding);
				}


			/**
			 * Crop the image by reducing the size of each border by a given amount.
			 * 
			 * Note: The method is very fast when shallow is true (but this will change the image padding).
			 *       When shallow is false, a new pixel buffer is created.
			 *
			 * @param	left    reduction of the left border (0 or negative means no change).
			 * @param	right   reduction of the right border (0 or negative means no change).
			 * @param	up	    reduction of the top border (0 or negative means no change).
			 * @param	down    reduction of the bottom border (0 or negative means no change).
			 * @param	shallow true to keep the same pixel buffer (with a different padding) and false to
			 * 					create a new pixel buffer.
			 * @param	padding padding of the resulting image (ignored for a shared image since the padding
			 * 					is contrained).
			 */
			void cropBorder(int64 left, int64 right, int64 up, int64 down, bool shallow, int64 padding = 0)
				{
				int64 x0 = 0, y0 = 0, sx = _lx, sy = _ly;
				if (left > 0) { x0 = left; sx -= left; }
				if (right > 0) { sx -= right; }
				if (up > 0) { y0 = up; sy -= up; }
				if (down > 0) { sy -= down; }
				if ((x0 >= _lx) || (y0 >= _ly) || (sx <= 0) || (sy <= 0)) { empty(); return;  }
				crop(x0, y0, sx, sy, shallow, padding);
				}


			/**
			 * Create a sub image (deep or shallow) obtained by reducing the size of each border by a given
			 * amount.
			 *
			 * @param	left    reduction of the left border (0 or negative means no change).
			 * @param	right   reduction of the right border (0 or negative means no change).
			 * @param	up	    reduction of the top border (0 or negative means no change).
			 * @param	down    reduction of the bottom border (0 or negative means no change).
			 * @param	shallow true to keep the same pixel buffer (with a different padding) and false to
			 * 					create a new pixel buffer.
			 * @param	padding padding of the resulting image (ignored for a shared image since the padding
			 * 					is contrained).
			 */
			Image get_cropBorder(int64 left, int64 right, int64 up, int64 down, bool shallow, int64 padding = 0) const
				{
				int64 x0 = 0, y0 = 0, sx = _lx, sy = _ly;
				if (left > 0) { x0 = left; sx -= left; }
				if (right > 0) { sx -= right; }
				if (up > 0) { y0 = up; sy -= up; }
				if (down > 0) { sy -= down; }
				if ((x0 >= _lx) || (y0 >= _ly) || (sx <= 0) || (sy <= 0)) { return Image();  }
				return Image(*this,x0, y0, sx, sy, shallow, padding);
				}


			/**
			 * Create a shallow sub-image (ie using the same pixel buffer).
			 * 
			 * Same as:
			 * 
			 *         get_crop(x0,y0,newlx,newly,true);
			 *
			 * This method is fast since no allocation is performed. It can be used to  
			 * work only on a portion of the image.
			 *
			 * @param	x0    x-coord of the upper left point.
			 * @param	y0    y-coord of the upper left point.
			 * @param	newlx width of the sub-image.
			 * @param	newly heigth of the sub-image.
			 *
			 * @return	a shared sub-image.
			 */
			Image sub_image(int64 x0, int64 y0, int64 newlx, int64 newly) const
				{
				return get_crop(x0, y0, newlx, newly, true);
				}


			/**
			 * Create a shallow sub-image (ie using the same pixel buffer).
			 * 
			 * Same as:
			 * 
			 *         get_crop(B,true);
			 *
			 * @param	B	The (closed) rectangle delimitying the portion to keep.
			 *
			 * @return	a shared sub-image.
			 **/
			Image sub_image(const iBox2 & B) const
				{
				return get_crop(B,true);
				}


			/**
			 * Swaps the content of the two images. Very fast. 
			 *
			 * @param [in,out]	im	The image to swap with
			 **/
			void swap(Image & im)
				{
				if (&im != this)
					{					
					mtools::swap<int64>(_lx, im._lx);
					mtools::swap<int64>(_ly, im._ly);
					mtools::swap<int64>(_stride, im._stride);
					mtools::swap<RGBc*>(_data, im._data);
					mtools::swap<uint32*>(_deletepointer, im._deletepointer);
					mtools::swap<void*>(_pcairo_surface, im._pcairo_surface);
					mtools::swap<void*>(_pcairo_context, im._pcairo_context);
					}
				}


			/**
			 * Expands the border of an image and set a given color for the new pixels.
			 * 
			 * This methods will always recreate the pixel buffer if the image is shared so the resulting
			 * image is guaranteed to have exclusive access to its pixel buffer (isShared() return false).
			 *
			 * @param	left    extension of the left border (0 or negative means no change).
			 * @param	right   extension of the right border (0 or negative means no change).
			 * @param	up	    reduction of the top border (0 or negative means no change).
			 * @param	down    extension of the bottom border (0 or negative means no change).
			 * @param	bkcolor Color to use for new pixels.
			 * @param	padding padding for the resulting image.
			 */
			void expand(int64 left, int64 right, int64 up, int64 down, RGBc bkcolor = RGBc::c_TransparentWhite, int64 padding = 0)
				{
				if (left < 0) { left = 0; }
				if (right < 0) { right = 0; }
				if (up < 0) { up = 0; }
				if (down < 0) { down = 0; }
				if ((left + right + down + up == 0) && (!isShared())) return;
				// otherwise, we recreate the buffer, even if no extension really take place
				(*this) = get_expand(left, right, up, down, bkcolor, padding);
				}


			/**
			 * Return an image obtained by expanding the border (and set a given color for the new pixels).
			 *
			 * The resulting image is never shared (it has its own pixel buffer). 
			 *
			 * @param	left   	extension of the left border (0 or negative means no change).
			 * @param	right  	extension of the right border (0 or negative means no change).
			 * @param	up	   	reduction of the top border (0 or negative means no change).
			 * @param	down   	extension of the bottom border (0 or negative means no change).
			 * @param	bkcolor	Color to use for new pixels.
			 * @param	padding	padding for the resulting image.
			 **/
			Image get_expand(int64 left, int64 right, int64 up, int64 down, RGBc bkcolor = RGBc::c_TransparentWhite, int64 padding = 0) const
				{
				if (left < 0) { left = 0; }
				if (right < 0) { right = 0; }
				if (up < 0) { up = 0; }
				if (down < 0) { down = 0; }
				if (up + down + left + right == 0) return Image(*this, false, padding);
				Image im(_lx + left + right, _ly + up + down, bkcolor, padding);
				im.blit(*this, left, up);
				return im;
				}


			/**
			 * Resize the image. Raw operation on the allocated memory. If the new buffer is smaller than
			 * the current one and shrinktofit is false, no new allocation is perfomed (except if the new
			 * size is zero in which case the allocated memory is freed). If shrink tofit is true, allocated
			 * memory is freed and a new buffer with the right size is allocated.
			 *
			 * @param	newlx	   	The new width.
			 * @param	newly	   	The new height.
			 * @param	shrinktofit	false to keep the current buffer if large enough and true to free and
			 * 						reallocate.
			 * @param	padding	   	The new padding (default 0).
			 **/
			void resizeRaw(int64 newlx, int64 newly, bool shrinktofit = false, int64 padding = 0)
				{
				if ((newlx <= 0) || (newly <= 0)) { empty(); return; }
				if (padding <= 0) { padding = 0; }
				if (!shrinktofit)
					{ // we try to keep the same buffer
					int64 newstride = (newlx + padding);
					if (newstride*newly <= _stride*_ly)
						{ // ok: the buffer and the temporary buffer are large enough 
						_removecairo();
						_lx = newlx;
						_ly = newly;
						_stride = newstride;
						return;
						}
					}
				// we must deallocate and recreate the buffer with the right size.
				*this = Image(newlx, newly, padding);
				}


			/**
			 * Resize the image. Raw operation on the allocated memory. If the new buffer is smaller than
			 * the current one and shrinktofit is false, no new allocation is perfomed (except if the new
			 * size is zero in which case the allocated memory is freed). If shrink tofit is true, allocated
			 * memory is freed and a new buffer with the right size is allocated.
			 *
			 * @param	newdim	   	The new dimensions.
			 * @param	shrinktofit	false to keep the current buffer if large enough and true to free and
			 * 						reallocate.
			 * @param	padding	   	The new padding (default 0).
			 *
			 * ### param	newly	The new height.
			 **/
			void resizeRaw(const iVec2 & newdim, bool shrinktofit = false, int64 padding = 0)
				{
				resizeRaw(newdim.X(), newdim.Y(), shrinktofit, padding);
				}


			/**
			 * Blit (part of) a sprite image.
			 *
			 * All input parameters are valid : regions outside of the source or destination
			 * image are automatically discarded (considered transparent).
			 *
			 * @param	sprite  	The sprite to blit.
			 * @param	dest_x  	x-coord of the upper left corner in the destination.
			 * @param	dest_y  	y-coord of the upper left corner in the destination.
			 * @param	sprite_x	x-coord of the upper left corner in the sprite.
			 * @param	sprite_y	y-coord of the upper left corner in the sprite.
			 * @param	sx			width of the part of the sprite to blit.
			 * @param	sy			height of the part of the sprite to blit
			 **/
			void blit(const Image & sprite, int64 dest_x, int64 dest_y, int64 sprite_x, int64 sprite_y, int64 sx, int64 sy)
				{
				if (sprite_x < 0) { dest_x -= sprite_x; sx += sprite_x; sprite_x = 0; }
				if (sprite_y < 0) { dest_y -= sprite_y; sy += sprite_y; sprite_y = 0; }
				if (dest_x < 0) { sprite_x -= dest_x;   sx += dest_x; dest_x = 0; }
				if (dest_y < 0) { sprite_y -= dest_y;   sy += dest_y; dest_y = 0; }
				if ((dest_x >= _lx) || (dest_y >= _ly) || (sprite_x >= sprite._lx) || (sprite_x >= sprite._ly)) return;
				sx -= std::max<int64>(0, (dest_x + sx - _lx));
				sy -= std::max<int64>(0, (dest_y + sy - _ly));
				sx -= std::max<int64>(0, (sprite_x + sx - sprite._lx));
				sy -= std::max<int64>(0, (sprite_y + sy - sprite._ly));
				if ((sx <= 0) || (sy <= 0)) return;
				//pixman_blt((uint32_t *)sprite._data, (uint32_t *)_data, sprite._stride , _stride , 32, 32, sprite_x, sprite_y, dest_x, dest_y, sx, sy);  // supper than the blit function below, (strange...)
				_blitRegion(_data + (dest_y*_stride) + dest_x, _stride, sprite._data + (sprite_y*_stride) + sprite_x, sprite._stride, sx, sy);
				}


			/**
			 * Blit (part of) a sprite image.
			 * 
			 * All input parameters are valid : regions outside of the source or destination image are
			 * automatically discarded (considered transparent).
			 *
			 * @param	sprite	  	The sprite to blit.
			 * @param	dest_posd 	position of the upper left corner in the destination.
			 * @param	sprite_box	sub-image of the sprite to blit.
			 **/
			void blit(const Image & sprite, const iVec2 & dest_pos, const iBox2 & sprite_box)
				{
				blit(sprite, dest_pos.X(), dest_pos.Y(), sprite_box.min[0], sprite_box.min[1], sprite_box.max[0] - sprite_box.min[0] + 1, sprite_box.max[1] - sprite_box.min[1] + 1);
				}


			/**
			 * Blit a sprite.
			 * 
			 * All input parameters are valid : regions outside of the destination are automatically
			 * discarded (considered transparent).
			 *
			 * @param	sprite	The sprite to blit.
			 * @param	dext_x	x-coord of the upper left corner in the destination.
			 * @param	dest_y	y-coord of the upper left corner in the destination.
			 **/
			void blit(const Image & sprite, int64 dest_x, int64 dest_y)
				{
				blit(sprite, dest_x, dest_y, 0, 0, sprite._lx, sprite._ly);
				}


			/**
			 * Blit a sprite.
			 * 
			 * All input parameters are valid : regions outside of the destination are automatically
			 * discarded (considered transparent).
			 *
			 * @param	sprite  	The sprite to blit.
			 * @param	dest_pos	position of the upper left corner in the destination.
			 **/
			void blit(const Image & sprite, const iVec2 & dest_pos)
				{
				blit(sprite, dest_pos.X(), dest_pos.Y(), 0, 0, sprite._lx, sprite._ly);
				}



			/**
			 * Blit part of the image onto itself. Works even if the rectangle are overlapping.
			 *
			 * All input paramters are valid : regions outside of the source or destination
			 * image are automatically discarded (considered transparent).
			 *
			 * @param	dest_x	x-coord of the upper left corner in the destination
			 * @param	dest_y	y-coord of the upper left corner in the destination.
			 * @param	src_x 	x-coord of the upper left corner in the sprite.
			 * @param	src_y 	y-coord of the upper left corner in the sprite.
			 * @param	sx	  	width of the rectangle to blit.
			 * @param	sy	  	height of the rectangle to blit.
			 **/
			void blitInside(int64 dest_x, int64 dest_y, int64 src_x, int64 src_y,int64 sx, int64 sy)
				{
				if ((dest_x == src_x) && (dest_y == src_y)) return;
				if (src_x < 0) { dest_x -= src_x; sx += src_x; src_x = 0; }
				if (src_y < 0) { dest_y -= src_y; sy += src_y; src_y = 0; }
				if (dest_x < 0) { src_x -= dest_x;   sx += dest_x; dest_x = 0; }
				if (dest_y < 0) { src_y -= dest_y;   sy += dest_y; dest_y = 0; }
				if ((dest_x >= _lx) || (dest_y >= _ly) || (src_x >= _lx) || (src_x >= _ly)) return;
				sx -= std::max<int64>(0, (dest_x + sx - _lx));
				sy -= std::max<int64>(0, (dest_y + sy - _ly));
				sx -= std::max<int64>(0, (src_x + sx - _lx));
				sy -= std::max<int64>(0, (src_y + sy - _ly));
				if ((sx <= 0) || (sy <= 0)) return;
				if (((dest_x >= src_x) && (dest_x < src_x + sx)) && ((dest_y >= src_y) && (dest_y < src_y + sy))) { _blitRegionDown(_data + (dest_y*_stride) + dest_x, _stride, _data + (src_y*_stride) + src_x, _stride, sx, sy); return; } // overlap down
				if (((src_x >= dest_x) && (src_x < dest_x + sx)) && ((src_y >= dest_y) && (src_y < dest_y + sy))) { _blitRegionUp(_data + (dest_y*_stride) + dest_x, _stride, _data + (src_y*_stride) + src_x, _stride, sx, sy); return; } // overlap up
				_blitRegion(_data + (dest_y*_stride) + dest_x, _stride,  _data + (src_y*_stride) + src_x, _stride, sx, sy); // no overlap, use fast blitting
				}


			/**
			 * Blit part of the image onto itself. Works even if the rectangle are overlapping.
			 * 
			 * All input paramters are valid : regions outside of the source or destination image are
			 * automatically discarded (considered transparent).
			 *
			 * @param	dest_pos	position of the upper left corner of the destination rectangle.
			 * @param	src_box 	the source rectangle.
			 **/
			void blitInside(const iVec2 & dest_pos, const iBox2 & src_box)
				{
				blitInside(dest_pos.X(), dest_pos.Y(), src_box.min[0], src_box.min[1], src_box.max[0] - src_box.min[0] + 1, src_box.max[1] - src_box.min[1] + 1);
				}



			/**
			 * Blend (part of) a sprite image.
			 * 
			 * All input paramters are valid : regions outside of the source or destination image are
			 * automatically discarded (considered transparent).
			 *
			 * @param	sprite  	The sprite to blend.
			 * @param	dest_x  	x-coord of the upper left corner in the destination.
			 * @param	dest_y  	y-coord of the upper left corner in the destination.
			 * @param	sprite_x	x-coord of the upper left corner in the sprite.
			 * @param	sprite_y	y-coord of the upper left corner in the sprite.
			 * @param	sx			width of the part of the sprite to blit.
			 * @param	sy			height of the part of the sprite to blit.
			 * @param	opacity 	The opacity to multiply the sprite with when blending.
			 **/
			void blend(const Image & sprite, int64 dest_x, int64 dest_y, int64 sprite_x, int64 sprite_y, int64 sx, int64 sy, float opacity = 1.0f)
				{
				if (sprite_x < 0) { dest_x -= sprite_x; sx += sprite_x; sprite_x = 0; }
				if (sprite_y < 0) { dest_y -= sprite_y; sy += sprite_y; sprite_y = 0; }
				if (dest_x < 0) { sprite_x -= dest_x;   sx += dest_x; dest_x = 0; }
				if (dest_y < 0) { sprite_y -= dest_y;   sy += dest_y; dest_y = 0; }
				if ((dest_x >= _lx) || (dest_y >= _ly) || (sprite_x >= sprite._lx) || (sprite_x >= sprite._ly)) return;
				sx -= std::max<int64>(0, (dest_x + sx - _lx));
				sy -= std::max<int64>(0, (dest_y + sy - _ly));
				sx -= std::max<int64>(0, (sprite_x + sx - sprite._lx));
				sy -= std::max<int64>(0, (sprite_y + sy - sprite._ly));
				if ((sx <= 0) || (sy <= 0)) return;
				_blendRegionUp(_data + (dest_y*_stride) + dest_x, _stride, sprite._data + (sprite_y*_stride) + sprite_x, sprite._stride, sx, sy,opacity);
				}


			/**
			* Blend (part of) a sprite image.
			*
			* All input paramters are valid : regions outside of the source or destination image are
			* automatically discarded (considered transparent).
			*
			* @param	sprite  	The sprite to blend.
			* @param	dest_pos	position of the upper left corner of the destination rectangle.
			* @param	sprite_box 	the source rectangle.
			* @param	opacity 	The opacity to multiply the sprite with when blending.
			**/
			void blend(const Image & sprite, const iVec2 & dest_pos, const iBox2 & sprite_box, float opacity = 1.0f)
				{
				blend(sprite, dest_pos.X(), dest_pos.Y(), sprite_box.min[0], sprite_box.min[1], sprite_box.max[0] - sprite_box.min[0] + 1, sprite_box.max[1] - sprite_box.min[1] + 1, opacity);
				}


			/**
			 * Blend a sprite.
			 * 
			 * All input parameters are valid : regions outside of the destination are automatically
			 * discarded (considered transparent).
			 *
			 * @param	sprite 	The sprite to blit.
			 * @param	dext_x 	x-coord of the upper left corner in the destination.
			 * @param	dest_y 	y-coord of the upper left corner in the destination.
			 * @param	opacity	The opacity to multiply the sprite with when blending.
			 **/
			void blend(const Image & sprite, int64 dext_x, int64 dest_y, float opacity= 1.0f)
				{
				blend(sprite, dext_x, dest_y, 0, 0, sprite._lx, sprite._ly,opacity);
				}


			/**
			* Blend a sprite.
			*
			* All input parameters are valid : regions outside of the destination are automatically
			* discarded (considered transparent).
			*
			* @param	sprite 	The sprite to blit.
			* @param	dest_pos	position of the upper left corner of the destination rectangle.
			* @param	opacity	The opacity to multiply the sprite with when blending.
			**/
			void blend(const Image & sprite, const iVec2 & dest_pos, float opacity = 1.0f)
				{
				blend(sprite, dest_pos.X(), dest_pos.Y(), 0, 0, sprite._lx, sprite._ly, opacity);
				}


			/**
			* Blend part of the image onto itself. Works even if the rectangle are overlapping.
			*
			* All input paramters are valid : regions outside of the source or destination
			* image are automatically discarded (considered transparent).
			*
			* @param	dest_x	x-coord of the upper left corner in the destination
			* @param	dest_y	y-coord of the upper left corner in the destination.
			* @param	src_x 	x-coord of the upper left corner in the sprite.
			* @param	src_y 	y-coord of the upper left corner in the sprite.
			* @param	sx	  	width of the rectangle to blit.
			* @param	sy	  	height of the rectangle to blit.
			* @param	opacity	The opacity to multiply the sprite with when blending.
			**/
			void blendInside(int64 dest_x, int64 dest_y, int64 src_x, int64 src_y, int64 sx, int64 sy, float opacity = 1.0f)
				{
				if ((dest_x == src_x) && (dest_y == src_y)) return;
				if (src_x < 0) { dest_x -= src_x; sx += src_x; src_x = 0; }
				if (src_y < 0) { dest_y -= src_y; sy += src_y; src_y = 0; }
				if (dest_x < 0) { src_x -= dest_x;   sx += dest_x; dest_x = 0; }
				if (dest_y < 0) { src_y -= dest_y;   sy += dest_y; dest_y = 0; }
				if ((dest_x >= _lx) || (dest_y >= _ly) || (src_x >= _lx) || (src_x >= _ly)) return;
				sx -= std::max<int64>(0, (dest_x + sx - _lx));
				sy -= std::max<int64>(0, (dest_y + sy - _ly));
				sx -= std::max<int64>(0, (src_x + sx - _lx));
				sy -= std::max<int64>(0, (src_y + sy - _ly));
				if ((sx <= 0) || (sy <= 0)) return;
				if (((dest_x >= src_x) && (dest_x < src_x + sx)) && ((dest_y >= src_y) && (dest_y < src_y + sy))) { _blendRegionDown(_data + (dest_y*_stride) + dest_x, _stride, _data + (src_y*_stride) + src_x, _stride, sx, sy,opacity); return; }
				_blendRegionUp(_data + (dest_y*_stride) + dest_x, _stride, _data + (src_y*_stride) + src_x, _stride, sx, sy,opacity);
				}


			/**
			* Blend part of the image onto itself. Works even if the rectangle are overlapping.
			*
			* All input paramters are valid : regions outside of the source or destination
			* image are automatically discarded (considered transparent).
			*
			* @param	dest_pos	position of the upper left corner of the destination rectangle.
			* @param	src_box 	the source rectangle.
			* @param	opacity	The opacity to multiply the sprite with when blending.
			**/
			void blendInside(const iVec2 & dest_pos, const iBox2 & src_box, float opacity = 1.0f)
				{
				blendInside(dest_pos.X(), dest_pos.Y(), src_box.min[0], src_box.min[1], src_box.max[0] - src_box.min[0] + 1, src_box.max[1] - src_box.min[1] + 1, opacity);
				}


			/**
			 * Apply a mask given by (part of) a sprite image.
			 * 
			 * This operation is the same as blending (part of) the sprite onto the image except that only
			 * the alpha channel of the sprite is used. Its RGB color is discarded and replaced by that
			 * supplied as the input parameter color. [the alpha channel of the input color is also
			 * multiplied by that of the sprite before blending].
			 * 
			 * All input paramters are valid : regions outside of the source or destination image are
			 * automatically discarded (considered transparent).
			 *
			 * @param	sprite  	The sprite to blend.
			 * @param	dest_x  	x-coord of the upper left corner in the destination.
			 * @param	dest_y  	y-coord of the upper left corner in the destination.
			 * @param	sprite_x	x-coord of the upper left corner in the sprite.
			 * @param	sprite_y	y-coord of the upper left corner in the sprite.
			 * @param	sx			width of the part of the sprite to blit.
			 * @param	sy			height of the part of the sprite to blit.
			 * @param	color   	the color to use for the mask.
			 **/
			void mask(const Image & sprite, int64 dest_x, int64 dest_y, int64 sprite_x, int64 sprite_y, int64 sx, int64 sy, RGBc color)
				{
				if (sprite_x < 0) { dest_x -= sprite_x; sx += sprite_x; sprite_x = 0; }
				if (sprite_y < 0) { dest_y -= sprite_y; sy += sprite_y; sprite_y = 0; }
				if (dest_x < 0) { sprite_x -= dest_x;   sx += dest_x; dest_x = 0; }
				if (dest_y < 0) { sprite_y -= dest_y;   sy += dest_y; dest_y = 0; }
				if ((dest_x >= _lx) || (dest_y >= _ly) || (sprite_x >= sprite._lx) || (sprite_x >= sprite._ly)) return;
				sx -= std::max<int64>(0, (dest_x + sx - _lx));
				sy -= std::max<int64>(0, (dest_y + sy - _ly));
				sx -= std::max<int64>(0, (sprite_x + sx - sprite._lx));
				sy -= std::max<int64>(0, (sprite_y + sy - sprite._ly));
				if ((sx <= 0) || (sy <= 0)) return;
				_maskRegion(_data + (dest_y*_stride) + dest_x, _stride, sprite._data + (sprite_y*_stride) + sprite_x, sprite._stride, sx, sy, color);
				}


			/**
			 * Apply a mask given by (part of) a sprite image.
			 * 
			 * This operation is the same as blending (part of) the sprite onto the image except that only
			 * the alpha channel of the sprite is used. Its RGB color is discarded and replaced by that
			 * supplied as the input parameter color. [the alpha channel of the input color is also
			 * multiplied by that of the sprite before blending].
			 * 
			 * All input paramters are valid : regions outside of the source or destination image are
			 * automatically discarded (considered transparent).
			 *
			 * @param	sprite	  	The sprite to blend.
			 * @param	dest_pos  	position of the upper left corner of the destination rectangle.
			 * @param	sprite_box	the source rectangle.
			 * @param	color	  	the color to use for the mask.
			 **/
			void mask(const Image & sprite, const iVec2 & dest_pos, const iBox2 & sprite_box, RGBc color)
				{
				mask(sprite, dest_pos.X(), dest_pos.Y(), sprite_box.min[0], sprite_box.min[1], sprite_box.max[0] - sprite_box.min[0] + 1, sprite_box.max[1] - sprite_box.min[1] + 1, color);
				}


			/**
			* Apply a mask given by a sprite image.
			*
			* This operation is the same as blending (part of) the sprite onto the image except that only
			* the alpha channel of the sprite is used. Its RGB color is discarded and replaced by that
			* supplied as the input parameter color. [the alpha channel of the input color is also
			* multiplied by that of the sprite before blending].
			*
			* All input paramters are valid : regions outside of the destination are automatically
			* discarded (considered transparent).
			*
			* @param	sprite 	The sprite to blit.
			* @param	dext_x 	x-coord of the upper left corner in the destination.
			* @param	dest_y 	y-coord of the upper left corner in the destination.
			* @param	color   	the color to use for the mask.
			**/
			void mask(const Image & sprite, int64 dext_x, int64 dest_y, RGBc color)
				{
				mask(sprite, dext_x, dest_y, 0, 0, sprite._lx, sprite._ly, color);
				}


			/**
			 * Apply a mask given by a sprite image.
			 * 
			 * This operation is the same as blending (part of) the sprite onto the image except that only
			 * the alpha channel of the sprite is used. Its RGB color is discarded and replaced by that
			 * supplied as the input parameter color. [the alpha channel of the input color is also
			 * multiplied by that of the sprite before blending].
			 * 
			 * All input paramters are valid : regions outside of the destination are automatically
			 * discarded (considered transparent).
			 *
			 * @param	sprite  	The sprite to blit.
			 * @param	dest_pos	position of the upper left corner of the destination rectangle.
			 * @param	color   	the color to use for the mask.
			 **/
			void mask(const Image & sprite, const iVec2 & dest_pos, RGBc color)
				{
				mask(sprite, dest_pos.X(), dest_pos.Y(), 0, 0, sprite._lx, sprite._ly, color);
				}


			/**
			 * Rescale this image to a given size.
			 * 
			 * This method discard the current data buffer an create another one.
			 *
			 * @param	quality   	0 for (fast) low quality and 1 for (slow) high quality rescaling.
			 * @param	newlx	  	new image width.
			 * @param	newly	  	new image height.
			 * @param	newpadding	horizontal padding.
			 **/
			inline void rescale(int quality, int64 newlx, int64 newly, int64 newpadding = 0)
				{
				rescale(quality, newlx, newly, 0, 0, _lx, _ly, newpadding);
				}


			/**
			 * Rescale this image to a given size.
			 * 
			 * This method discard the current data buffer an create another one.
			 *
			 * @param	quality   	0 for (fast) low quality and 1 for (slow) high quality rescaling.
			 * @param	newsize   	new image size. 
			 * @param	newpadding	horizontal padding.
			 **/
			inline void rescale(int quality, const iVec2 & newsize, int64 newpadding = 0)
				{
				rescale(quality, newsize.X(), newsize.Y(), 0, 0, _lx, _ly, newpadding);
				}


			/**
			 * Crop a portion of this image and rescale it to a given size.
			 * 
			 * This method discard the current data buffer an create another one.
			 *
			 * @param	quality   	0 for (fast) low quality and 1 for (slow) high quality rescaling.
			 * @param	newlx	  	new image width.
			 * @param	newly	  	new image height.
			 * @param	x		  	x-coord of the upper left corner of the rectangle to crop.
			 * @param	y		  	y-coord of the upper left corner of the rectangle to crop.
			 * @param	sx		  	width of the rectangle to crop.
			 * @param	sy		  	height of the rectangle to crop.
			 * @param	newpadding	horizontal padding.
			 **/
			inline void rescale(int quality, int64 newlx, int64 newly, int64 x, int64 y, int64 sx, int64 sy, int64 newpadding = 0)
				{
				*this = get_rescale(quality, newlx, newly, x, y, sx, sy, newpadding);
				}


			/**
			 * Crop a portion of this image and rescale it to a given size.
			 * 
			 * This method discard the current data buffer an create another one.
			 *
			 * @param	quality   	0 for (fast) low quality and 1 for (slow) high quality rescaling.
			 * @param	newsize   	new image size.
			 * @param	B		  	rectangle to crop.
			 * @param	newpadding	horizontal padding.
			 **/
			inline void rescale(int quality, const iVec2 & newsize, const iBox2 & B, int64 newpadding = 0)
				{
				*this = get_rescale(quality, newsize.X(), newsize.Y(), B.min[0], B.min[1], B.max[0] - B.min[0] + 1, B.max[1] - B.min[1] + 1, newpadding);  
				}


			/**
			 * Return a copy of this image rescaled to a given size using the fastest method available (low
			 * quality).
			 *
			 * @param	quality   	0 for (fast) low quality and 1 for (slow) high quality rescaling.
			 * @param	newlx	  	returned image width.
			 * @param	newly	  	returned image height.
			 * @param	newpadding	horizontal padding.
			 *
			 * @return	A image obtained by rescaling the source to size newlx x newly.
			 **/
			inline Image get_rescale(int quality, int64 newlx, int64 newly, int64 newpadding = 0) const
				{
				return get_rescale(quality, newlx, newly, 0, 0, _lx, _ly, newpadding);
				}


			/**
			* Return a copy of this image rescaled to a given size using the fastest method available (low
			* quality).
			*
			* @param	quality   	0 for (fast) low quality and 1 for (slow) high quality rescaling.
			* @param	newsize   	new image size.
			* @param	newpadding	horizontal padding.
			*
			* @return	A image obtained by rescaling the source to size newlx x newly.
			**/
			inline Image get_rescale(int quality, const iVec2 & newsize, int64 newpadding = 0) const
				{
				return get_rescale(quality, newsize.X(), newsize.Y(), 0, 0, _lx, _ly, newpadding);
				}


			/**
			* Return a copy of a portion of this image, rescaled to a given size.
			*
			* @param	quality   	0 for (fast) low quality and 1 for (slow) high quality rescaling.
			* @param	newlx	  	returned image width.
			* @param	newly	  	returned image height.
			* @param	x		  	x-coord of the upper left corner of the rectangle to rescale.
			* @param	y		  	y-coord of the upper left corner of the rectangle to rescale.
			* @param	sx		  	width of the rectangle to rescale.
			* @param	sy		  	height of the rectangle to rescale.
			* @param	newpadding	horizontal padding.
			*
			* @return	an image of size newlx x newly containing the rescaling of the rectangle [x, x+
			* 			sx]x[y, y+sy] of the source image.
			**/
			inline Image get_rescale(int quality, int64 newlx, int64 newly, int64 x, int64 y, int64 sx, int64 sy, int64 newpadding = 0) const
				{
				if ((newlx <= 0) || (newly <= 0)) { return Image(); }
				Image im(newlx, newly, newpadding);
				im.blit_rescaled(quality, *this, 0, 0, newlx, newly, x, y, sx, sy);
				return im;
				}


			/**
			 * Return a copy of a portion of this image, rescaled to a given size.
			 *
			 * @param	quality   	0 for (fast) low quality and 1 for (slow) high quality rescaling.
			 * @param	newsize   	new image size.
			 * @param	B		  	rectangle to rescale.
			 * @param	newpadding	horizontal padding.
			 *
			 * @return	an image of size newlx x newly containing the rescaling of the rectangle [x, x+
			 * 			sx]x[y, y+sy] of the source image.
			 **/
			inline Image get_rescale(int quality, const iVec2 & newsize, const iBox2 & B, int64 newpadding = 0) const
				{
				return get_rescale(quality, newsize.X(), newsize.Y(), B.min[0], B.min[1], B.max[0] - B.min[0] + 1, B.max[1] - B.min[1] + 1, newpadding);
				}


			/**
			 * Rescale a sprite image and then blit it onto this image.
			 *
			 * @param	quality	0 for (fast) low quality and 1 for (slow) high quality rescaling.
			 * @param	sprite 	The sprite image to rescale and then blit.
			 * @param	dest_x 	x-coord of the upper left corner of the destination rectangle.
			 * @param	dest_y 	y-coord of the upper left corner of the destination rectangle.
			 * @param	dest_sx	width of the destination rectangle.
			 * @param	dest_sy	height of the destination rectangle.
			 *
			 * @return	The real quality of the rescaling performed. At least quality but may be higher.
			 **/
			inline int blit_rescaled(int quality, const Image & sprite, int64 dest_x, int64 dest_y, int64 dest_sx, int64 dest_sy)
				{
				return blit_rescaled(quality, sprite, dest_x, dest_y, dest_sx, dest_sy, 0, 0, sprite._lx, sprite._ly);
				}


			/**
			 * Rescale a sprite image and then blit it onto this image.
			 *
			 * @param	quality 	0 for (fast) low quality and 1 for (slow) high quality rescaling.
			 * @param	sprite  	The sprite image to rescale and then blit.
			 * @param	dest_box	The destination rectangle.
			 *
			 * @return	The real quality of the rescaling performed. At least quality but may be higher.
			 **/
			inline int blit_rescaled(int quality, const Image & sprite, const iBox2 & dest_box)
				{
				return blit_rescaled(quality, sprite, dest_box.min[0], dest_box.min[1], dest_box.max[0] - dest_box.min[0] + 1, dest_box.max[1] - dest_box.min[1] + 1, 0, 0, sprite._lx, sprite._ly);
				}


			/**
			 * Rescale a portion of a sprite image and then blit it onto this image.
			 *
			 * @param	quality  	in [0,5]. 0 = low quality (fast) and 5 = max quality (slow).
			 * @param	sprite   	The sprite image.
			 * @param	dest_x   	x-coord of the upper left corner of the destination rectangle.
			 * @param	dest_y   	y-coord of the upper left corner of the destination rectangle.
			 * @param	dest_sx  	width of the destination rectangle.
			 * @param	dest_sy  	height of the destination rectangle.
			 * @param	sprite_x 	x-coord of the upper left corner of the sprite rectangle to blit.
			 * @param	sprite_y 	y-coord of the upper left corner of the sprite rectangle to blit.
			 * @param	sprite_sx	width of the sprite rectangle.
			 * @param	sprite_sy	height of the sprite rectangle.
			 *
			 * @return	The real quality of the rescaling performed. At least quality but may be higher.
			 **/
			inline int blit_rescaled(int quality, const Image & sprite, int64 dest_x, int64 dest_y, int64 dest_sx, int64 dest_sy, int64 sprite_x, int64 sprite_y, int64 sprite_sx, int64 sprite_sy)
				{
				const int MAX_QUALITY = 10;
				if (quality <= 0) quality = 0; else if (quality >= MAX_QUALITY) quality = MAX_QUALITY;
				if ((dest_sx <= 0) || (dest_sy <= 0)) return MAX_QUALITY; 
				if ((sprite_sx <= 0) || (sprite_sy <= 0)) return MAX_QUALITY;
				MTOOLS_INSURE(&sprite != this);
				MTOOLS_INSURE((dest_x >= 0) && (dest_x + dest_sx <= lx()));
				MTOOLS_INSURE((dest_y >= 0) && (dest_y + dest_sy <= ly()));
				MTOOLS_INSURE((sprite_x >= 0) && (sprite_x + sprite_sx <= sprite.lx()));
				MTOOLS_INSURE((sprite_y >= 0) && (sprite_y + sprite_sy <= sprite.ly()));
				if ((dest_sx == sprite_sx) && (dest_sy == sprite_sy))
					{ // no rescaling
					_blitRegion(_data + (dest_y*_stride) + dest_x, _stride, sprite._data + (sprite_y*_stride) + sprite_x, sprite._stride, dest_sx, dest_sy);
					return MAX_QUALITY;
					}
				if ((dest_sx <= sprite_sx) && (dest_sy <= sprite_sy))
					{ // downscaling
					if ((dest_sx == 1) || (dest_sy == 1))
						{ // box average does not work for flat images. use _nearest neighbour. (TODO, improve that). 
						_nearest_neighbour_scaling(_data + (dest_y*_stride) + dest_x, _stride, dest_sx, dest_sy, sprite._data + (sprite_y*sprite._stride) + sprite_x, sprite._stride, sprite_sx, sprite_sy);
						return MAX_QUALITY; // cannot do any better. 
						}
					if (!quality)
						{ // quality = 0, we use fastest method : nearest neighbour.
						_nearest_neighbour_scaling(_data + (dest_y*_stride) + dest_x, _stride, dest_sx, dest_sy, sprite._data + (sprite_y*sprite._stride) + sprite_x, sprite._stride, sprite_sx, sprite_sy);
						return 0; // worst quality. 
						}
					// use box average downscaling					
					RGBc * dest_data = _data + (dest_y*_stride) + dest_x;
					uint64 dest_stride = (uint64)_stride;
					uint64 dst_sx = (uint64)dest_sx;
					uint64 dst_sy = (uint64)dest_sy;
					RGBc * src_data = sprite._data + (sprite_y*sprite._stride);
					uint64 src_stride = (uint64)sprite._stride;
					uint64 src_sx = (uint64)sprite_sx;
					uint64 src_sy = (uint64)sprite_sy;
					uint64 stepx = (1ULL << (2*(MAX_QUALITY - quality)));
					int quality_x = quality;
					while(dst_sx*stepx > src_sx) { stepx >>= 2; quality_x++; }
					uint64 stepy = (1ULL << (2*(MAX_QUALITY - quality)));
					int quality_y = quality;
					while (dst_sy*stepy > src_sy) { stepy >>= 2; quality_y++; }
					_boxaverage_downscaling(dest_data, dest_stride, dst_sx, dst_sy, src_data, src_stride, src_sx, src_sy, stepx, stepy);
					return (int)std::min<uint64>(quality_x,quality_y); 
					}
				if ((dest_sx >= sprite_sx) && (dest_sy >= sprite_sy))
					{ // upscaling, quality > 0
					if ((sprite_sx == 1) || (sprite_sy == 1))
						{ // use _nearest neighbour. (TODO, improve that). 
						_nearest_neighbour_scaling(_data + (dest_y*_stride) + dest_x, _stride, dest_sx, dest_sy, sprite._data + (sprite_y*sprite._stride) + sprite_x, sprite._stride, sprite_sx, sprite_sy);
						return MAX_QUALITY; // cannot do any better. 
						}
					if (!quality)
						{ // quality = 0, use fastest method. 
						_nearest_neighbour_scaling(_data + (dest_y*_stride) + dest_x, _stride, dest_sx, dest_sy, sprite._data + (sprite_y*sprite._stride) + sprite_x, sprite._stride, sprite_sx, sprite_sy);
						return 0;
						}
					// use linear interpolation
					_linear_upscaling(_data + (dest_y*_stride) + dest_x, _stride, dest_sx, dest_sy, sprite._data + (sprite_y*sprite._stride) + sprite_x, sprite._stride, sprite_sx, sprite_sy);
					return MAX_QUALITY;
					}
				// mix up/down scaling -> use nearest neighbour
				_nearest_neighbour_scaling(_data + (dest_y*_stride) + dest_x, _stride, dest_sx, dest_sy, sprite._data + (sprite_y*sprite._stride) + sprite_x, sprite._stride, sprite_sx, sprite_sy);
				return MAX_QUALITY;
				}


			/**
			* Rescale a portion of a sprite image and then blit it onto this image.
			*
			* @param	quality   	in [0,5]. 0 = low quality (fast) and 5 = max quality (slow).
			* @param	sprite	  	The sprite image.
			* @param	dest_box	The destination rectangle.
			* @param	sprite_box	The rectangle part of the sprite to rescale and blit.
			*
			* @return	The real quality of the rescaling performed. At least quality but may be higher.
			**/
			inline int blit_rescaled(int quality, const Image & sprite, const iBox2 & dest_box, const iBox2 & sprite_box)
				{
				return blit_rescaled(quality, sprite, dest_box.min[0], dest_box.min[1], dest_box.max[0] - dest_box.min[0] + 1, dest_box.max[1] - dest_box.min[1] + 1,
					sprite_box.min[0], sprite_box.min[1], sprite_box.max[0] - sprite_box.min[0] + 1, sprite_box.max[1] - sprite_box.min[1] + 1);
				}


			/**
			 * Find the (closed) minimal bounding rectangle enclosing the image.
			 *
			 * @param	bk_color	The 'background' color which is not part of the image.
			 *
			 * @return	the minimal bounding box. 
			 **/
			iBox2 minBoundingBox(RGBc bk_color)
				{
				int64 minx = _lx + 1, maxx = -1;
				int64 miny = _ly + 1, maxy = -1;
				for (int64 j = 0; j < _ly; j++)
					{
					for (int64 i = 0; i < _lx; i++)
						{
						if (operator()(i, j) != bk_color)
							{
							if (i < minx) minx = i;
							if (i > maxx) maxx = i;
							if (j < miny) miny = j;
							if (j > maxy) maxy = j;
							}
						}
					}
				return iBox2(minx, maxx, miny, maxy);
				}


			/**
			 * Find the (closed) minimal bounding rectangle enclosing the image. Only pixels whose alpha channel is
			 * not zero are considered part of the image.
			 *
			 * @return	the minimal bounding box.
			 **/
			iBox2 minBoundingBox()								
				{
				int64 minx = _lx + 1, maxx = -1;
				int64 miny = _ly + 1, maxy = -1;
				for (int64 j = 0; j < _ly; j++)
					{
					for (int64 i = 0; i < _lx; i++)
						{
						if (operator()(i, j).comp.A != 0)
							{
							if (i < minx) minx = i;
							if (i > maxx) maxx = i;
							if (j < miny) miny = j;
							if (j > maxy) maxy = j;
							}
						}
					}
				return iBox2(minx,maxx,miny,maxy);				
				}



			/**
			* Serializes the image into an OBaseArchive.
			**/
			void serialize(OBaseArchive & ar) const
				{
				ar << "Image";
				ar & _lx;
				ar & _ly;
				ar & _stride;
				ar.newline();
				if ((_lx <= 0) || (_ly <= 0) || (_stride < _lx)) return;
				for (int64 j = 0; j < _ly; j++)
					{
					ar.opaqueArray(_data + _stride*j, _lx * 4);
					ar.newline();
					}
				}


			/**
			* Deserializes the image from a IBaseArchive.
			**/
			void deserialize(IBaseArchive & ar)
				{
				empty();
				ar & _lx;
				ar & _ly;
				ar & _stride;
				if ((_lx <= 0) || (_ly <= 0) || (_stride < _lx)) { empty(); return; }
				_allocate(_ly, _stride, nullptr);
				for (int64 j = 0; j < _ly; j++)
					{
					ar.opaqueArray(_data + _stride*j, _lx * 4);
					}
				}





			 /**
			  * draw geometric figure.
			  *
			  * @return	true if empty, false if not.
			  **/
			// fill
			
			// draw text



			/**
			* Saves the image into a file in PNG format.
			*
			* @param	filename	name of the file (should have extension "png").
			*
			* @return	true if the operation succedded and false if it failed.
			**/
			bool _save_png(const std::string & filename) const
				{
				if (!_createcairo(false)) return false;
				return (cairo_surface_write_to_png((cairo_surface_t*)_pcairo_surface, filename.c_str()) == CAIRO_STATUS_SUCCESS);
				}



			/**
			* Load the image from a file in PGN format.
			*
			* @param	filename	name of the file (should have extension "png" or "jpg").
			*
			* @return	true if the operation succedded and false if it failed (and in this case, the object
			* 			is set to an empty image).
			**/
			bool load_png(const std::string & filename)
				{
				empty();
				cairo_surface_t * psurface = cairo_image_surface_create_from_png(filename.c_str());
				if (cairo_surface_status(psurface) != CAIRO_STATUS_SUCCESS) { cairo_surface_destroy(psurface); return false; }
				cairo_format_t format = cairo_image_surface_get_format(psurface);
				if ((format != CAIRO_FORMAT_ARGB32) && (format != CAIRO_FORMAT_RGB24)) { cairo_surface_destroy(psurface); return false; }
				_lx = cairo_image_surface_get_width(psurface);
				_ly = cairo_image_surface_get_height(psurface);
				_stride = cairo_image_surface_get_stride(psurface);
				if ((_lx <= 0) || (_ly <= 0) || (_stride % 4 != 0) || (_stride < 4 * _lx)) { empty(); cairo_surface_destroy(psurface); return false; }
				_stride /= 4;
				_allocate(_ly, _stride, nullptr);
				uint32 * psrc = (uint32 *)cairo_image_surface_get_data(psurface);
				uint32 * pdst = (uint32*)_data;
				uint32 mask = ((format == CAIRO_FORMAT_RGB24) ? 0xFF000000 : 0);
				for (int64 j = 0; j < _ly; j++)
					{
					for (int64 i = 0;i < _lx; i++)
						{
						pdst[i + _stride*j] = (psrc[i + _stride*j] | mask);
						}
					}
				cairo_surface_destroy(psurface);
				return true;
				}


			/**
			 * Query if the image is empty
			 *
			 * @return	true if empty, false if not.
			 **/
			bool isEmpty() const { return(_data == nullptr); }


			/**
			 * Width of the image in pixels. Same as width().
			 **/
			int64 lx() const { return _lx; }


			/**
			* Width of the image in pixels. Same as lx().
			**/
			int64 width() const { return _lx; }


			/**
			* Height of the image in pixels. Same as height().
			**/
			int64 ly() const { return _ly; }


			/**
			* Height of the image in pixels. Same as ly().
			**/
			int64 heigth() const { return _ly; }



			/**
			 * Horizontal padding of the image: number of uint32 following the end of each horizontal line
			 * (except the last one).
			 **/
			int64 padding() const { return(_stride - _lx); }


			/**
			 * Sets the horizontal padding value for this image.
			 *
			 * If the newx padding differs from the previous one, the pixel buffer is re-created.  
			 *
			 * @param	padding	The new padding.
			 **/
			void setPadding(int64 newpadding)
				{
				if (newpadding < 0) newpadding = 0;
				if (newpadding == padding()) return;
				*this = Image(*this, false, newpadding);
				}


			/**
			* Return a pointer to the pixel buffer (const version).
			**/
			const RGBc * data() const { return _data; }


			/**
			* Return a pointer to the pixel buffer.
			**/
			RGBc * data() { return _data; }


			/**
			 * Get the color at a given position.
			 * No bound check !
			 *
			 * @param	x	The x coordinate.
			 * @param	y	The y coordinate.
			 *
			 * @return	The color at position (x,y).
			 **/
			inline RGBc & operator()(const int64 x, const int64 y) { MTOOLS_ASSERT((x >= 0) && (x < _lx)); MTOOLS_ASSERT((y >= 0) && (y < _ly)); return _data[x + _stride*y]; }


			/**
			 * Get the color at a given position. No bound check !
			 *
			 * @param	pos	The position to look at.
			 *
			 * @return	The color at position pos.
			 **/
			inline RGBc & operator()(const iVec2 & pos) 
				{ 
				const int64 x = pos.X();
				const int64 y = pos.Y();
				MTOOLS_ASSERT((x >= 0) && (x < _lx)); MTOOLS_ASSERT((y >= 0) && (y < _ly)); 
				return _data[x + _stride*y]; 
				}


			/**
			* Get the color at a given position (const version).
			* No bound check !
			*
			* @param	x	The x coordinate.
			* @param	y	The y coordinate.
			*
			* @return	The color at position (x,y).
			**/
			inline const RGBc & operator()(const int64 x, const int64 y) const { MTOOLS_ASSERT((x >= 0) && (x < _lx)); MTOOLS_ASSERT((y >= 0) && (y < _ly)); return _data[x + _stride*y]; }


			/**
			* Get the color at a given position. (const version)
			* No bound check !
			*
			* @param	pos	The position to look at.
			*
			* @return	The color at position pos.
			**/
			inline const RGBc & operator()(const iVec2 & pos) const
				{
				const int64 x = pos.X();
				const int64 y = pos.Y();
				MTOOLS_ASSERT((x >= 0) && (x < _lx)); MTOOLS_ASSERT((y >= 0) && (y < _ly));
				return _data[x + _stride*y];
				}


			/**
			 * Sets a pixel. Does nothing if the position is outside of the image.
			 *
			 * @param	x	 	The x coordinate.
			 * @param	y	 	The y coordinate.
			 * @param	color	color to set.
			 **/
			inline void setPixel(const int64 x, const int64 y, const RGBc color)
				{
				if ((x >= 0) && (x < _lx) && (y >= 0) && (y < _ly)) { _data[x + _stride*y] = color; }
				}


			/**
			 * Sets a pixel. Does nothing if the position is outside of the image.
			 *
			 * @param	pos  	The position to look at.
			 * @param	color	color to set.
			 **/
			inline void setPixel(const iVec2 & pos, const RGBc color)
				{
				const int64 x = pos.X();
				const int64 y = pos.Y();
				if ((x >= 0) && (x < _lx) && (y >= 0) && (y < _ly)) { _data[x + _stride*y] = color; }
				}



			/**
			 * Query the color of a pixel. Return default color if outside of the image.
			 *
			 * @param	x				The x coordinate.
			 * @param	y				The y coordinate.
			 * @param	defaultcolor	The default color if (x,y) outside of the image.
			 *
			 * @return	The pixel color.
			 **/
			inline RGBc getPixel(const int64 x, const int64 y, const RGBc defaultcolor = RGBc::c_TransparentWhite) const
				{
				if ((x >= 0) && (x < _lx) && (y >= 0) && (y < _ly)) { return _data[x + _stride*y]; }
				return defaultcolor;
				}


			/**
			 * Query the color of a pixel. Return default color if outside of the image.
			 *
			 * @param	pos				The position to look at. 
			 * @param	defaultcolor	The default color if (x,y) is outside of the image.
			 *
			 * @return	The pixel color.
			 **/
			inline RGBc getPixel(const iVec2 & pos, const RGBc defaultcolor = RGBc::c_TransparentWhite) const
				{
				const int64 x = pos.X();
				const int64 y = pos.Y();
				if ((x >= 0) && (x < _lx) && (y >= 0) && (y < _ly)) { return _data[x + _stride*y]; }
				return defaultcolor;
				}



			/** Reverse this image along its Y-axis */
			void reverseY()
				{
				if (_ly < 2) return;
				for (int j = 0; j < _ly / 2; j++)
					{
					RGBc * p1 = _data + _stride*j;
					RGBc * p2 = _data + _stride*(_ly - 1 - j);
					for (int64 i = 0; i < _lx; i++) { mtools::swap<RGBc>(p1[i],p2[i]); }
					}
				}


			/**
			 * Return a new image obtained by reversing this image along its Y-axis.
			 **/
			Image get_reverseY(int64 padding = 0) const
				{
				Image im(_lx, _ly,padding);
				if (_lx < 20)
					{ 
					for (int64 j = 0; j < _ly; j++)
						{
						RGBc * psrc = _data + _stride*j;
						RGBc * pdst = im._data + im._stride*(_ly - 1 - j);
						for (int64 i = 0; i < _lx; i++) { pdst[i] = psrc[i]; }
						}
					}
				else
					{
					for (int64 j = 0; j < _ly; j++)
						{
						RGBc * psrc = _data + _stride*j;
						RGBc * pdst = im._data + im._stride*(_ly - 1 - j);
						memcpy(pdst, psrc, _lx * 4);
						}
					}
				return im;
				}



			/** Empty this image (the resulting image has size 0x0). */
			void empty()
				{
				_removecairo();
				_deallocate();
				_lx = 0;
				_ly = 0;
				_stride = 0;
				}


			/**
			 * Clears this image to a given color
			 *
			 * @param	bkColor	the color to use.
			 **/
			void clear(RGBc bkColor)
				{
				//pixman_fill((uint32_t*)_data, _stride, 32, 0, 0, _lx, _ly, bkColor.color); // slow...
				_fillRegion(_data, _stride, _lx, _ly, bkColor);
				}


			/**
			 * Write some info about this image into an std::string.
			 **/
			std::string toString() const
				{
				std::string s("Image [");
				s += mtools::toString(_lx) + "x" + mtools::toString(_ly) + " stride " + mtools::toString(_stride) + "]";
				if (refcount() >1) { s += " (SHARED : ref count " + mtools::toString(refcount()) + ")"; }
				return s;
				}


			/**
			 * Query if the image shares its pixel buffer with another image. Equivalent to checking if
			 * refcount() = 1.
			 *
			 * @return	true if shared, false if not.
			 */
			bool isShared() const
				{
				return(refcount() != 1);
				}


			/**
			 * Query the number of images sharing the same data buffer.
			 *
			 * @return	Number of image sharing the same data buffer (1 is the image is not shared).
			 */
			uint32 refcount() const
				{
				return ((_deletepointer != nullptr) ? (*_deletepointer) : 1);
				}

		private:

			/*************************************************************
			* MISC PRIVATE METHODS                                       *
			*                                                            *
			**************************************************************/


			/* fill a region with a given color */
			inline static void _fillRegion(RGBc * pdest, int64 dest_stride, int64 sx, int64 sy, RGBc color)
				{
				for (int64 j = 0; j < sy; j++)
					{
					const int64 offdest = j*dest_stride;
					for (int64 i = 0; i < sx; i++)
						{
						pdest[offdest + i] = color;
						}
					}
				}



			/*************************************************************
			* RESCALING                                                  *
			*                                                            *
			**************************************************************/


			/* apply nearest neighour scaling. fast ! 
			   work for downscaling and upscaling both. */
			static void _nearest_neighbour_scaling(RGBc * dest, int64 dest_stride, int64 dest_lx, int64 dest_ly, RGBc * src, int64 src_stride, int64 src_lx, int64 src_ly)
				{

				if ((src_lx == dest_lx) && (src_ly == dest_ly)) { _blitRegion(dest, dest_stride, src, src_stride, src_lx, src_ly); return; }
				MTOOLS_ASSERT((src_lx < 1000000) && (src_ly < 1000000)); // must be smaller than 2^20 to use fp arithmetic with FP_PRECISION = 43
				const int64 FP_PRECISION = 43;
				const double fbx = ((double)src_lx) / ((double)dest_lx);
				const int64  ibx = (int64)(fbx * (((int64)1) << FP_PRECISION));
				const double fby = ((double)src_ly) / ((double)dest_ly);
				const int64  iby = (int64)(fby * (((int64)1) << FP_PRECISION));
				int64 iay = (iby / 2);
				int64 offdest = 0;
				const int64 endj = dest_stride*dest_ly;
				while (offdest < endj)
					{
					const int64 offsrc = src_stride*(iay >> FP_PRECISION);
					int64 iax = (ibx / 2);
					for (int i = 0; i < dest_lx; i++)
						{
						dest[offdest + i] = src[offsrc + (iax >> FP_PRECISION)];
						iax += ibx;
						}
					iay += iby;
					offdest += dest_stride;
					}
				}




			/* upscale image via linear interpolation. Work only for upscaling.  */ 
			static void _linear_upscaling(RGBc * dest_data, uint64 dest_stride, uint64 dest_sx, uint64 dest_sy, RGBc * src_data, uint64 src_stride, uint64 src_sx, uint64 src_sy)
				{
				MTOOLS_ASSERT((src_sx < 1000000) && (src_sy < 1000000)); // must be smaller than 2^20
				MTOOLS_ASSERT(dest_sx >= src_sx);
				MTOOLS_ASSERT(dest_sy >= src_sy);
				MTOOLS_ASSERT(src_sx >= 2);
				MTOOLS_ASSERT(src_sy >= 2);
				const uint64 FP_PRECISION = 43;
				const uint64 FP_PRECISION_COLOR1 = 33;
				const uint64 FP_PRECISION_COLOR2 = FP_PRECISION + (FP_PRECISION - FP_PRECISION_COLOR1);
				const uint64 unit = (1ULL << FP_PRECISION);
				const uint64 step_x = ((src_sx - 1)*unit) / (dest_sx - 1);
				const uint64 step_y = ((src_sy - 1)*unit) / (dest_sy - 1);
				uint64 offy = 0;
				uint64 js = 0;
				for (uint64 jd = 0; jd < dest_sy; jd++)
					{
					MTOOLS_ASSERT(js < src_sy - 1);
					const uint64 c_offy = unit - offy;
					uint64 offx = 0;
					uint64 is = 0;
					uint64 id = 0;
					while(id < dest_sx)
						{
						MTOOLS_ASSERT(is < (src_sx - 1));
						const uint64 psrc = is + js*src_stride;
						const RGBc c00 = src_data[psrc];
						const RGBc c10 = src_data[psrc + 1];
						const RGBc c01 = src_data[psrc + src_stride];
						const RGBc c11 = src_data[psrc + src_stride + 1];
						const uint64 h1R = ((c00.comp.R * c_offy) + (c01.comp.R * offy)) >> FP_PRECISION_COLOR1;
						const uint64 h1G = ((c00.comp.G * c_offy) + (c01.comp.G * offy)) >> FP_PRECISION_COLOR1;
						const uint64 h1B = ((c00.comp.B * c_offy) + (c01.comp.B * offy)) >> FP_PRECISION_COLOR1;
						const uint64 h1A = ((c00.comp.A * c_offy) + (c01.comp.A * offy)) >> FP_PRECISION_COLOR1;
						const uint64 h2R = ((c10.comp.R * c_offy) + (c11.comp.R * offy)) >> FP_PRECISION_COLOR1;
						const uint64 h2G = ((c10.comp.G * c_offy) + (c11.comp.G * offy)) >> FP_PRECISION_COLOR1;
						const uint64 h2B = ((c10.comp.B * c_offy) + (c11.comp.B * offy)) >> FP_PRECISION_COLOR1;
						const uint64 h2A = ((c10.comp.A * c_offy) + (c11.comp.A * offy)) >> FP_PRECISION_COLOR1;
						while(offx <= unit)
							{
							const uint64 c_offx = unit - offx;
							const uint64 rsR = ((h1R * c_offx) + (h2R * offx)) >> FP_PRECISION_COLOR2;
							const uint64 rsG = ((h1G * c_offx) + (h2G * offx)) >> FP_PRECISION_COLOR2;
							const uint64 rsB = ((h1B * c_offx) + (h2B * offx)) >> FP_PRECISION_COLOR2;
							const uint64 rsA = ((h1A * c_offx) + (h2A * offx)) >> FP_PRECISION_COLOR2;
							dest_data[jd*dest_stride + id] = RGBc((uint8)rsR, (uint8)rsG, (uint8)rsB, (uint8)rsA);
							offx += step_x;
							id++;
							}
						offx -= unit; is++;
						}
					offy += step_y;
					if (offy > unit) { offy -= unit; js++; }
					}
				}


			/* Call the correct template version of _boxaverage_downscaling2() depending on the input parameters.
			   this method choose the stepping nto the src image and the value of the BIT_FP template parameter. */
			static void _boxaverage_downscaling(RGBc * dest_data, uint64 dest_stride, uint64 dest_sx, uint64 dest_sy, RGBc * src_data, uint64 src_stride, uint64 src_lx, uint64 src_ly, uint64 src_stepx = 1, uint64 src_stepy = 1)
				{
				const uint64 src_sx = src_lx / src_stepx;	// for the _boxaverage method, it is the same as a destination
				const uint64 src_sy = src_ly / src_stepy;	// image of size (src_lx/src_stepx , src_ly/src_stepy)
				const uint64 bx = 1 + (src_sx/dest_sx) + (((src_sx % dest_sx) != 0) ? 1 : 0); // upper bound on horizontal ratio 
				const uint64 by = 1 + (src_sy/dest_sy) + (((src_sy % dest_sy) != 0) ? 1 : 0); // upper bound on vertical ratio
				const uint64 v = bx*by;	// upper bound on the number of source pixels per dest pixel.  
				if ((src_stepx == 1) && (src_stepy == 1))
					{ //perfect downscaling
					uint64 a = 16;
					if (v <= a) { _boxaverage_downscaling2<10>(dest_data, dest_stride, dest_sx, dest_sy, src_data, src_stride, src_sx, src_sy); return; } a *= 4;  // at most 16 pixels per dest pixel
					if (v <= a) { _boxaverage_downscaling2<9>(dest_data, dest_stride, dest_sx, dest_sy, src_data, src_stride, src_sx, src_sy); return; } a *= 4;   // at most 64 pixels per dest pixel
					if (v <= a) { _boxaverage_downscaling2<8>(dest_data, dest_stride, dest_sx, dest_sy, src_data, src_stride, src_sx, src_sy); return; } a *= 4;   // at most 256 pixels per dest pixel
					if (v <= a) { _boxaverage_downscaling2<7>(dest_data, dest_stride, dest_sx, dest_sy, src_data, src_stride, src_sx, src_sy); return; } a *= 4;   // at most 1024 pixels per dest pixel
					if (v <= a) { _boxaverage_downscaling2<6>(dest_data, dest_stride, dest_sx, dest_sy, src_data, src_stride, src_sx, src_sy); return; } a *= 4;   // at most 4096 pixels per dest pixel 
					if (v <= a) { _boxaverage_downscaling2<5>(dest_data, dest_stride, dest_sx, dest_sy, src_data, src_stride, src_sx, src_sy); return; } a *= 4;   // at most 16384 pixels per dest pixel
					// scale factor too large. use stochastic anyway. 					
					uint64 stepx = (bx/128) + 1;
					uint64 stepy = (by/128) + 1;
					_boxaverage_downscaling(dest_data, dest_stride, dest_sx, dest_sy, src_data, src_stride, src_lx, src_ly, stepx, stepy);
					return; 
					}
				else
					{ // stochastic downscaling. 					
					uint64 a = 16;
					FastRNG gen;
					FastLaw lawx((uint32)src_stepx);
					FastLaw lawy((uint32)src_stepy);
					if (v <= a)
						{ // at most 16 pixels per dest pixel
						_boxaverage_downscaling2<10, true>( dest_data, dest_stride, dest_sx, dest_sy, src_data, src_stride, src_sx, src_sy,
					        [&](uint64 x, uint64 y) -> RGBc { uint32 g = gen(); return src_data[(y*src_stepy + lawy(g))*src_stride + x*src_stepx + lawx(g >> 16)].color; },
					        [&](uint64 x, uint64 y, RGBc c) { dest_data[y*dest_stride + x] = c;  });
						return;
						} a *= 4;
					if (v <= a)
						{ // at most 64 pixels per dest pixel
						_boxaverage_downscaling2<9, true>(dest_data, dest_stride, dest_sx, dest_sy, src_data, src_stride, src_sx, src_sy,
							[&](uint64 x, uint64 y) -> RGBc { uint32 g = gen(); return src_data[(y*src_stepy + lawy(g))*src_stride + x*src_stepx + lawx(g >> 16)].color; },
							[&](uint64 x, uint64 y, RGBc c) { dest_data[y*dest_stride + x] = c;  });
						return;
						} a *= 4;
					if (v <= a)
						{ // at most 256 pixels per dest pixel
						_boxaverage_downscaling2<8, true>(dest_data, dest_stride, dest_sx, dest_sy, src_data, src_stride, src_sx, src_sy,
							[&](uint64 x, uint64 y) -> RGBc { uint32 g = gen(); return src_data[(y*src_stepy + lawy(g))*src_stride + x*src_stepx + lawx(g >> 16)].color; },
							[&](uint64 x, uint64 y, RGBc c) { dest_data[y*dest_stride + x] = c;  });
						return;
						} a *= 4;
					if (v <= a)
						{ // at most 1024 pixels per dest pixel
						_boxaverage_downscaling2<7, true>(dest_data, dest_stride, dest_sx, dest_sy, src_data, src_stride, src_sx, src_sy,
							[&](uint64 x, uint64 y) -> RGBc { uint32 g = gen(); return src_data[(y*src_stepy + lawy(g))*src_stride + x*src_stepx + lawx(g >> 16)].color; },
							[&](uint64 x, uint64 y, RGBc c) { dest_data[y*dest_stride + x] = c;  });
						return;
						} a *= 4;
					if (v <= a)
						{ // at most 4096 pixels per dest pixel
						_boxaverage_downscaling2<6, true>(dest_data, dest_stride, dest_sx, dest_sy, src_data, src_stride, src_sx, src_sy,
							[&](uint64 x, uint64 y) -> RGBc { uint32 g = gen(); return src_data[(y*src_stepy + lawy(g))*src_stride + x*src_stepx + lawx(g >> 16)].color; },
							[&](uint64 x, uint64 y, RGBc c) { dest_data[y*dest_stride + x] = c;  });
						return;
						} a *= 4;
					if (v <= a)
						{ // at most 16384 pixels per dest pixel
						_boxaverage_downscaling2<5, true>(dest_data, dest_stride, dest_sx, dest_sy, src_data, src_stride, src_sx, src_sy,
							[&](uint64 x, uint64 y) -> RGBc { uint32 g = gen(); return src_data[(y*src_stepy + lawy(g))*src_stride + x*src_stepx + lawx(g >> 16)].color; },
							[&](uint64 x, uint64 y, RGBc c) { dest_data[y*dest_stride + x] = c;  });
						return;
						} a *= 4;
					// downsampling ratio is still too big. increase the step even more.
					uint64 spc_x = (bx/128) + 1; 
					uint64 spc_y = (by/128) + 1;
					_boxaverage_downscaling(dest_data, dest_stride, dest_sx, dest_sy, src_data, src_stride, src_lx, src_ly, src_stepx*spc_x, src_stepy*spc_y);
					return;
					}
				}


			/* call _boxaverage_downscaling_FP32 with the correct template parameters for BIT_FP and BIT_DIV */
			template<uint64 BIT_FP_REDUCE, bool USE_FUNCION_CALL = false, typename READ_FUNCTOR = _dummy_read_functor, typename WRITE_FUNCTOR = _dummy_write_functor>
			static void _boxaverage_downscaling2(RGBc * dest_data, uint64 dest_stride, uint64 dest_sx, uint64 dest_sy, RGBc * src_data, uint64 src_stride, uint64 src_sx, uint64 src_sy, READ_FUNCTOR funread = _dummy_read_functor(), WRITE_FUNCTOR funwrite = _dummy_write_functor())
				{
				const uint64 bx = (src_sx / dest_sx); // lower bound on horizontal ratio 
				const uint64 by = (src_sy / dest_sy); // lower bound on vertical ratio
				const uint64 v = bx*by;	// lower bound on the number of source pixels per dest pixel.  
				const uint64 bit_div = 31 + 2*BIT_FP_REDUCE + (highestBit(v) - 1);
				MTOOLS_ASSERT(bit_div >= 47);
				switch (bit_div)
					{
					case 47: { _boxaverage_downscaling_FP32<40, BIT_FP_REDUCE, 48, USE_FUNCION_CALL>(dest_data, dest_stride, dest_sx, dest_sy, src_data, src_stride, src_sx, src_sy, funread, funwrite); return; }
					case 48: { _boxaverage_downscaling_FP32<40, BIT_FP_REDUCE, 48, USE_FUNCION_CALL>(dest_data, dest_stride, dest_sx, dest_sy, src_data, src_stride, src_sx, src_sy, funread, funwrite); return; }
					case 49: { _boxaverage_downscaling_FP32<40, BIT_FP_REDUCE, 49, USE_FUNCION_CALL>(dest_data, dest_stride, dest_sx, dest_sy, src_data, src_stride, src_sx, src_sy, funread, funwrite); return; }
					case 50: { _boxaverage_downscaling_FP32<40, BIT_FP_REDUCE, 50, USE_FUNCION_CALL>(dest_data, dest_stride, dest_sx, dest_sy, src_data, src_stride, src_sx, src_sy, funread, funwrite); return; }
					case 51: { _boxaverage_downscaling_FP32<40, BIT_FP_REDUCE, 51, USE_FUNCION_CALL>(dest_data, dest_stride, dest_sx, dest_sy, src_data, src_stride, src_sx, src_sy, funread, funwrite); return; }
					case 52: { _boxaverage_downscaling_FP32<40, BIT_FP_REDUCE, 52, USE_FUNCION_CALL>(dest_data, dest_stride, dest_sx, dest_sy, src_data, src_stride, src_sx, src_sy, funread, funwrite); return; }
					case 53: { _boxaverage_downscaling_FP32<40, BIT_FP_REDUCE, 53, USE_FUNCION_CALL>(dest_data, dest_stride, dest_sx, dest_sy, src_data, src_stride, src_sx, src_sy, funread, funwrite); return; }
					default: { _boxaverage_downscaling_FP32<40, BIT_FP_REDUCE, 54, USE_FUNCION_CALL>(dest_data, dest_stride, dest_sx, dest_sy, src_data, src_stride, src_sx, src_sy, funread, funwrite); return; }
					}
				}


		
			/* dummy (default) functors */
			struct _dummy_read_functor { RGBc operator()(uint64 x, uint64 y) { return  RGBc::c_Black; } };
			struct _dummy_write_functor { void operator()(uint64 x, uint64 y, RGBc color) { return; } };


			/* Downscaling using box average algorithm. 
			   Optimized for SSE 4.2 (todo: optimize for AVX2). SSE is enabled is MTOOLS_USE_SSE is defined.
			   The method uses only integer calculation 
			   - BIT_FP : number of bits for computing position and aera in fixed position.(40 is good). 
			   - BIT_FP_REDUCE : number of bits of the aera multiplied by the color of each pixel (must decrease as the ratio of the aera of src/dest increase). 
			   - BIT_DIV : number of bit for fixed precision division used to divide the by the aera before writing the final color (must decrease it as LX_LY_RED decreases). 

			   Ugly hack: since VS does not optimize inline template functor, in order to keep max speed, we switch how we access the source and dest image. Either:
			   1) Directly warp on the destination image 
			   2) Call a function to read/write pixels.
			   The mehod chosen depend on the value of USE_FUNCION_CALL. In the first case, the last two parameters are ignored (funread, funxrite) and in the second case,
			   the parameters dest_data,dest_stride,src_data, src_stride are irrelevant...
			*/
			template<uint64 BIT_FP = 40, uint64 BIT_FP_REDUCE = 10, uint64 BIT_DIV = 50, bool USE_FUNCION_CALL = false, typename READ_FUNCTOR = _dummy_read_functor, typename WRITE_FUNCTOR = _dummy_write_functor> 
			static void _boxaverage_downscaling_FP32(RGBc * dest_data, uint64 dest_stride, uint64 dest_sx, uint64 dest_sy, RGBc * src_data, uint64 src_stride, uint64 src_sx, uint64 src_sy, READ_FUNCTOR funread = _dummy_read_functor(), WRITE_FUNCTOR funwrite = _dummy_write_functor())
				{
					size_t tmpsize = 16 * (dest_sx + 1);
					uint32 * tmp = (uint32*)malloc(tmpsize);
					MTOOLS_ASSERT(tmp != nullptr);
					MTOOLS_ASSERT(((uint64)tmp) % 16 == 0); // temporary buffer must be 16 bytes aligned. 
					MTOOLS_ASSERT(dest_sx >= 2);
					MTOOLS_ASSERT(dest_sy >= 2);
					MTOOLS_ASSERT(dest_sx <= src_sx);
					MTOOLS_ASSERT(dest_sy <= src_sy);
					#define MTOOLS_ind_A_geq_B_U64(A,B) ((~(A - B)) >> 63) // 1 if A >= B and 0 otherwise. 
					const uint64 LL     = (1ULL << BIT_FP);
					const uint32 LL_RED = (1ULL << BIT_FP_REDUCE);
					const uint64 LX     = (uint64)((1ULL << BIT_FP)*((double)src_sx) / ((double)dest_sx)); // x ratio
					const uint64 LY     = (uint64)((1ULL << BIT_FP)*((double)src_sy) / ((double)dest_sy)); // y ratio
					const uint64 LX_LY_RED = (uint32)((1ULL << (BIT_FP_REDUCE*2))*((double)(src_sx*src_sy)) / ((double)(dest_sx*dest_sy))); // aera of a destination pixel wrt to the source. 
					MTOOLS_ASSERT(LX_LY_RED * 256 < 0xFFFFFFFF); // aera * color of pixel must hold in a uint32.
					const uint64 ONE_OVER_LX_LY_RED = ((1ULL << BIT_DIV) / LX_LY_RED);
					MTOOLS_ASSERT(ONE_OVER_LX_LY_RED <= 0xFFFFFFFF); // must be 32 bit [so that we can use AVX2 later for storing the final color: TODO].
					memset(tmp, 0, tmpsize); // clear the temporary buffer
					uint64 epsy = 0;
					uint64 dj = 0;
					for (uint64 sj = 0; sj < src_sy; sj++)
						{ // run over all the line of the source
						epsy += LL;
						const uint64 overflowy = MTOOLS_ind_A_geq_B_U64(epsy, LY);	// 1 if epsy >= LY and 0 otherwise. 
						const uint64 ry = overflowy*(epsy - LY);
						const uint32 p2y = (uint32)(ry >> (BIT_FP - BIT_FP_REDUCE));
						const uint32 p1y = LL_RED - p2y;
						{
						uint64 epsx = 0;
						uint64 di = 0;
						for (uint64 si = 0; si < src_sx; si++)
							{ // run over a line on the source image
							epsx += LL;
							const uint64 overflowx = MTOOLS_ind_A_geq_B_U64(epsx, LX);	// 1 if epsx >= LX and 0 otherwise. 
							const uint64 rx = overflowx*(epsx - LX);
							const uint32 p2x = (uint32)(rx >> (BIT_FP - BIT_FP_REDUCE));
							const uint32 p1x = LL_RED - p2x;							
							uint32 coul;							
							if (!USE_FUNCION_CALL) // <- conditional removed at compile time since USE_FUNCION_CALL is a compile time constant. 
								{ // fast access, optimized by compiler
								coul = src_data[src_stride*sj + si].color;
								}
							else
								{ // use function call instead
								coul = funread(si, sj).color;
								}												
							auto off = 4 * di;
							const uint32 aera1 = p1y*p1x;
							const uint32 aera2 = p1y*p2x;
							#ifdef MTOOLS_USE_SSE
								{							
								__m128i * sse_tmp = reinterpret_cast<__m128i*>(tmp + off);
								__m128i v = _mm_set_epi32((coul >> 24) & 0xFF, (coul >> 16) & 0xFF, (coul >> 8) & 0xFF, coul & 0xFF);
								__m128i a1 = _mm_set1_epi32(aera1);
								_mm_store_si128(sse_tmp, _mm_add_epi32(_mm_load_si128(sse_tmp), _mm_mullo_epi32(a1, v)));
								__m128i a2 = _mm_set1_epi32(aera2);
								_mm_store_si128(sse_tmp + 1, _mm_add_epi32(_mm_load_si128(sse_tmp + 1), _mm_mullo_epi32(a2, v)));
								}
							#else					
								tmp[off] += aera1*(coul & 0xFF);
								tmp[off + 1] += aera1*((coul >> 8) & 0xFF);
								tmp[off + 2] += aera1*((coul >> 16) & 0xFF);
								tmp[off + 3] += aera1*((coul >> 24) & 0xFF);
								tmp[off + 4] += aera2*(coul & 0xFF);
								tmp[off + 5] += aera2*((coul >> 8) & 0xFF);
								tmp[off + 6] += aera2*((coul >> 16) & 0xFF);
								tmp[off + 7] += aera2*((coul >> 24) & 0xFF);
							#endif				
							di += overflowx;
							epsx -= LX*overflowx;
							}
						}
						if (overflowy)
							{
							for (uint64 k = 0; k < dest_sx; k++)
								{ // normalize
								auto off = 4*k;
								uint32 c1 = ((tmp[off    ] * ONE_OVER_LX_LY_RED) >> BIT_DIV); c1 |= ((c1 & 256) >> 8) * 255;
								uint32 c2 = ((tmp[off + 1] * ONE_OVER_LX_LY_RED) >> BIT_DIV); c2 |= ((c2 & 256) >> 8) * 255;
								uint32 c3 = ((tmp[off + 2] * ONE_OVER_LX_LY_RED) >> BIT_DIV); c3 |= ((c3 & 256) >> 8) * 255;
								uint32 c4 = ((tmp[off + 3] * ONE_OVER_LX_LY_RED) >> BIT_DIV); c4 |= ((c4 & 256) >> 8) * 255;
								if (!USE_FUNCION_CALL) // <- conditional removed at compile time since USE_FUNCION_CALL is a compile time constant. 
									{ // fast access, optimized by compiler
									dest_data[dest_stride*dj + k].color = c1 + (c2 << 8) + (c3 << 16) + (c4 << 24);
									}
								else
									{ // use function call instead
									funwrite(k, dj, c1 + (c2 << 8) + (c3 << 16) + (c4 << 24));
									}
								}
							memset(tmp, 0, (dest_sx + 1) * 16); // clear the temporary buffer							
							// redo the line for the remainders
							uint64 epsx = 0;
							uint64 di = 0;
							for (uint64 si = 0; si < src_sx; si++)
								{ // run over a line on the source image
								epsx += LL;
								const uint64 overflowx = MTOOLS_ind_A_geq_B_U64(epsx, LX);	// 1 if epsx >= LX and 0 otherwise. 
								const uint64 rx = overflowx*(epsx - LX);
								const uint32 p2x = (uint32)(rx >> (BIT_FP - BIT_FP_REDUCE));
								const uint32 p1x = LL_RED - p2x;
								uint32 coul;
								if (!USE_FUNCION_CALL) // <- conditional removed at compile time since USE_FUNCION_CALL is a compile time constant. 
									{ // fast access, optimized by compiler
									coul = src_data[src_stride*sj + si].color;
									}
								else
									{ // use function call instead
									coul = funread(si, sj).color;
									}
								auto off = 4 * di;
								const uint32 aera1 = p2y*p1x;
								const uint32 aera2 = p2y*p2x;
								#ifdef MTOOLS_USE_SSE
									{
									__m128i * sse_tmp = reinterpret_cast<__m128i*>(tmp + off);
									__m128i v = _mm_set_epi32((coul >> 24) & 0xFF, (coul >> 16) & 0xFF, (coul >> 8) & 0xFF, coul & 0xFF);
									__m128i a1 = _mm_set1_epi32(aera1);
									_mm_store_si128(sse_tmp, _mm_add_epi32(_mm_load_si128(sse_tmp), _mm_mullo_epi32(a1, v)));
									__m128i a2 = _mm_set1_epi32(aera2);
									_mm_store_si128(sse_tmp + 1, _mm_add_epi32(_mm_load_si128(sse_tmp + 1), _mm_mullo_epi32(a2, v)));
									}
								#else					
									tmp[off]     += aera1*(coul & 0xFF);
									tmp[off + 1] += aera1*((coul >> 8) & 0xFF);
									tmp[off + 2] += aera1*((coul >> 16) & 0xFF);
									tmp[off + 3] += aera1*((coul >> 24) & 0xFF);
									tmp[off + 4] += aera2*(coul & 0xFF);
									tmp[off + 5] += aera2*((coul >> 8) & 0xFF);
									tmp[off + 6] += aera2*((coul >> 16) & 0xFF);
									tmp[off + 7] += aera2*((coul >> 24) & 0xFF);
								#endif				
								di += overflowx;
								epsx -= LX*overflowx;
								}
								
							}
						dj += overflowy;
						epsy -= LY*overflowy;
						}
					if (dj < dest_sy)
						{ // flush the last line
						for (uint64 k = 0; k < dest_sx; k++)
							{
							auto off = 4 * k;
							uint32 c1 = ((tmp[off] * ONE_OVER_LX_LY_RED) >> BIT_DIV); c1 |= ((c1 & 256) >> 8) * 255;
							uint32 c2 = ((tmp[off + 1] * ONE_OVER_LX_LY_RED) >> BIT_DIV); c2 |= ((c2 & 256) >> 8) * 255;
							uint32 c3 = ((tmp[off + 2] * ONE_OVER_LX_LY_RED) >> BIT_DIV); c3 |= ((c3 & 256) >> 8) * 255;
							uint32 c4 = ((tmp[off + 3] * ONE_OVER_LX_LY_RED) >> BIT_DIV); c4 |= ((c4 & 256) >> 8) * 255;
							if (!USE_FUNCION_CALL) // <- conditional removed at compile time since USE_FUNCION_CALL is a compile time constant. 
								{ // fast access, optimized by compiler
								dest_data[dest_stride*dj + k].color = c1 + (c2 << 8) + (c3 << 16) + (c4 << 24);
								}
							else
								{ // use function call instead
								funwrite(k, dj, c1 + (c2 << 8) + (c3 << 16) + (c4 << 24));
								}
							}
						dj++;
						}
					MTOOLS_ASSERT(dj == dest_sy);
					free(tmp);
					return;
					#undef MTOOLS_ind_A_geq_B_U64
				}
				


			/*************************************************************
			* BLITTING / BLENDING / MASKING                              *
			*                                                            *
			**************************************************************/


			/* fast blitting of a region, do not work for overlap */
			inline static void _blitRegion(RGBc * pdest, int64 dest_stride, RGBc * psrc, int64 src_stride, int64 sx, int64 sy)
				{
				if (sx < 20) 
					{ // for small width, faster to copy by element by element than using memcpy
					_blitRegionUp(pdest, dest_stride, psrc, src_stride, sx, sy);
					return;
					}
				// memcpy for each line
				for (int64 j = 0; j < sy; j++)
					{
					memcpy(pdest + j*dest_stride, psrc + j*src_stride, 4 * sx);
					}
				}


			/* blit a region, in increasing order */
			inline static void _blitRegionUp(RGBc * pdest, int64 dest_stride, RGBc * psrc, int64 src_stride, int64 sx, int64 sy)
				{
				for (int64 j = 0; j < sy; j++)
					{
					RGBc * pdest2 = pdest + j*dest_stride;
					RGBc * psrc2 = psrc + j*src_stride;
					for (int64 i = 0; i < sx; i++)
						{
						pdest2[i] = psrc2[i];
						}
					}
				return;
				}


			/* blit a region, in decreasing order */
			inline static void _blitRegionDown(RGBc * pdest, int64 dest_stride, RGBc * psrc, int64 src_stride, int64 sx, int64 sy)
				{
				for (int64 j = sy-1; j >= 0; j--)
					{
					RGBc * pdest2 = pdest + j*dest_stride;
					RGBc * psrc2 = psrc + j*src_stride;
					for (int64 i = sx-1; i >= 0; i--)
						{
						pdest2[i] = psrc2[i];
						}
					}
				return;
				}


			/* blend a region, in increasing order */
			inline static void _blendRegionUp(RGBc * pdest, int64 dest_stride, RGBc * psrc, int64 src_stride, int64 sx, int64 sy, float op)
				{
				uint32 uop = (uint32)(256 * op);
				for (int64 j = 0; j < sy; j++)
					{
					for (int64 i = 0; i < sx; i++)
						{
						pdest[i].blend(psrc[i], uop);
						}
					pdest += dest_stride;
					psrc += src_stride;
					}
				return;
				}


			/* blend a region, in decreasing order */
			inline static void _blendRegionDown(RGBc * pdest, int64 dest_stride, RGBc * psrc, int64 src_stride, int64 sx, int64 sy, float op)
				{
				uint32 uop = (uint32)(256 * op);
				for (int64 j = sy - 1; j >= 0; j--)
					{
					RGBc * pdest2 = pdest + j*dest_stride;
					RGBc * psrc2 = psrc + j*src_stride;
					for (int64 i = sx - 1; i >= 0; i--)
						{
						pdest2[i].blend(psrc2[i], uop);
						}
					}
				return;
				}



			/* mask a region */
			inline static void _maskRegion(RGBc * pdest, int64 dest_stride, RGBc * psrc, int64 src_stride, int64 sx, int64 sy, RGBc color)
				{
				uint32 uop = mtools::convertAlpha_0xFF_to_0x100(color.comp.A);
				for (int64 j = 0; j < sy; j++)
					{
					for (int64 i = 0; i < sx; i++)
						{
						pdest[i].blend(color, convertAlpha_0xFF_to_0x100(psrc[i].comp.A));
						}
					pdest += dest_stride;
					psrc += src_stride;
					}
				return;
				}


			/*************************************************************
			* CAIRO                                                      *
			* These methods affect: _pcairo_surface, _pcairo_context     *
			**************************************************************/

			/* tell cairo that the data buffer is possibly dirty */
			void _cairomarkdirty() const
				{
				if (_pcairo_surface != nullptr)	cairo_surface_mark_dirty((cairo_surface_t *)_pcairo_surface);
				}

			/* flush all cairo operation */
			void _cairoflush() const
				{
				if (_pcairo_surface != nullptr)	cairo_surface_flush((cairo_surface_t *)_pcairo_surface);
				}

			/* remove the cairo objects */
			void _removecairo() const
				{
				if (_pcairo_context != nullptr) { cairo_destroy((cairo_t *)_pcairo_surface); _pcairo_context = nullptr; }
				if (_pcairo_surface != nullptr) { cairo_surface_destroy((cairo_surface_t *)_pcairo_surface); _pcairo_surface = nullptr; }
				}

			/* create the cairo objects if needed */
			bool _createcairo(bool stopOnError) const
				{
				if (_pcairo_surface == nullptr)
					{
					_pcairo_surface = cairo_image_surface_create_for_data((unsigned char *)_data, CAIRO_FORMAT_ARGB32, (int)_lx, (int)_ly, (int)(4 * _stride));
					if (cairo_surface_status((cairo_surface_t*)_pcairo_surface) != CAIRO_STATUS_SUCCESS)
						{
						_removecairo();
						if (stopOnError) MTOOLS_ERROR("Cannot create CAIRO surface");
						return false;
						}
					}
				if (_pcairo_context == nullptr)
					{
					_pcairo_context = cairo_create((cairo_surface_t*)_pcairo_surface);
					if (cairo_status((cairo_t*)_pcairo_context) != CAIRO_STATUS_SUCCESS)
						{
						_removecairo();
						if (stopOnError) MTOOLS_ERROR("Cannot create CAIRO context");
						return false;
						}
					}
				_cairomarkdirty();
				return true;
				}



			/*************************************************************
			* MEMORY MANAGEMENT                                          *
			* These methods affect: _deletepointer and _data             *
			**************************************************************/

			/* allocate memory, updates _data, and _deletepointer */
			void _allocate(int64 ly, int64 stride, RGBc * databuffer)
				{
				int64 memsize = 16 + ((databuffer == nullptr) ? 4 * ly*stride : 0); // 16 byte + the image buffer size if needed.
				_deletepointer = (uint32*)malloc(memsize);
				if (_deletepointer == nullptr) { MTOOLS_ERROR(std::string("malloc error: cannot allocate ") + mtools::toStringMemSize(memsize)); }
				(*_deletepointer) = 1; // set reference count to 1
				_data = (databuffer == nullptr) ? ((RGBc*)(_deletepointer + 4)) : databuffer; // if allocated, buffer start 16 bytes (4 uint32) after the deletepointr.
				}

			/* decrease reference count and deallocate if not buffer not referenced anymore */
			void _deallocate()
				{
				if ((_deletepointer != nullptr) && ((--(*_deletepointer)) == 0))
					{ // deallocate 
					free(_deletepointer);
					}
				// not allowed to access the buffer anymore, so we null the adress.  
				_deletepointer = nullptr;
				_data = nullptr;
				}

			/* copy buffer pointer and increment the reference count */
			void _shallow_copy(uint32 * deletepointer, RGBc * data)
				{
				(*deletepointer)++;
				_deletepointer = deletepointer;
				_data = data;
				}


			/********************************************
			* DATA                                      *
			********************************************/

			int64				_lx;						// width of the image in pixels
			int64				_ly;						// height of the image in pixels
			int64				_stride;					// length of a line in pixels
			uint32 *			_deletepointer;				// pointer to the beginning of the allocated memory
			RGBc *				_data;						// pointer to the image buffer
			mutable void *		_pcairo_surface;			// pointer to the optional cairo surface
			mutable void *		_pcairo_context;			// pointer to the optional cairo context

		};


	}

/* end of file */

