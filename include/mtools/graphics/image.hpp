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

#if (MTOOLS_USE_CAIRO)
#include <cairo.h>
#endif

#include "../io/console.hpp"


// use libpng
#define cimg_use_png
// use libjpeg
#define cimg_use_jpeg

#if (MTOOLS_USE_OPENMP)
// use openmp (only for GCC for the time being)
#if defined __GNUC__ && !defined __clang__ 
#define cimg_use_openmp
#endif
#endif

#if defined (_MSC_VER) 
#pragma warning( push )				// disable some warnings
#pragma warning( disable : 4146 )	//
#pragma warning( disable : 4197 )	//
#pragma warning( disable : 4244 )	//
#pragma warning( disable : 4267 )	//
#pragma warning( disable : 4305 )	//
#pragma warning( disable : 4309 )	//
#pragma warning( disable : 4312 )	//
#pragma warning( disable : 4319 )	//
#pragma warning( disable : 4723 )	//
#endif

// disable CImg's graphic capabilility when in console mode only
// or when using osx.
#if (MTOOLS_BASIC_CONSOLE) || defined __APPLE__
#define cimg_display 0		
#endif

#include <CImg.h>	    // the header for the cimg library
#undef min
#undef max

#if defined (_MSC_VER) 
#pragma warning( pop )
#endif


namespace mtools
	{


	/* forward declaration */
	class Font;


	// TODO : make it 
#define DEFAULT_MIN_THICKNESS	0.1


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
	 * used if  MTOOLS_USE_SSE is non zero. 
	 * 
	 **/
	class Image
		{

		public:

			static const bool DEFAULT_AA			= true;			///< default mode is to use antialiasing.
			static const bool DEFAULT_BLEND			= true;			///< default mode is to use blending.
			static const bool DEFAULT_GRID_ALIGN    = true;			///< default mode is to align to grid for faster drawing.


			/******************************************************************************************************************************************************
			*******************************************************************************************************************************************************
			*																				   																      *
			*                                                      CONSTRUCTION / COPY / ASSIGNEMENT                                                              *
			*																																					  *
			*******************************************************************************************************************************************************
			*******************************************************************************************************************************************************/

			/**
			 * Default constructor.
			 * 
			 * 	Construct an empty image.
			 */
			inline Image() :	_lx(0), _ly(0), _stride(0),
								_deletepointer(nullptr), _data(nullptr),
								_pcairo_surface(nullptr), _pcairo_context(nullptr)
				{}


			/**
			 * Create a image from a file.
			 *
			 * @param	filename Name of the file (must have extension "png" or "jpg"). If the operation fails,
			 * 					 the image is empty.
			 */
			inline Image(const char * filename) : Image()
				{
				load(filename);
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
			inline Image(int64 lx, int64 ly, int64 padding = 0) :   _lx(lx), _ly(ly), _stride(lx + ((padding < 0) ? 0 : padding)),
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
			inline Image(const iVec2 & dim, int64 padding = 0) : Image(dim.X(), dim.Y(),padding)
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
			inline Image(int64 lx, int64 ly, RGBc bkColor, int64 padding = 0) : Image(lx, ly, padding)
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
			inline Image(const iVec2 & dim, RGBc bkColor, int64 padding = 0) : Image(dim.X(), dim.Y(), bkColor, padding)
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
			inline Image(RGBc * data, int64 lx, int64 ly, bool shallow, int64 padding = 0) : _lx(lx), _ly(ly), _stride(lx + padding),
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
			inline Image(RGBc * data, const iVec2 & dim, bool shallow, int64 padding = 0) : Image(data, dim.X(), dim.Y(), shallow, padding)
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
			inline Image(Image && source) : _lx(source._lx), _ly(source._ly), _stride(source._stride),
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
			inline Image(const Image & source) : Image(source, 0, 0, source._lx, source._ly, true)
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
			inline Image(const Image & source, bool shallow, int64 padding = 0) : Image(source, 0, 0, source._lx, source._ly, shallow, padding)
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
			inline Image(const Image & source, int64 x0, int64 y0, int64 newlx, int64 newly, bool shallow, int64 padding = 0) :
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
			inline Image(const Image & source, const iBox2 & B, bool shallow, int64 padding = 0) : Image(source, B.min[0], B.min[1], B.max[0] - B.min[0] + 1, B.max[1] - B.min[1] + 1, shallow, padding)
				{
				}


			/**
			 * Return a deep copy of the object with its own pixel buffer.
			 *
			 * @param	padding padding for the returned image.
			 *
			 * @return	A copy of the image which does not share its pixel buffer with anyone.
			 */
			inline Image get_standalone(int64 padding = 0) const
				{
				return Image(*this,false,padding); // ctor + move operator or in place contruction. 
				}


			/**
			 * Make a deep copy of im into this object.
			 * Same as *this = im.get_standalone();
			 * 
			 * @param	im	The image to copy. 
			 **/
			inline void assign(const Image & im)
				{
				*this = im.get_standalone();
				}


			/**
			* Move the content of this image into anotheer image. This image is left empty.
			*
			* @param [in,out]	dest Destination image (its current content is discarded).
			*/
			inline void move_to(Image & dest)
				{
				if (this != &dest)
					{
					dest.empty();
					dest._lx = _lx;
					dest._ly = _ly;
					dest._stride = _stride;
					dest._data = _data;
					dest._deletepointer = _deletepointer;
					dest._pcairo_surface = _pcairo_surface;
					dest._pcairo_context = _pcairo_context;
					_lx = 0;
					_ly = 0;
					_stride = 0;
					_data = nullptr;
					_deletepointer = nullptr;
					_pcairo_context = nullptr;
					_pcairo_surface = nullptr;
					}
				return;
				}


			/**
			 * Move assignment operator.
			 **/
			inline Image & operator=(Image && source)
				{
				source.move_to(*this);
				return *this;
				}


			/**
			 * Shallow assignment operator. Make a copy of the image that shares the same pixel buffer as the source.
			 */
			inline Image & operator=(const Image & source)
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


			/******************************************************************************************************************************************************
			*******************************************************************************************************************************************************
			*																				   																      *
			*                                             CROPPING / EXPANDING / RAW RESIZING IMAGE                                                               *
			*																																					  *
			*******************************************************************************************************************************************************
			*******************************************************************************************************************************************************/

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
			inline void crop(int64 x0, int64 y0, int64 newlx, int64 newly, bool shallow, int64 padding = 0)
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
			inline void crop(const iBox2 & B, bool shallow, int64 padding = 0)
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
			inline Image get_crop(int64 x0, int64 y0, int64 newlx, int64 newly, bool shallow, int64 padding = 0) const
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
			inline Image get_crop(const iBox2 & B, bool shallow, int64 padding = 0) const
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
			inline void cropBorder(int64 left, int64 right, int64 up, int64 down, bool shallow, int64 padding = 0)
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
			inline Image get_cropBorder(int64 left, int64 right, int64 up, int64 down, bool shallow, int64 padding = 0) const
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
			inline Image sub_image(int64 x0, int64 y0, int64 newlx, int64 newly) const
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
			inline Image sub_image(const iBox2 & B) const
				{
				return get_crop(B,true);
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
			inline void expand(int64 left, int64 right, int64 up, int64 down, RGBc bkcolor = RGBc::c_Transparent, int64 padding = 0)
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
			inline Image get_expand(int64 left, int64 right, int64 up, int64 down, RGBc bkcolor = RGBc::c_Transparent, int64 padding = 0) const
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
			inline void resizeRaw(int64 newlx, int64 newly, bool shrinktofit = false, int64 padding = 0)
				{
				if ((newlx <= 0) || (newly <= 0)) { empty(); return; }
				if (padding <= 0) { padding = 0; }
				if ((_lx == newlx) && (_ly == newly) && ((newlx + padding) == _stride)) return;
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
			inline void resizeRaw(const iVec2 & newdim, bool shrinktofit = false, int64 padding = 0)
				{
				resizeRaw(newdim.X(), newdim.Y(), shrinktofit, padding);
				}



			/******************************************************************************************************************************************************
			*******************************************************************************************************************************************************
			*																				   																      *
			*                                                             CImg CONVERSION                                                                 *
			*																																					  *
			*******************************************************************************************************************************************************
			*******************************************************************************************************************************************************/


			/**
			 * Initialize this image from a CImg image. current image content is discarded.
			 *
			 * @param	im		   	The source CImg image.
			 * @param	premult		(Optional) True to perform alpha pre-multiplication during conversion.
			 **/
			void fromCImg(const cimg_library::CImg<unsigned char> & im, bool premult = true)
				{
				empty(); 
				if (im.is_empty()) return; 
				resizeRaw(im.width(), im.height(),true,0);
				RGBc * pdest = _data;
				const size_t pad = (size_t)(_stride - _lx);
				const unsigned char * pR = im.data(0, 0, 0, 0);
				const unsigned char * pG = im.data(0, 0, 0, 1);
				const unsigned char * pB = im.data(0, 0, 0, 2);
				if (im.spectrum() == 3)
					{
					for (int64 j = 0; j < _ly; j++)
						{
						for (int64 i = 0; i < _lx; i++) { *(pdest++) = RGBc(*(pR++), *(pG++), *(pB++), 255); }
						pdest += pad;
						}						
					return;
					}
				if (im.spectrum() == 4)
					{
					const unsigned char * pA = im.data(0, 0, 0, 3);
					if (premult)
						{
						for (int64 j = 0; j < _ly; j++)
							{
							for (int64 i = 0; i < _lx; i++) { RGBc cc = RGBc(*(pR++), *(pG++), *(pB++), *(pA++)); cc.premultiply();  *(pdest++) = cc; }
							pdest += pad;
							}
						}
					else
						{
						for (int64 j = 0; j < _ly; j++)
							{
							for (int64 i = 0; i < _lx; i++) { *(pdest++) = RGBc(*(pR++), *(pG++), *(pB++), *(pA++)); }
							pdest += pad;
							}
						}
					return;
					}
				MTOOLS_DEBUG("Invalid CImg image");
				clear(RGBc::c_White);
				return;
				}


			/**
			 * Copy the content of this image into a CImg image.
			 *
			 * @param [in,out]	im				  	The destination CImg image. Its current content is
			 * 										discarded and its number of channels is set to 4.
			 * @param 		  	remove_premul   	(Optional) True to remove alpha channel premultiplication
			 * 										when converting.
			 **/
			void toCImg(cimg_library::CImg<unsigned char> & im, bool remove_premult = true) const
				{
				if (isEmpty()) { im.assign(); return; }
				im.assign((unsigned int)_lx, (unsigned int)_ly, 1, 4);
				const RGBc * psrc = _data;
				const size_t pad = (size_t)(_stride - _lx);
				unsigned char * pR = im.data(0, 0, 0, 0);
				unsigned char * pG = im.data(0, 0, 0, 1);
				unsigned char * pB = im.data(0, 0, 0, 2);
				unsigned char * pA = im.data(0, 0, 0, 3);
				if (remove_premult)
					{
					for (int64 j = 0; j < _ly; j++)
						{
						for (int64 i = 0; i < _lx; i++)
							{
							RGBc col = *(psrc++); col.unpremultiply();
							*(pR++) = col.comp.R; *(pG++) = col.comp.G; *(pB++) = col.comp.B; *(pA++) = col.comp.A;
							}
						psrc += pad;
						}
					}
				else
					{
					for (int64 j = 0; j < _ly; j++)
						{
						for (int64 i = 0; i < _lx; i++)
							{
							RGBc col = *(psrc++);
							*(pR++) = col.comp.R; *(pG++) = col.comp.G; *(pB++) = col.comp.B; *(pA++) = col.comp.A;
							}
						psrc += pad;
						}
					}
				return;
				}


			/******************************************************************************************************************************************************
			*******************************************************************************************************************************************************
			*																				   																      *
			*                                                      BLITTING / BLENDING / MASKING                                                                  *
			*																																					  *
			*******************************************************************************************************************************************************
			*******************************************************************************************************************************************************/


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
			inline void blit(const Image & sprite, int64 dest_x, int64 dest_y, int64 sprite_x, int64 sprite_y, int64 sx, int64 sy)
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
				_blitRegion(_data + (dest_y*_stride) + dest_x, _stride, sprite._data + (sprite_y*sprite._stride) + sprite_x, sprite._stride, sx, sy);
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
			inline void blit(const Image & sprite, const iVec2 & dest_pos, const iBox2 & sprite_box)
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
			inline void blit(const Image & sprite, int64 dest_x, int64 dest_y)
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
			inline void blit(const Image & sprite, const iVec2 & dest_pos)
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
			inline void blitInside(int64 dest_x, int64 dest_y, int64 src_x, int64 src_y,int64 sx, int64 sy)
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
			inline void blitInside(const iVec2 & dest_pos, const iBox2 & src_box)
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
			inline void blend(const Image & sprite, int64 dest_x, int64 dest_y, int64 sprite_x, int64 sprite_y, int64 sx, int64 sy, float opacity = 1.0f)
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
				_blendRegionUp(_data + (dest_y*_stride) + dest_x, _stride, sprite._data + (sprite_y*sprite._stride) + sprite_x, sprite._stride, sx, sy,opacity);
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
			inline void blend(const Image & sprite, const iVec2 & dest_pos, const iBox2 & sprite_box, float opacity = 1.0f)
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
			inline void blend(const Image & sprite, int64 dext_x, int64 dest_y, float opacity= 1.0f)
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
			inline void blend(const Image & sprite, const iVec2 & dest_pos, float opacity = 1.0f)
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
			inline void blendInside(int64 dest_x, int64 dest_y, int64 src_x, int64 src_y, int64 sx, int64 sy, float opacity = 1.0f)
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
			inline void blendInside(const iVec2 & dest_pos, const iBox2 & src_box, float opacity = 1.0f)
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
			inline void mask(const Image & sprite, int64 dest_x, int64 dest_y, int64 sprite_x, int64 sprite_y, int64 sx, int64 sy, RGBc color)
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
				_maskRegion(_data + (dest_y*_stride) + dest_x, _stride, sprite._data + (sprite_y*sprite._stride) + sprite_x, sprite._stride, sx, sy, color);
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
			inline void mask(const Image & sprite, const iVec2 & dest_pos, const iBox2 & sprite_box, RGBc color)
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
			inline void mask(const Image & sprite, int64 dext_x, int64 dest_y, RGBc color)
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
			inline void mask(const Image & sprite, const iVec2 & dest_pos, RGBc color)
				{
				mask(sprite, dest_pos.X(), dest_pos.Y(), 0, 0, sprite._lx, sprite._ly, color);
				}



			/******************************************************************************************************************************************************
			*******************************************************************************************************************************************************
			*																				   																      *
			*                                                                    RESCALING                                                                        *
			*																																					  *
			*******************************************************************************************************************************************************
			*******************************************************************************************************************************************************/


			/**
			 * Rescale this image to a given size.
			 * 
			 * This method discard the current data buffer an create another one.
			 *
			 * All input paramters are valid : regions outside of the source or destination image are
			 * automatically discarded (considered transparent).
			 *
			 * @param	quality   	in [0,10]. 0 = low quality (fast) and 10 = max quality (slow).
			 * @param	newlx	  	new image width.
			 * @param	newly	  	new image height.
			 * @param	newpadding	horizontal padding.
			 *
			 * @return	The real quality of the rescaling performed. At least quality but may be higher.
			 **/
			inline int rescale(int quality, int64 newlx, int64 newly, int64 newpadding = 0)
				{
				return rescale(quality, newlx, newly, 0, 0, _lx, _ly, newpadding);
				}


			/**
			 * Rescale this image to a given size.
			 * 
			 * This method discard the current data buffer an create another one.
			 *
			 * All input paramters are valid : regions outside of the source or destination image are
			 * automatically discarded (considered transparent).
			 *
			 * @param	quality  	in [0,10]. 0 = low quality (fast) and 10 = max quality (slow).
			 * @param	newsize   	new image size.
			 * @param	newpadding	horizontal padding.
			 *
			 * @return	The real quality of the rescaling performed. At least quality but may be higher.
			 **/
			inline int rescale(int quality, const iVec2 & newsize, int64 newpadding = 0)
				{
				return rescale(quality, newsize.X(), newsize.Y(), 0, 0, _lx, _ly, newpadding);
				}


			/**
			 * Crop a portion of this image and rescale it to a given size.
			 * 
			 * This method discard the current data buffer an create another one.
			 *
			 * All input paramters are valid : regions outside of the source or destination image are
			 * automatically discarded (considered transparent).
			 *
			 * @param	quality  	in [0,10]. 0 = low quality (fast) and 10 = max quality (slow).
			 * @param	newlx	  	new image width.
			 * @param	newly	  	new image height.
			 * @param	x		  	x-coord of the upper left corner of the rectangle to crop.
			 * @param	y		  	y-coord of the upper left corner of the rectangle to crop.
			 * @param	sx		  	width of the rectangle to crop.
			 * @param	sy		  	height of the rectangle to crop.
			 * @param	newpadding	horizontal padding.
			 *
			 * @return	The real quality of the rescaling performed. At least quality but may be higher.
			 **/
			inline int rescale(int quality, int64 newlx, int64 newly, int64 x, int64 y, int64 sx, int64 sy, int64 newpadding = 0)
				{
				if ((newlx <= 0) || (newly <= 0)) { empty();  return 10; }
				Image im(newlx, newly, newpadding);
				int q = im.blit_rescaled(quality, *this, 0, 0, newlx, newly, x, y, sx, sy);
				*this = im; 
				return q;
				}


			/**
			 * Crop a portion of this image and rescale it to a given size.
			 * 
			 * This method discard the current data buffer an create another one.
			 *
			 * All input paramters are valid : regions outside of the source or destination image are
			 * automatically discarded (considered transparent).
			 *
			 * @param	quality  	in [0,10]. 0 = low quality (fast) and 10 = max quality (slow).
			 * @param	newsize   	new image size.
			 * @param	B		  	rectangle to crop.
			 * @param	newpadding	horizontal padding.
			 * 						
			 * @return	The real quality of the rescaling performed. At least quality but may be higher.
			 **/
			inline int rescale(int quality, const iVec2 & newsize, const iBox2 & B, int64 newpadding = 0)
				{
				return rescale(quality, newsize.X(), newsize.Y(), B.min[0], B.min[1], B.max[0] - B.min[0] + 1, B.max[1] - B.min[1] + 1, newpadding);
				}


			/**
			 * Return a copy of this image rescaled to a given size using the fastest method available (low
			 * quality).
			 *
			 * All input paramters are valid : regions outside of the source or destination image are
			 * automatically discarded (considered transparent).
			 *
			 * @param	quality  	in [0,10]. 0 = low quality (fast) and 10 = max quality (slow).
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
			* All input paramters are valid : regions outside of the source or destination image are
			* automatically discarded (considered transparent).
			*
			* @param	quality  	in [0,10]. 0 = low quality (fast) and 10 = max quality (slow).
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
			* All input paramters are valid : regions outside of the source or destination image are
			* automatically discarded (considered transparent).
			*
			* @param	quality  	in [0,10]. 0 = low quality (fast) and 10 = max quality (slow).
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
			 * All input paramters are valid : regions outside of the source or destination image are
			 * automatically discarded (considered transparent).
			 *
			 * @param	quality  	in [0,10]. 0 = low quality (fast) and 10 = max quality (slow).
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
			 * All input paramters are valid : regions outside of the source or destination image are
			 * automatically discarded (considered transparent).
			 *
			 * @param	quality  	in [0,10]. 0 = low quality (fast) and 10 = max quality (slow).
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
			 * All input paramters are valid : regions outside of the source or destination image are
			 * automatically discarded (considered transparent).
			 *
			 * @param	quality  	in [0,10]. 0 = low quality (fast) and 10 = max quality (slow).
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
			 * All input paramters are valid : regions outside of the source or destination image are
			 * automatically discarded (considered transparent).
			 *
			 * @param	quality  	in [0,10]. 0 = low quality (fast) and 10 = max quality (slow).
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
				if ((dest_x >= lx()) || (dest_y >= ly())) return MAX_QUALITY;
				if ((sprite_x >= sprite.lx()) || (sprite_y >= sprite.ly())) return MAX_QUALITY;
				if ((dest_x + dest_sx <= 0) || (dest_y + dest_sy <= 0)) return MAX_QUALITY;
				if ((sprite_x + sprite_sx <= 0) || (sprite_y + sprite_sy <= 0)) return MAX_QUALITY;

				if (overlapMemoryWith(sprite))
					{ // this and sprite overlap so we must make a copy of sprite
					return blit_rescaled(quality, sprite.get_standalone(), dest_x, dest_y, dest_sx, dest_sy, sprite_x, sprite_y, sprite_sx, sprite_sy);
					}

				if (dest_x < 0)
					{
					int64 new_dest_sx = dest_sx + dest_x;
					if (new_dest_sx <= 0) return MAX_QUALITY;
					int64 new_dest_x = 0;
					int64 new_sprite_sx = (int64)(sprite_sx * ((double)new_dest_sx) / ((double)dest_sx));
					if (new_sprite_sx <= 0) return MAX_QUALITY;
					int64 new_sprite_x = sprite_x + (int64)(sprite_sx * ((double)(-dest_x)) / ((double)(dest_sx)));
					dest_x = new_dest_x;
					dest_sx = new_dest_sx;
					sprite_x = new_sprite_x;
					sprite_sx = new_sprite_sx;
					}

				if (dest_y < 0)
					{
					int64 new_dest_sy = dest_sy + dest_y;
					if (new_dest_sy <= 0) return MAX_QUALITY;
					int64 new_dest_y = 0;
					int64 new_sprite_sy = (int64)(sprite_sy * ((double)new_dest_sy) / ((double)dest_sy));
					if (new_sprite_sy <= 0) return MAX_QUALITY;
					int64 new_sprite_y = sprite_y + (int64)(sprite_sy * ((double)(-dest_y)) / ((double)(dest_sy)));
					dest_y = new_dest_y;
					dest_sy = new_dest_sy;
					sprite_y = new_sprite_y;
					sprite_sy = new_sprite_sy;
					}

				if (sprite_x < 0)
					{
					int64 new_sprite_sx = sprite_sx + sprite_x;
					if (new_sprite_sx <= 0) return MAX_QUALITY;
					int64 new_sprite_x = 0;
					int64 new_dest_sx = (int64)(dest_sx * ((double)new_sprite_sx) / ((double)sprite_sx));
					if (new_dest_sx <= 0) return MAX_QUALITY;
					int64 new_dest_x = dest_x + (int64)(dest_sx * ((double)(-sprite_x)) / ((double)(sprite_sx)));
					dest_x = new_dest_x;
					dest_sx = new_dest_sx;
					sprite_x = new_sprite_x;
					sprite_sx = new_sprite_sx;
					}

				if (sprite_y < 0)
					{
					int64 new_sprite_sy = sprite_sy + sprite_y;
					if (new_sprite_sy <= 0) return MAX_QUALITY;
					int64 new_sprite_y = 0;
					int64 new_dest_sy = (int64)(dest_sy * ((double)new_sprite_sy) / ((double)sprite_sy));
					if (new_dest_sy <= 0) return MAX_QUALITY;
					int64 new_dest_y = dest_y + (int64)(dest_sy * ((double)(-sprite_y)) / ((double)(sprite_sy)));
					dest_y = new_dest_y;
					dest_sy = new_dest_sy;
					sprite_y = new_sprite_y;
					sprite_sy = new_sprite_sy;
					}

				if (dest_x + dest_sx > lx())
					{
					int64 new_dest_sx = lx() - dest_x;
					if (new_dest_sx <= 0) return MAX_QUALITY;
					int64 new_sprite_sx = (int64)(sprite_sx * ((double)new_dest_sx) / ((double)dest_sx));
					if (new_sprite_sx <= 0) return MAX_QUALITY;
					dest_sx = new_dest_sx;
					sprite_sx = new_sprite_sx;
					}
			
				if (dest_y + dest_sy > ly())
					{
					int64 new_dest_sy = ly() - dest_y;
					if (new_dest_sy <= 0) return MAX_QUALITY;
					int64 new_sprite_sy = (int64)(sprite_sy * ((double)new_dest_sy) / ((double)dest_sy));
					if (new_sprite_sy <= 0) return MAX_QUALITY;
					dest_sy = new_dest_sy;
					sprite_sy = new_sprite_sy;
					}

				if (sprite_x + sprite_sx > sprite.lx())
					{
					int64 new_sprite_sx = sprite.lx() - sprite_x;
					if (new_sprite_sx <= 0) return MAX_QUALITY;
					int64 new_dest_sx = (int64)(dest_sx * ((double)new_sprite_sx) / ((double)sprite_sx));
					if (new_dest_sx <= 0) return MAX_QUALITY;
					dest_sx = new_dest_sx;
					sprite_sx = new_sprite_sx;
					}

				if (sprite_y + sprite_sy > sprite.ly())
					{
					int64 new_sprite_sy = sprite.ly() - sprite_y;
					if (new_sprite_sy <= 0) return MAX_QUALITY;
					int64 new_dest_sy = (int64)(dest_sy * ((double)new_sprite_sy) / ((double)sprite_sy));
					if (new_dest_sy <= 0) return MAX_QUALITY;
					dest_sy = new_dest_sy;
					sprite_sy = new_sprite_sy;
					}

				// normally, this should never fail but we make sure nevertheless since it cost nothing
				// compared the rest of the method.
				MTOOLS_INSURE((dest_x >= 0) && (dest_x + dest_sx <= lx()));
				MTOOLS_INSURE((dest_y >= 0) && (dest_y + dest_sy <= ly()));
				MTOOLS_INSURE((sprite_x >= 0) && (sprite_x + sprite_sx <= sprite.lx()));
				MTOOLS_INSURE((sprite_y >= 0) && (sprite_y + sprite_sy <= sprite.ly()));

				if ((dest_sx == sprite_sx) && (dest_sy == sprite_sy))
					{ // no rescaling
					_blitRegion(_data + (dest_y*_stride) + dest_x, _stride, sprite._data + (sprite_y*sprite._stride) + sprite_x, sprite._stride, dest_sx, dest_sy);
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
			* @param	quality  	in [0,10]. 0 = low quality (fast) and 10 = max quality (slow).
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



			/******************************************************************************************************************************************************
			*******************************************************************************************************************************************************
			*																				   																      *
			*                                                            TEXT DRAWING METHODS                                                                     *
			*																																					  *
			*******************************************************************************************************************************************************
			*******************************************************************************************************************************************************/



			/**
			 * Draw background of the rectangle that enclosed a given text. Color bkcolor is blend over the
			 * image.
			 *
			 * @param	x	   	x coordinate of the text reference position.
			 * @param	y	   	y coordinate of the text reference position.
			 * @param	txt	   	the text.
			 * @param	txt_pos	Positioning method (combination of MTOOLS_TEXT_XCENTER, MTOOLS_TEXT_LEFT,
			 * 					MTOOLS_TEXT_RIGHT, MTOOLS_TEXT_TOP, MTOOLS_TEXT_BOTTOM, MTOOLS_TEXT_YCENTER).
			 * @param	bkcolor	The color to blend over.
			 * @param	font   	the font to use.
			 **/
			void draw_text_background(int64 x, int64 y, const std::string & txt, int txt_pos, RGBc bkcolor, const Font * font); 


			/**
			 * Draw background of the rectangle that enclosed a given text. Color bkcolor is blend over the
			 * image. This version uses the default font (gFont).
			 *
			 * @param	x			x coordinate of the text reference position.
			 * @param	y			y coordinate of the text reference position.
			 * @param	txt			the text.
			 * @param	txt_pos 	Positioning method (combination of MTOOLS_TEXT_XCENTER, MTOOLS_TEXT_LEFT,
			 * 						MTOOLS_TEXT_RIGHT, MTOOLS_TEXT_TOP, MTOOLS_TEXT_BOTTOM, MTOOLS_TEXT_YCENTER).
			 * @param	bkcolor 	The color to blend over.
			 * @param	fontsize	the font size to use.
			 **/
			void draw_text_background(int64 x, int64 y, const std::string & txt, int txt_pos, RGBc bkcolor, int fontsize);


			/**
			 * Draw background of the rectangle that enclosed a given text. Color bkcolor is blend over the
			 * image.
			 *
			 * @param	pos	   	the text reference position.
			 * @param	txt	   	the text.
			 * @param	txt_pos	Positioning method (combination of MTOOLS_TEXT_XCENTER, MTOOLS_TEXT_LEFT,
			 * 					MTOOLS_TEXT_RIGHT, MTOOLS_TEXT_TOP, MTOOLS_TEXT_BOTTOM, MTOOLS_TEXT_YCENTER).
			 * @param	bkcolor	The color to blend over.
			 * @param	font   	the font to use.
			 **/
			inline void draw_text_background(const iVec2 & pos, const std::string & txt, int txt_pos, RGBc bkcolor, const Font * font)
				{
				draw_text_background(pos.X(), pos.Y(), txt, txt_pos, bkcolor, font);
				}


			/**
			 * Draw background of the rectangle that enclosed a given text. Color bkcolor is blend over the
			 * image. This version uses the default font (gFont).
			 *
			 * @param	pos			the text reference position.
			 * @param	txt			the text.
			 * @param	txt_pos 	Positioning method (combination of MTOOLS_TEXT_XCENTER, MTOOLS_TEXT_LEFT,
			 * 						MTOOLS_TEXT_RIGHT, MTOOLS_TEXT_TOP, MTOOLS_TEXT_BOTTOM, MTOOLS_TEXT_YCENTER).
			 * @param	bkcolor 	The color to blend over.
			 * @param	fontsize	the font size to use.
			 **/
			inline void draw_text_background(const iVec2 & pos, const std::string & txt, int txt_pos, RGBc bkcolor, int fontsize)
				{
				draw_text_background( pos.X(), pos.Y() ,  txt, txt_pos, bkcolor, fontsize);
				}


			/**
			 * Draws a text on the image, with a given color and using a given font.
			 *
			 * @param	x	   	x coordinate of the text reference position.
			 * @param	y	   	y coordinate of the text reference position.
			 * @param	txt	   	the text to draw.
			 * @param	txt_pos	Positioning method (combination of MTOOLS_TEXT_XCENTER, MTOOLS_TEXT_LEFT,
			 * 					MTOOLS_TEXT_RIGHT, MTOOLS_TEXT_TOP, MTOOLS_TEXT_BOTTOM, MTOOLS_TEXT_YCENTER).
			 * @param	color  	The color to blend over.
			 * @param	font   	the font to use.
			 **/
			void draw_text(int64 x, int64 y, const std::string & txt, int txt_pos, RGBc color, const Font * font);


			/**
			 * Draws a text on the image with a given color. Use the default font [gFont].
			 *
			 * @param	x			x coordinate of the text reference position.
			 * @param	y			y coordinate of the text reference position.
			 * @param	txt			the text to draw.
			 * @param	txt_pos 	Positioning method (combination of MTOOLS_TEXT_XCENTER, MTOOLS_TEXT_LEFT,
			 * 						MTOOLS_TEXT_RIGHT, MTOOLS_TEXT_TOP, MTOOLS_TEXT_BOTTOM, MTOOLS_TEXT_YCENTER).
			 * @param	color   	The color to blend over.
			 * @param	fontsize	the font size to use.
			 **/
			void draw_text(int64 x, int64 y, const std::string & txt, int txt_pos, RGBc color, int fontsize);


			/**
			* Draws a text on the image, with a given color and using a given font.
			*
			* @param	pos	   	the text reference position.
			* @param	txt	   	the text to draw.
			* @param	txt_pos	Positioning method (combination of MTOOLS_TEXT_XCENTER, MTOOLS_TEXT_LEFT,
			* 					MTOOLS_TEXT_RIGHT, MTOOLS_TEXT_TOP, MTOOLS_TEXT_BOTTOM, MTOOLS_TEXT_YCENTER).
			* @param	color  	The color to blend over.
			* @param	font   	the font to use.
			**/
			inline void draw_text(const iVec2 & pos, const std::string & txt, int txt_pos, RGBc color, const Font * font)
				{
				draw_text(pos.X(), pos.Y(), txt, txt_pos, color, font);
				}


			/**
			* Draws a text on the image with a given color. Use the default font [gFont].
			*
			* @param	pos			the text reference position.
			* @param	txt			the text to draw
			* @param	txt_pos 	Positioning method (combination of MTOOLS_TEXT_XCENTER, MTOOLS_TEXT_LEFT,
			* 						MTOOLS_TEXT_RIGHT, MTOOLS_TEXT_TOP, MTOOLS_TEXT_BOTTOM, MTOOLS_TEXT_YCENTER).
			* @param	color   	The color to blend over.
			* @param	fontsize	the font size to use.
			**/
			inline void draw_text(const iVec2 & pos, const std::string & txt, int txt_pos, RGBc color, int fontsize)
				{
				draw_text(pos.X(), pos.Y(), txt, txt_pos, color, fontsize);
				}



			/******************************************************************************************************************************************************
			*******************************************************************************************************************************************************
			*																				   																      *
			*                                                            DRAWING DOTS, LINES AND CURVES                                                           *
			*																																					  *
			*******************************************************************************************************************************************************
			*******************************************************************************************************************************************************/


			/**
			 * Draw a (square) dot on the image.
			 * 
			 * Use setPixel() and blendPixel or operator() for faster methods to set a single pixel. 
			 * 
			 * @param	P			Position of the center. 
			 * @param	color   	The color.
			 * @param	blending	true to use blending.
			 * @param	penwidth	The pen width (radius of the square: 0 = single pixel). 
			 **/
			void draw_dot(iVec2 P, RGBc color, bool blending, int32 penwidth = 0)
				{
				if (isEmpty()) return;
				if (color.isOpaque()) { blending = false; }
				if (penwidth <= 0)
					{
					if (blending) _updatePixel<true, true, false, false>(P.X(), P.Y(), color, 0, 0); else _updatePixel<false, true, false, false>(P.X(), P.Y(), color, 0, penwidth);
					}
				else
					{
					_correctPenOpacity(color, penwidth);
					if (blending) _updatePixel<true, true, false, true>(P.X(), P.Y(), color, 0, 0); else _updatePixel<false, true, false, true>(P.X(), P.Y(), color, 0, penwidth);
					}
				}


			/**
			 * Draw an horizontal line.
			 *
			 * @param	y			The y coordinate of the line
			 * @param	x1			x value of the start point
			 * @param	x2			x value of the end point
			 * @param	color   	The color to use.
			 * @param	draw_P2 	true to draw the end point.
			 * @param	blending	true to use blending
			 **/
			inline void draw_horizontal_line(int64 y, int64 x1, int64 x2, RGBc color, bool draw_P2 = true, bool blending = true)
				{
				if (isEmpty()) return;
				if ((blending) && (!color.isOpaque())) _horizontalLine<true, true>(y, x1, x2, color, draw_P2); else _horizontalLine<false, true>(y, x1, x2, color, draw_P2);
				}


			/**
			 * Draw a (tick) horizontal line.
			 * 
			 * The tickness correspond to the penwidth of the other method (but here it may also be non-integer using antialiasing).
			 *
			 * @param	y			The y coordinate of the line.
			 * @param	x1			x value of the start point.
			 * @param	x2			x value of the end point.
			 * @param	thickness	The thickness.
			 * @param	color   	The color to use.
			 * @param	draw_P2 	true to draw the end point.
			 * @param	blending	true to use blending.
			 **/
			inline void draw_horizontal_line(int64 y, int64 x1, int64 x2, double thickness, RGBc color, bool draw_P2 = true, bool blending = true, double min_tick = DEFAULT_MIN_THICKNESS)
				{
				if (isEmpty() || (thickness <= 0)) return;
				if (blending) _tickHorizontalLine<true, true>(y, x1, x2, color, draw_P2, thickness, min_tick); else _tickHorizontalLine<false, true>(y, x1, x2, color, draw_P2, thickness,min_tick);
				}


			/**
			 * Draw a vertical line.
			 *
			 * @param	x			The x coordinate of the line.
			 * @param	y1			y value of the start point.
			 * @param	y2			y value of the end point.
			 * @param	color   	The color to use.
			 * @param	draw_P2 	true to draw the end point.
			 * @param	blending	true to use blending.
			 **/
			inline void draw_vertical_line(int64 x, int64 y1, int64 y2, RGBc color, bool draw_P2 = true, bool blending = true)
				{
				if (isEmpty()) return;
				if (blending) _verticalLine<true, true>(x,y1,y2, color, draw_P2); else _verticalLine<false, true>(x,y1,y2, color, draw_P2);
				}


			/**
			 * Draw a (tick) vertical line.
			 *
			 * @param	x			The x coordinate of the line.
			 * @param	y1			y value of the start point.
			 * @param	y2			y value of the end point.
			 * @param	thickness	The thickness.
			 * @param	color   	The color to use.
			 * @param	draw_P2 	true to draw the end point.
			 * @param	blending	true to use blending.
			 **/
			inline void draw_vertical_line(int64 x, int64 y1, int64 y2, double thickness, RGBc color, bool draw_P2 = true, bool blending = true, double min_tick = DEFAULT_MIN_THICKNESS)
				{
				if (isEmpty() || (thickness <= 0)) return;
				if (blending) _tickVerticalLine<true, true>(x, y1, y2, color, draw_P2, thickness, min_tick); else _tickVerticalLine<false, true>(x, y1, y2, color, draw_P2, thickness, min_tick);
				}


			/**
			 * Draw a line.
			 *
			 * @param	P1	   	First point.
			 * @param	P2	   	Second endpoint.
			 * @param	color  	The color to use.
			 * @param	draw_P2	true to draw the endpoint P2.
			 **/
			inline void draw_line(iVec2 P1, iVec2 P2, RGBc color, bool draw_P2)
				{
				if (isEmpty()) return;
				_lineBresenham<false, true, false, false,false,false>(P1, P2, color, draw_P2, 0,0);
				}


			/**
			* Draw a line. 
			*
			* @param	x1		   	x-coord of the first point.
			* @param	y1		   	y-coord of the first point.
			* @param	x2		   	x-coord of the second point.
			* @param	y2		   	y-coord of the second point.
			* @param	color	   	The color to use.
			* @param	draw_P2	true to draw the endpoint P2.
			**/
			MTOOLS_FORCEINLINE void draw_line(int64 x1, int64 y1, int64 x2, int64 y2, RGBc color, bool draw_P2)
				{
				draw_line({ x1, y1 }, { x2, y2 }, color,draw_P2);
				}


			/**
			 * Draw a line. 
			 *
			 * @param	P1		   	First point.
			 * @param	P2		   	Second endpoint.
			 * @param	color	   	The color to use.
			 * @param	draw_P2	   	true to draw the endpoint P2.
			 * @param	blending   	true to use blending instead of simply overwriting the color.
			 * @param	antialiased	true to draw an antialised line.
			 * @param	penwidth   	The pen width (0 = unit width)
			 **/
			inline void draw_line(iVec2 P1, iVec2 P2, RGBc color, bool draw_P2, bool blending, bool antialiased, int32 penwidth = 0)
				{
				if (isEmpty()) return;
				if (penwidth <= 0)
					{
					if (antialiased)
						{
						const int64 of = 10;
						if (!_csLineClip(P1, P2, iBox2(-of, _lx - 1 + of, -of, _ly - 1 + of))) return; // choose box bigger to diminish clipping error. 
						if (blending) _lineBresenhamAA<true, true, false>(P1, P2, color, draw_P2, 0); else _lineBresenhamAA<false, true, false>(P1, P2, color, draw_P2, 0);
						return;
						}
					if ((blending) && (!color.isOpaque())) _lineBresenham<true, true, false, false,false,false>(P1, P2, color, draw_P2, 0, 0); else _lineBresenham<false, true, false, false, false, false>(P1, P2, color, draw_P2, 0, 0);
					return;
					}
				_correctPenOpacity(color, penwidth);
				if (antialiased)
					{
					const int64 of = 10 + 2 * penwidth;
					if (!_csLineClip(P1, P2, iBox2(-of, _lx - 1 + of, -of, _ly - 1 + of))) return; // choose box bigger to diminish clipping error. 
					if (blending) _lineBresenhamAA<true, true, true>(P1, P2, color, draw_P2, penwidth); else _lineBresenhamAA<false, true, true>(P1, P2, color, draw_P2, penwidth);
					return;
					}
				if ((blending) && (!color.isOpaque())) _lineBresenham<true,true, false, true, false, false>(P1, P2, color, draw_P2, penwidth, 0); else _lineBresenham<false,true, false, true, false, false>(P1, P2, color, draw_P2, penwidth, 0);
				return;
				}


			/**
			 * Draw a line. portion outside the image is clipped.
			 *
			 * @param	x1		   	x-coord of the first point.
			 * @param	y1		   	y-coord of the first point.
			 * @param	x2		   	x-coord of the second point.
			 * @param	y2		   	y-coord of the second point.
			 * @param	color	   	The color to use.
			 * @param	draw_P2	   	true to draw the endpoint P2.
			 * @param	blending   	true to use blending instead of simply copying the color.
			 * @param	antialiased	true to draw an antialised line.
			 * @param	penwidth   	The pen width (0 = unit width)
			 **/
			MTOOLS_FORCEINLINE void draw_line(int64 x1, int64 y1, int64 x2, int64 y2, RGBc color, bool draw_P2, bool blending, bool antialiased, int32 penwidth = 0)
				{
				draw_line({ x1, y1 }, { x2,y2 }, color, draw_P2, blending, antialiased, penwidth);
				}


			/**
			 * Draw a quadratic (rational) Bezier curve.
			 *
			 * @param	P1				The first point.
			 * @param	P2				The second point.
			 * @param	PC				The control point.
			 * @param	wc				The control point weight. Must be positive (faster for wc = 1 =
			 * 							classic quad Bezier curve).
			 * @param	color			The color to use.
			 * @param	draw_P2			true to draw the endpoint P2.
			 * @param	blending		true to use blending.
			 * @param	antialiasing	true to use antialiasing.
			 * @param	penwidth		The pen width (0 = unit width)
			 **/
			void draw_quad_bezier(iVec2 P1, iVec2 P2, iVec2 PC, float wc, RGBc color, bool draw_P2, bool blending, bool antialiasing, int32 penwidth = 0)
				{
				if ((isEmpty()) || (wc <= 0)) return;
				iBox2 mbr(P1);
				mbr.swallowPoint(P2);
				mbr.swallowPoint(PC);
				iBox2 B = imageBox();
				if (penwidth <= 0)
					{
					if (intersectionRect(mbr,B).isEmpty()) return;  // nothing to draw
					if (antialiasing) // if using antialiasing, we always check the range (and its cost is negligible compared to the antialiasing computations).
						{
						if (wc == 1)
							{
							if (blending)  _plotQuadBezier<true, true, true, false>(P1.X(), P1.Y(), PC.X(), PC.Y(), P2.X(), P2.Y(), color, draw_P2, penwidth); else _plotQuadBezier<false, true, true, false>(P1.X(), P1.Y(), PC.X(), PC.Y(), P2.X(), P2.Y(), color, draw_P2, penwidth);
							}
						else
							{
							if (blending)  _plotQuadRationalBezier<true, true, true, false>(P1.X(), P1.Y(), PC.X(), PC.Y(), P2.X(), P2.Y(), wc, color, draw_P2, penwidth); else _plotQuadRationalBezier<false, true, true, false>(P1.X(), P1.Y(), PC.X(), PC.Y(), P2.X(), P2.Y(), wc, color, draw_P2, penwidth);
							}
						return;
						}
					// check if we stay inside the image to remove bound check if possible
					if (!mbr.isIncludedIn(B))
						{ // must check bounds
						if (wc == 1)
							{
							if ((blending) && (!color.isOpaque()))  _plotQuadBezier<true, true, false, false>(P1.X(), P1.Y(), PC.X(), PC.Y(), P2.X(), P2.Y(), color, draw_P2, penwidth); else _plotQuadBezier<false, true, false, false>(P1.X(), P1.Y(), PC.X(), PC.Y(), P2.X(), P2.Y(), color, draw_P2, penwidth);
							}
						else
							{
							if ((blending) && (!color.isOpaque()))  _plotQuadRationalBezier<true, true, false, false>(P1.X(), P1.Y(), PC.X(), PC.Y(), P2.X(), P2.Y(), wc, color, draw_P2, penwidth); else _plotQuadRationalBezier<false, true, false, false>(P1.X(), P1.Y(), PC.X(), PC.Y(), P2.X(), P2.Y(), wc, color, draw_P2, penwidth);
							}
						return;
						}
					// no need to check bounds
					if (wc == 1)
						{
						if ((blending) && (!color.isOpaque()))  _plotQuadBezier<true, false, false, false>(P1.X(), P1.Y(), PC.X(), PC.Y(), P2.X(), P2.Y(), color, draw_P2, penwidth); else _plotQuadBezier<false, false, false, false>(P1.X(), P1.Y(), PC.X(), PC.Y(), P2.X(), P2.Y(), color, draw_P2, penwidth);
						}
					else
						{
						if ((blending) && (!color.isOpaque()))  _plotQuadRationalBezier<true, false, false, false>(P1.X(), P1.Y(), PC.X(), PC.Y(), P2.X(), P2.Y(), wc, color, draw_P2, penwidth); else _plotQuadRationalBezier<false, false, false, false>(P1.X(), P1.Y(), PC.X(), PC.Y(), P2.X(), P2.Y(), wc, color, draw_P2, penwidth);
						}
					return;
					}
				// penwidth >= 1
				_correctPenOpacity(color, penwidth);
				mbr.enlarge(penwidth);
				if (intersectionRect(mbr, B).isEmpty()) return;  // nothing to draw
				// always check range since cost is negligible compared to using a large pen. 
				if (antialiasing)
					{
					if (wc == 1)
						{
						if (blending)  _plotQuadBezier<true, true, true, true>(P1.X(), P1.Y(), PC.X(), PC.Y(), P2.X(), P2.Y(), color, draw_P2, penwidth); else _plotQuadBezier<false, true, true, true>(P1.X(), P1.Y(), PC.X(), PC.Y(), P2.X(), P2.Y(), color, draw_P2, penwidth);
						}
					else
						{
						if (blending)  _plotQuadRationalBezier<true, true, true, true>(P1.X(), P1.Y(), PC.X(), PC.Y(), P2.X(), P2.Y(), wc, color, draw_P2, penwidth); else _plotQuadRationalBezier<false, true, true, true>(P1.X(), P1.Y(), PC.X(), PC.Y(), P2.X(), P2.Y(), wc, color, draw_P2, penwidth);
						}
					return;
					}
				if (wc == 1)
					{
					if ((blending) && (!color.isOpaque()))  _plotQuadBezier<true, true, false, true>(P1.X(), P1.Y(), PC.X(), PC.Y(), P2.X(), P2.Y(), color, draw_P2, penwidth); else _plotQuadBezier<false, true, false, true>(P1.X(), P1.Y(), PC.X(), PC.Y(), P2.X(), P2.Y(), color, draw_P2, penwidth);
					}
				else
					{
					if ((blending) && (!color.isOpaque()))  _plotQuadRationalBezier<true, true, false, true>(P1.X(), P1.Y(), PC.X(), PC.Y(), P2.X(), P2.Y(), wc, color, draw_P2, penwidth); else _plotQuadRationalBezier<false, true, false, true>(P1.X(), P1.Y(), PC.X(), PC.Y(), P2.X(), P2.Y(), wc, color, draw_P2, penwidth);
					}
				return;
				}


			/**
			* Draw a cubic Bezier curve.
			*
			* @param	P1				The first point.
			* @param	P2				The second point.
			* @param	PA				The first control point.
			* @param	PB				The second control point.
			* @param	color			The color to use
			* @param	draw_P2			true to draw the endpoint P2.
			* @param	blending		true to use blending.
			* @param	antialiasing	true to use antialiasing.
			* @param	penwidth		The pen width (0 = unit width)
			**/
			void draw_cubic_bezier(iVec2 P1, iVec2 P2, iVec2 PA, iVec2 PB,  RGBc color, bool draw_P2, bool blending, bool antialiasing, int32 penwidth = 0)
				{
				if (isEmpty()) return;
				iBox2 mbr(P1);
				mbr.swallowPoint(P2);
				mbr.swallowPoint(PA);
				mbr.swallowPoint(PB);
				iBox2 B = imageBox();
				if (penwidth <= 0)
					{
					if (intersectionRect(mbr, B).isEmpty()) return;  // nothing to draw
					if (antialiasing) // if using antialiasing, we always check the range (and its cost is negligible compared to the antialiasing computations).
						{
						if (blending) _plotCubicBezier<true, true, true,false>(P1.X(), P1.Y(), PA.X(), PA.Y(), PB.X(), PB.Y(), P2.X(), P2.Y(), color, draw_P2, penwidth); else _plotCubicBezier<false, true, true, false>(P1.X(), P1.Y(), PA.X(), PA.Y(), PB.X(), PB.Y(), P2.X(), P2.Y(), color, draw_P2, penwidth);
						return;
						}
					// check if we stay inside the image to remove bound check is possible
					if (!mbr.isIncludedIn(B))
						{ // must check bounds
						if ((blending) && (!color.isOpaque())) _plotCubicBezier<true, true, false, false>(P1.X(), P1.Y(), PA.X(), PA.Y(), PB.X(), PB.Y(), P2.X(), P2.Y(), color, draw_P2, penwidth); else _plotCubicBezier<false, true, false, false>(P1.X(), P1.Y(), PA.X(), PA.Y(), PB.X(), PB.Y(), P2.X(), P2.Y(), color, draw_P2, penwidth);
						return;
						}
					// no need to check bounds
					if ((blending) && (!color.isOpaque()))  _plotCubicBezier<true, false, false, false>(P1.X(), P1.Y(), PA.X(), PA.Y(), PB.X(), PB.Y(), P2.X(), P2.Y(), color, draw_P2, penwidth); else _plotCubicBezier<false, false, false, false>(P1.X(), P1.Y(), PA.X(), PA.Y(), PB.X(), PB.Y(), P2.X(), P2.Y(), color, draw_P2, penwidth);
					return;
					}
				// use large pen
				_correctPenOpacity(color, penwidth);
				mbr.enlarge(penwidth);
				if (intersectionRect(mbr, B).isEmpty()) return;  // nothing to draw
				//always check bounds
				if (antialiasing) 
					{
					if (blending) _plotCubicBezier<true, true, true, true>(P1.X(), P1.Y(), PA.X(), PA.Y(), PB.X(), PB.Y(), P2.X(), P2.Y(), color, draw_P2, penwidth); else _plotCubicBezier<false, true, true, true>(P1.X(), P1.Y(), PA.X(), PA.Y(), PB.X(), PB.Y(), P2.X(), P2.Y(), color, draw_P2, penwidth);
					return;
					}
				if ((blending) && (!color.isOpaque())) _plotCubicBezier<true, true, false, true>(P1.X(), P1.Y(), PA.X(), PA.Y(), PB.X(), PB.Y(), P2.X(), P2.Y(), color, draw_P2, penwidth); else _plotCubicBezier<false, true, false, true>(P1.X(), P1.Y(), PA.X(), PA.Y(), PB.X(), PB.Y(), P2.X(), P2.Y(), color, draw_P2, penwidth);
				}
			

			/**
			 * Draw a quadratic spline.
			 *
			 * @param	nbpoints	   	number of point to interpolated
			 * @param	tabPoints	   	array of points to interpolate. 
			 * @param	color		   	The color tu use.
			 * @param	draw_last_point	true to draw the last point.
			 * @param	blending	   	true to use blending.
			 * @param	antialiased	   	true to use anti-aliasing.
			 * @param	penwidth		The pen width (0 = unit width)
			 **/
			inline void draw_quad_spline(size_t nbpoints, const iVec2 * tabPoints, RGBc color, bool draw_last_point, bool blending, bool antialiased, int32 penwidth = 0)
				{
				if (isEmpty()) return;
				switch(nbpoints)
					{
					case 0: {return;}
					case 1:
						{
						if (draw_last_point)
							{
							draw_dot(tabPoints[0], color, blending, penwidth);
							}
						return;
						}
					case 2:
						{
						draw_line(tabPoints[0], tabPoints[1], color, draw_last_point,blending,antialiased, penwidth);
						return;
						}
					default:
						{
						// we make a copy of the array because the drawing method destroy it.
						const size_t STATIC_SIZE = 32;
						int64 static_tabX[STATIC_SIZE]; int64 * tabX = static_tabX;
						int64 static_tabY[STATIC_SIZE]; int64 * tabY = static_tabY;
						if (nbpoints > STATIC_SIZE)
							{ // use dynamic array instead. 
							tabX = new int64[nbpoints];
							tabY = new int64[nbpoints];
							}
						for (size_t k = 0; k < nbpoints; k++) { tabX[k] = tabPoints[k].X();  tabY[k] = tabPoints[k].Y(); }
						// always check the range
						if (penwidth <= 0)
							{
							if (antialiased)
								{
								if (blending) _plotQuadSpline<true, true, true, false>(nbpoints - 1, tabX, tabY, color, draw_last_point, penwidth); else _plotQuadSpline<false, true, true, false>(nbpoints - 1, tabX, tabY, color, draw_last_point, penwidth);
								}
							else
								{
								if ((blending) && (!color.isOpaque())) _plotQuadSpline<true, true, false, false>(nbpoints - 1, tabX, tabY, color, draw_last_point, penwidth); else _plotQuadSpline<false, true, false, false>(nbpoints - 1, tabX, tabY, color, draw_last_point, penwidth);
								}
							}
						else
							{
							_correctPenOpacity(color, penwidth);
							if (antialiased)
								{
								if (blending) _plotQuadSpline<true, true, true, true>(nbpoints - 1, tabX, tabY, color, draw_last_point, penwidth); else _plotQuadSpline<false, true, true, true>(nbpoints - 1, tabX, tabY, color, draw_last_point, penwidth);
								}
							else
								{
								if ((blending) && (!color.isOpaque())) _plotQuadSpline<true, true, false, true>(nbpoints - 1, tabX, tabY, color, draw_last_point, penwidth); else _plotQuadSpline<false, true, false, true>(nbpoints - 1, tabX, tabY, color, draw_last_point, penwidth);
								}
							}
						if (tabX != static_tabX) { delete[] tabX; delete[] tabY; } // release memory if dynamically allocated. 
						return;
						}
					}
				}


			/**
			* Draw a quadratic spline.
			*
			* @param	tabPoints	   	std vector containing the points interpolated by the spline.
			* @param	color		   	The color tu use.
			* @param	draw_last_point	true to draw the last point.
			* @param	blending	   	true to use blending.
			* @param	antialiased	   	true to use anti-aliasing.
			* @param	penwidth		The pen width (0 = unit width)
			**/
			MTOOLS_FORCEINLINE void draw_quad_spline(const std::vector<iVec2> & tabPoints, RGBc color, bool draw_last_point, bool blending, bool antialiased, int32 penwidth = 0)
				{
				draw_quad_spline(tabPoints.size(), tabPoints.data(), color, draw_last_point, blending, antialiased, penwidth);
				}


			/**
			* Draw a cubic spline.
			*
			* @param	nbpoints	   	number of point to interpolated
			* @param	tabPoints	   	array of points to interpolate.
			* @param	color		   	The color tu use.
			* @param	draw_last_point	true to draw the last point.
			* @param	blending	   	true to use blending.
			* @param	antialiased	   	true to use anti-aliasing.
			* @param	penwidth		The pen width (0 = unit width)
			**/
			inline void draw_cubic_spline(size_t nbpoints, const iVec2 * tabPoints, RGBc color, bool draw_last_point, bool blending, bool antialiased, int32 penwidth = 0)
				{
				if (isEmpty()) return;
				switch (nbpoints)
					{
					case 0: {return;}
					case 1:
						{
						if (draw_last_point)
							{
							draw_dot(tabPoints[0], color, blending, penwidth);
							}
						return;
						}
					case 2:
						{
						draw_line(tabPoints[0], tabPoints[1], color, draw_last_point, blending, antialiased, penwidth);
						return;
						}
					case 3:
						{
						draw_quad_spline(nbpoints, tabPoints, color, draw_last_point, blending, antialiased, penwidth);
						return;
						}
					default:
						{
						// we make a copy of the array because the drawing method destroy it.
						const size_t STATIC_SIZE = 32;
						int64 static_tabX[STATIC_SIZE]; int64 * tabX = static_tabX;
						int64 static_tabY[STATIC_SIZE]; int64 * tabY = static_tabY;
						if (nbpoints > STATIC_SIZE)
							{ // use dynamic array instead. 
							tabX = new int64[nbpoints];
							tabY = new int64[nbpoints];
							}
						for (size_t k = 0; k < nbpoints; k++) { tabX[k] = tabPoints[k].X();  tabY[k] = tabPoints[k].Y(); }
						// always check the range....
						if (penwidth <= 0)
							{
							if (antialiased)
								{
								if (blending)  _plotCubicSpline<true, true, true, false>(nbpoints - 1, tabX, tabY, color, draw_last_point, penwidth); else _plotCubicSpline<false, true, true, false>(nbpoints - 1, tabX, tabY, color, draw_last_point, penwidth);
								}
							else
								{
								if ((blending) && (!color.isOpaque())) _plotCubicSpline<true, true, false, false>(nbpoints - 1, tabX, tabY, color, draw_last_point, penwidth); else _plotCubicSpline<false, true, false, false>(nbpoints - 1, tabX, tabY, color, draw_last_point, penwidth);
								}
							}
						else
							{
							_correctPenOpacity(color, penwidth);
							if (antialiased)
								{
								if (blending) _plotCubicSpline<true, true, true, true>(nbpoints - 1, tabX, tabY, color, draw_last_point, penwidth); else _plotCubicSpline<false, true, true, true>(nbpoints - 1, tabX, tabY, color, draw_last_point, penwidth);
								}
							else
								{
								if ((blending) && (!color.isOpaque())) _plotCubicSpline<true, true, false, true>(nbpoints - 1, tabX, tabY, color, draw_last_point, penwidth); else _plotCubicSpline<false, true, false, true>(nbpoints - 1, tabX, tabY, color, draw_last_point, penwidth);
								}
							}
						if (tabX != static_tabX) { delete[] tabX; delete[] tabY; } // release memory if dynamically allocated. 
						return;
						}
					}
				}


			/**
			* Draw a cubic spline.
			*
			* @param	tabPoints	   	std vector containing the points interpolated by the spline.
			* @param	color		   	The color tu use.
			* @param	draw_last_point	true to draw the last point.
			* @param	blending	   	true to use blending.
			* @param	antialiased	   	true to use anti-aliasing.
			* @param	penwidth		The pen width (0 = unit width)
			**/
			MTOOLS_FORCEINLINE void draw_cubic_spline(const std::vector<iVec2> & tabPoints, RGBc color, bool draw_last_point, bool blending, bool antialiased, int32 penwidth = 0)
				{
				draw_cubic_spline(tabPoints.size(), tabPoints.data(), color, draw_last_point, blending, antialiased, penwidth);
				}




			/******************************************************************************************************************************************************
			*******************************************************************************************************************************************************
			*																				   																      *
			*                                                                  DRAWING SHAPES                                                                     *
			*																																					  *
			*******************************************************************************************************************************************************
			*******************************************************************************************************************************************************/


			/**
			 * draw a rectangle of given size and color over this image. Portion outside the image is
			 * clipped.
			 *
			 * @param	dest_box	position of the rectangle to draw.
			 * @param	color   	the color to use.
			 * @param	blend   	true to use blending and false to simply copy the color.
			 * @param	penwidth	The pen width (0 = unit width)
			 **/
			inline void draw_rectangle(const iBox2 & dest_box, RGBc color, bool blend, int32 penwidth = 0)
				{
				if (dest_box.isEmpty()) return;
				if (penwidth <= 0) penwidth = 0;
				if (color.isOpaque()) blend = false;
				double tickness = 2*penwidth  + 1; // does not work if not using integer penwidth !
				draw_horizontal_line(dest_box.min[1], dest_box.min[0] - penwidth, dest_box.max[0] + penwidth, tickness, color, true, blend);
				draw_horizontal_line(dest_box.max[1], dest_box.min[0] - penwidth, dest_box.max[0] + penwidth, tickness, color, true, blend);
				draw_vertical_line(dest_box.min[0], dest_box.min[1] + penwidth + 1, dest_box.max[1] - penwidth - 1, tickness, color, true, blend);
				draw_vertical_line(dest_box.max[0], dest_box.min[1] + penwidth + 1, dest_box.max[1] - penwidth - 1, tickness, color, true, blend);
				}


			/**
			 * draw a rectangle of given size and color over this image. Portion outside the image is
			 * clipped.
			 *
			 * @param	x			x-coordinate of the rectangle upper left corner.
			 * @param	y			y-coordinate of the rectangle upper left corner.
			 * @param	sx			rectangle width (if <= 0 nothing is drawn).
			 * @param	sy			rectangle height (if <= 0 nothing is drawn).
			 * @param	color   	the color to use.
			 * @param	blend   	true to use blending and false to simply copy the color.
			 * @param	penwidth	The pen width (0 = unit width)
			 **/
			MTOOLS_FORCEINLINE void draw_rectangle(int64 x, int64 y, int64 sx, int64 sy, RGBc color, bool blend, int32 penwidth)
				{
				draw_rectangle(iBox2(x, x + sx - 1, y, y + sy - 1), color, blend, penwidth);
				}


			/**
			* Fill the interior of a rectangle rectangle. Portion outside the image is clipped.
			*
			* The boundary of the rectangle is not drawn. To fill the whole rectangle with its boundary, use draw_box() instead.
			*
			* @param	dest_box	position of the rectangle to draw.
			* @param	fillcolor	the color to use.
			* @param	blend   	true to use blending and false to simply copy the color.
			**/
			MTOOLS_FORCEINLINE void fill_rectangle(const iBox2 & dest_box, RGBc fillcolor, bool blend)
				{
				fill_rectangle(dest_box.min[0], dest_box.min[1], dest_box.max[0] - dest_box.min[0] + 1, dest_box.max[1] - dest_box.min[1] + 1, fillcolor, blend);
				}


			/**
			* draw a filled rectangle of given size and color over this image. Portion outside the image
			* is clipped.
			*
			* The boundary of the rectangle is not drawn. To fill the whole rectangle with its boundary, use draw_box() instead.
			*
			* @param	x			x-coordinate of the rectangle upper left corner.
			* @param	y			y-coordinate of the rectangle upper left corner.
			* @param	sx			rectangle width (if <= 0 nothing is drawn).
			* @param	sy			rectangle height (if <= 0 nothing is drawn).
			* @param	fillcolor	the color to use.
			* @param	blend   	true to use blending and false to simply copy the color.
			**/
			inline void fill_rectangle(int64 x, int64 y, int64 sx, int64 sy, RGBc fillcolor, bool blend)
				{
				if (isEmpty()) return;
				_draw_box(x + 1, y + 1, sx - 2, sy - 2, fillcolor, blend);
				}


			/**
			* Fill a (closed) box with a given color. Portion outside the image is clipped.
			*
			* @param	dest_box 	position of the rectangle to draw.
			* @param	fillcolor	the color to use.
			* @param	blend	 	true to use blending and false to simply copy the color.
			**/
			MTOOLS_FORCEINLINE void draw_box(const iBox2 & dest_box, RGBc fillcolor, bool blend)
				{
				draw_box(dest_box.min[0], dest_box.min[1], dest_box.max[0] - dest_box.min[0] + 1, dest_box.max[1] - dest_box.min[1] + 1, fillcolor, blend);
				}


			/**
			 * Fill a (closed) box with a given color. Portion outside the image is clipped.
			 * 
			 * @param	x		 	x-coordinate of the rectangle upper left corner.
			 * @param	y		 	y-coordinate of the rectangle upper left corner.
			 * @param	sx		 	rectangle width (if <= 0 nothing is drawn).
			 * @param	sy		 	rectangle height (if <= 0 nothing is drawn).
			 * @param	fillcolor	the color to use.
			 * @param	blend	 	true to use blending and false to simply copy the color.
			 **/
			inline void draw_box(int64 x, int64 y, int64 sx, int64 sy, RGBc fillcolor, bool blend)
				{
				if (isEmpty()) return;
				_draw_box(x, y, sx, sy, fillcolor, blend);
				}


			/**
			 * Draw a triangle. Portion outside the image is clipped.
			 *
			 * @param	P1		   	The first point.
			 * @param	P2		   	The second point.
			 * @param	P3		   	The third point.
			 * @param	color	   	The color.
			 * @param	blending   	true to use blending and false to write over.
			 * @param	antialiased	true to use antialiased lines.
			 * @param	penwidth   	The pen width (0 = unit width)
			 **/
			inline void draw_triangle(iVec2 P1, iVec2 P2, iVec2 P3, RGBc color, bool blending, bool antialiased, int32 penwidth = 0)
				{
				if (isEmpty()) return;
				if ((penwidth <= 0)&&(!antialiased)&&(blending)&& (!color.isOpaque()))
					{ // draw without overlap
					_lineBresenham<true, true, false, false, false, false>(P1, P2, color, true, penwidth, 0);
					_lineBresenham_avoid<true, true, false, false, false>(P2, P3, P1, color, 0, 0);
					_lineBresenham_avoid_both_sides_triangle<true, true, false, false, false>(P1, P3, P2, color,0);
					return;
					}
				// default drawing
				draw_line(P1, P2, color, false, blending, antialiased, penwidth);
				draw_line(P2, P3, color, false, blending, antialiased, penwidth);
				draw_line(P3, P1, color, false, blending, antialiased, penwidth);
				return;
				}


			/**
			 * Draw a triangle. Portion outside the image is clipped.
			 *
			 * @param	x1		   	x-coord of the first point.
			 * @param	y1		   	y-coord of the first point.
			 * @param	x2		   	x-coord of the second point.
			 * @param	y2		   	y-coord of the second point.
			 * @param	x3		   	x-coord of the third point.
			 * @param	y3		   	y-coord of the third point.
			 * @param	color	   	The color.
			 * @param	blending   	true to use blending and false to write over.
			 * @param	antialiased	true to use antialiased lines.
			 * @param	penwidth   	The pen width (0 = unit width)
			 **/
			MTOOLS_FORCEINLINE void draw_triangle(int64 x1, int64 y1, int64 x2, int64 y2, int64 x3, int64 y3, RGBc color, bool blending, bool antialiased, int32 penwidth = 0)
				{
				draw_triangle({ x1,y1 }, { x2,y2 }, { x3,y3 }, color, blending, antialiased, penwidth);
				}


			/**
			* Fill the interior of a triangle. Portion outside the image is clipped.
			*
			* Only the interior is filled, the boundary lines are not drawn/filled.
			*
			* @param	P1		   	The first point
			* @param	P2		   	The second point.
			* @param	P3		   	The third point.
			* @param	fillcolor  	The fill color.
			* @param	blending   	true to use blending and false to write over.
			**/
			inline void fill_triangle(iVec2 P1, iVec2 P2, iVec2 P3, RGBc fillcolor, bool blending)
				{
				if (isEmpty()) return;
				iBox2 mbr(P1);
				mbr.swallowPoint(P2);
				mbr.swallowPoint(P3);
				iBox2 B= imageBox();
				if (intersectionRect(mbr, B).isEmpty()) return; // nothing to draw. 
				if (mbr.isIncludedIn(B))
					{
					if ((blending) && (!fillcolor.isOpaque())) _draw_triangle_interior<true, false>(P1, P2, P3, fillcolor); else _draw_triangle_interior<false, false>(P1, P2, P3, fillcolor);
					}
				else
					{
					if ((blending) && (!fillcolor.isOpaque())) _draw_triangle_interior<true, true>(P1, P2, P3, fillcolor); else _draw_triangle_interior<false, true>(P1, P2, P3, fillcolor);
					}
				}



			/**
			* Fill the interior of a triangle. Portion outside the image is clipped.
			* 
			* Only the interior is filled, the boundary lines are not drawn/filled.
			*
			* @param	x1		 	x-coord of the first point.
			* @param	y1		 	y-coord of the first point.
			* @param	x2		 	x-coord of the second point.
			* @param	y2		 	y-coord of the second point.
			* @param	x3		 	x-coord of the third point.
			* @param	y3		 	y-coord of the third point.
			* @param	fillcolor	The fill color.
			* @param	blending 	true to use blending and false to write over.
			**/
			MTOOLS_FORCEINLINE void fill_triangle(int64 x1, int64 y1, int64 x2, int64 y2, int64 x3, int64 y3, RGBc fillcolor, bool blending)
				{
				fill_triangle({ x1,y1 }, { x2,y2 }, { x3,y3 }, fillcolor, blending);
				}


			/**
			 * Draw a polygon.
			 *
			 * @param	nbvertices 	Number of vertices in the polygon.
			 * @param	tabPoints  	the list of points in clockwise or counterclockwise order.
			 * @param	color	   	The color tu use.
			 * @param	blending   	true to use blending.
			 * @param	antialiased	true to draw antialiased lines.
			 * @param	penwidth   	The pen width (0 = unit width)
			 **/
			inline void draw_polygon(size_t nbvertices, const iVec2 * tabPoints, RGBc color, bool blending, bool antialiased, int32 penwidth = 0)
				{
				if (isEmpty()) return;
				if ((color.isOpaque())&&(!antialiased)) blending = false;
				switch (nbvertices)
					{
					case 0: { return; }
					case 1:
						{
						draw_dot(*tabPoints, color, blending, penwidth);
						return;
						}
					case 2:
						{
						draw_line(tabPoints[0], tabPoints[1], color, true, blending, antialiased, penwidth);
						return;
						}
					case 3:
						{
						draw_triangle(tabPoints[0], tabPoints[1], tabPoints[2], color, blending, antialiased, penwidth);
						return;
						}
					default:
						{
						if ((penwidth <= 0) && (!antialiased) && (blending) && (!color.isOpaque()))
							{ // draw without overlap
							_lineBresenham<true, true, false, false, false, false>(tabPoints[0], tabPoints[1], color, true, penwidth, 0);
							for (size_t i = 1; i < nbvertices - 1; i++)
								{
								_lineBresenham_avoid<true, true,false,false,false>(tabPoints[i], tabPoints[i + 1], tabPoints[i - 1], color, 0,0);
								}
							_lineBresenham_avoid_both_sides<true, true, false, false, false>(tabPoints[nbvertices - 1], tabPoints[0], tabPoints[nbvertices - 2], tabPoints[1], color,0);
							return;
							}
						for (size_t i = 0; i < nbvertices; i++)
							{
							draw_line(tabPoints[i], tabPoints[(i + 1) % nbvertices], color, false, blending, antialiased, penwidth);
							}
						return;
						}
					}
				}


			/**
			 * Draw a polygon.
			 *
			 * @param	tabPoints  	std vector of polygon vertice in clockwise or counterclockwise order.
			 * @param	color	   	The color tu use.
			 * @param	blending   	true to use blending.
			 * @param	antialiased	true to draw antialiased lines.
			 * @param	penwidth   	The pen width (0 = unit width)
			 **/
			MTOOLS_FORCEINLINE void draw_polygon(const std::vector<iVec2> & tabPoints, RGBc color, bool blending, bool antialiased, int32 penwidth = 0)
				{
				draw_polygon(tabPoints.size(), tabPoints.data(), color, blending, antialiased, penwidth);
				}


			/**
			* Fill the interior of a convex polygon. The boundary lines are not drawn.
			*
			* @param	nbvertices 	Number of vertices in the polygon.
			* @param	tabPoints  	the list of points in clockwise or counterclockwise order.
			* @param	fillcolor  	The color to use.
			* @param	blending   	true to use blending.
			**/
			inline void fill_convex_polygon(size_t nbvertices, const iVec2 * tabPoints, RGBc fillcolor, bool blending)
				{
				if (isEmpty() || nbvertices < 3) return;
				if (fillcolor.isOpaque()) blending = false;
				if (nbvertices == 3)
					{
					fill_triangle(tabPoints[0], tabPoints[1], tabPoints[2], fillcolor, blending);
					return;
					}
				//Compute the barycenter
				double X = 0, Y = 0;
				for (size_t i = 0; i < nbvertices; i++) { X += tabPoints[i].X(); Y += tabPoints[i].Y(); }
				iVec2 G((int64)(X / nbvertices), (int64)(Y / nbvertices));
				for (size_t i = 0; i < nbvertices; i++)
					{
					fill_triangle(tabPoints[i], tabPoints[(i + 1) % nbvertices], G, fillcolor, blending);
					}
				if (blending)
					{
					_lineBresenham_avoid<true, true, false, false, false>(tabPoints[0], G, tabPoints[nbvertices - 1], tabPoints[1], fillcolor, 0,0);
					for (size_t i = 1; i < nbvertices; i++)
						{
						_lineBresenham_avoid_both_sides<true, true, false, false, false>(tabPoints[i], G, tabPoints[i - 1], tabPoints[(i + 1) % nbvertices], tabPoints[0], tabPoints[i - 1], fillcolor,0);
						}
					return;
					}
				for (size_t i = 0; i < nbvertices; i++)
					{
					draw_line(G, tabPoints[i], fillcolor, false);
					}
				}


			/**
			* Fill the interior of a convex polygon. The edge are not drawn.
			*
			* @param	tabPoints  	std vector of polygon vertice in clockwise or counterclockwise order.
			* @param	color	   	The color tu use.
			* @param	blending   	true to use blending.
			* @param	antialiased	true to draw antialiased lines.
			**/
			MTOOLS_FORCEINLINE void fill_convex_polygon(const std::vector<iVec2> & tabPoints, RGBc fillcolor, bool blending)
				{
				fill_convex_polygon(tabPoints.size(), tabPoints.data(), fillcolor, blending);
				}	



			/**
			* Draw an (integer-valued) circle
			*
			* @param	center center.
			* @param	radius radius
			* @param	color  color
			* @param	aa	   (Optional) true to use antialiasing.
			* @param	blend  (Optional) true to use blending.
			*/
			void draw_circle(iVec2 center, int64 radius, RGBc color, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND)
			{
				if (isEmpty() || (radius < 0)) return;
				const int64 margin = (aa ? 1 : 0);
				iBox2 circleBox(center.X() - radius - margin, center.X() + radius + margin, center.Y() - radius - margin, center.Y() + radius + margin);
				iBox2 imBox = imageBox();
				iBox2 B = intersectionRect(circleBox, imBox);
				if (B.isEmpty()) return; // nothing to draw.
				if (circleBox.isIncludedIn(imBox))
				{ // included
					if (aa)
					{
						if (blend) _draw_circle_AA<true, false, false>(center.X(), center.Y(), radius, color, color); else _draw_circle_AA<false, false, false>(center.X(), center.Y(), radius, color, color);
					}
					else
					{
						if (blend) _draw_circle<true, false, true, false>(center.X(), center.Y(), radius, color, color); else _draw_circle<false, false, true, false>(center.X(), center.Y(), radius, color, color);
					}
					return;
				}
				// not included
				if ((circleBox.ly() < 16 * B.ly()) && (B.area() * 8 > circleBox.area()))
				{ // still faster to use draw everything using the first method while checking the range
					if (aa)
					{
						if (blend) _draw_circle_AA<true, true, false>(center.X(), center.Y(), radius, color, color); else _draw_circle_AA<false, true, false>(center.X(), center.Y(), radius, color, color);
					}
					else
					{
						if (blend) _draw_circle<true, true, true, false>(center.X(), center.Y(), radius, color, color); else _draw_circle<false, true, true, false>(center.X(), center.Y(), radius, color, color);
					}
					return;
				}
				// use alternate method
				double rr = (double)radius;
				if (aa)
				{
					if (blend) _draw_ellipse2_AA<true, false>(B, center, rr, rr, color, color); else _draw_ellipse2_AA<false, false>(B, center, rr, rr, color, color);
				}
				else
				{
					if (blend) _draw_ellipse2<true, true, false>(B, center, rr, rr, color, color); else _draw_ellipse2<false, true, false>(B, center, rr, rr, color, color);
				}
				return;
			}


			/**
			* Draw an (integer-valued) filled circle .
			*
			* @param	center    center.
			* @param	radius    radius.
			* @param	color	  color.
			* @param	fillcolor color to fill the inside of the circle.
			* @param	aa		  (Optional) true to use antialiasing.
			* @param	blend	  (Optional) true to use blending.
			*/
			void draw_filled_circle(iVec2 center, int64 radius, RGBc color, RGBc fillcolor, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND)
				{
				if (isEmpty() || (radius < 0)) return;
				const int64 margin = (aa ? 1 : 0);
				iBox2 circleBox(center.X() - radius - margin, center.X() + radius + margin, center.Y() - radius - margin, center.Y() + radius + margin);
				iBox2 imBox = imageBox();
				iBox2 B = intersectionRect(circleBox, imBox);
				if (B.isEmpty()) return; // nothing to draw.
				if (circleBox.isIncludedIn(imBox))
					{ // included
					if (aa)
						{
						if (blend) _draw_circle_AA<true, false, true>(center.X(), center.Y(), radius, color, fillcolor); else _draw_circle_AA<false, false, true>(center.X(), center.Y(), radius, color, fillcolor);
						}
					else
						{
						if (blend) _draw_circle<true, false, true, true>(center.X(), center.Y(), radius, color, fillcolor); else _draw_circle<false, false, true, true>(center.X(), center.Y(), radius, color, fillcolor);
						}
					return;
					}
				// not included
				if ((circleBox.ly() < 8 * B.ly()) && (B.area() * 4 > circleBox.area()))
					{ // still faster to use draw everything using the first method while checking the range
					if (aa)
						{
						if (blend) _draw_circle_AA<true, true, true>(center.X(), center.Y(), radius, color, fillcolor); else _draw_circle_AA<false, true, true>(center.X(), center.Y(), radius, color, fillcolor);
						}
					else
						{
						if (blend) _draw_circle<true, true, true, true>(center.X(), center.Y(), radius, color, fillcolor); else _draw_circle<false, true, true, true>(center.X(), center.Y(), radius, color, fillcolor);
						}
					return;
					}
				// use alternate method
				double rr = (double)radius;
				if (aa)
					{
					if (blend) _draw_ellipse2_AA<true, true>(B, center, rr, rr, color, fillcolor); else _draw_ellipse2_AA<false, true>(B, center, rr, rr, color, fillcolor);
					}
				else
					{
					if (blend) _draw_ellipse2<true, true, true>(B, center, rr, rr, color, fillcolor); else _draw_ellipse2<false, true, true>(B, center, rr, rr, color, fillcolor);
					}
				return;
				}



			/**
			* Draw an (real-valued) circle
			*
			* @param	center center.
			* @param	radius radius
			* @param	color  color
			* @param	aa	   (Optional) true to use antialiasing.
			* @param	blend  (Optional) true to use blending.
			* @param	grid_align (Optional) true to align to nearest integer value (faster drawing).
			*/
			void draw_circle(fVec2 center, double radius, RGBc color, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, bool grid_align = DEFAULT_GRID_ALIGN)
				{
				if (isEmpty() || (radius <= 0)) return;
				if ((grid_align)||(isIntegerValued(radius) && isIntegerValued(center)))
					{
					draw_circle( iVec2{(int64)round(center.X()), (int64)round(center.Y())}, (int64)round(radius), color, aa, blend);
					return;
					}
				iBox2 B = imageBox();
				if (aa)
					{
					if (blend) _draw_ellipse2_AA<true, false>(B, center, radius, radius, color, color); else _draw_ellipse2_AA<false, false>(B, center, radius, radius, color, color);
					}
				else
					{
					if (blend) _draw_ellipse2<true, true, false>(B, center, radius, radius, color, color); else _draw_ellipse2<false, true, false>(B, center, radius, radius, color, color);
					}
				return;
				}


			/**
			* Draw part of a (real-valued) circle
			*
			* @param	circlepart	combination of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT
			* @param	center center.
			* @param	radius radius
			* @param	color  color
			* @param	aa	   (Optional) true to use antialiasing.
			* @param	blend  (Optional) true to use blending.
			*/
			void draw_part_circle(int circlepart, fVec2 center, double radius, RGBc color, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND)
				{
				if (isEmpty() || (radius <= 0)) return;
				iBox2 B = intersectionRect(imageBox(), _ellipseBBox(center,radius,radius).get_split(circlepart));
				if (aa)
					{
					if (blend) _draw_ellipse2_AA<true, false>(B, center, radius, radius, color, color); else _draw_ellipse2_AA<false, false>(B, center, radius, radius, color, color);
					}
					else
					{
					if (blend) _draw_ellipse2<true, true, false>(B, center, radius, radius, color, color); else _draw_ellipse2<false, true, false>(B, center, radius, radius, color, color);
					}
				return;
				}


			/**
			* Draw an (real-valued) filled circle .
			*
			* @param	center    center.
			* @param	radius    radius.
			* @param	color	  color.
			* @param	fillcolor color to fill the inside of the circle.
			* @param	aa		  (Optional) true to use antialiasing.
			* @param	blend	  (Optional) true to use blending.
			* @param	grid_align (Optional) true to align to nearest integer value (faster drawing).
			*/
			void draw_filled_circle(fVec2 center, double radius, RGBc color, RGBc fillcolor, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, bool grid_align = DEFAULT_GRID_ALIGN)
				{
				if (isEmpty() || (radius <= 0)) return;
				if ((grid_align) || (isIntegerValued(radius) && isIntegerValued(center)))
					{
					draw_filled_circle(iVec2{ (int64)round(center.X()), (int64)round(center.Y()) }, (int64)round(radius), color, fillcolor, aa, blend);
					return;
					}
				iBox2 B = imageBox();
				if (aa)
					{
					if (blend) _draw_ellipse2_AA<true, true>(B, center, radius, radius, color, fillcolor); else _draw_ellipse2_AA<false, true>(B, center, radius, radius, color, fillcolor);
					}
				else
					{
					if (blend) _draw_ellipse2<true, true, true>(B, center, radius, radius, color, fillcolor); else _draw_ellipse2<false, true, true>(B, center, radius, radius, color, fillcolor);
					}
				return;
				}


			/**
			* Draw part of a (real-valued) filled circle .
			*
			* @param	circlepart	combination of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT
			* @param	center    center.
			* @param	radius    radius.
			* @param	color	  color.
			* @param	fillcolor color to fill the inside of the circle.
			* @param	aa		  (Optional) true to use antialiasing.
			* @param	blend	  (Optional) true to use blending.
			*/
			void draw_part_filled_circle(int circlepart, fVec2 center, double radius, RGBc color, RGBc fillcolor, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND)
				{
				if (isEmpty() || (radius <= 0)) return;
				iBox2 B = intersectionRect(imageBox(), _ellipseBBox(center, radius, radius).get_split(circlepart));
				if (aa)
					{
					if (blend) _draw_ellipse2_AA<true, true>(B, center, radius, radius, color, fillcolor); else _draw_ellipse2_AA<false, true>(B, center, radius, radius, color, fillcolor);
					}
				else
					{
					if (blend) _draw_ellipse2<true, true, true>(B, center, radius, radius, color, fillcolor); else _draw_ellipse2<false, true, true>(B, center, radius, radius, color, fillcolor);
					}
				return;
				}


			/**
			* Draw a thick (real-valued) circle
			*
			* @param	center    center.
			* @param	radius    (outer) radius.
			* @param	thickness thickness of the circle. The inner radius is radius - thickness.
			* @param	color	  color.
			* @param	aa		  (Optional) true to use antialiasing.
			* @param	blend	  (Optional) true to use blending.
			* @param	grid_align (Optional) true to align to nearest integer value (faster drawing).
			*/
			void draw_thick_circle(fVec2 center, double radius, double thickness, RGBc color, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, bool grid_align = DEFAULT_GRID_ALIGN, double min_tick = DEFAULT_MIN_THICKNESS)
			{
				if (isEmpty() || (radius <= 0)) return;
				if (thickness < 2)
					{
					if (thickness < 1) { color.multOpacity((float)((thickness < min_tick) ? min_tick : thickness)); }
					draw_circle(center, radius, color, aa, blend, grid_align);
					return;
					}
				double radius2 = std::max<double>(radius - thickness, 0);
				iBox2 B = imageBox();
				if (blend) _draw_ellipse_thick_AA<true, false>(B, center, radius2, radius2, radius, radius, color, color); else _draw_ellipse_thick_AA<false, false>(B, center, radius2, radius2, radius, radius, color, color);
				return;
			}


			/**
			* Draw part of a thick (real-valued) circle
			*
			* @param	circlepart	combination of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT
			* @param	center    center.
			* @param	radius    (outer) radius.
			* @param	thickness thickness of the circle. The inner radius is radius - thickness.
			* @param	color	  color.
			* @param	aa		  (Optional) true to use antialiasing.
			* @param	blend	  (Optional) true to use blending.
			*/
			void draw_part_thick_circle(int circlepart, fVec2 center, double radius, double thickness, RGBc color, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, double min_tick = DEFAULT_MIN_THICKNESS)
				{
				if (isEmpty() || (radius <= 0)) return;
				if (thickness < 2)
					{
					if (thickness < 1) { color.multOpacity((float)((thickness < min_tick) ? min_tick : thickness)); }
					draw_part_circle(circlepart, center, radius, color, aa, blend);
					return;
					}
				double radius2 = std::max<double>(radius - thickness, 0);
				iBox2 B = intersectionRect(imageBox(), _ellipseBBox(center, radius, radius).get_split(circlepart));
				if (blend) _draw_ellipse_thick_AA<true, false>(B, center, radius2, radius2, radius, radius, color, color); else _draw_ellipse_thick_AA<false, false>(B, center, radius2, radius2, radius, radius, color, color);
				return;
				}


			/**
			* Draw a thick (real-valued) filled circle
			*
			* @param	center    center.
			* @param	radius    (outer) radius.
			* @param	thickness thickness of the circle. The inner radius is radius - thickness.
			* @param	color	  color.
			* @param	fillcolor color to fill the circle.
			* @param	aa		  (Optional) true to use antialiasing.
			* @param	blend	  (Optional) true to use blending.
			*/
			void draw_thick_filled_circle(fVec2 center, double radius, double thickness, RGBc color, RGBc fillcolor, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, bool grid_align = DEFAULT_GRID_ALIGN, double min_tick = DEFAULT_MIN_THICKNESS)
				{
				if (isEmpty() || (radius <= 0)) return;
				if (thickness < 2)
					{
					if (thickness < 1) { color.multOpacity((float)((thickness < min_tick) ? min_tick : thickness)); }
					draw_filled_circle(center, radius, color, fillcolor, aa, blend, grid_align);
					return;
					}
				double radius2 = std::max<double>(radius - thickness, 0);
				iBox2 B = imageBox();
				if (blend) _draw_ellipse_thick_AA<true, true>(B, center, radius2, radius2, radius, radius, color, fillcolor); else _draw_ellipse_thick_AA<false, true>(B, center, radius2, radius2, radius, radius, color, fillcolor);
				return;
				}


			/**
			* Draw `part of a thick (real-valued) filled circle
			*
			* @param	circlepart	combination of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT
			* @param	center    center.
			* @param	radius    (outer) radius.
			* @param	thickness thickness of the circle. The inner radius is radius - thickness.
			* @param	color	  color.
			* @param	fillcolor color to fill the circle.
			* @param	aa		  (Optional) true to use antialiasing.
			* @param	blend	  (Optional) true to use blending.
			*/
			void draw_part_thick_filled_circle(int circlepart, fVec2 center, double radius, double thickness, RGBc color, RGBc fillcolor, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, double min_tick = DEFAULT_MIN_THICKNESS)
				{
				if (isEmpty() || (radius <= 0)) return;
				if (thickness < 2)
					{
					if (thickness < 1) { color.multOpacity((float)((thickness < min_tick) ? min_tick : thickness)); }
					draw_part_filled_circle(circlepart, center, radius, color, fillcolor, aa, blend);
					return;
					}
				double radius2 = std::max<double>(radius - thickness, 0);
				iBox2 B = intersectionRect(imageBox(), _ellipseBBox(center, radius, radius).get_split(circlepart));
				if (blend) _draw_ellipse_thick_AA<true, true>(B, center, radius2, radius2, radius, radius, color, fillcolor); else _draw_ellipse_thick_AA<false, true>(B, center, radius2, radius2, radius, radius, color, fillcolor);
				return;
				}



			/**
			* Draw an (integer-valued) ellipse.
			*
			* @param	center center.
			* @param	rx	   radius along the x-axis.
			* @param	ry	   raduis along the y-axis.
			* @param	color  color.
			* @param	aa	   (Optional) true to use antialiasing.
			* @param	blend  (Optional) true to use blending.
			*/
			inline void draw_ellipse(iVec2 center, int64 rx, int64 ry, RGBc color, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND)
				{
				if (rx == ry) { draw_circle(center, rx, color, aa, blend); return; }
				draw_ellipse_in_box(iBox2(center.X() - rx, center.X() + rx, center.Y() - ry, center.Y() + ry), color, aa, blend);
				}


			/**
			* Draw a filled (integer-valued) ellipse.
			*
			* @param	center    center.
			* @param	rx		  radius along the x-axis.
			* @param	ry		  raduis along the y-axis.
			* @param	color	  color.
			* @param	fillcolor color to fill the ellipse.
			* @param	aa		  (Optional) true to use antialiasing.
			* @param	blend	  (Optional) true to use blending.
			*/
			inline void draw_filled_ellipse(iVec2 center, int64 rx, int64 ry, RGBc color, RGBc fillcolor, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND)
				{
				if (rx == ry) { draw_filled_circle(center, rx, color, fillcolor, aa, blend); return; }
				draw_filled_ellipse_in_box(iBox2(center.X() - rx, center.X() + rx, center.Y() - ry, center.Y() + ry), color, fillcolor, aa, blend);
				}


			/**
			* Draw a (real-valued) ellipse.
			*
			* @param	center center.
			* @param	rx	   radius along the x-axis.
			* @param	ry	   raduis along the y-axis.
			* @param	color  color.
			* @param	aa	   (Optional) true to use antialiasing.
			* @param	blend  (Optional) true to use blending.
			* @param	grid_align (Optional) true to align to nearest integer value (faster drawing).
			*/
			inline void draw_ellipse(fVec2 center, double rx, double ry, RGBc color, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, bool grid_align = true)
				{
				if (rx == ry) { draw_circle(center, rx, color, aa, blend, grid_align); return; }
				draw_ellipse_in_box(fBox2(center.X() - rx, center.X() + rx, center.Y() - ry, center.Y() + ry), color, aa, blend, grid_align);
				}


			/**
			* Draw part of a (real-valued) ellipse.
			*
		    * @param	part   combination of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT
			* @param	center center.
			* @param	rx	   radius along the x-axis.
			* @param	ry	   raduis along the y-axis.
			* @param	color  color.
			* @param	aa	   (Optional) true to use antialiasing.
			* @param	blend  (Optional) true to use blending.
			*/
			inline void draw_part_ellipse(int part, fVec2 center, double rx, double ry, RGBc color, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND)
				{
				if (rx == ry) { draw_part_circle(part,center, rx, color, aa, blend); return; }
				draw_part_ellipse_in_box(part,fBox2(center.X() - rx, center.X() + rx, center.Y() - ry, center.Y() + ry), color, aa, blend);
				}


			/**
			* Draw a filled (real-valued) ellipse.
			*
			* @param	center    center.
			* @param	rx		  radius along the x-axis.
			* @param	ry		  raduis along the y-axis.
			* @param	color	  color.
			* @param	fillcolor color to fill the ellipse.
			* @param	aa		  (Optional) true to use antialiasing.
			* @param	blend	  (Optional) true to use blending.
			* @param	grid_align (Optional) true to align to nearest integer value (faster drawing).
			*/
			inline void draw_filled_ellipse(fVec2 center, double rx, double ry, RGBc color, RGBc fillcolor, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, bool grid_align = true)
				{
				if (rx == ry) { draw_filled_circle(center, rx, color, fillcolor, aa, blend, grid_align); return; }
				draw_filled_ellipse_in_box(fBox2(center.X() - rx, center.X() + rx, center.Y() - ry, center.Y() + ry), color, fillcolor, aa, blend, grid_align);
				}


			/**
			* Draw part of a filled (real-valued) ellipse.
			*
			* @param	part   combination of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT
			* @param	center    center.
			* @param	rx		  radius along the x-axis.
			* @param	ry		  raduis along the y-axis.
			* @param	color	  color.
			* @param	fillcolor color to fill the ellipse.
			* @param	aa		  (Optional) true to use antialiasing.
			* @param	blend	  (Optional) true to use blending.
			*/
			inline void draw_part_filled_ellipse(int part, fVec2 center, double rx, double ry, RGBc color, RGBc fillcolor, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND)
				{
				if (rx == ry) { draw_part_filled_circle(part, center, rx, color, fillcolor, aa, blend); return; }
				draw_part_filled_ellipse_in_box(part, fBox2(center.X() - rx, center.X() + rx, center.Y() - ry, center.Y() + ry), color, fillcolor, aa, blend);
				}


			/**
			* Draw an ellipse inside an (integer-valued) box.
			*
			* @param	ellipseBox	  bounding box for the ellipse.
			* @param	color color.
			* @param	aa    (Optional) true to use antialiasing.
			* @param	blend (Optional) true to use blending.
			*/
			void draw_ellipse_in_box(const iBox2 & ellipseBox, RGBc color, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND)
			{
				if ((isEmpty()) || (ellipseBox.isEmpty())) return;
				iBox2 imBox = imageBox();
				iBox2 B = intersectionRect(imBox, ellipseBox);
				if (B.isEmpty()) return; // nothing to draw.
				const int64 margin = (aa ? 1 : 0);
				if (ellipseBox.getEnlarge(margin).isIncludedIn(imBox))
				{ // included
					if (aa)
					{
						if (blend) _draw_ellipse_in_rect_AA<true, false>(ellipseBox.min[0], ellipseBox.min[1], ellipseBox.max[0], ellipseBox.max[1], color);
						else _draw_ellipse_in_rect_AA<false, false>(ellipseBox.min[0], ellipseBox.min[1], ellipseBox.max[0], ellipseBox.max[1], color);
					}
					else
					{
						if (blend) _draw_ellipse_in_rect<true, false, true, false>(ellipseBox.min[0], ellipseBox.min[1], ellipseBox.max[0], ellipseBox.max[1], color, color);
						else _draw_ellipse_in_rect<false, false, true, false>(ellipseBox.min[0], ellipseBox.min[1], ellipseBox.max[0], ellipseBox.max[1], color, color);
					}
					return;
				}
				// not included
				if ((ellipseBox.ly() < 16 * B.ly()) && (ellipseBox.lx() < 16*B.lx()) && (B.area() * 8 > ellipseBox.area()))
				{ // still faster to use draw everything using the first method while checking the range
					if (aa)
					{
						if (blend) _draw_ellipse_in_rect_AA<true, true>(ellipseBox.min[0], ellipseBox.min[1], ellipseBox.max[0], ellipseBox.max[1], color);
						else _draw_ellipse_in_rect_AA<false, true>(ellipseBox.min[0], ellipseBox.min[1], ellipseBox.max[0], ellipseBox.max[1], color);
					}
					else
					{
						if (blend) _draw_ellipse_in_rect<true, true, true, false>(ellipseBox.min[0], ellipseBox.min[1], ellipseBox.max[0], ellipseBox.max[1], color, color);
						else _draw_ellipse_in_rect<false, true, true, false>(ellipseBox.min[0], ellipseBox.min[1], ellipseBox.max[0], ellipseBox.max[1], color, color);
					}
					return;
				}
				// use alternate method
				double rx = ((double)(ellipseBox.max[0] - ellipseBox.min[0])) / 2;
				double ry = ((double)(ellipseBox.max[1] - ellipseBox.min[1])) / 2;
				fVec2 center = { ((double)(ellipseBox.max[0] + ellipseBox.min[0])) / 2 , ((double)(ellipseBox.max[1] + ellipseBox.min[1])) / 2 };
				if (aa)
				{
					if (blend) _draw_ellipse2_AA<true, false>(B, center, rx, ry, color, color);
					else _draw_ellipse2_AA<false, false>(B, center, rx, ry, color, color);
				}
				else
				{
					if (blend) _draw_ellipse2<true, true, false>(B, center, rx, ry, color, color);
					else _draw_ellipse2<false, true, false>(B, center, rx, ry, color, color);
				}
				return;
			}


			/**
			* Draw a filled ellipse inside an (integer-valued) box.
			*
			* @param	ellipseBox		  bounding box for the ellipse.
			* @param	color	  color.
			* @param	fillcolor color to fill the ellipse.
			* @param	aa		  (Optional) true to use antialiasing.
			* @param	blend	  (Optional) true to use blending.
			*/
			void draw_filled_ellipse_in_box(const iBox2 & ellipseBox, RGBc color, RGBc fillcolor, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND)
			{
				if ((isEmpty()) || (ellipseBox.isEmpty())) return;
				const int64 llx = ellipseBox.max[0] - ellipseBox.min[0];
				const int64 lly = ellipseBox.max[1] - ellipseBox.min[1];
				if ((llx == lly) && (llx & 1))
					{ 
					iVec2 center((ellipseBox.max[0] + ellipseBox.min[0]) / 2, (ellipseBox.max[1] + ellipseBox.min[1]) / 2);
					int64 rad = llx / 2;
					draw_filled_circle(center, rad, color, fillcolor, aa, blend);
					return;
					}
				iBox2 imBox = imageBox();
				iBox2 B = intersectionRect(imBox, ellipseBox);
				if (B.isEmpty()) return; // nothing to draw.
				if (!aa)
				{
					const int64 margin = (aa ? 1 : 0);
					if (ellipseBox.getEnlarge(margin).isIncludedIn(imBox))
					{ // included
						if (blend) _draw_ellipse_in_rect<true, false, true, true>(ellipseBox.min[0], ellipseBox.min[1], ellipseBox.max[0], ellipseBox.max[1], color, fillcolor);
						else _draw_ellipse_in_rect<false, false, true, true>(ellipseBox.min[0], ellipseBox.min[1], ellipseBox.max[0], ellipseBox.max[1], color, fillcolor);
						return;
					}
					// not included
					if ((ellipseBox.ly() < 8 * B.ly()) && (ellipseBox.lx() < 8 * B.lx()) && (B.area() * 4 > ellipseBox.area()))
					{ // still faster to use draw everything using the first method while checking the range
						if (blend) _draw_ellipse_in_rect<true, true, true, true>(ellipseBox.min[0], ellipseBox.min[1], ellipseBox.max[0], ellipseBox.max[1], color, fillcolor);
						else _draw_ellipse_in_rect<false, true, true, true>(ellipseBox.min[0], ellipseBox.min[1], ellipseBox.max[0], ellipseBox.max[1], color, fillcolor);
						return;
					}
				}
				// use alternate method
				double rx = ((double)(ellipseBox.max[0] - ellipseBox.min[0])) / 2;
				double ry = ((double)(ellipseBox.max[1] - ellipseBox.min[1])) / 2;
				fVec2 center = { ((double)(ellipseBox.max[0] + ellipseBox.min[0])) / 2 , ((double)(ellipseBox.max[1] + ellipseBox.min[1])) / 2 };
				if (aa)
				{
					if (blend) _draw_ellipse2_AA<true, true>(B, center, rx, ry, color, fillcolor);
					else _draw_ellipse2_AA<false, true>(B, center, rx, ry, color, fillcolor);
				}
				else
				{
					if (blend) _draw_ellipse2<true, true, true>(B, center, rx, ry, color, fillcolor);
					else _draw_ellipse2<false, true, true>(B, center, rx, ry, color, fillcolor);
				}
				return;
			}


			/**
			* Draw an ellipse inside a (real-valued) box.
			*
			* @param	ellipseBox	  bounding box for the ellipse.
			* @param	color color.
			* @param	aa    (Optional) true to use antialiasing.
			* @param	blend (Optional) true to use blending.
			* @param	grid_align (Optional) true to align to nearest integer value (faster drawing).
			*/
			void draw_ellipse_in_box(const fBox2 & ellipseBox, RGBc color, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, bool grid_align = true)
				{
				if (isEmpty() || (ellipseBox.isEmpty())) return;
				if ((grid_align) || (isIntegerValued(ellipseBox)))
					{
					draw_ellipse_in_box(iBox2((int64)round(ellipseBox.min[0]), (int64)round(ellipseBox.max[0]), (int64)round(ellipseBox.min[1]), (int64)round(ellipseBox.max[1])), color, aa, blend);
					return;
					}
				// use alternate method
				iBox2 B = imageBox();
				double rx = (ellipseBox.max[0] - ellipseBox.min[0]) / 2;
				double ry = (ellipseBox.max[1] - ellipseBox.min[1]) / 2;
				fVec2 center = { (ellipseBox.max[0] + ellipseBox.min[0]) / 2 , (ellipseBox.max[1] + ellipseBox.min[1]) / 2 };
				if (aa)
					{
					if (blend) _draw_ellipse2_AA<true, false>(B, center, rx, ry, color, color);
					else _draw_ellipse2_AA<false, false>(B, center, rx, ry, color, color);
					}
				else
					{
					if (blend) _draw_ellipse2<true, true, false>(B, center, rx, ry, color, color);
					else _draw_ellipse2<false, true, false>(B, center, rx, ry, color, color);
					}
				return;
				}


			/**
			* Draw part of an ellipse inside a (real-valued) box.
			*
			* @param	part   combination of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT
			* @param	ellipseBox	  bounding box for the ellipse.
			* @param	color color.
			* @param	aa    (Optional) true to use antialiasing.
			* @param	blend (Optional) true to use blending.
			*/
			void draw_part_ellipse_in_box(int part, const fBox2 & ellipseBox, RGBc color, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND)
				{
				if (isEmpty() || (ellipseBox.isEmpty())) return;
				double rx = (ellipseBox.max[0] - ellipseBox.min[0]) / 2;
				double ry = (ellipseBox.max[1] - ellipseBox.min[1]) / 2;
				fVec2 center = { (ellipseBox.max[0] + ellipseBox.min[0]) / 2 , (ellipseBox.max[1] + ellipseBox.min[1]) / 2 };
				iBox2 B = intersectionRect(imageBox(), _ellipseBBox(center,rx,ry).get_split(part));
				if (aa)
					{
					if (blend) _draw_ellipse2_AA<true, false>(B, center, rx, ry, color, color);
					else _draw_ellipse2_AA<false, false>(B, center, rx, ry, color, color);
					}
				else
					{
					if (blend) _draw_ellipse2<true, true, false>(B, center, rx, ry, color, color);
					else _draw_ellipse2<false, true, false>(B, center, rx, ry, color, color);
					}
				return;
				}


			/**
			* Draw a filled ellipse inside a (real-valued) box.
			*
			* @param	ellipseBox		  bounding box for the ellipse.
			* @param	color	  color.
			* @param	fillcolor color to fill the ellipse.
			* @param	aa		  (Optional) true to use antialiasing.
			* @param	blend	  (Optional) true to use blending.
			* @param	grid_align (Optional) true to align to nearest integer value (faster drawing).
			*/
			void draw_filled_ellipse_in_box(const fBox2 & ellipseBox, RGBc color, RGBc fillcolor, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, bool grid_align = true)
				{
				if (isEmpty() || (ellipseBox.isEmpty())) return;
				if ((grid_align) || (isIntegerValued(ellipseBox)))
					{
					draw_filled_ellipse_in_box(iBox2((int64)round(ellipseBox.min[0]), (int64)round(ellipseBox.max[0]), (int64)round(ellipseBox.min[1]), (int64)round(ellipseBox.max[1])), color, fillcolor, aa, blend);
					return;
					}
				// use alternate method
				iBox2 B = imageBox();
				double rx = (ellipseBox.max[0] - ellipseBox.min[0]) / 2;
				double ry = (ellipseBox.max[1] - ellipseBox.min[1]) / 2;
				fVec2 center = { (ellipseBox.max[0] + ellipseBox.min[0]) / 2 , (ellipseBox.max[1] + ellipseBox.min[1]) / 2 };
				if (aa)
					{
					if (blend) _draw_ellipse2_AA<true, true>(B, center, rx, ry, color, fillcolor);
					else _draw_ellipse2_AA<false, true>(B, center, rx, ry, color, fillcolor);
					}
				else
					{
					if (blend) _draw_ellipse2<true, true, true>(B, center, rx, ry, color, fillcolor);
					else _draw_ellipse2<false, true, true>(B, center, rx, ry, color, fillcolor);
					}
				return;
				}


			/**
			* Draw a part of filled ellipse inside a (real-valued) box.
			*
			* @param	part   combination of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT
			* @param	ellipseBox		  bounding box for the ellipse.
			* @param	color	  color.
			* @param	fillcolor color to fill the ellipse.
			* @param	aa		  (Optional) true to use antialiasing.
			* @param	blend	  (Optional) true to use blending.
			*/
			void draw_part_filled_ellipse_in_box(int part, const fBox2 & ellipseBox, RGBc color, RGBc fillcolor, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND)
				{
				if (isEmpty() || (ellipseBox.isEmpty())) return;
				double rx = (ellipseBox.max[0] - ellipseBox.min[0]) / 2;
				double ry = (ellipseBox.max[1] - ellipseBox.min[1]) / 2;
				fVec2 center = { (ellipseBox.max[0] + ellipseBox.min[0]) / 2 , (ellipseBox.max[1] + ellipseBox.min[1]) / 2 };
				iBox2 B = intersectionRect(imageBox(), _ellipseBBox(center, rx, ry).get_split(part));
				if (aa)
					{
					if (blend) _draw_ellipse2_AA<true, true>(B, center, rx, ry, color, fillcolor);
					else _draw_ellipse2_AA<false, true>(B, center, rx, ry, color, fillcolor);
					}
				else
					{
					if (blend) _draw_ellipse2<true, true, true>(B, center, rx, ry, color, fillcolor);
					else _draw_ellipse2<false, true, true>(B, center, rx, ry, color, fillcolor);
					}
				return;
				}


			/**
			* Draw a thick (real valued) ellipse
			*
			* @param	center	    center.
			* @param	rx		    (outer) radius along the x-axis.
			* @param	ry		    (outer) radius along the y-axis.
			* @param	thickness_x thickness along the x-axis inner x-radius = rx - thickness_x.
			* @param	thickness_y thickness along the y-axis inner y-radius = ry - thickness_y.
			* @param	color	    color.
			* @param	aa		    (Optional) true to use antialiasing.
			* @param	blend	    (Optional) true to use blending.
			* @param	grid_align  (Optional) true to align to nearest integer value (faster drawing).
			*/
			inline void draw_thick_ellipse(fVec2 center, double rx, double ry, double thickness_x, double thickness_y, RGBc color, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, bool grid_align = true, double min_tick = DEFAULT_MIN_THICKNESS)
			{
				if (isEmpty()) return;
				thickness_x = std::max<double>(thickness_x, 0);
				thickness_y = std::max<double>(thickness_y, 0);
				if ((thickness_x < 2) && (thickness_y < 2))
					{
					double th = (thickness_x + thickness_y)/2;
					if (th < 1) { color.multOpacity((float)((th < min_tick) ? min_tick : th)); }
					draw_ellipse(center, rx, ry, color, aa, blend, grid_align);
					return;
					}
				const double arx = std::max<double>(rx - thickness_x, 0);
				const double ary = std::max<double>(ry - thickness_y, 0);
				const iBox2 B = imageBox();
				if (blend) _draw_ellipse_thick_AA<true, false>(B, center, arx, ary, rx, ry, color, color);
				else _draw_ellipse_thick_AA<false, false>(B, center, arx, ary, rx, ry, color, color);
			}


			/**
			* Draw part of a thick (real valued) ellipse
			*
			* @param	part		combination of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT
			* @param	center	    center.
			* @param	rx		    (outer) radius along the x-axis.
			* @param	ry		    (outer) radius along the y-axis.
			* @param	thickness_x thickness along the x-axis inner x-radius = rx - thickness_x.
			* @param	thickness_y thickness along the y-axis inner y-radius = ry - thickness_y.
			* @param	color	    color.
			* @param	aa		    (Optional) true to use antialiasing.
			* @param	blend	    (Optional) true to use blending.
			*/
			inline void draw_part_thick_ellipse(int part, fVec2 center, double rx, double ry, double thickness_x, double thickness_y, RGBc color, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, double min_tick = DEFAULT_MIN_THICKNESS)
			{
				if (isEmpty()) return;
				thickness_x = std::max<double>(thickness_x, 0);
				thickness_y = std::max<double>(thickness_y, 0);
				if ((thickness_x < 2) && (thickness_y < 2))
					{
					double th = (thickness_x + thickness_y) / 2;
					if (th < 1) { color.multOpacity((float)((th < min_tick) ? min_tick : th)); }
					draw_part_ellipse(part,center, rx, ry, color, aa, blend);
					return;
					}
				const double arx = std::max<double>(rx - thickness_x, 0);
				const double ary = std::max<double>(ry - thickness_y, 0);
				iBox2 B = intersectionRect(imageBox(), _ellipseBBox(center, rx, ry).get_split(part));
				if (blend) _draw_ellipse_thick_AA<true, false>(B, center, arx, ary, rx, ry, color, color);
				else _draw_ellipse_thick_AA<false, false>(B, center, arx, ary, rx, ry, color, color);
			}



			/**
			* Draw a thick filled (real valued) ellipse
			*
			* @param	center	    center.
			* @param	rx		    (outer) radius along the x-axis.
			* @param	ry		    (outer) radius along the y-axis.
			* @param	thickness_x thickness along the x-axis inner x-radius = rx - thickness_x.
			* @param	thickness_y thickness along the y-axis inner y-radius = ry - thickness_y.
			* @param	color	    color.
			* @param	fillcolor   color to fill the ellipse.
			* @param	aa		    (Optional) true to use antialiasing.
			* @param	blend	    (Optional) true to use blending.
			* @param	grid_align  (Optional) true to align to nearest integer value (faster drawing).
			*/
			inline void draw_thick_filled_ellipse(fVec2 center, double rx, double ry, double thickness_x, double thickness_y, RGBc color, RGBc fillcolor, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, bool grid_align = true, double min_tick = DEFAULT_MIN_THICKNESS)
			{
				if (isEmpty()) return;
				thickness_x = std::max<double>(thickness_x, 0);
				thickness_y = std::max<double>(thickness_y, 0);
				if ((thickness_x < 2) && (thickness_y < 2))
					{
					double th = (thickness_x + thickness_y) / 2;
					if (th < 1) { color.multOpacity((float)((th < min_tick) ? min_tick : th)); }
					draw_filled_ellipse(center, rx, ry, color, fillcolor, aa, blend, grid_align);
					return;
					}
				const double arx = std::max<double>(rx - thickness_x, 0);
				const double ary = std::max<double>(ry - thickness_y, 0);
				const iBox2 B = imageBox();
				if (blend) _draw_ellipse_thick_AA<true, true>(B, center, arx, ary, rx, ry, color, fillcolor);
				else _draw_ellipse_thick_AA<false, true>(B, center, arx, ary, rx, ry, color, fillcolor);
			}


			/**
			* Draw part of a thick filled (real valued) ellipse
			*
			* @param	part		combination of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT
			* @param	center	    center.
			* @param	rx		    (outer) radius along the x-axis.
			* @param	ry		    (outer) radius along the y-axis.
			* @param	thickness_x thickness along the x-axis inner x-radius = rx - thickness_x.
			* @param	thickness_y thickness along the y-axis inner y-radius = ry - thickness_y.
			* @param	color	    color.
			* @param	fillcolor   color to fill the ellipse.
			* @param	aa		    (Optional) true to use antialiasing.
			* @param	blend	    (Optional) true to use blending.
			*/
			inline void draw_part_thick_filled_ellipse(int part, fVec2 center, double rx, double ry, double thickness_x, double thickness_y, RGBc color, RGBc fillcolor, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, double min_tick = DEFAULT_MIN_THICKNESS)
				{
				if (isEmpty()) return;
				thickness_x = std::max<double>(thickness_x, 0);
				thickness_y = std::max<double>(thickness_y, 0);
				if ((thickness_x < 2) && (thickness_y < 2))
					{
					double th = (thickness_x + thickness_y) / 2;
					if (th < 1) { color.multOpacity((float)((th < min_tick) ? min_tick : th)); }
					draw_part_filled_ellipse(part,center, rx, ry, color, fillcolor, aa, blend);
					return;
					}
				const double arx = std::max<double>(rx - thickness_x, 0);
				const double ary = std::max<double>(ry - thickness_y, 0);
				iBox2 B = intersectionRect(imageBox(), _ellipseBBox(center, rx, ry).get_split(part));
				if (blend) _draw_ellipse_thick_AA<true, true>(B, center, arx, ary, rx, ry, color, fillcolor);
				else _draw_ellipse_thick_AA<false, true>(B, center, arx, ary, rx, ry, color, fillcolor);
				}


			/**
			* Draw a thick ellipse inside a (real-valued) box.
			*
			* @param	B		    ellipse outer bounding box.
			* @param	thickness_x thickness along the x-axis (the ellipse inner radius is obtained by
			* 						substracting this quantity from the bounding box radius).
			* @param	thickness_y thickness along the y-axis (the ellipse inner radius is obtained by
			* 						substracting this quantity from the bounding box radius).
			* @param	color	    color.
			* @param	aa		    (Optional) true to use antialiasing.
			* @param	blend	    (Optional) true to use blending.
			* @param	grid_align  (Optional) true to align to nearest integer value (faster drawing).
			*/
			inline void draw_thick_ellipse_in_box(const fBox2 & B, double thickness_x, double thickness_y, RGBc color, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, bool grid_align = true, double min_tick = DEFAULT_MIN_THICKNESS)
				{
				fVec2 center((B.max[0] + B.min[0]) / 2, (B.max[1] + B.min[1]) / 2);
				double rx = (B.max[0] - B.min[0]) / 2;
				double ry = (B.max[1] - B.min[1]) / 2;
				draw_thick_ellipse(center, rx, ry, thickness_x, thickness_y, color, aa, blend, grid_align, min_tick);
				}


			/**
			* Draw part of a thick ellipse inside a (real-valued) box.
			*
			* @param	part		combination of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT
			* @param	B		    ellipse outer bounding box.
			* @param	thickness_x thickness along the x-axis (the ellipse inner radius is obtained by
			* 						substracting this quantity from the bounding box radius).
			* @param	thickness_y thickness along the y-axis (the ellipse inner radius is obtained by
			* 						substracting this quantity from the bounding box radius).
			* @param	color	    color.
			* @param	aa		    (Optional) true to use antialiasing.
			* @param	blend	    (Optional) true to use blending.
			*/
			inline void draw_part_thick_ellipse_in_box(int part, const fBox2 & B, double thickness_x, double thickness_y, RGBc color, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, double min_tick = DEFAULT_MIN_THICKNESS)
				{
				fVec2 center((B.max[0] + B.min[0]) / 2, (B.max[1] + B.min[1]) / 2);
				double rx = (B.max[0] - B.min[0]) / 2;
				double ry = (B.max[1] - B.min[1]) / 2;
				draw_part_thick_ellipse(part, center, rx, ry, thickness_x, thickness_y, color, aa, blend, min_tick);
				}


			/**
			* Draw a thick filled ellipse inside a (real-valued) box.
			*
			* @param	B		    ellipse outer bounding box.
			* @param	thickness_x thickness along the x-axis (the ellipse inner radius is obtained by
			* 						substracting this quantity from the bounding box radius).
			* @param	thickness_y thickness along the y-axis (the ellipse inner radius is obtained by
			* 						substracting this quantity from the bounding box radius).
			* @param	color	    color.
			* @param	fillcolor   color to fill the ellipse.
			* @param	aa		    (Optional) true to use antialiasing.
			* @param	blend	    (Optional) true to use blending.
			* @param	grid_align  (Optional) true to align to nearest integer value (faster drawing).
			*/
			inline void draw_thick_filled_ellipse_in_box(const fBox2 & B, double thickness_x, double thickness_y, RGBc color, RGBc fillcolor, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, bool grid_align = true, double min_tick = DEFAULT_MIN_THICKNESS)
				{
				fVec2 center((B.max[0] + B.min[0]) / 2, (B.max[1] + B.min[1]) / 2);
				double rx = (B.max[0] - B.min[0]) / 2;
				double ry = (B.max[1] - B.min[1]) / 2;
				draw_thick_filled_ellipse(center, rx, ry, thickness_x, thickness_y, color, fillcolor, aa, blend, grid_align, min_tick);
				}


			/**
			* Draw part of a thick filled ellipse inside a (real-valued) box.
			*
			* @param	part		combination of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT
			* @param	B		    ellipse outer bounding box.
			* @param	thickness_x thickness along the x-axis (the ellipse inner radius is obtained by
			* 						substracting this quantity from the bounding box radius).
			* @param	thickness_y thickness along the y-axis (the ellipse inner radius is obtained by
			* 						substracting this quantity from the bounding box radius).
			* @param	color	    color.
			* @param	fillcolor   color to fill the ellipse.
			* @param	aa		    (Optional) true to use antialiasing.
			* @param	blend	    (Optional) true to use blending.
			*/
			inline void draw_part_thick_filled_ellipse_in_box(int part, const fBox2 & B, double thickness_x, double thickness_y, RGBc color, RGBc fillcolor, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, double min_tick = DEFAULT_MIN_THICKNESS)
				{
				fVec2 center((B.max[0] + B.min[0]) / 2, (B.max[1] + B.min[1]) / 2);
				double rx = (B.max[0] - B.min[0]) / 2;
				double ry = (B.max[1] - B.min[1]) / 2;
				draw_part_thick_filled_ellipse(part, center, rx, ry, thickness_x, thickness_y, color, fillcolor, aa, blend, min_tick);
				}



			/******************************************************************************************************************************************************
			*******************************************************************************************************************************************************
			*																				   																      *
			*                                                                 PIXEL ACCESS METHODS                                                                *
			*																																					  *
			*******************************************************************************************************************************************************
			*******************************************************************************************************************************************************/


			/**
			* Return a pointer to the pixel buffer (const version).
			**/
			MTOOLS_FORCEINLINE const RGBc * data() const { return _data; }


			/**
			* Return a pointer to the pixel buffer.
			**/
			MTOOLS_FORCEINLINE RGBc * data() { return _data; }


			/**
			 * Return a pointer to a given pixel in the image. No bound check.
			 *
			 * @param	x The x coordinate.
			 * @param	y The y coordinate.
			 *
			 * @return	a pointer to the correpsonding image pixel.
			 */
			MTOOLS_FORCEINLINE const RGBc * offset(int64 x, int64 y) const 
				{ 
				MTOOLS_ASSERT(!isEmpty());
				MTOOLS_ASSERT((x >= 0) && (x < _lx));
				MTOOLS_ASSERT((y >= 0) && (y < _ly));
				return _data + (y*_stride) + x;
				}


			/**
			* Return a pointer to a given pixel in the image. No bound check.
			*
			* @param	x The x coordinate.
			* @param	y The y coordinate.
			*
			* @return	a pointer to the correpsonding image pixel.
			*/
			MTOOLS_FORCEINLINE RGBc * offset(int64 x, int64 y)
				{
				MTOOLS_ASSERT(!isEmpty());
				MTOOLS_ASSERT((x >= 0) && (x < _lx));
				MTOOLS_ASSERT((y >= 0) && (y < _ly));
				return _data + (y*_stride) + x;
				}



			/**
			 * Return a pointer to a given pixel in the image. No bound check.
			 *
			 * @param	pos The position.
			 *
			 * @return	a pointer to the correpsonding image pixel.
			 */
			MTOOLS_FORCEINLINE const RGBc * offset(iVec2 pos) const
				{
				return offset(pos.X(), pos.Y());
				}


			/**
			* Return a pointer to a given pixel in the image. No bound check.
			*
			* @param	pos The position.
			*
			* @return	a pointer to the correpsonding image pixel.
			*/
			MTOOLS_FORCEINLINE RGBc * offset(iVec2 pos)
				{
				return offset(pos.X(), pos.Y());
				}

	

			/**
			 * Get the color at a given position.
			 * No bound check !
			 *
			 * @param	x	The x coordinate.
			 * @param	y	The y coordinate.
			 *
			 * @return	The color at position (x,y).
			 **/
			MTOOLS_FORCEINLINE RGBc & operator()(const int64 x, const int64 y) { MTOOLS_ASSERT((x >= 0) && (x < _lx)); MTOOLS_ASSERT((y >= 0) && (y < _ly)); return _data[x + _stride*y]; }


			/**
			 * Get the color at a given position. No bound check !
			 *
			 * @param	pos	The position to look at.
			 *
			 * @return	The color at position pos.
			 **/
			MTOOLS_FORCEINLINE RGBc & operator()(const iVec2 & pos)
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
			MTOOLS_FORCEINLINE const RGBc & operator()(const int64 x, const int64 y) const { MTOOLS_ASSERT((x >= 0) && (x < _lx)); MTOOLS_ASSERT((y >= 0) && (y < _ly)); return _data[x + _stride*y]; }


			/**
			* Get the color at a given position. (const version)
			* No bound check !
			*
			* @param	pos	The position to look at.
			*
			* @return	The color at position pos.
			**/
			MTOOLS_FORCEINLINE const RGBc & operator()(const iVec2 & pos) const
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
			MTOOLS_FORCEINLINE void setPixel(const int64 x, const int64 y, const RGBc color)
				{
				if ((x >= 0) && (x < _lx) && (y >= 0) && (y < _ly)) { _data[x + _stride*y] = color; }
				}


			/**
			 * Sets a pixel. Does nothing if the position is outside of the image.
			 *
			 * @param	pos  	The position to consider.
			 * @param	color	color to set.
			 **/
			MTOOLS_FORCEINLINE void setPixel(const iVec2 & pos, const RGBc color)
				{
				const int64 x = pos.X();
				const int64 y = pos.Y();
				if ((x >= 0) && (x < _lx) && (y >= 0) && (y < _ly)) { _data[x + _stride*y] = color; }
				}


			/**
			* Blend a color over a given pixel. Does nothing if the position is outside of the image.
			* To improve performance, remove bound checking using (*this)(x,y).blend(color).
			* 
			* @param	x	 	The x coordinate.
			* @param	y	 	The y coordinate.
			* @param	color	color to blend over
			**/
			MTOOLS_FORCEINLINE void blendPixel(const int64 x, const int64 y, const RGBc color)
				{
				if ((x >= 0) && (x < _lx) && (y >= 0) && (y < _ly)) { _data[x + _stride*y].blend(color); }
				}


			/**
			* Blend a color over a given pixel. Does nothing if the position is outside of the image.
			* To improve performance, remove bound checking using (*this)(x,y).blend(color).
			*
			* @param	pos  	The position to consider.
			* @param	color	color to blend over
			**/
			MTOOLS_FORCEINLINE void blendPixel(const iVec2 & pos, const RGBc color)
				{
				const int64 x = pos.X();
				const int64 y = pos.Y();
				if ((x >= 0) && (x < _lx) && (y >= 0) && (y < _ly)) { _data[x + _stride*y].blend(color); }
				}


			/**
			 * Blend a color over a given pixel. Does nothing if the position is outside of the image. To
			 * improve performance, remove bound checking using (*this)(x,y).blend(color,op).
			 *
			 * @param	x	 	The x coordinate.
			 * @param	y	 	The y coordinate.
			 * @param	color	color to blend over.
			 * @param	op   	opacity pre-multiplier in [0.0f , 1.0f].
			 **/
			MTOOLS_FORCEINLINE void blendPixel(const int64 x, const int64 y, const RGBc color, float op)
				{
				if ((x >= 0) && (x < _lx) && (y >= 0) && (y < _ly)) { _data[x + _stride*y].blend(color,op); }
				}


			/**
			 * Blend a color over a given pixel. Does nothing if the position is outside of the image. To
			 * improve performance, remove bound checking using (*this)(x,y).blend(color,op).
			 *
			 * @param	pos  	The position to consider.
			 * @param	color	color to blend over.
			 * @param	op   	opacity pre-multiplier in [0.0f , 1.0f].
			 **/
			MTOOLS_FORCEINLINE void blendPixel(const iVec2 & pos, const RGBc color, float op)
				{
				const int64 x = pos.X();
				const int64 y = pos.Y();
				if ((x >= 0) && (x < _lx) && (y >= 0) && (y < _ly)) { _data[x + _stride*y].blend(color,op); }
				}


			/**
			 * Blend a color over a given pixel. Does nothing if the position is outside of the image. To
			 * improve performance, remove bound checking using (*this)(x,y).blend(color,op).
			 *
			 * @param	x	 	The x coordinate.
			 * @param	y	 	The y coordinate.
			 * @param	color	color to blend over.
			 * @param	op   	opacity pre-multiplier in the range [0, 0x100]. (use
			 * 					convertAlpha_0xFF_to_0x100() to convert to that range if needed).
			 **/
			MTOOLS_FORCEINLINE void blendPixel(const int64 x, const int64 y, const RGBc color, uint32 op)
				{
				if ((x >= 0) && (x < _lx) && (y >= 0) && (y < _ly)) { _data[x + _stride*y].blend(color, op); }
				}


			/**
			 * Blend a color over a given pixel. Does nothing if the position is outside of the image. To
			 * improve performance, remove bound checking using (*this)(x,y).blend(color,op).
			 *
			 * @param	pos  	The position to consider.
			 * @param	color	color to blend over.
			 * @param	op   	opacity pre-multiplier in the range [0, 0x100]. (use
			 * 					convertAlpha_0xFF_to_0x100() to convert to that range if needed).
			 **/
			MTOOLS_FORCEINLINE void blendPixel(const iVec2 & pos, const RGBc color, uint32 op)
				{
				const int64 x = pos.X();
				const int64 y = pos.Y();
				if ((x >= 0) && (x < _lx) && (y >= 0) && (y < _ly)) { _data[x + _stride*y].blend(color, op); }
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
			MTOOLS_FORCEINLINE RGBc getPixel(const int64 x, const int64 y, const RGBc defaultcolor = RGBc::c_Transparent) const
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
			MTOOLS_FORCEINLINE RGBc getPixel(const iVec2 & pos, const RGBc defaultcolor = RGBc::c_Transparent) const
				{
				const int64 x = pos.X();
				const int64 y = pos.Y();
				if ((x >= 0) && (x < _lx) && (y >= 0) && (y < _ly)) { return _data[x + _stride*y]; }
				return defaultcolor;
				}



			/******************************************************************************************************************************************************
			*******************************************************************************************************************************************************
			*																				   																      *
			*                                                                    SHARE RELATED METHODS                                                            *
			*																																					  *
			*******************************************************************************************************************************************************
			*******************************************************************************************************************************************************/


			/**
			* Query if the image shares its pixel buffer with another image. Equivalent to checking if
			* refcount() = 1.
			*
			* @return	true if shared, false if not.
			*/
			MTOOLS_FORCEINLINE bool isShared() const
				{
				return(refcount() != 1);
				}


			/**
			 * Determines if the image use an external memory buffer.
			 *
			 * @return	true if the memory buffer was supplied when creating the image and false if it is
			 * 			created and managed by the Image object.
			 */
			MTOOLS_FORCEINLINE bool useExternalBuffer() const
				{
				return _isExternalBuffer();
				}


			/**
			 * Query is both image share the same memory buffer.
			 * 
			 * This does not mean that there pixel intersect since they could be distincts sub-images using
			 * the same memory buffer. Use instead overlapMemoryWith() to detect if some pixels are shared.
			 *
			 * @param	im The image to check against.
			 *
			 * @return	true if they share the same memory buffer and false otherwise.
			 */
			MTOOLS_FORCEINLINE bool shareBufferWith(const Image & im) const
				{
				uint32 * p1 = _beginOriginalBuffer();
				uint32 * p2 = im._beginOriginalBuffer();
				return ((p1 == p2) && (p1 != nullptr));
				}
				


			/**
			 * Query if the memory buffer of two images overlap. This is more precise than shareBuffer()
			 * because two images may share the same buffer but can still use distinct memory region.  
			 *
			 * @param	im The image to compare against.
			 *
			 * @return	true if some pixel are shared and false otherwise. 
			 */
			MTOOLS_FORCEINLINE bool overlapMemoryWith(const Image & im) const
				{
				if (!shareBufferWith(im)) return false;
				// ok, both image have the same memory buffer. 
				RGBc * p = (RGBc *)_beginOriginalBuffer();
				MTOOLS_INSURE(_stride == im._stride);

				int64 offa = (int64)(_data - p) - 4;
				MTOOLS_INSURE(offa >= 0);
				int64 xa = offa % _stride, ya = offa / _stride;
				iBox2 Ba(xa, xa + _lx - 1, ya, ya + _ly - 1);

				int64 offb = (int64)(im._data - p) - 4; 
				MTOOLS_INSURE(offb >= 0);
				int64 xb = offb % _stride, yb = offb / _stride;
				iBox2 Bb(xb, xb + im._lx - 1, yb, yb + im._ly - 1);

				return(!(intersectionRect(Ba, Bb).isEmpty()));
				}

			
			/**
			* Query the number of images sharing the same data buffer.
			*
			* @return	Number of image sharing the same data buffer (1 is the image is not shared).
			*/
			MTOOLS_FORCEINLINE uint32 refcount() const
				{
				return ((_deletepointer != nullptr) ? (*_deletepointer) : 1);
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
			inline bool standalone(int64 padding = 0)
				{
				if (!isShared()) return false;
				*this = get_standalone(padding); // use move assignement operator. 
				return true;
				}


			/******************************************************************************************************************************************************
			*******************************************************************************************************************************************************
			*																				   																      *
			*                                                          LOADING/SAVING METHODS                                                                     *
			*																																					  *
			*******************************************************************************************************************************************************
			*******************************************************************************************************************************************************/

#if (MTOOLS_USE_CAIRO)

			/**
			* Saves the image into a file in PNG format.
			* uses cairo.
			*
			* @param	filename	name of the file (should have extension "png").
			*
			* @return	true if the operation succedded and false if it failed.
			**/
			/*
			bool save_png(const char * filename) const
				{
				if (!_createcairo(false)) return false;
				return (cairo_surface_write_to_png((cairo_surface_t*)_pcairo_surface, filename) == CAIRO_STATUS_SUCCESS);
				}
			*/


			/**
			* Load the image from a file in PGN format.
			* uses cairo. 
			* 
			* @param	filename	name of the file (should have extension "png" or "jpg").
			*
			* @return	true if the operation succedded and false if it failed (and in this case, the object
			* 			is set to an empty image).
			**/
			/*
			bool load_png(const char * filename)
				{
				empty();
				cairo_surface_t * psurface = cairo_image_surface_create_from_png(filename);
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
			*/

#endif 


			/**
			 * Saves the image into a file. uses CImg's save method (hence support all formats supported by
			 * CImg).
			 *
			 * @param	filename name of the file.
			 * @param	number   number to apped to the file name (if positive)
			 * @param	digits   number of digits to use.
			 */
			void save(const char * filename, const int number = -1, const unsigned int digits = 6) const
				{
				cimg_library::CImg<unsigned char> im;
				toCImg(im);
				im.save(filename, number, digits);
				}


			/**
			 * Load the image from a file. Uses CImg's load method (hence support all formats supported by
			 * CImg).
			 *
			 * @param	filename name of the file.
			 */
			void load(const char * filename)
				{
				cimg_library::CImg<unsigned char> im;
				im.load(filename);
				fromCImg(im);
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
					ar.opaqueArray(_data + _stride*j, (size_t)_lx);
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
					ar.opaqueArray(_data + _stride*j, (size_t)_lx);
					}
				}


			/******************************************************************************************************************************************************
			*******************************************************************************************************************************************************
			*																				   																      *
			*                                                                    MISC METHODS                                                                     *
			*																																					  *
			*******************************************************************************************************************************************************
			*******************************************************************************************************************************************************/



			/**
			* Width of the image in pixels. Same as width().
			**/
			MTOOLS_FORCEINLINE int64 lx() const { return _lx; }


			/**
			* Width of the image in pixels. Same as lx().
			**/
			MTOOLS_FORCEINLINE int64 width() const { return _lx; }


			/**
			* Height of the image in pixels. Same as height().
			**/
			MTOOLS_FORCEINLINE int64 ly() const { return _ly; }


			/**
			* Height of the image in pixels. Same as ly().
			**/
			MTOOLS_FORCEINLINE int64 height() const { return _ly; }


			/**
			* Return the image size into a iVec2 structure.
			**/
			MTOOLS_FORCEINLINE mtools::iVec2 dimension() const { return mtools::iVec2(_lx, _ly); }


			/**
			* Return a iBox2 representing the image size i.e. iBox2(0, lx()-1, 0, ly() - 1)
			**/
			MTOOLS_FORCEINLINE iBox2 imageBox() const { return iBox2(0, _lx - 1, 0, _ly - 1); }


			/**
			* return the image aspect ratio lx/ly.
			**/
			MTOOLS_FORCEINLINE double aspectRatio() const
				{
				if (isEmpty()) { MTOOLS_DEBUG("Image::aspectRatio() called with empty image !"); return 1.0; }
				return(((double)_lx) / (double)_ly); 
				}


			/**
			* Query if the image is empty
			*
			* @return	true if empty, false if not.
			**/
			MTOOLS_FORCEINLINE bool isEmpty() const { return(_data == nullptr); }


			/**
			 * Return the number of pixels in this image. 
			 * use memorySize() to get number of bytes used by the obejct.
			 *
			 * @return	the number of pixels in the images. 
			 */
			MTOOLS_FORCEINLINE int64 size() const
				{
				return (isEmpty() ? 0 : _lx*_ly);
				}


			/**
			 * Return the total memory occupies by this object.
			 *
			 * @return	the number of bytes used by the object.
			 */
			MTOOLS_FORCEINLINE int64 memorySize() const
				{
				return sizeof(Image)  + (!isEmpty() ? 0 : 4*_ly*_stride);
				}


			/**
			 * 	Empty this image (the resulting image has size 0x0).
			 **/
			inline void empty()
				{
				_removecairo();
				_deallocate();
				_lx = 0;
				_ly = 0;
				_stride = 0;
				}



			/**
			* Return the first color of the image.
			* If the image is empty return Transparent_White otherwise return the color at coord (0,0).
			*
			* @return  The main color.
			**/
			MTOOLS_FORCEINLINE RGBc toRGBc() const
				{
				if (isEmpty()) return RGBc::c_Transparent;
				return getPixel({ 0,0 });
				}


			/**
			 * Clears this image to a given color.
			 *
			 * @param	bkColor	the color to use.
			 **/
			MTOOLS_FORCEINLINE void clear(RGBc bkColor)
				{
				//pixman_fill((uint32_t*)_data, _stride, 32, 0, 0, _lx, _ly, bkColor.color); // slow...
				_fillRegion(_data, _stride, _lx, _ly, bkColor);
				}


			/**
			* Blend a color over the whole image.
			*
			* @param	bkColorBlend	the color to blend over the whole image.
			**/
			MTOOLS_FORCEINLINE void clearBlend(RGBc bkColorBlend)
				{
				_blendRegion(_data, _stride, _lx, _ly, bkColorBlend);
				}


			/**
			* Fill the image with a checkerboard pattern. 
			* Default colors are two slightly distincts shades of gray.
			*
			* @param   color1      The first color.
			* @param   color2      The second color.
			* @param   sizeSquare  The side lengh of an elementary square.
			*
			* @return  The image for chaining.
			**/
			void checkerboard(mtools::RGBc color1 = mtools::RGBc(200, 200, 200, 255), mtools::RGBc color2 = mtools::RGBc(220, 220, 220, 255), int64 sizeSquare = 50)
				{
				if (isEmpty()) return;
				if (color1 == color2) { clear(color1); return; }
				int64 nx = 0, ny = 0, cy = 0, cx = 0;
				RGBc * p = _data;
				for (int64 j = 0; j < _ly; j++)
					{
					cx = cy; nx = 0;
					for (int64 i = 0; i < _lx; i++)
						{
						p[i] = ((cx == 0) ? color1 : color2);
						if ((++nx) == sizeSquare) { cx = 1 - cx; nx = 0; }
						}
					p += _stride;
					if ((++ny) == sizeSquare) { cy = 1 - cy; ny = 0; }
					}
				return;
				}



			/**
			* Horizontal padding of the image: number of uint32 following the end of each horizontal line
			* (except the last one).
			**/
			MTOOLS_FORCEINLINE int64 padding() const { return(_stride - _lx); }


			/**
			 * return the number of uint32 for each line of tghe image. we have the relation:
			 * 
			 * stride() = width() + padding()
			 *
			 * @return	the image stride
			 */
			MTOOLS_FORCEINLINE int64 stride() const { return(_stride); }


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
			* Equality operator.
			*
			* @param	im The image to compare against.
			*
			* @return	true if the images are visually equivalent: they have the same size (lx, ly) and the
			* 			same pixel color (but the padding may differ).
			*/
			inline bool operator==(const Image & im) const
				{
				if ((_lx != im._lx) || (_ly != im._ly)) return false;
				if ((_data == nullptr) || (_data == im._data)) return true;
				for (int64 j = 0; j < _ly; j++)
					{
					if (memcmp(_data + j*_stride, im._data + j*im._stride, (size_t)(_lx * 4)) != 0) return false;
					}
				return true;
				}


			/**
			* Find the (closed) minimal bounding rectangle enclosing the image.
			*
			* @param	bk_color	The 'background' color which is not part of the image.
			*
			* @return	the minimal bounding box.
			**/
			inline iBox2 minBoundingBox(RGBc bk_color)
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
			inline iBox2 minBoundingBox()
				{
				int64 minx = _lx + 1, maxx = -1;
				int64 miny = _ly + 1, maxy = -1;
				for (int64 j = 0; j < _ly; j++)
					{
					for (int64 i = 0; i < _lx; i++)
						{
						if (!(operator()(i, j).isTransparent()))
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
			* Swaps the content of the two images. Very fast.
			*
			* @param [in,out]	im	The image to swap with
			**/
			inline void swap(Image & im)
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
						memcpy(pdst, psrc, (size_t)(_lx * 4));
						}
					}
				return im;
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






			/******************************************************************************************************************************************************
			*******************************************************************************************************************************************************
			*																				   																      *
			*                                                              CANVAS RELATED METHODS                                                                 *
			*																																					  *
			*******************************************************************************************************************************************************
			*******************************************************************************************************************************************************/


			/**
			* Return the position (i,j) of the pixel in the image associated with the absolute coordinate
			* (x,y) w.r.t. to a mapping rectangle R.
			*
			* @param   R       the rectangle defining the range represented by the image.
			* @param   coord   the absolute coordinate to transform.
			*
			* @return  the associated pixel the image (no clipping, may be outside of the image).
			**/
			MTOOLS_FORCEINLINE mtools::iVec2 canvas_getImageCoord(const mtools::fBox2 & R, const mtools::fVec2 & coord) const { return R.absToPixel(coord, dimension()); }


			/**
			* Return the absolute position of a pixel (i,j) of the image according to a mapping rectangle R.
			*
			* @param   R       the rectangle defining the range represented by the image.
			* @param   pixpos  The position of the pixel in the image.
			*
			* @return  the associated absolute coordinate.
			**/
			MTOOLS_FORCEINLINE mtools::fVec2 canvas_getAbsCoord(const mtools::fBox2 & R, const mtools::iVec2 & pixpos) const { return R.pixelToAbs(pixpos, dimension()); }


			/**
			* Return an enlarged rectangle of R with the same centering but such that the ratio of the
			* returned  rectangle match the ratio of the image.
			*
			* @param   R   the original rectangle.
			*
			* @return  A possibly enlarged rectangle (with same centering) with same aspect ratio as the
			*          image.
			**/
			MTOOLS_FORCEINLINE mtools::fBox2 canvas_respectImageAspectRatio(const mtools::fBox2 & R) const { return(R.fixedRatioEnclosingRect(aspectRatio())); }


			/**
			* The canonical range rectangle corresponding to the image size
			**/
			MTOOLS_FORCEINLINE mtools::fBox2 canvas_getCanonicalRange() const { MTOOLS_ASSERT((_lx > 0) && (_ly > 0)); return mtools::fBox2(0, (double)_lx, 0, (double)_ly); }



			/**
			 * Draw background of the rectangle that enclosed a given text. Color bkcolor is blend over the
			 * image.
			 * 
			 * Use absolute coordinate (canvas method).
			 *
			 * @param	R	   	the absolute range represented in the image.
			 * @param	pos	   	the text reference position.
			 * @param	txt	   	the text.
			 * @param	txt_pos	Positioning method (combination of MTOOLS_TEXT_XCENTER, MTOOLS_TEXT_LEFT,
			 * 					MTOOLS_TEXT_RIGHT, MTOOLS_TEXT_TOP, MTOOLS_TEXT_BOTTOM, MTOOLS_TEXT_YCENTER).
			 * @param	bkcolor	The color to blend over.
			 * @param	font   	the font to use.
			 **/
			MTOOLS_FORCEINLINE void canvas_draw_text_background(const mtools::fBox2 & R, const fVec2 & pos, const std::string & txt, int txt_pos, RGBc bkcolor, const Font * font)
				{
				draw_text_background(R.absToPixel(pos,dimension()), txt, txt_pos, bkcolor, font);
				}


			/**
			 * Draw background of the rectangle that enclosed a given text. Color bkcolor is blend over the
			 * image. This version uses the default font (gFont).
			 * 
			 * Use absolute coordinate (canvas method).
			 *
			 * @param	R			the absolute range represented in the image.
			 * @param	pos			the text reference position.
			 * @param	txt			the text.
			 * @param	txt_pos 	Positioning method (combination of MTOOLS_TEXT_XCENTER, MTOOLS_TEXT_LEFT,
			 * 						MTOOLS_TEXT_RIGHT, MTOOLS_TEXT_TOP, MTOOLS_TEXT_BOTTOM, MTOOLS_TEXT_YCENTER).
			 * @param	bkcolor 	The color to blend over.
			 * @param	fontsize	the font size to use.
			 **/
			MTOOLS_FORCEINLINE void canvas_draw_text_background(const mtools::fBox2 & R, const fVec2 & pos, const std::string & txt, int txt_pos, RGBc bkcolor, int fontsize)
				{
				draw_text_background(R.absToPixel(pos, dimension()), txt, txt_pos, bkcolor, fontsize);
				}


			/**
			* Draws a text on the image, with a given color and using a given font.
			*
			* Use absolute coordinate (canvas method).
			*
			* @param   R               the absolute range represented in the image.
			* @param	pos	   	the text reference position.
			* @param	txt	   	the text to draw.
			* @param	txt_pos	Positioning method (combination of MTOOLS_TEXT_XCENTER, MTOOLS_TEXT_LEFT,
			* 					MTOOLS_TEXT_RIGHT, MTOOLS_TEXT_TOP, MTOOLS_TEXT_BOTTOM, MTOOLS_TEXT_YCENTER).
			* @param	color  	The color to blend over.
			* @param	font   	the font to use.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_text(const mtools::fBox2 & R, const fVec2 & pos, const std::string & txt, int txt_pos, RGBc color, const Font * font)
				{
				draw_text(R.absToPixel(pos, dimension()), txt, txt_pos, color, font);
				}


			/**
			 * Draws a text on the image with a given color. Use the default font [gFont].
			 * 
			 * Use absolute coordinate (canvas method).
			 *
			 * @param	R			the absolute range represented in the image.
			 * @param	pos			the text reference position.
			 * @param	txt			the text to draw.
			 * @param	txt_pos 	Positioning method (combination of MTOOLS_TEXT_XCENTER, MTOOLS_TEXT_LEFT,
			 * 						MTOOLS_TEXT_RIGHT, MTOOLS_TEXT_TOP, MTOOLS_TEXT_BOTTOM, MTOOLS_TEXT_YCENTER).
			 * @param	color   	The color to blend over.
			 * @param	fontsize	the font size to use.
			 **/
			MTOOLS_FORCEINLINE void canvas_draw_text(const mtools::fBox2 & R, const fVec2 & pos, const std::string & txt, int txt_pos, RGBc color, int fontsize)
				{
				draw_text(R.absToPixel(pos, dimension()), txt, txt_pos, color, fontsize);
				}


			/**
			* Draw a (square) dot on the image.
			*
			* Use absolute coordinate (canvas method).
			*
			* @param	R			the absolute range represented in the image.
			* @param	P			Position of the center.
			* @param	color   	The color.
			* @param	blending	true to use blending.
			* @param	penwidth	The pen width (radius of the square: 0 = single pixel).
			**/
			MTOOLS_FORCEINLINE void canvas_draw_dot(const mtools::fBox2 & R, fVec2 P, RGBc color, bool blending, int32 penwidth = 0)
				{
				draw_dot(R.absToPixel(P, dimension()), color, blending, penwidth);
				}


			/**
			* Draw an horizontal line  (canvas method).
			*
			* @param	R			the absolute range represented in the image.
			* @param	y			The y coordinate of the line.
			* @param	x1			x value of the start point.
			* @param	x2			x value of the end point.
			* @param	color   	The color to use.
			* @param	draw_P2 	true to draw the end point.
			* @param	blending	true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_horizontal_line(const mtools::fBox2 & R, double y, double x1, double x2, RGBc color, bool draw_P2 = true, bool blending = true)
				{
				const iVec2 P1 = R.absToPixel({ x1,y }, dimension());
				const iVec2 P2 = R.absToPixel({ x2,y }, dimension());
				draw_horizontal_line(P1.Y(), P1.X(), P2.X(), color, draw_P2, blending);
				}


			/**
			* Draw a thick horizontal line  (canvas method).
			*
			* @param	R			the absolute range represented in the image.
			* @param	y			The y coordinate of the line.
			* @param	x1			x value of the start point.
			* @param	x2			x value of the end point.
			* @param	thickness	The thickness.
			* @param    relativethickness	true to scale tickness with range and false to use constant thickness.
			* @param	color   	The color to use.
			* @param	draw_P2 	true to draw the end point.
			* @param	blending	true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_thick_horizontal_line(const mtools::fBox2 & R, double y, double x1, double x2, double thickness, bool relativethickness, RGBc color, bool draw_P2 = true, bool blending = true, double min_tick = DEFAULT_MIN_THICKNESS)
				{
				const iVec2 P1 = R.absToPixel({ x1,y }, dimension());
				const iVec2 P2 = R.absToPixel({ x2,y }, dimension());
				const double th = (relativethickness ? boxTransform_dy(thickness, R, fBox2(-0.5, lx() - 0.5, -0.5, ly() - 0.5)) : thickness);
				draw_horizontal_line(P1.Y(), P1.X(), P2.X(), th, color, draw_P2, blending, min_tick);
				}


			/**
			* Draw a vertical line (canvas method)
			*
			* @param	R			the absolute range represented in the image.
			* @param	x			The x coordinate of the line.
			* @param	y1			y value of the start point.
			* @param	y2			y value of the end point.
			* @param	color   	The color to use.
			* @param	draw_P2 	true to draw the end point.
			* @param	blending	true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_vertical_line(const mtools::fBox2 & R, double x, double y1, double y2, RGBc color, bool draw_P2 = true, bool blending = true)
				{
				const iVec2 P1 = R.absToPixel({ x,y1 }, dimension());
				const iVec2 P2 = R.absToPixel({ x,y2 }, dimension());
				draw_vertical_line(P1.X(), P1.Y(), P2.Y(), color, draw_P2, blending);
				}


			/**
			* Draw a (tick) vertical line (canvas method).
			*
			* @param	R			the absolute range represented in the image.
			* @param	x			The x coordinate of the line.
			* @param	y1			y value of the start point.
			* @param	y2			y value of the end point.
			* @param	thickness	The thickness.
			* @param    relativethickness	true to scale tickness with range and false to use constant thickness.
			* @param	color   	The color to use.
			* @param	draw_P2 	true to draw the end point.
			* @param	blending	true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_thick_vertical_line(const mtools::fBox2 & R, double x, double y1, double y2, double thickness, bool relativethickness, RGBc color, bool draw_P2 = true, bool blending = true, double min_tick = DEFAULT_MIN_THICKNESS)
				{
				const iVec2 P1 = R.absToPixel({ x,y1 }, dimension());
				const iVec2 P2 = R.absToPixel({ x,y2 }, dimension());
				const double th = (relativethickness ? boxTransform_dx(thickness, R, fBox2(-0.5, lx() - 0.5, -0.5, ly() - 0.5)) : thickness);
				draw_vertical_line(P1.X(), P1.Y(), P2.Y(), th, color, draw_P2, blending, min_tick);
				}



			/**
			* Draw a line.
			*
			* Use absolute coordinate (canvas method).
			*
			* @param	R		the absolute range represented in the image.
			* @param	P1	   	First point.
			* @param	P2	   	Second endpoint.
			* @param	color  	The color to use.
			* @param	draw_P2	true to draw the endpoint P2.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_line(const mtools::fBox2 & R, fVec2 P1, fVec2 P2, RGBc color, bool draw_P2)
				{
				const auto dim = dimension();
				draw_line(R.absToPixel(P1, dim), R.absToPixel(P2, dim), color, draw_P2);
				}


			/**
			* Draw a line.
			*
			* Use absolute coordinate (canvas method).
			*
			* @param	R			the absolute range represented in the image.
			* @param	P1		   	First point.
			* @param	P2		   	Second endpoint.
			* @param	color	   	The color to use.
			* @param	draw_P2	   	true to draw the endpoint P2.
			* @param	blending   	true to use blending instead of simply overwriting the color.
			* @param	antialiased	true to draw an antialised line.
			* @param	penwidth   	The pen width (0 = unit width)
			**/
			MTOOLS_FORCEINLINE void canvas_draw_line(const mtools::fBox2 & R, fVec2 P1, fVec2 P2, RGBc color, bool draw_P2, bool blending, bool antialiased, int32 penwidth = 0)
				{
				const auto dim = dimension();
				draw_line(R.absToPixel(P1, dim), R.absToPixel(P2, dim), color, draw_P2,blending, antialiased, penwidth);
				}


			/**
			* Draw a quadratic (rational) Bezier curve.
			*
			* Use absolute coordinate (canvas method).
			*
			* @param	R				the absolute range represented in the image.
			* @param	P1				The first point.
			* @param	P2				The second point.
			* @param	PC				The control point.
			* @param	wc				The control point weight. Must be positive (faster for wc = 1 =
			* 							classic quad Bezier curve).
			* @param	color			The color to use.
			* @param	draw_P2			true to draw the endpoint P2.
			* @param	blending		true to use blending.
			* @param	antialiasing	true to use antialiasing.
			* @param	penwidth		The pen width (0 = unit width)
			**/
			MTOOLS_FORCEINLINE void canvas_draw_quad_bezier(const mtools::fBox2 & R, fVec2 P1, fVec2 P2, fVec2 PC, float wc, RGBc color, bool draw_P2, bool blending, bool antialiasing, int32 penwidth = 0)
				{
				const auto dim = dimension();
				draw_quad_bezier(R.absToPixel(P1, dim), R.absToPixel(P2, dim), R.absToPixel(PC, dim), wc, color, draw_P2, blending, antialiasing, penwidth);
				}


			/**
			* Draw a cubic Bezier curve.
			*
			* Use absolute coordinate (canvas method).
			*
			* @param	R				the absolute range represented in the image.
			* @param	P1				The first point.
			* @param	P2				The second point.
			* @param	PA				The first control point.
			* @param	PB				The second control point.
			* @param	color			The color to use
			* @param	draw_P2			true to draw the endpoint P2.
			* @param	blending		true to use blending.
			* @param	antialiasing	true to use antialiasing.
			* @param	penwidth		The pen width (0 = unit width)
			**/
			MTOOLS_FORCEINLINE void canvas_draw_cubic_bezier(const mtools::fBox2 & R, fVec2 P1, fVec2 P2, fVec2 PA, fVec2 PB, RGBc color, bool draw_P2, bool blending, bool antialiasing, int32 penwidth = 0)
				{
				const auto dim = dimension();
				draw_cubic_bezier(R.absToPixel(P1, dim), R.absToPixel(P2, dim), R.absToPixel(PA, dim), R.absToPixel(PB, dim), color, draw_P2, blending, antialiasing, penwidth);
				}


			/**
			* Draw a quadratic spline.
			*
			* Use absolute coordinate (canvas method).
			*
			* @param	R				the absolute range represented in the image.
			* @param	tabPoints	   	std vector containing the points interpolated by the spline.
			* @param	color		   	The color tu use.
			* @param	draw_last_point	true to draw the last point.
			* @param	blending	   	true to use blending.
			* @param	antialiased	   	true to use anti-aliasing.
			* @param	penwidth		The pen width (0 = unit width)
			**/
			MTOOLS_FORCEINLINE void canvas_draw_quad_spline(const mtools::fBox2 & R, const std::vector<fVec2> & tabPoints, RGBc color, bool draw_last_point, bool blending, bool antialiased, int32 penwidth = 0)
				{
				const auto dim = dimension();
				const size_t N = tabPoints.size();
				std::vector<iVec2> tab;
				tab.reserve(N);
				for (size_t i=0; i < N; i++) { tab.push_back(R.absToPixel(tabPoints[i], dim)); }
				draw_quad_spline(tab, color, draw_last_point, blending, antialiased, penwidth);
				}


			/**
			* Draw a cubic spline.
			*
			* Use absolute coordinate (canvas method).
			*
			* @param	R				the absolute range represented in the image.
			* @param	tabPoints	   	std vector containing the points interpolated by the spline.
			* @param	color		   	The color tu use.
			* @param	draw_last_point	true to draw the last point.
			* @param	blending	   	true to use blending.
			* @param	antialiased	   	true to use anti-aliasing.
			* @param	penwidth		The pen width (0 = unit width)
			**/
			MTOOLS_FORCEINLINE void canvas_draw_cubic_spline(const mtools::fBox2 & R, const std::vector<fVec2> & tabPoints, RGBc color, bool draw_last_point, bool blending, bool antialiased, int32 penwidth = 0)
				{
				const auto dim = dimension();
				const size_t N = tabPoints.size();
				std::vector<iVec2> tab;
				tab.reserve(N);
				for (size_t i = 0; i < N; i++) { tab.push_back(R.absToPixel(tabPoints[i], dim)); }
				draw_cubic_spline(tab, color, draw_last_point, blending, antialiased, penwidth);
				}


			/**
			 * draw a rectangle of given size and color over this image. Portion outside the image is
			 * clipped.
			 * 
			 * Use absolute coordinate (canvas method).
			 *
			 * @param	R			the absolute range represented in the image.
			 * @param	dest_box	position of the rectangle to draw.
			 * @param	color   	the color to use.
			 * @param	blend   	true to use blending and false to simply copy the color.
			 * @param	penwidth	The pen width (0 = unit width)
			 **/
			MTOOLS_FORCEINLINE void canvas_draw_rectangle(const mtools::fBox2 & R, const fBox2 & dest_box, RGBc color, bool blend, int32 penwidth = 0)
				{
				draw_rectangle(R.absToPixel(dest_box,dimension()), color, blend, penwidth);
				}


			/**
			 * Fill the interior of a rectangle rectangle. Portion outside the image is clipped.
			 * 
			 * The boundary of the rectangle is not drawn. To fill the whole rectangle with its boundary,
			 * use draw_box() instead.
			 * 
			 * Use absolute coordinate (canvas method).
			 *
			 * @param	R		 	the absolute range represented in the image.
			 * @param	dest_box 	position of the rectangle to draw.
			 * @param	fillcolor	the color to use.
			 * @param	blend	 	true to use blending and false to simply copy the color.
			 **/
			MTOOLS_FORCEINLINE void canvas_fill_rectangle(const mtools::fBox2 & R, const fBox2 & dest_box, RGBc fillcolor, bool blend)
				{
				fill_rectangle(R.absToPixel(dest_box, dimension()), fillcolor, blend);
				}


			/**
			 * Fill a (closed) box with a given color. Portion outside the image is clipped.
			 * 
			 * Use absolute coordinate (canvas method).
			 *
			 * @param	R		 	the absolute range represented in the image.
			 * @param	dest_box 	position of the rectangle to draw.
			 * @param	fillcolor	the color to use.
			 * @param	blend	 	true to use blending and false to simply copy the color.
			 **/
			MTOOLS_FORCEINLINE void canvas_draw_box(const mtools::fBox2 & R, const fBox2 & dest_box, RGBc fillcolor, bool blend)
				{
				draw_box(R.absToPixel(dest_box, dimension()), fillcolor, blend);
				}


			/**
			 * Draw a triangle. Portion outside the image is clipped.
			 * 
			 * Use absolute coordinate (canvas method).
			 *
			 * @param	R		   	the absolute range represented in the image.
			 * @param	P1		   	The first point.
			 * @param	P2		   	The second point.
			 * @param	P3		   	The third point.
			 * @param	color	   	The color.
			 * @param	blending   	true to use blending and false to write over.
			 * @param	antialiased	true to use antialiased lines.
			 * @param	penwidth   	The pen width (0 = unit width)
			 **/
			MTOOLS_FORCEINLINE void canvas_draw_triangle(const mtools::fBox2 & R, fVec2 P1, fVec2 P2, fVec2 P3, RGBc color, bool blending, bool antialiased, int32 penwidth = 0)
				{
				const auto dim = dimension();
				draw_triangle(R.absToPixel(P1, dim), R.absToPixel(P2, dim), R.absToPixel(P3, dim), color, blending, antialiased, penwidth);
				}


			/**
			 * Fill the interior of a triangle. Portion outside the image is clipped.
			 * 
			 * Only the interior is filled, the boundary lines are not drawn/filled.
			 * 
			 * Use absolute coordinate (canvas method).
			 *
			 * @param	R		 	the absolute range represented in the image.
			 * @param	P1		 	The first point.
			 * @param	P2		 	The second point.
			 * @param	P3		 	The third point.
			 * @param	fillcolor	The fill color.
			 * @param	blending 	true to use blending and false to write over.
			 **/
			MTOOLS_FORCEINLINE void canvas_fill_triangle(const mtools::fBox2 & R, fVec2 P1, fVec2 P2, fVec2 P3, RGBc fillcolor, bool blending)
				{
				const auto dim = dimension();
				fill_triangle(R.absToPixel(P1, dim), R.absToPixel(P2, dim), R.absToPixel(P3, dim), fillcolor, blending);
				}


			/**
			* Draw a polygon.
			*
			* Use absolute coordinate (canvas method).
			*
			* @param	R				the absolute range represented in the image.
			* @param	tabPoints  	std vector of polygon vertice in clockwise or counterclockwise order.
			* @param	color	   	The color tu use.
			* @param	blending   	true to use blending.
			* @param	antialiased	true to draw antialiased lines.
			* @param	penwidth   	The pen width (0 = unit width)
			**/
			MTOOLS_FORCEINLINE void canvas_draw_polygon(const mtools::fBox2 & R, const std::vector<fVec2> & tabPoints, RGBc color, bool blending, bool antialiased, int32 penwidth = 0)
				{
				const auto dim = dimension();
				const size_t N = tabPoints.size();
				std::vector<iVec2> tab;
				tab.reserve(N);
				for (size_t i = 0; i < N; i++) { tab.push_back(R.absToPixel(tabPoints[i], dim)); }
				draw_polygon(tab, color, blending, antialiased, penwidth);
				}


			/**
			* Fill the interior of a convex polygon. The edge are not drawn.
			*
			* Use absolute coordinate (canvas method).
			*
			* @param	R				the absolute range represented in the image.
			* @param	tabPoints  	std vector of polygon vertice in clockwise or counterclockwise order.
			* @param	color	   	The color tu use.
			* @param	blending   	true to use blending.
			* @param	antialiased	true to draw antialiased lines.
			**/
			MTOOLS_FORCEINLINE void canvas_fill_convex_polygon(const mtools::fBox2 & R, const std::vector<fVec2> & tabPoints, RGBc fillcolor, bool blending)
				{
				const auto dim = dimension();
				const size_t N = tabPoints.size();
				std::vector<iVec2> tab;
				tab.reserve(N);
				for (size_t i = 0; i < N; i++) { tab.push_back(R.absToPixel(tabPoints[i], dim)); }
				fill_convex_polygon(tab, fillcolor, blending);
				}


			/**
			* Draw a circle.
			*
			* @param	R	   the absolute range represented in the image.
			* @param	center center
			* @param	radius radius
			* @param	color  color
			* @param	aa	   (Optional) true to use antialiasing.
			* @param	blend  (Optional) true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_circle(const fBox2 & R, fVec2 center, double radius, RGBc color, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND)
			{
				const double EPS = 0.1;
				if (isEmpty()) return;
				const fBox2 imBox(-0.5, lx() - 0.5, -0.5, ly() - 0.5);
				double rx = boxTransform_dx(radius, R, imBox);
				double ry = boxTransform_dy(radius, R, imBox);
				if (std::abs<double>(rx - ry) < EPS) draw_circle(boxTransform(center, R, imBox), rx, color, aa, blend);
				else draw_ellipse(boxTransform(center, R, imBox), rx, ry, color, aa, blend);
			}


			/**
			* Draw part of a circle.
			*
			* @param	R	   the absolute range represented in the image.
			* @param	part   combination of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT
			* @param	center center
			* @param	radius radius
			* @param	color  color
			* @param	aa	   (Optional) true to use antialiasing.
			* @param	blend  (Optional) true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_part_circle(const fBox2 & R, int part, fVec2 center, double radius, RGBc color, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND)
				{
				const double EPS = 0.1;
				if (isEmpty()) return;
				_reverseVerticalCirclePart(part);
				const fBox2 imBox(-0.5, lx() - 0.5, -0.5, ly() - 0.5);
				double rx = boxTransform_dx(radius, R, imBox);
				double ry = boxTransform_dy(radius, R, imBox);
				if (std::abs<double>(rx - ry) < EPS) draw_part_circle(part,boxTransform(center, R, imBox), rx, color, aa, blend);
				else draw_part_ellipse(part,boxTransform(center, R, imBox), rx, ry, color, aa, blend);
				}



			/**
			* Draw a filled circle.
			*
			* @param	R			the absolute range represented in the image.
			* @param	center		center
			* @param	radius		radius
			* @param	color		color
			* @param	fillcolor	color to fill the circle.
			* @param	aa			(Optional) true to use antialiasing.
			* @param	blend		(Optional) true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_filled_circle(const fBox2 & R, fVec2 center, double radius, RGBc color, RGBc fillcolor, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND)
			{
				const double EPS = 0.1;
				if (isEmpty()) return;
				const fBox2 imBox(-0.5, lx() - 0.5, -0.5, ly() - 0.5);
				const double rx = boxTransform_dx(radius, R, imBox);
				const double ry = boxTransform_dy(radius, R, imBox);
				if (std::abs<double>(rx - ry) < EPS) draw_filled_circle(boxTransform(center, R, imBox), rx, color, fillcolor, aa, blend);
				else draw_filled_ellipse(boxTransform(center, R, imBox), rx, ry, color, fillcolor, aa, blend);
			}


			/**
			* Draw part of a filled circle.
			*
			* @param	R			the absolute range represented in the image.
			* @param	part		combination of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT
			* @param	center		center
			* @param	radius		radius
			* @param	color		color
			* @param	fillcolor	color to fill the circle.
			* @param	aa			(Optional) true to use antialiasing.
			* @param	blend		(Optional) true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_part_filled_circle(const fBox2 & R, int part, fVec2 center, double radius, RGBc color, RGBc fillcolor, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND)
			{
				const double EPS = 0.1;
				if (isEmpty()) return;
				_reverseVerticalCirclePart(part);
				const fBox2 imBox(-0.5, lx() - 0.5, -0.5, ly() - 0.5);
				const double rx = boxTransform_dx(radius, R, imBox);
				const double ry = boxTransform_dy(radius, R, imBox);
				if (std::abs<double>(rx - ry) < EPS) draw_part_filled_circle(part,boxTransform(center, R, imBox), rx, color, fillcolor, aa, blend);
				else draw_part_filled_ellipse(part,boxTransform(center, R, imBox), rx, ry, color, fillcolor, aa, blend);
			}


			/**
			* Draw a thick circle.
			*
			* @param	R					the absolute range represented in the image.
			* @param	center				center
			* @param	radius				radius
			* @param	thickness			thickness.
			* @param    relativethickness	true to scale tickness with range and false to use constant thcikness.
			* @param	color				color
			* @param	aa					(Optional) true to use antialiasing.
			* @param	blend				(Optional) true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_thick_circle(const fBox2 & R, fVec2 center, double radius, double thickness, bool relativethickness, RGBc color, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, double min_tick = DEFAULT_MIN_THICKNESS)
			{
				const double EPS = 0.1;
				if (isEmpty()) return;
				const fBox2 imBox(-0.5, lx() - 0.5, -0.5, ly() - 0.5);
				const double rx = boxTransform_dx(radius, R, imBox);
				const double ry = boxTransform_dy(radius, R, imBox);
				if (std::abs<double>(rx - ry) < EPS)
				{
					if (relativethickness) draw_thick_circle(boxTransform(center, R, imBox), rx, boxTransform_dx(thickness, R, imBox), color, aa, blend, min_tick);
					else draw_thick_circle(boxTransform(center, R, imBox), rx, thickness, color, aa, blend, min_tick);
				}
				else
				{
					if (relativethickness) draw_thick_ellipse(boxTransform(center, R, imBox), rx, ry, boxTransform_dx(thickness, R, imBox), boxTransform_dy(thickness, R, imBox), color, aa, blend, min_tick);
					else draw_thick_ellipse(boxTransform(center, R, imBox), rx, ry, thickness, thickness, color, aa, blend, min_tick);
				}
			}


			/**
			* Draw part of a thick circle.
			*
			* @param	R					the absolute range represented in the image.
			* @param	part				combination of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT
			* @param	center				center
			* @param	radius				radius
			* @param	thickness			thickness.
			* @param    relativethickness	true to scale tickness with range and false to use constant thcikness.
			* @param	color				color
			* @param	aa					(Optional) true to use antialiasing.
			* @param	blend				(Optional) true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_part_thick_circle(const fBox2 & R, int part, fVec2 center, double radius, double thickness, bool relativethickness, RGBc color, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, double min_tick = DEFAULT_MIN_THICKNESS)
			{
				const double EPS = 0.1;
				if (isEmpty()) return;
				_reverseVerticalCirclePart(part);
				const fBox2 imBox(-0.5, lx() - 0.5, -0.5, ly() - 0.5);
				const double rx = boxTransform_dx(radius, R, imBox);
				const double ry = boxTransform_dy(radius, R, imBox);
				if (std::abs<double>(rx - ry) < EPS)
					{
					if (relativethickness) draw_part_thick_circle(part,boxTransform(center, R, imBox), rx, boxTransform_dx(thickness, R, imBox), color, aa, blend, min_tick);
					else draw_part_thick_circle(part,boxTransform(center, R, imBox), rx, thickness, color, aa, blend, min_tick);
					}
				else
				{
					if (relativethickness) draw_part_thick_ellipse(part,boxTransform(center, R, imBox), rx, ry, boxTransform_dx(thickness, R, imBox), boxTransform_dy(thickness, R, imBox), color, aa, blend, min_tick);
					else draw_part_thick_ellipse(part,boxTransform(center, R, imBox), rx, ry, thickness, thickness, color, aa, blend, min_tick);
				}
			}


			/**
			* Draw a thick filled circle.
			*
			* @param	R					the absolute range represented in the image.
			* @param	center				center
			* @param	radius				radius
			* @param	thickness			thickness.
			* @param    relativethickness	true to scale tickness with range and false to use constant thcikness.
			* @param	color				color
			* @param	fillcolor			color to fill the circle.
			* @param	aa					(Optional) true to use antialiasing.
			* @param	blend				(Optional) true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_thick_filled_circle(const fBox2 & R, fVec2 center, double radius, double thickness, bool relativethickness, RGBc color, RGBc fillcolor, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, double min_tick = DEFAULT_MIN_THICKNESS)
			{
				const double EPS = 0.1;
				if (isEmpty()) return;
				const fBox2 imBox(-0.5, lx() - 0.5, -0.5, ly() - 0.5);
				const double rx = boxTransform_dx(radius, R, imBox);
				const double ry = boxTransform_dy(radius, R, imBox);
				if (std::abs<double>(rx - ry) < EPS)
				{
					if (relativethickness) draw_thick_filled_circle(boxTransform(center, R, imBox), rx, boxTransform_dx(thickness, R, imBox), color, fillcolor, aa, blend, min_tick);
					else draw_thick_filled_circle(boxTransform(center, R, imBox), rx, thickness, color, fillcolor, aa, blend, min_tick);
				}
				else
				{
					if (relativethickness) draw_thick_filled_ellipse(boxTransform(center, R, imBox), rx, ry, boxTransform_dx(thickness, R, imBox), boxTransform_dy(thickness, R, imBox), color, fillcolor, aa, blend, min_tick);
					else draw_thick_filled_ellipse(boxTransform(center, R, imBox), rx, ry, thickness, thickness, color, fillcolor, aa, blend, min_tick);
				}
			}


			/**
			* Draw part of a thick filled circle.
			*
			* @param	R					the absolute range represented in the image.
			* @param	part				combination of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT
			* @param	center				center
			* @param	radius				radius
			* @param	thickness			thickness.
			* @param    relativethickness	true to scale tickness with range and false to use constant thcikness.
			* @param	color				color
			* @param	fillcolor			color to fill the circle.
			* @param	aa					(Optional) true to use antialiasing.
			* @param	blend				(Optional) true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_part_thick_filled_circle(const fBox2 & R, int part, fVec2 center, double radius, double thickness, bool relativethickness, RGBc color, RGBc fillcolor, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, double min_tick = DEFAULT_MIN_THICKNESS)
			{
				const double EPS = 0.1;
				if (isEmpty()) return;
				_reverseVerticalCirclePart(part);
				const fBox2 imBox(-0.5, lx() - 0.5, -0.5, ly() - 0.5);
				const double rx = boxTransform_dx(radius, R, imBox);
				const double ry = boxTransform_dy(radius, R, imBox);
				if (std::abs<double>(rx - ry) < EPS)
					{
					if (relativethickness) draw_part_thick_filled_circle(part, boxTransform(center, R, imBox), rx, boxTransform_dx(thickness, R, imBox), color, fillcolor, aa, blend, min_tick);
					else draw_part_thick_filled_circle(part, boxTransform(center, R, imBox), rx, thickness, color, fillcolor, aa, blend, min_tick);
					}
				else
					{
					if (relativethickness) draw_part_thick_filled_ellipse(part, boxTransform(center, R, imBox), rx, ry, boxTransform_dx(thickness, R, imBox), boxTransform_dy(thickness, R, imBox), color, fillcolor, aa, blend, min_tick);
					else draw_part_thick_filled_ellipse(part, boxTransform(center, R, imBox), rx, ry, thickness, thickness, color, fillcolor, aa, blend, min_tick);
					}
			}


			/**
			* Draw an ellipse.
			*
			* @param	R	   the absolute range represented in the image.
			* @param	center center
			* @param	rx	   radius along the x-axis.
			* @param	ry	   radius along the y-axis.
			* @param	color  color
			* @param	aa	   (Optional) true to use antialiasing.
			* @param	blend  (Optional) true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_ellipse(const fBox2 & R, fVec2 center, double rx, double ry, RGBc color, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND)
				{
				if (isEmpty()) return;
				const fBox2 imBox(-0.5, lx() - 0.5, -0.5, ly() - 0.5);
				draw_ellipse(boxTransform(center, R, imBox), boxTransform_dx(rx, R, imBox), boxTransform_dy(ry, R, imBox), color, aa, blend);
				}


			/**
			* Draw part of an ellipse.
			*
			* @param	R	   the absolute range represented in the image.
			* @param	part   combination of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT
			* @param	center center
			* @param	rx	   radius along the x-axis.
			* @param	ry	   radius along the y-axis.
			* @param	color  color
			* @param	aa	   (Optional) true to use antialiasing.
			* @param	blend  (Optional) true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_part_ellipse(const fBox2 & R, int part, fVec2 center, double rx, double ry, RGBc color, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND)
				{
				if (isEmpty()) return;
				_reverseVerticalCirclePart(part);
				const fBox2 imBox(-0.5, lx() - 0.5, -0.5, ly() - 0.5);
				draw_part_ellipse(part, boxTransform(center, R, imBox), boxTransform_dx(rx, R, imBox), boxTransform_dy(ry, R, imBox), color, aa, blend);
				}


			/**
			* Draw a filled ellipse.
			*
			* @param	R			the absolute range represented in the image.
			* @param	center		center
			* @param	rx			radius along the x-axis.
			* @param	ry			radius along the y-axis.
			* @param	color		color
			* @param	fillcolor	color to fill the ellipse.
			* @param	aa			(Optional) true to use antialiasing.
			* @param	blend		(Optional) true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_filled_ellipse(const fBox2 & R, fVec2 center, double rx, double ry, RGBc color, RGBc fillcolor, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND)
				{
				if (isEmpty()) return;
				const fBox2 imBox(-0.5, lx() - 0.5, -0.5, ly() - 0.5);
				draw_filled_ellipse(boxTransform(center, R, imBox), boxTransform_dx(rx, R, imBox), boxTransform_dy(ry, R, imBox), color, fillcolor, aa, blend);
				}


			/**
			* Draw part of a filled ellipse.
			*
			* @param	R			the absolute range represented in the image.
			* @param	part		combination of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT
			* @param	center		center
			* @param	rx			radius along the x-axis.
			* @param	ry			radius along the y-axis.
			* @param	color		color
			* @param	fillcolor	color to fill the ellipse.
			* @param	aa			(Optional) true to use antialiasing.
			* @param	blend		(Optional) true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_part_filled_ellipse(const fBox2 & R, int part, fVec2 center, double rx, double ry, RGBc color, RGBc fillcolor, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND)
				{
				if (isEmpty()) return;
				_reverseVerticalCirclePart(part);
				const fBox2 imBox(-0.5, lx() - 0.5, -0.5, ly() - 0.5);
				draw_part_filled_ellipse(part,boxTransform(center, R, imBox), boxTransform_dx(rx, R, imBox), boxTransform_dy(ry, R, imBox), color, fillcolor, aa, blend);
				}


			/**
			* Draw a thick ellipse.
			*
			* @param	R					the absolute range represented in the image.
			* @param	center				center
			* @param	rx					radius along the x-axis.
			* @param	ry					radius along the y-axis.
			* @param	thickness_x			thickness on the x-axis.
			* @param	thickness_y			thickness on the y-axis.
			* @param    relativethickness	true to scale tickness with range and false to use constant thcikness.
			* @param	color				color
			* @param	aa					(Optional) true to use antialiasing.
			* @param	blend				(Optional) true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_thick_ellipse(const fBox2 & R, fVec2 center, double rx, double ry, double thickness_x, double thickness_y, bool relativethickness, RGBc color, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, double min_tick = DEFAULT_MIN_THICKNESS)
				{
				if (isEmpty()) return;
				const fBox2 imBox(-0.5, lx() - 0.5, -0.5, ly() - 0.5);
				const double frx = boxTransform_dx(rx, R, imBox);
				const double fry = boxTransform_dy(ry, R, imBox);
				if (relativethickness) draw_thick_ellipse(boxTransform(center, R, imBox), frx, fry, boxTransform_dx(thickness_x, R, imBox), boxTransform_dy(thickness_y, R, imBox), color, aa, blend, min_tick);
				else draw_thick_ellipse(boxTransform(center, R, imBox), frx, fry, thickness_x, thickness_y, color, aa, blend, min_tick);
				}


			/**
			* Draw part of a thick ellipse.
			*
			* @param	R					the absolute range represented in the image.
			* @param	part				combination of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT
			* @param	center				center
			* @param	rx					radius along the x-axis.
			* @param	ry					radius along the y-axis.
			* @param	thickness_x			thickness on the x-axis.
			* @param	thickness_y			thickness on the y-axis.
			* @param    relativethickness	true to scale tickness with range and false to use constant thcikness.
			* @param	color				color
			* @param	aa					(Optional) true to use antialiasing.
			* @param	blend				(Optional) true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_part_thick_ellipse(const fBox2 & R, int part, fVec2 center, double rx, double ry, double thickness_x, double thickness_y, bool relativethickness, RGBc color, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, double min_tick = DEFAULT_MIN_THICKNESS)
				{
				if (isEmpty()) return;
				_reverseVerticalCirclePart(part);
				const fBox2 imBox(-0.5, lx() - 0.5, -0.5, ly() - 0.5);
				const double frx = boxTransform_dx(rx, R, imBox);
				const double fry = boxTransform_dy(ry, R, imBox);
				if (relativethickness) draw_part_thick_ellipse(part, boxTransform(center, R, imBox), frx, fry, boxTransform_dx(thickness_x, R, imBox), boxTransform_dy(thickness_y, R, imBox), color, aa, blend, min_tick);
				else draw_part_thick_ellipse(part, boxTransform(center, R, imBox), frx, fry, thickness_x, thickness_y, color, aa, blend, min_tick);
				}


			/**
			* Draw a thick filled ellipse.
			*
			* @param	R					the absolute range represented in the image.
			* @param	center				center
			* @param	rx					radius along the x-axis.
			* @param	ry					radius along the y-axis.
			* @param	thickness_x			thickness on the x-axis.
			* @param	thickness_y			thickness on the y-axis.
			* @param    relativethickness	true to scale tickness with range and false to use constant thcikness.
			* @param	color				color
			* @param	fillcolor			color to fill the ellipse.
			* @param	aa					(Optional) true to use antialiasing.
			* @param	blend				(Optional) true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_thick_filled_ellipse(const fBox2 & R, fVec2 center, double rx, double ry, double thickness_x, double thickness_y, bool relativethickness, RGBc color, RGBc fillcolor, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, double min_tick = DEFAULT_MIN_THICKNESS)
				{
				if (isEmpty()) return;
				const fBox2 imBox(-0.5, lx() - 0.5, -0.5, ly() - 0.5);
				const double frx = boxTransform_dx(rx, R, imBox);
				const double fry = boxTransform_dy(ry, R, imBox);
				if (relativethickness) draw_thick_filled_ellipse(boxTransform(center, R, imBox), frx, fry, boxTransform_dx(thickness_x, R, imBox), boxTransform_dy(thickness_y, R, imBox), color, fillcolor, aa, blend, min_tick);
				else draw_thick_filled_ellipse(boxTransform(center, R, imBox), frx, fry, thickness_x, thickness_y, color, fillcolor, aa, blend, min_tick);
				}


			/**
			* Draw a part of thick filled ellipse.
			*
			* @param	R					the absolute range represented in the image.
			* @param	part				combination of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT
			* @param	center				center
			* @param	rx					radius along the x-axis.
			* @param	ry					radius along the y-axis.
			* @param	thickness_x			thickness on the x-axis.
			* @param	thickness_y			thickness on the y-axis.
			* @param    relativethickness	true to scale tickness with range and false to use constant thcikness.
			* @param	color				color
			* @param	fillcolor			color to fill the ellipse.
			* @param	aa					(Optional) true to use antialiasing.
			* @param	blend				(Optional) true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_part_thick_filled_ellipse(const fBox2 & R, int part, fVec2 center, double rx, double ry, double thickness_x, double thickness_y, bool relativethickness, RGBc color, RGBc fillcolor, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, double min_tick = DEFAULT_MIN_THICKNESS)
				{
				if (isEmpty()) return;
				_reverseVerticalCirclePart(part);
				const fBox2 imBox(-0.5, lx() - 0.5, -0.5, ly() - 0.5);
				const double frx = boxTransform_dx(rx, R, imBox);
				const double fry = boxTransform_dy(ry, R, imBox);
				if (relativethickness) draw_part_thick_filled_ellipse(part, boxTransform(center, R, imBox), frx, fry, boxTransform_dx(thickness_x, R, imBox), boxTransform_dy(thickness_y, R, imBox), color, fillcolor, aa, blend, min_tick);
				else draw_part_thick_filled_ellipse(part, boxTransform(center, R, imBox), frx, fry, thickness_x, thickness_y, color, fillcolor, aa, blend, min_tick);
				}


			/**
			* Draw an ellipse with a given bounding box.
			*
			* @param	R			the absolute range represented in the image.
			* @param	ellipseBox	the ellipse bounding box.
			* @param	color		color
			* @param	aa			(Optional) true to use antialiasing.
			* @param	blend		(Optional) true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_ellipse_in_box(const fBox2 & R, const fBox2 & ellipseBox, RGBc color, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND)
				{
				if (isEmpty()) return;
				const fBox2 imBox(-0.5, lx() - 0.5, -0.5, ly() - 0.5);
				draw_ellipse_in_box(boxTransform(ellipseBox, R, imBox), color, aa, blend);
				}


			/**
			* Draw part of an ellipse with a given bounding box.
			*
			* @param	R			the absolute range represented in the image.
			* @param	part		combination of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT
			* @param	ellipseBox	the ellipse bounding box.
			* @param	color		color
			* @param	aa			(Optional) true to use antialiasing.
			* @param	blend		(Optional) true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_part_ellipse_in_box(const fBox2 & R, int part, const fBox2 & ellipseBox, RGBc color, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND)
				{
				if (isEmpty()) return;
				_reverseVerticalCirclePart(part);
				const fBox2 imBox(-0.5, lx() - 0.5, -0.5, ly() - 0.5);
				draw_part_ellipse_in_box(part,boxTransform(ellipseBox, R, imBox), color, aa, blend);
				}


			/**
			* Draw a filled ellipse with a given bounding box.
			*
			* @param	R			the absolute range represented in the image.
			* @param	ellipseBox	the ellipse bounding box.
			* @param	color		color
			* @param	fillcolor	color to fill the ellipse.
			* @param	aa			(Optional) true to use antialiasing.
			* @param	blend		(Optional) true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_filled_ellipse_in_box(const fBox2 & R, const fBox2 & ellipseBox, RGBc color, RGBc fillcolor, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND)
				{
				if (isEmpty()) return;
				const fBox2 imBox(-0.5, lx() - 0.5, -0.5, ly() - 0.5);
				draw_filled_ellipse_in_box(boxTransform(ellipseBox, R, imBox), color, fillcolor, aa, blend);
				}


			/**
			* Draw part of a filled ellipse with a given bounding box.
			*
			* @param	R			the absolute range represented in the image.
			* @param	part		combination of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT
			* @param	ellipseBox	the ellipse bounding box.
			* @param	color		color
			* @param	fillcolor	color to fill the ellipse.
			* @param	aa			(Optional) true to use antialiasing.
			* @param	blend		(Optional) true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_part_filled_ellipse_in_box(const fBox2 & R, int part, const fBox2 & ellipseBox, RGBc color, RGBc fillcolor, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND)
				{
				if (isEmpty()) return;
				_reverseVerticalCirclePart(part);
				const fBox2 imBox(-0.5, lx() - 0.5, -0.5, ly() - 0.5);
				draw_part_filled_ellipse_in_box(part, boxTransform(ellipseBox, R, imBox), color, fillcolor, aa, blend);
				}


			/**
			* Draw a thick ellipse with a given bounding box.
			*
			* @param	R					the absolute range represented in the image.
			* @param	ellipseBox			the ellipse bounding box.
			* @param	thickness_x			thickness on the x-axis.
			* @param	thickness_y			thickness on the y-axis.
			* @param    relativethickness	true to scale tickness with range and false to use constant thcikness.
			* @param	color				color
			* @param	aa					(Optional) true to use antialiasing.
			* @param	blend				(Optional) true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_thick_ellipse_in_box(const fBox2 & R, const fBox2 & ellipseBox, double thickness_x, double thickness_y, bool relativethickness, RGBc color, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, double min_tick = DEFAULT_MIN_THICKNESS)
				{
				if (isEmpty()) return;
				const fBox2 imBox(-0.5, lx() - 0.5, -0.5, ly() - 0.5);
				const fBox2 B = boxTransform(ellipseBox, R, imBox);
				if (relativethickness) draw_thick_ellipse_in_box(B, boxTransform_dx(thickness_x, R, imBox), boxTransform_dy(thickness_y, R, imBox), color, aa, blend, min_tick);
				else draw_thick_ellipse_in_box(B, thickness_x, thickness_y, color, aa, blend, min_tick);
				}


			/**
			* Draw part of a thick ellipse with a given bounding box.
			*
			* @param	R					the absolute range represented in the image.
			* @param	part				combination of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT
			* @param	ellipseBox			the ellipse bounding box.
			* @param	thickness_x			thickness on the x-axis.
			* @param	thickness_y			thickness on the y-axis.
			* @param    relativethickness	true to scale tickness with range and false to use constant thcikness.
			* @param	color				color
			* @param	aa					(Optional) true to use antialiasing.
			* @param	blend				(Optional) true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_part_thick_ellipse_in_box(const fBox2 & R, int part, const fBox2 & ellipseBox, double thickness_x, double thickness_y, bool relativethickness, RGBc color, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, double min_tick = DEFAULT_MIN_THICKNESS)
				{
				if (isEmpty()) return;
				_reverseVerticalCirclePart(part);
				const fBox2 imBox(-0.5, lx() - 0.5, -0.5, ly() - 0.5);
				const fBox2 B = boxTransform(ellipseBox, R, imBox);
				if (relativethickness) draw_part_thick_ellipse_in_box(part, B, boxTransform_dx(thickness_x, R, imBox), boxTransform_dy(thickness_y, R, imBox), color, aa, blend, min_tick);
				else draw_part_thick_ellipse_in_box(part, B, thickness_x, thickness_y, color, aa, blend, min_tick);
				}


			/**
			* Draw a thick filled ellipse with a given bounding box.
			*
			* @param	R					the absolute range represented in the image.
			* @param	ellipseBox			the ellipse bounding box.
			* @param	thickness_x			thickness on the x-axis.
			* @param	thickness_y			thickness on the y-axis.
			* @param    relativethickness	true to scale tickness with range and false to use constant thcikness.
			* @param	color				color
			* @param	fillcolor			color to fill the ellipse.
			* @param	aa					(Optional) true to use antialiasing.
			* @param	blend				(Optional) true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_thick_filled_ellipse_in_box(const fBox2 & R, const fBox2 & ellipseBox, double thickness_x, double thickness_y, bool relativethickness, RGBc color, RGBc fillcolor, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, double min_tick = DEFAULT_MIN_THICKNESS)
			{
				if (isEmpty()) return;
				const fBox2 imBox(-0.5, lx() - 0.5, -0.5, ly() - 0.5);
				const fBox2 B = boxTransform(ellipseBox, R, imBox);
				if (relativethickness) draw_thick_filled_ellipse_in_box(B, boxTransform_dx(thickness_x, R, imBox), boxTransform_dy(thickness_y, R, imBox), color, fillcolor, aa, blend, min_tick);
				else draw_thick_filled_ellipse_in_box(B, thickness_x, thickness_y, color, fillcolor, aa, blend, min_tick);
			}


			/**
			* Draw part of a thick filled ellipse with a given bounding box.
			*
			* @param	R					the absolute range represented in the image.
			* @param	part				combination of BOX_SPLIT_UP, BOX_SPLIT_DOWN, BOX_SPLIT_LEFT, BOX_SPLIT_RIGHT, BOX_SPLIT_UP_LEFT, BOX_SPLIT_UP_RIGHT, BOX_SPLIT_DOWN_LEFT, BOX_SPLIT_DOWN_RIGHT
			* @param	ellipseBox			the ellipse bounding box.
			* @param	thickness_x			thickness on the x-axis.
			* @param	thickness_y			thickness on the y-axis.
			* @param    relativethickness	true to scale tickness with range and false to use constant thcikness.
			* @param	color				color
			* @param	fillcolor			color to fill the ellipse.
			* @param	aa					(Optional) true to use antialiasing.
			* @param	blend				(Optional) true to use blending.
			**/
			MTOOLS_FORCEINLINE void canvas_draw_part_thick_filled_ellipse_in_box(const fBox2 & R, int part, const fBox2 & ellipseBox, double thickness_x, double thickness_y, bool relativethickness, RGBc color, RGBc fillcolor, bool aa = DEFAULT_AA, bool blend = DEFAULT_BLEND, double min_tick = DEFAULT_MIN_THICKNESS)
			{
				if (isEmpty()) return;
				_reverseVerticalCirclePart(part);
				const fBox2 imBox(-0.5, lx() - 0.5, -0.5, ly() - 0.5);
				const fBox2 B = boxTransform(ellipseBox, R, imBox);
				if (relativethickness) draw_part_thick_filled_ellipse_in_box(part, B, boxTransform_dx(thickness_x, R, imBox), boxTransform_dy(thickness_y, R, imBox), color, fillcolor, aa, blend, min_tick);
				else draw_part_thick_filled_ellipse_in_box(part, B, thickness_x, thickness_y, color, fillcolor, aa, blend, min_tick);
			}


			/**
			* Draw the integer grid (ie line of the form (x,j) and (i,y)) where (i,j) are integers.
			*
			* @param   R           The rect representing the range of the image.
			* @param   color       The color.
			**/
			inline void canvas_draw_grid(const mtools::fBox2 & R, mtools::RGBc color = mtools::RGBc::c_Gray, float opacity = 0.5f)
				{
				if (isEmpty()) return;
				color.multOpacity(opacity);
				const double ex  = R.max[0] - R.min[0];
				const double xmin = R.min[0] - ex;
				const double xmax = R.max[0] + ex;
				const double ey = R.max[1] - R.min[1];
				const double ymin = R.min[1] - ey;
				const double ymax = R.max[1] + ey;
				if (R.lx() <= _lx / 2) 
					{ 
					mtools::int64 i = (mtools::int64)R.min[0] - 2; 
					while (i++ < (mtools::int64)R.max[0] + 2) { canvas_draw_vertical_line(R, (double)i,ymin,ymax, color,true,true); } 
					}
				if (R.ly() <= _ly / 2) 
					{ 
					mtools::int64 j = (mtools::int64)R.min[1] - 2; 
					while (j++ < (mtools::int64)R.max[1] + 2) { canvas_draw_horizontal_line(R, (double)j, xmin,xmax, color, true,true); } 
					};
				}


			/**
			* Draw the cells around integer points ie draw line of the form (x,j+1/2) and (i+1/2,y)
			*
			* @param   R           The rect representing the range of the image.
			* @param   color       The color.
			**/
			inline void canvas_draw_cells(const mtools::fBox2 & R, mtools::RGBc color = mtools::RGBc::c_Gray, float opacity = 0.5f)
				{
				if (isEmpty()) return;
				color.multOpacity(opacity);
				const double ex = R.max[0] - R.min[0];
				const double xmin = R.min[0] - ex;
				const double xmax = R.max[0] + ex;
				const double ey = R.max[1] - R.min[1];
				const double ymin = R.min[1] - ey;
				const double ymax = R.max[1] + ey;
				if (R.lx() <= _lx / 2)
					{
					mtools::int64 i = (mtools::int64)R.min[0] - 2;
					while (i++ < (mtools::int64)R.max[0] + 2) { canvas_draw_vertical_line(R, i - 0.5, ymin, ymax, color, true, true); }
					}
				if (R.ly() <= _ly / 2)
					{
					mtools::int64 j = (mtools::int64)R.min[1] - 2;
					while (j++ < (mtools::int64)R.max[1] + 2) { canvas_draw_horizontal_line(R, j - 0.5, xmin, xmax, color, true, true); }
					}
				}


			/**
			* Draw the axes on an image.
			*
			* @param   R           The rect representing the range of the image.
			* @param   color       The color.
			**/
			inline void canvas_draw_axes(const mtools::fBox2 & R, float scaling = 1.0f, mtools::RGBc color = RGBc::c_Black, float opacity = 1.0f)
				{
				color.multOpacity(opacity);
				scaling = scaling * ((float)(std::sqrt(_lx*_ly) / 1000.0));
				double tick = (scaling < 4.0f) ? 0.0f : ((scaling - 1) / 8);
				tick = 2 * tick + 1;
				const double ex = R.max[0] - R.min[0];
				const double ey = R.max[1] - R.min[1];
				canvas_draw_thick_horizontal_line(R, 0, R.min[0] - ex, R.max[0] + ex, tick, false, color, true, true);
				canvas_draw_thick_vertical_line(R, 0, R.min[1] - ey, R.max[1] + ey, tick, false, color, true, true);
				}


			/**
			* Add the graduations on the axis.
			*
			* @param   R       The rect representing the range of the image.
			* @param   scaling The scaling factor for the graduation size (default 1.0). Note that there is
			*                  already a scaling of the graduation w.r.t. the image size. This parameter
			*                  enables to multiply the automatic scaling with a new factor.
			* @param   color   The color.
			**/
			void canvas_draw_graduations(const mtools::fBox2 & R, float scaling = 1.0f, mtools::RGBc color = mtools::RGBc::c_Black, float opacity = 1.0f)
				{
				color.multOpacity(opacity);
				scaling = scaling*((float)(std::sqrt(_lx*_ly) / 1000.0));
				double tick = (scaling < 4.0f) ? 0.0f : ((scaling - 1) / 8);
				tick = 2 * tick + 1;
				int64 gradsize = 1 + (int64)(3 * scaling);
				const int64 winx = _lx, winy = _ly;
				int64 py = winy - 1 - (int64)ceil(((-R.min[1]) / (R.max[1] - R.min[1]))*winy - ((double)1.0 / 2.0));
				int64 px = (int64)ceil(((-R.min[0]) / (R.max[0] - R.min[0]))*winx - ((double)1.0 / 2.0));
				if ((px > -1) && (px < winx))
					{
					int64 l, zz; double k, xx, kk, pp, xx2, op, v1, v2;
					op = ::log10(R.ly()); 
					if (op<0) { l = ((int64)(op)) - 1; } else { l = ((int64)(op)); }
					k = ::pow(10.0, (double)(l));
					v1 = floor(R.min[1] / k); v1 = v1 - 1; v2 = floor(R.max[1] / k);
					v2 = v2 + 1;
					kk = k; pp = kk / 5;
					if ((v2 - v1) < 5) { kk = k / 2; pp = kk / 5; } else { if ((v2 - v1) > 8) { kk = k * 2; pp = kk / 2; v1 = ((v1 / 2) * 2) - 2; } }
					xx = k*v1; xx2 = k*v1;
					while (xx2 <= (R.max[1] + 2 * k))
						{
						xx = xx + kk; xx2 = xx2 + pp;
						zz = (int64)R.absToPixel(mtools::fVec2(0, xx), mtools::iVec2(winx, winy)).Y();
						if ((zz >= -10) && (zz < winy + 10)) { if (xx != 0) { draw_horizontal_line(zz, px - 2 * gradsize, px + 2 * gradsize, tick, color, true, true); } }
						zz = (int64)R.absToPixel(mtools::fVec2(0, xx2), mtools::iVec2(winx, winy)).Y();
						if ((zz > -2) && (zz < winy + 1)) { if (xx2 != 0) { draw_horizontal_line(zz, px - gradsize, px + gradsize, tick, color, true, true); } }
						}
					}
				if ((py > -1) && (py < winy))
					{
					int64 l, zz; double k, xx, kk, pp, xx2, op, v1, v2;
					op = ::log10(R.lx()); 
					if (op<0) { l = ((int64)op) - 1; } else { l = (int64)op; }
					k = ::pow(10.0, (double)(l));
					v1 = floor(R.min[0] / k);  v1 = v1 - 1; v2 = floor(R.max[0] / k);  v2 = v2 + 1;
					kk = k; pp = kk / 5;
					if ((v2 - v1) < 5) { kk = k / 2; pp = kk / 5; } else { if ((v2 - v1) > 8) { kk = k * 2; pp = kk / 2; v1 = ((v1 / 2) * 2) - 2; } }
					xx = k*v1; xx2 = k*v1;
					while (xx2 <= (R.max[0] + 2 * k))
						{
						xx = xx + kk; xx2 = xx2 + pp;
						zz = (int64)R.absToPixel(mtools::fVec2(xx, 0), mtools::iVec2(winx, winy)).X();
						if ((zz >= -30) && (zz < winx + 30)) { if (xx != 0) { draw_vertical_line(zz, py - 2 * gradsize, py + 2 * gradsize, tick, color, true, true);} }
						zz = (int64)R.absToPixel(mtools::fVec2(xx2, 0), mtools::iVec2(winx, winy)).X();
						if ((zz > -2) && (zz < winx + 1)) { if (xx2 != 0) { draw_vertical_line(zz, py - gradsize, py + gradsize, tick, color, true, true);} }
						}
					}
				}


			/**
			* Draw the numbering on the axis.
			*
			* @param   R       The rect representing the range of the image.
			* @param   scaling The scaling factor for the numbers. (default 1.0). Note that there is already
			*                  a scaling of the font w.r.t. the image size. This parameter enables to
			*                  multiply this automatic scaling with a new factor.
			* @param   color   The color.
			* @param   opacity The opacity.
			*
			* @return  the image for chaining.
			**/
			void canvas_draw_numbers(const mtools::fBox2 & R, float scaling = 1.0, mtools::RGBc color = mtools::RGBc::c_Black, float opacity = 1.0f);



			/******************************************************************************************************************************************************
			*******************************************************************************************************************************************************
			*******************************************************************************************************************************************************
			*******************************************************************************************************************************************************
			*******************************************************************************************************************************************************
			*																				   																      *
			*                                                                 PRIVATE METHODS                                                                     *
			*																																					  *
			*******************************************************************************************************************************************************
			*******************************************************************************************************************************************************
			*******************************************************************************************************************************************************
			*******************************************************************************************************************************************************
			*******************************************************************************************************************************************************/


//		private:



			/******************************************************************************************************************************************************
			*																				   																      *
			*                                                                    RESCALING                                                                        *
			*																																					  *
			*******************************************************************************************************************************************************/


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

			/* dummy (default) functors */
			struct _dummy_read_functor { RGBc operator()(uint64 x, uint64 y) { return  RGBc::c_Black; } };
			struct _dummy_write_functor { void operator()(uint64 x, uint64 y, RGBc color) { return; } };


			/* call _boxaverage_downscaling_FP32 with the correct template parameters for BIT_FP and BIT_DIV */
			template<uint64 BIT_FP_REDUCE, bool USE_FUNCION_CALL = false, typename READ_FUNCTOR = _dummy_read_functor, typename WRITE_FUNCTOR = _dummy_write_functor>
			inline static void _boxaverage_downscaling2(RGBc * dest_data, uint64 dest_stride, uint64 dest_sx, uint64 dest_sy, RGBc * src_data, uint64 src_stride, uint64 src_sx, uint64 src_sy, READ_FUNCTOR funread = _dummy_read_functor(), WRITE_FUNCTOR funwrite = _dummy_write_functor())
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


			/* Downscaling using box average algorithm. 
			   Optimized for SSE 4.2 (todo: optimize for AVX2). SSE is enabled if MTOOLS_USE_SSE is non zero.
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
					size_t tmpsize = (size_t)(16 * (dest_sx + 1));
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
							// only VS specific code for the moment. TODO : make this portable...
							#if (MTOOLS_USE_SSE) && defined _MSC_VER
								{							
								__m128i * sse_tmp = reinterpret_cast<__m128i*>(tmp + off);
								__m128i v = _mm_set_epi32((coul >> 24) & 0xFF, (coul >> 16) & 0xFF, (coul >> 8) & 0xFF, coul & 0xFF);
								__m128i a1 = _mm_set1_epi32(aera1);
								_mm_store_si128(sse_tmp, _mm_add_epi32(_mm_load_si128(sse_tmp), _mm_mullo_epi32(a1, v)));
								__m128i a2 = _mm_set1_epi32(aera2);
								_mm_store_si128(sse_tmp + 1, _mm_add_epi32(_mm_load_si128(sse_tmp + 1), _mm_mullo_epi32(a2, v)));
								}
							#else					
								{
								tmp[off] += aera1*(coul & 0xFF);
								tmp[off + 1] += aera1*((coul >> 8) & 0xFF);
								tmp[off + 2] += aera1*((coul >> 16) & 0xFF);
								tmp[off + 3] += aera1*((coul >> 24) & 0xFF);
								tmp[off + 4] += aera2*(coul & 0xFF);
								tmp[off + 5] += aera2*((coul >> 8) & 0xFF);
								tmp[off + 6] += aera2*((coul >> 16) & 0xFF);
								tmp[off + 7] += aera2*((coul >> 24) & 0xFF);
								}
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
							memset(tmp, 0, (size_t)((dest_sx + 1) * 16)); // clear the temporary buffer							
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
								// only VS specific code for the moment. TODO : make this portable...
								#if (MTOOLS_USE_SSE) && defined _MSC_VER
									{
									__m128i * sse_tmp = reinterpret_cast<__m128i*>(tmp + off);
									__m128i v = _mm_set_epi32((coul >> 24) & 0xFF, (coul >> 16) & 0xFF, (coul >> 8) & 0xFF, coul & 0xFF);
									__m128i a1 = _mm_set1_epi32(aera1);
									_mm_store_si128(sse_tmp, _mm_add_epi32(_mm_load_si128(sse_tmp), _mm_mullo_epi32(a1, v)));
									__m128i a2 = _mm_set1_epi32(aera2);
									_mm_store_si128(sse_tmp + 1, _mm_add_epi32(_mm_load_si128(sse_tmp + 1), _mm_mullo_epi32(a2, v)));
									}
								#else
									{
									tmp[off] += aera1*(coul & 0xFF);
									tmp[off + 1] += aera1*((coul >> 8) & 0xFF);
									tmp[off + 2] += aera1*((coul >> 16) & 0xFF);
									tmp[off + 3] += aera1*((coul >> 24) & 0xFF);
									tmp[off + 4] += aera2*(coul & 0xFF);
									tmp[off + 5] += aera2*((coul >> 8) & 0xFF);
									tmp[off + 6] += aera2*((coul >> 16) & 0xFF);
									tmp[off + 7] += aera2*((coul >> 24) & 0xFF);
									}
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
				



			/******************************************************************************************************************************************************
			*																				   																      *
			*                                                          BLITTING / BLENDING / MASKING                                                              *
			*																																					  *
			*******************************************************************************************************************************************************/


			/* fast blitting of a region, do not work for overlap */
			MTOOLS_FORCEINLINE static void _blitRegion(RGBc * pdest, int64 dest_stride, RGBc * psrc, int64 src_stride, int64 sx, int64 sy)
				{
				if (sx < 20) 
					{ // for small width, faster to copy by element by element than using memcpy
					_blitRegionUp(pdest, dest_stride, psrc, src_stride, sx, sy);
					return;
					}
				// memcpy for each line
				for (int64 j = 0; j < sy; j++)
					{
					memcpy(pdest + j*dest_stride, psrc + j*src_stride, (size_t)(4 * sx));
					}
				}


			/* blit a region, in increasing order */
			MTOOLS_FORCEINLINE static void _blitRegionUp(RGBc * pdest, int64 dest_stride, RGBc * psrc, int64 src_stride, int64 sx, int64 sy)
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
			MTOOLS_FORCEINLINE static void _blitRegionDown(RGBc * pdest, int64 dest_stride, RGBc * psrc, int64 src_stride, int64 sx, int64 sy)
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
			MTOOLS_FORCEINLINE static void _blendRegionUp(RGBc * pdest, int64 dest_stride, RGBc * psrc, int64 src_stride, int64 sx, int64 sy, float op)
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
			MTOOLS_FORCEINLINE static void _blendRegionDown(RGBc * pdest, int64 dest_stride, RGBc * psrc, int64 src_stride, int64 sx, int64 sy, float op)
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
			MTOOLS_FORCEINLINE static void _maskRegion(RGBc * pdest, int64 dest_stride, RGBc * psrc, int64 src_stride, int64 sx, int64 sy, RGBc color)
				{
				for (int64 j = 0; j < sy; j++)
					{
					for (int64 i = 0; i < sx; i++)
						{
						pdest[i].blend(color, psrc[i].opacityInt());
						}
					pdest += dest_stride;
					psrc += src_stride;
					}
				return;
				}





			/******************************************************************************************************************************************************
			*																				   																      *
			*                                                                   LINE AND CURVE DRAWING                                                                      *
			*																																					  *
			*******************************************************************************************************************************************************/




			/** change the opacity to match with the pen width **/ 
			MTOOLS_FORCEINLINE void  _correctPenOpacity(RGBc & color, int32 penwidth)
				{
				if ((penwidth <= 0) || (color.comp.A <= 3) || (color.comp.A == 255)) return;
				float a =  1.0f - pow(1.0f - ((float)color.comp.A / 255.0f), 1.0f / (2*penwidth + 1.0f));				
				color.opacity(a);
				}



			/** update a pixel / pen dot 
			    fast: compiler optimizes away all template conditionnals **/
			template<bool BLEND, bool CHECKRANGE, bool USE_OP, bool USE_PEN> MTOOLS_FORCEINLINE void  _updatePixel(int64 x, int64 y, RGBc color, int32 op, int32 penwidth)
				{
				MTOOLS_ASSERT((!USE_PEN)||(penwidth >0));
				MTOOLS_ASSERT((!USE_OP) || ((op >= 0) && (op <= 256)));
				if (USE_PEN)
					{
					if (USE_OP) { color.multOpacityInt(op); }
					const int64 d = penwidth;
					if (CHECKRANGE)
						{
						const int64 xmin = (x < d) ? 0 : (x - d);
						const int64 xmax = (x >= _lx - d) ? (_lx - 1) : (x + d);
						const int64 ymin = (y < d) ? 0 : (y - d);
						const int64 ymax = (y >= _ly - d) ? (_ly - 1) : (y + d);
						const int64 sx = xmax - xmin;
						const int64 sy = ymax - ymin;
						RGBc * p = _data + ymin*_stride + xmin;
						for (int64 j = 0; j <= sy; j++)
							{
							for (int64 i = 0; i <= sx; i++)
								{
								if (BLEND) { p[i].blend(color); }
								else { p[i] = color; }
								}
							p += _stride;
							}
						return;
						}
					else
						{
						MTOOLS_ASSERT((x - d >= 0) && (x + d < _lx) && (y - d >= 0) && (y +d < _ly));
						const int64 L = (d << 1);
						RGBc * p = _data + (y - d)*_stride + x - d;
						for (int64 j = 0; j <= L; j++)
							{
							for (int64 i = 0; i <= L; i++)
								{
								if (BLEND) { p[i].blend(color); }
								else { p[i] = color; }
								}
							p += _stride;
							}
						return;
						}
					}
				else
					{
					if (CHECKRANGE)
						{
						if (USE_OP)
							{
							if (BLEND) { blendPixel(x, y, color, (uint32)op); return; } else { color.multOpacityInt(op); setPixel(x, y, color); return; }
							}
						else
							{
							if (BLEND) { blendPixel(x, y, color); return; } else { setPixel(x, y, color); return; }
							}
						}
					else
						{
						MTOOLS_ASSERT((x >= 0) && (x < _lx) && (y >= 0) && (y < _ly));
						if (USE_OP)
							{
							if (BLEND) { operator()(x, y).blend(color, (uint32)op); return; } else { color.multOpacityInt(op); operator()(x, y) = color; return; }
							}
						else
							{
							if (BLEND) { operator()(x, y).blend(color); return; } else { operator()(x, y) = color; return; }
							}
						}
					}
				}


			template<bool BLEND> MTOOLS_FORCEINLINE void  _updatePixel2(RGBc * p, RGBc color)
				{
				// compiler optimizes away the unused cases. 
				if (!BLEND) { *p = color; return; }
				if (BLEND) { (*p).blend(color); return; }
				}


			/** draw the line [x1,x2] x {y}, nothing if x2 < x1. **/
			template<bool blend, bool checkrange> MTOOLS_FORCEINLINE void _hline(int64 x1, int64 x2, int64 y, RGBc color)
				{ // compiler optimizes away the template conditional statements.
				MTOOLS_ASSERT((checkrange) || ((y >= 0) && (y < _ly))); // y range should always be ok. 
				MTOOLS_ASSERT((checkrange) || ((x1 >= 0) && (x2 < _lx)) || (x2 < x1));
				if (checkrange)
					{ // clamp
					x1 = std::max<int64>(0, x1);
					x2 = std::min<int64>(_lx - 1, x2);
					if ((y < 0) || (y >= _ly)) return;
					}
				RGBc * p = _data + y*_stride + x1;
				while (x1 <= x2)
					{
					if (blend) { (*p).blend(color); }
					else { *p = color; }
					p++; x1++;
					}
				}



			/** Used by CSLineClip() to compute the region where the point lies **/
			MTOOLS_FORCEINLINE static int _csLineClipCode(const iVec2 & P, const iBox2 & B)
				{
				int c = 0;
				const int64 xx = P.X();
				const int64 yy = P.Y();
				if (xx < B.min[0]) c |= 1;
				if (xx > B.max[0]) c |= 2;
				if (yy < B.min[1]) c |= 4;
				if (yy > B.max[1]) c |= 8;
				return c;
				}


			/**
			 * Cohen-Sutherland Line clipping algorithm.
			 *
			 * @param [in,out]	P1	The first point.
			 * @param [in,out]	P2	The second point.
			 * @param	B		  	The rectangle to clip into.
			 *
			 * @return	true if a line should be drawn and false if it should be discarded. If true, P1 and
			 * 			P2 are now inside the closed rectangle B and delimit the line to draw. 
			 **/
			MTOOLS_FORCEINLINE static bool _csLineClip(iVec2 & P1, iVec2 & P2, const iBox2 & B)
				{
				int c1 = _csLineClipCode(P1, B);
				int c2 = _csLineClipCode(P2, B);
				while (1)
					{
					if ((c1 == 0) && (c2 == 0)) { return true; } // both point inside		
					if ((c1 & c2) != 0) { return false; } //AND of both codes != 0.Line is outside. Reject line
					int temp = (c1 == 0) ? c2 : c1; //Decide if point1 is inside, if not, calculate intersection		
					{
					int64 x = 0, y = 0;
					const double m = ((double)(P2.Y() - P1.Y())) / (P2.X() - P1.X());
					if (temp & 8)
						{ //Line clips top edge
						x = P1.X() + (int64)round((B.max[1] - P1.Y()) / m);
						y = B.max[1];
						}
					else if (temp & 4)
						{ 	//Line clips bottom edge
						x = P1.X() + (int64)round((B.min[1] - P1.Y()) / m);
						y = B.min[1];
						}
					else if (temp & 1)
						{ 	//Line clips left edge
						x = B.min[0];
						y = P1.Y() + (int64)round(m*(B.min[0] - P1.X()));
						}
					else if (temp & 2)
						{ 	//Line clips right edge
						x = B.max[0];
						y = P1.Y() + (int64)round(m*(B.max[0] - P1.X()));
						}
					if (temp == c1) //Check which point we had selected earlier as temp, and replace its co-ordinates
						{
						P1.X() = x; P1.Y() = y;
						c1 = _csLineClipCode(P1, B);
						}
					else
						{
						P2.X() = x; P2.Y() = y;
						c2 = _csLineClipCode(P2, B);
						}
					}
					}
				}


			/* draw a vertical line */
			template<bool blend, bool checkrange> MTOOLS_FORCEINLINE void _verticalLine(int64 x, int64 y1, int64 y2, RGBc color, bool draw_P2)
				{
				if (y2 < y1) { if (!draw_P2) { y2++; } mtools::swap(y1, y2); } else { if (!draw_P2) { y2--; } }
				if (checkrange)
					{
					if ((x < 0) || (x >= _lx)) return;
					if ((y2 < 0) || (y1 >= _ly)) return;
					y1 = (y1 < 0) ? 0 : y1;
					y2 = (y2 >= _ly) ? (_ly - 1) : y2;
					}
				RGBc * p = _data + y1*_stride + x;
				int64 s = y2 - y1; 
				while (s >= 0) { _updatePixel2<blend>(p, color); p += _stride; s--; }
				}


			/* draw an horizontal line */
			template<bool blend, bool checkrange> MTOOLS_FORCEINLINE void _horizontalLine(int64 y, int64 x1, int64 x2, RGBc color, bool draw_P2)
				{
				if (x2 < x1) { if (!draw_P2) { x2++; }  mtools::swap(x1, x2); } else { if (!draw_P2) x2--; }
				if (checkrange)
					{
					if ((y < 0) || (y >= _ly)) return;
					if ((x2 < 0) || (x1 >= _lx)) return;
					x1 = (x1 < 0) ? 0 : x1;
					x2 = (x2 >= _lx) ? (_lx - 1) : x2;
					}
				RGBc * p = _data + y*_stride + x1;
				int64 s = x2 - x1;
				while (s >= 0) { _updatePixel2<blend>(p, color); p++; s--; }
				}


			/* draw a tick vertical line with aliasing */
			template<bool blend, bool checkrange> MTOOLS_FORCEINLINE void _tickVerticalLine(int64 x, int64 y1, int64 y2, RGBc color, bool draw_P2, double tickness, double min_tick = DEFAULT_MIN_THICKNESS)
				{
				if (tickness <= 0) return;
				if (tickness < min_tick) tickness = min_tick;
				double f = (tickness/2) + 0.5f;
				int64 d = (int64)f;
				if (d == 0) 
					{ 
					double op = color.opacity()*tickness;
					_verticalLine<blend, checkrange>(x, y1, y2, color.getOpacity((float)op), draw_P2);
					return; 
					}
				int64 xmin = x-d;
				int64 xmax = x+d;
				double r = f - d;
				RGBc c = color.getOpacity((float)(color.opacity()*r));
				if (!c.isTransparent())
					{
					_verticalLine<blend, checkrange>(xmin, y1, y2, c, draw_P2);
					_verticalLine<blend, checkrange>(xmax, y1, y2, c, draw_P2);
					}
				xmin++;
				while (xmin < xmax) { _verticalLine<blend, checkrange>(xmin, y1, y2, color, draw_P2); xmin++; }
				}


			/* draw a tick horizontal line with aliasing */
			template<bool blend, bool checkrange> MTOOLS_FORCEINLINE void _tickHorizontalLine(int64 y, int64 x1, int64 x2, RGBc color, bool draw_P2, double tickness, double min_tick = DEFAULT_MIN_THICKNESS)
				{
				if (tickness <= 0) return;
				if (tickness < min_tick) tickness = min_tick;
				double f = (tickness / 2) + 0.5f;
				int64 d = (int64)f;
				if (d == 0) 
					{ 
					double op = color.opacity()*tickness; 
					_horizontalLine<blend, checkrange>(y, x1, x2, color.getOpacity((float)op), draw_P2); 
					return; 
					}
				int64 ymin = y - d;
				int64 ymax = y + d;
				double r = f - d;
				RGBc c = color.getOpacity((float)(color.opacity()*r));
				if (!c.isTransparent())
					{
					_horizontalLine<blend, checkrange>(ymin, x1, x2, c, draw_P2);
					_horizontalLine<blend, checkrange>(ymax, x1, x2, c, draw_P2);
					}
				ymin++;
				while (ymin < ymax) { _horizontalLine<blend, checkrange>(ymin, x1, x2, color, draw_P2); ymin++; }
				}



			/* stucture holding info on  a bresenham line */
			struct _bdir
				{
				int64 dx, dy;			// step size in each direction
				int64 stepx, stepy;		// directions (+/-1)
				int64 rat;				// ratio max(dx,dy)/min(dx,dy) to speed up computations
				int64 amul;				// multiplication factor to compute aa values. 
				bool x_major;			// true if the line is xmajor (ie dx > dy) and false if y major (dy >= dx).
				};


			/* stucture holding a position on a bresenham line */
			struct _bpos
				{
				int64 x, y; // current pos
				int64 frac; // fractional part
				};

			
			/**
			* Construct the structure containing the info for a bresenham line 
			* and a position on the line.
			* The line goes from P1 to P2 and the position is set to P1. 
			* return the number of pixels in the half open segment [P1, P2[
			*/
			MTOOLS_FORCEINLINE int64 _init_line(const iVec2 P1, const iVec2 P2, _bdir & linedir, _bpos & linepos)
				{
				const int64 EXP = 10;
				if (P1 == P2)
					{ // default horizontal line
					MTOOLS_DEBUG("P1 = P2 : default horizontal line.");
					linedir.x_major = true;
					linedir.dx = 2;
					linedir.dy = 0;
					linedir.stepx = 1;
					linedir.stepy = 1; 
					linedir.rat = 0;
					linedir.amul = ((int64)1 << 60)/2;
					linepos.x = P1.X();
					linepos.y = P1.Y();
					linepos.frac = -2;
					return 0;
					}
				MTOOLS_ASSERT(P1 != P2);
				int64 dx = P2.X() - P1.X(); if (dx < 0) { dx = -dx;  linedir.stepx = -1; } else { linedir.stepx = 1; } dx <<= EXP;
				int64 dy = P2.Y() - P1.Y(); if (dy < 0) { dy = -dy;  linedir.stepy = -1; } else { linedir.stepy = 1; } dy <<= EXP;
				linedir.dx = dx;
				linedir.dy = dy;
				if (dx >= dy) 
					{ 
					linedir.x_major = true; 
					linedir.rat = (dy == 0) ? 0 : (dx/dy);  
					}
				else 
					{ 
					linedir.x_major = false; 
					linedir.rat = (dx == 0) ? 0 : (dy/dx); 
					}
				linepos.x = P1.X();
				linepos.y = P1.Y();
				int64 flagdir = (P2.X() > P1.X()) ? 1 : 0; // used to copensante frac so that line [P1,P2] = [P2,P1]. 
				linepos.frac = ((linedir.x_major) ? (dy - (dx >> 1)) : (dx - (dy >> 1))) - flagdir;		
				linedir.amul = ((int64)1 << 60) / (linedir.x_major ? linedir.dx : linedir.dy);			
				return ((linedir.x_major ? dx : dy) >> EXP);
				}


			/**
			* Construct the structure containg the info for a bresenham line
			* and a position on the line. Version for non_integer valued line.
			* 
			* put in P1,P2 the start and end pixels whch are obtained from fP1 and fP2 by rounding to the nearest integer.
			* 
			* Return the number of pixels in the half open segment [P1, P2[
			*/
			int64 _init_line(fVec2 Pf1, fVec2 Pf2, _bdir & linedir, _bpos & linepos, iVec2 & P1, iVec2 & P2)
				{
				const int64 PRECISION = 1024*16; // 512 * 128;
				bool sw = false;
				if ((Pf1.X() > Pf2.X()) || ((Pf1.X() == Pf2.X()) && (Pf1.Y() > Pf2.Y())))
					{
					sw = true;
					mtools::swap(Pf1, Pf2);
					}

				P1.X() = (int64)round(Pf1.X()); P1.Y() = (int64)round(Pf1.Y());
				P2.X() = (int64)round(Pf2.X()); P2.Y() = (int64)round(Pf2.Y());

				/* basic algo, 
				if (sw) { mtools::swap(P1, P2); }
				return _init_line(P1, P2, linedir, linepos);
				*/

				linepos.x = P1.X();
				linepos.y = P1.Y();
				const int64 adx = abs(P2.X() - P1.X());
				const int64 ady = abs(P2.Y() - P1.Y());
				const int64 len = (adx > ady) ? adx : ady;
				const double fdx = (Pf2.X() - Pf1.X());
				const double fdy = (Pf2.Y() - Pf1.Y());

				if (adx == ady)
					{ // edge cases
					if (sw) { mtools::swap(P1, P2); }
					return _init_line(P1, P2, linedir, linepos);
					}
				else
					{
					if (adx > ady)
						{ // x major
						linedir.x_major = true;
						const double mul = fdy / fdx;
						double f1 = mul*(P1.X() - Pf1.X()) + Pf1.Y() - P1.Y(); // how much above
						double f2 = mul*(P2.X() - Pf2.X()) + Pf2.Y() - P2.Y(); // how much below
						int64 if1 = (int64)((2 * PRECISION) * f1); if (if1 <= -PRECISION) { if1 = -PRECISION + 1; } else if (if1 >= PRECISION) { if1 = PRECISION - 1; }
						int64 if2 = (int64)((2 * PRECISION) * f2); if (if2 <= -PRECISION) { if2 = -PRECISION + 1; } else if (if2 >= PRECISION) { if2 = PRECISION - 1; }
						if (fdx < 0) { linedir.stepx = -1; } else { linedir.stepx = +1; }
						if (fdy < 0) { linedir.stepy = -1;  if1 = -if1; if2 = -if2; } else { linedir.stepy = +1; }
						linedir.dx = adx * (2 * PRECISION);
						linedir.dy = ady * (2 * PRECISION);
						linedir.dy += -if1 + if2;
						MTOOLS_ASSERT(linedir.dy >= 0);
						MTOOLS_ASSERT(linedir.dy <= linedir.dx);
						linedir.rat = (linedir.dy == 0) ? 0 : (linedir.dx / linedir.dy);
						linedir.amul = ((int64)1 << 60) / linedir.dx;
						linepos.frac = (if1 - PRECISION)*adx + linedir.dy;
						}
					else
						{ // y major
						linedir.x_major = false;
						const double mul = fdx / fdy;
						double f1 = mul*(P1.Y() - Pf1.Y()) + Pf1.X() - P1.X();
						double f2 = mul*(P2.Y() - Pf2.Y()) + Pf2.X() - P2.X();
						int64 if1 = (int64)((2 * PRECISION) * f1); if (if1 <= -PRECISION) { if1 = -PRECISION + 1;} else if (if1 >= PRECISION) { if1 = PRECISION - 1; }
						int64 if2 = (int64)((2 * PRECISION) * f2); if (if2 <= -PRECISION) { if2 = -PRECISION + 1;} else if (if2 >= PRECISION) { if2 = PRECISION - 1; }
						if (fdx < 0) { linedir.stepx = -1;  if1 = -if1; if2 = -if2; } else { linedir.stepx = +1; }
						if (fdy < 0) { linedir.stepy = -1; } else { linedir.stepy = +1; }
						linedir.dy = ady * (2 * PRECISION);
						linedir.dx = adx * (2 * PRECISION);
						linedir.dx += -if1 + if2;
						MTOOLS_ASSERT(linedir.dx >= 0);
						MTOOLS_ASSERT(linedir.dx <= linedir.dy);
						linedir.rat = (linedir.dx == 0) ? 0 : (linedir.dy / linedir.dx);
						linedir.amul = ((int64)1 << 60) / linedir.dy;
						linepos.frac = (if1 - PRECISION)*ady + linedir.dx;
						}
					}
				if (sw)
					{
					mtools::swap(P1, P2);
					_reverse_line(linedir, linepos, len);
					MTOOLS_ASSERT(linepos.x == P1.X());
					MTOOLS_ASSERT(linepos.y == P1.Y());
					}
				return len;
				}


			/** reverse the direction of the line. */
			MTOOLS_FORCEINLINE  void _reverse_line(_bdir & linedir, _bpos & linepos)
			{
				linedir.stepx *= -1;
				linedir.stepy *= -1;
				if (linedir.x_major)
					{
					linepos.frac = -linedir.dx - 1 - linepos.frac;
					linepos.frac += 2 * linedir.dy;
					}
				else
					{
					linepos.frac = -linedir.dy - 1 - linepos.frac;
					linepos.frac += 2 * linedir.dx;
					}
			}


			/** make n step along the line then reserve it */
			MTOOLS_FORCEINLINE  void _reverse_line(_bdir & linedir, _bpos & linepos, int64 n)
				{
				MTOOLS_ASSERT(n >= 0);
				_move_line(linedir, linepos, n);
				_reverse_line(linedir, linepos);
				}


			/* compute the value for antialiasing the half side of a line during bresenham algorithm:
			   side true = correspond to the outside being on the left of the line we follow the line in its increasing direction.
			               correspond to drawing a filled polygone in clockwise order. */
			template<bool side, bool x_major, int64 stepx, int64 stepy>  MTOOLS_FORCEINLINE int32 _line_aa(_bdir & linedir, _bpos & linepos)
				{				
				int64 a;				
				if (x_major)
					{
					a = linedir.dy;
					a = (((a - linepos.frac)*linedir.amul) >> 52);
					if (side) { if (stepx != stepy) a = 256 - a; } else { if (stepx == stepy) a = 256 - a; }
					}
				else
					{
					a = linedir.dx;
					a = (((a - linepos.frac)*linedir.amul) >> 52);
					if (side) { if (stepx == stepy) a = 256 - a; } else { if (stepx != stepy) a = 256 - a; }
					}
				a = (a >> 2) + (a >> 1) + 32; // compensate
				MTOOLS_ASSERT((a >= 0) && (a <= 256));
				return (int32)a;
				}


			/* same as above but a bit slower because not templated */
			template<bool side>  MTOOLS_FORCEINLINE int32 _line_aa(_bdir & linedir, _bpos & linepos)
				{
				int64 a;
				if (linedir.x_major)
					{
					a = linedir.dy;
					a = (((a - linepos.frac)*linedir.amul) >> 52);
					if (side) { if (linedir.stepx != linedir.stepy) a = 256 - a; } else { if (linedir.stepx == linedir.stepy) a = 256 - a; }
					}
				else
					{
					a = linedir.dx;
					a = (((a - linepos.frac)*linedir.amul) >> 52);
					if (side) { if (linedir.stepx == linedir.stepy) a = 256 - a; } else { if (linedir.stepx != linedir.stepy) a = 256 - a; }
					}
				a = (a >> 2) + (a >> 1) + 32; // compensate
				MTOOLS_ASSERT((a >= 0) && (a <= 256));
				return (int32)a;
				}


			/** move on the line by one pixel */
			template<bool x_major> MTOOLS_FORCEINLINE void _move_line(const _bdir & linedir, _bpos & pos)
				{
				if (x_major)
					{
					if (pos.frac >= 0) { pos.y += linedir.stepy; pos.frac -= linedir.dx; }
					pos.x += linedir.stepx; pos.frac += linedir.dy;
					}
				else
					{
					if (pos.frac >= 0) { pos.x += linedir.stepx; pos.frac -= linedir.dy; }
					pos.y += linedir.stepy; pos.frac += linedir.dx;
					}
				}


			/**
			* Move the position pos by len pixels along a bresenham line.
			*/
			inline void _move_line(const _bdir & linedir, _bpos & pos, int64 len)
				{
				if (linedir.x_major)
					{
					if (linedir.dx == 0) return;
					pos.x += linedir.stepx*len;
					pos.frac += linedir.dy*len;
					int64 u = pos.frac / linedir.dx;
					pos.y += linedir.stepy*u;
					pos.frac -= u*linedir.dx;
					if (pos.frac >= linedir.dy) { pos.frac -= linedir.dx; pos.y += linedir.stepy; }
					}
				else
					{
					if (linedir.dy == 0) return;
					pos.y += linedir.stepy*len;
					pos.frac += linedir.dx*len;
					int64 u = pos.frac / linedir.dy;
					pos.x += linedir.stepx*u;
					pos.frac -= u*linedir.dy;
					if (pos.frac >= linedir.dx) { pos.frac -= linedir.dy; pos.x += linedir.stepx; }
					}
				}


			/**
			 * Move the position pos by 1 pixel horizontally along the given bresenham line.
			 * return the number of pixel traveled by the bresenham line.
			 *
			 * [x_major can be deduced from linedir but is given as template paramter for speed optimization]
			 */
			template<bool x_major> MTOOLS_FORCEINLINE int64 _move_line_x_dir(const _bdir & linedir, _bpos & pos)
				{ 
				MTOOLS_ASSERT(linedir.x_major == x_major);  // make sure template argument supplied is correct. 
				MTOOLS_ASSERT(linedir.dx > 0); // not a vertical line
				if (x_major) // compiler optimizes away the template conditionals
					{
					if (pos.frac >= 0) { pos.y +=  linedir.stepy; pos.frac -= linedir.dx; }
					pos.x += linedir.stepx; pos.frac += linedir.dy;
					return 1;
					}
				else
					{
					int64 r = (pos.frac < ((linedir.dx << 1) - linedir.dy)) ? linedir.rat : ((linedir.dx - pos.frac) / linedir.dx); // use rat value if just after a step.
					pos.y += (r*linedir.stepy);
					pos.frac += (r*linedir.dx);
					if (pos.frac < linedir.dx) { pos.y += linedir.stepy; pos.frac += linedir.dx; r++;  }
					MTOOLS_ASSERT((pos.frac >= linedir.dx)&&(pos.frac < 2*linedir.dx));
					pos.frac -= linedir.dy;  pos.x += linedir.stepx;
					return r;
					}
				}


			/**
			* Move the position pos by a given number of pixels horizontal along a bresenham line.
			* return the number of pixel traveled by the bresenham line.
			* do nothing and return 0 if lenx <= 0.
			*/
			inline int64 _move_line_x_dir(const _bdir & linedir, _bpos & pos, const int64 lenx)
				{
				if (lenx <= 0) return 0;
				MTOOLS_ASSERT(linedir.dx > 0); // not a vertical line
				if (linedir.x_major)
					{
					pos.x    += linedir.stepx*lenx;
					pos.frac += linedir.dy*lenx;
					int64 u   = pos.frac / linedir.dx;
					pos.y    += linedir.stepy*u;
					pos.frac -= u*linedir.dx;
					if (pos.frac >= linedir.dy) { pos.frac -= linedir.dx; pos.y += linedir.stepy; }
					return lenx;
					}
				else
					{
					int64 k = ((lenx - 1)*linedir.dy) / linedir.dx;
					pos.frac += k*linedir.dx;
					pos.y += k*linedir.stepy;
					int64 u = pos.frac / linedir.dy;
					pos.frac -= u*linedir.dy;
					if (pos.frac >= linedir.dx) { u++; pos.frac -= linedir.dy; }
					MTOOLS_ASSERT((u <= lenx) && (u >= lenx - 4));
					pos.x += u*linedir.stepx;
					while (u != lenx) { k += _move_line_x_dir<false>(linedir, pos); u++; }
					return k;
					}
				}



			/**
			* Move the position pos by one pixel vertically along the given bresenham line.
			* return the number of pixel traveled by the bresenham line.
			*
			* [x_major can be deduced from linedir but given as template paramter for speed optimization]
			*/
			template<bool x_major> MTOOLS_FORCEINLINE int64 _move_line_y_dir(const _bdir & linedir, _bpos & pos)
				{
				MTOOLS_ASSERT(linedir.x_major == x_major);  // make sure tempalte argument supplied is correct. 
				MTOOLS_ASSERT(linedir.dy > 0); // not an horizontal line
				if (x_major) // compiler optimizes away the template conditionals
					{
					int64 r = (pos.frac < ((linedir.dy << 1) - linedir.dx)) ? linedir.rat : ((linedir.dy - pos.frac) / linedir.dy); // use rat value if just after a step.
					pos.x += (r*linedir.stepx);
					pos.frac += (r*linedir.dy);
					if (pos.frac < linedir.dy) { pos.x += linedir.stepx; pos.frac += linedir.dy; r++; }
					MTOOLS_ASSERT((pos.frac >= linedir.dy) && (pos.frac < 2 * linedir.dy));
					pos.frac -= linedir.dx;  pos.y += linedir.stepy;
					return r;
					}
				else
					{
					if (pos.frac >= 0) { pos.x += linedir.stepx; pos.frac -= linedir.dy; }
					pos.y += linedir.stepy; pos.frac += linedir.dx;
					return 1;
					}
				}


			/**
			* Move the position pos along a line by a given number of pixels vertically along a bresenham line.
			* return the number of pixel traveled by the bresenham line.
			* do nothing and return 0 if leny <= 0.
			*/
			inline int64 _move_line_y_dir(const _bdir & linedir, _bpos & pos, const int64 leny)
				{
				if (leny <= 0) return 0; 
				MTOOLS_ASSERT(linedir.dy > 0); // not an horizontal line
				if (linedir.x_major) // compiler optimizes away the template conditionals
					{
					int64 k = ((leny-1)*linedir.dx) / linedir.dy;
					pos.frac += k*linedir.dy; 
					pos.x += k*linedir.stepx; 
					int64 u = pos.frac/linedir.dx;
					pos.frac -= u*linedir.dx;
					if (pos.frac >= linedir.dy) { u++; pos.frac -= linedir.dx; }					
					MTOOLS_ASSERT((u <= leny)&&(u >= leny-4)); 
					pos.y += u*linedir.stepy;
					while(u != leny) { k += _move_line_y_dir<true>(linedir, pos); u++; }
					return k;
					}
				else
					{
					pos.y += linedir.stepy*leny;
					pos.frac += linedir.dx*leny;
					int64 u = pos.frac / linedir.dy;
					pos.x += linedir.stepx*u;
					pos.frac -= u*linedir.dy;
					if (pos.frac >= linedir.dx) { pos.frac -= linedir.dy; pos.x += linedir.stepx; }
					return leny;
					}
				}


			/**
			 * Move the position until it is in the closed box B. Return the number of step performed by the
			 * walk or -1 if the line never enters the box. (in this case, pos may be anywhere.)
			 **/
			inline int64 _move_inside_box(const _bdir & linedir, _bpos & pos, const iBox2 & B)
				{
				if (B.isEmpty()) return -1;
				if (B.isInside({ pos.x,pos.y })) return 0;
				int64 tot = 0;
				if (pos.x < B.min[0])
					{
					if ((linedir.stepx < 0)||(linedir.dx ==0)) return -1;
					tot += _move_line_x_dir(linedir, pos, B.min[0] - pos.x);
					}
				else if (pos.x > B.max[0])
					{
					if ((linedir.stepx > 0) ||(linedir.dx == 0)) return -1;
					tot += _move_line_x_dir(linedir, pos, pos.x - B.max[0]);
					}
				if (pos.y < B.min[1])
					{
					if ((linedir.stepy < 0) || (linedir.dy == 0)) return -1;
					tot += _move_line_y_dir(linedir, pos, B.min[1] - pos.y);
					}
				else if (pos.y > B.max[1])
					{
					if ((linedir.stepy > 0) || (linedir.dy == 0)) return -1;
					tot += _move_line_y_dir(linedir, pos, pos.y - B.max[1]);
					}
				if (!B.isInside({ pos.x, pos.y })) return -1;
				return tot;
				}


			/**
			 * Compute number of pixel of the line before it exits the box B. If the box is empty of if pos
			 * is not in it, return 0.
			 **/
			inline int64 _lenght_inside_box(const _bdir & linedir, const _bpos & pos, const iBox2 & B)
				{
				if (!B.isInside({ pos.x, pos.y })) return 0; 
				const int64 hx = 1 + ((linedir.stepx > 0) ? (B.max[0] - pos.x) : (pos.x - B.min[0])); // number of horizontal step before exit. 
				const int64 hy = 1 + ((linedir.stepy > 0) ? (B.max[1] - pos.y) : (pos.y - B.min[1])); // number of vertical step before exit. 
				int64 nx = -1, ny = -1;				
				if (linedir.dx != 0) { _bpos tmp = pos; nx = _move_line_x_dir(linedir, tmp, hx); }
				if (linedir.dy != 0) { _bpos tmp = pos; ny = _move_line_y_dir(linedir, tmp, hy); }
				if (nx == -1) { return ny; }
				if (ny == -1) { return nx; }
				return std::min<int64>(nx, ny);
				}



			/**
			* Return the number of pixels that composed the Bressenham segment [P,Q|.
			* 
			* closed = true to compute the lenght of [P,Q] and false for [P,Q[.
			**/
			MTOOLS_FORCEINLINE int64 _lengthBresenham(iVec2 P, iVec2 Q, bool closed = false)
				{
				return std::max<int64>(abs(P.X() - Q.X()), abs(P.Y() - Q.Y())) + (closed ? 1 : 0);
				}


			/**
			 * Compute the position of the next pixel after P on a Bresenham segment [P,Q].
			 * if P = Q, return P. 
			 **/
			MTOOLS_FORCEINLINE iVec2 _nextPosInLine(iVec2 P, iVec2 Q)
				{
				if (P == Q) return P;
				_bdir line;
				_bpos pos;
				_init_line(P, Q, line, pos);
				if (line.x_major) _move_line<true>(line, pos); else _move_line<false>(line, pos);
				return iVec2(pos.x, pos.y);
				}




			/** draw a pixel on a bresenham line */
			template<bool blend, bool checkrange, bool useop, bool usepen, bool useaa, bool side>
			MTOOLS_FORCEINLINE void _update_pixel_bresenham(_bdir & line, _bpos & pos, RGBc color, int32 op, int32 penwidth)
				{
				if (useaa)
				{
					int32 aa = _line_aa<side>(line, pos);
					if (useop) { aa *= op; aa >>= 8; }
					_updatePixel<blend, checkrange, true, usepen>(pos.x, pos.y, color, aa, penwidth);
					}
				else
					{
					_updatePixel<blend, checkrange, useop, usepen>(pos.x, pos.y, color, op, penwidth);
					}
				}


			/**
			* Draw len pixels along a given bresenham segment [pos, pos + len[
			* template parameter control (side) antialiasing, blending, checkrange and pen width.
			*
			* exactly len pixel are painted
			**/
			template<bool blend, bool checkrange, bool useop, bool usepen, bool useaa, bool side>  MTOOLS_FORCEINLINE void _lineBresenham(_bdir line, _bpos pos, int64 len, RGBc color, int32 penwidth, int32 op)
				{
				if (checkrange)
					{
					iBox2 B;
					if (penwidth <= 0) { B = iBox2(0, _lx - 1, 0, _ly - 1); }
					else { B = iBox2(-penwidth - 2, _lx + penwidth + 1, -penwidth - 2, _ly + penwidth + 1); }
					int64 r = _move_inside_box(line, pos, B);
					if (r < 0) return; // nothing to draw
					len -= r;
					len = std::min<int64>(len, _lenght_inside_box(line, pos, B));
					}
				if (line.x_major)
					{
					while (--len >= 0)
						{
						_update_pixel_bresenham<blend, usepen, useop, usepen, useaa, side>(line, pos, color, op, penwidth);
						_move_line<true>(line, pos);
						}
					}
				else
					{
					while (--len >= 0)
						{
						_update_pixel_bresenham<blend, usepen, useop, usepen, useaa, side>(line, pos, color, op, penwidth);
						_move_line<false>(line, pos);
						}
					}
				}


			/**
			* Draw a segment [P1,P2] using Bresenham's algorithm, (possibly with side antialiasing)
			*
			* The algorithm is symmetric: the line [P1,P2] is always equal
			* to the line drawn in the other direction [P2,P1].
			*
			* template parameter control (side) antialiasing, blending, checkrange and pen width.
			**/
			template<bool blend, bool checkrange, bool useop, bool usepen, bool useaa, bool side>  MTOOLS_FORCEINLINE void _lineBresenham(const iVec2 P1, const iVec2 P2, RGBc color, bool draw_last, int32 penwidth, int32 op)
				{
				if (P1 == P2)
					{
					if (draw_last)
						{
						if (penwidth <= 0) { _updatePixel<blend, checkrange,useop, false>(P1.X(), P1.Y(), color, op, penwidth); }
						else { _updatePixel<blend, checkrange, useop, true>(P1.X(), P1.Y(), color, op, penwidth); }
						}
					return;
					}
				_bdir line;
				_bpos pos;
				int64 len = _init_line(P1, P2, line, pos) + (draw_last ? 1 : 0);
				_lineBresenham<blend, checkrange,useop, usepen, useaa, side>(line, pos, len, color, penwidth, op);
				return;
				}


			/**
			* Return the max distance where the two line intersect. linea and lineb must share the same start pixel. 
			* segment are considered open ended : [a, a + lena[
			**/
			template<bool checkrange> int64 _lineBresenham_find_max_intersection(_bdir linea, _bpos posa, int64 lena, _bdir lineb, _bpos posb, int64 lenb)
				{
				MTOOLS_ASSERT((posa.x == posb.x) && (posa.y == posb.y));
				int64 r = 0;
				if (checkrange)
					{
					iBox2 B(0, _lx - 1, 0, _ly - 1);
					r = _move_inside_box(linea, posa, B); // move the first line into the box. 
					if (r < 0) return 1; // nothing to draw
					lena -= r;
					_move_line(lineb, posb, r); // move the second line
					lenb -= r;
					lena = std::min<int64>(lena, _lenght_inside_box(linea, posa, B)); // number of pixels still to draw. 
					}
				lena--;
				lenb--;
				int64 l = 0;
				int64 maxp = 0;
				if (linea.x_major)
					{
					if (lineb.x_major)
						{
						int64 o = 0;
						while ((o <= 1) && (l <= lena) && (l <= lenb))
							{
							o = abs(posa.x - posb.x) + abs(posa.y - posb.y);
							if (o == 0) maxp = l;
							_move_line<true>(linea, posa);
							_move_line<true>(lineb, posb);
							l++;
							}
						}
					else
						{
						int64 o = 0;
						while ((o <= 1) && (l <= lena) && (l <= lenb))
							{
							o = abs(posa.x - posb.x) + abs(posa.y - posb.y);
							if (o == 0) maxp = l;
							_move_line<true>(linea, posa);
							_move_line<false>(lineb, posb);
							l++;
							}
						}
					}
				else
					{
					if (lineb.x_major)
						{
						int64 o = 0;
						while ((o <= 1) && (l <= lena) && (l <= lenb))
							{
							o = abs(posa.x - posb.x) + abs(posa.y - posb.y);
							if (o == 0) maxp = l;
							_move_line<false>(linea, posa);
							_move_line<true>(lineb, posb);
							l++;
							}
						}
					else
						{
						int64 o = 0;
						while ((o <= 1) && (l <= lena) && (l <= lenb))
							{
							o = abs(posa.x - posb.x) + abs(posa.y - posb.y);
							if (o == 0) maxp = l;
							_move_line<false>(linea, posa);
							_move_line<false>(lineb, posb);
							l++;
							}
						}
					}
				return ((maxp == 0) ? 1 : (r + maxp));
				}


			/**
			* Return the max distance from P where [P,Q] and [P,Q2] intersect.
			**/
			template<bool checkrange> MTOOLS_FORCEINLINE int64 _lineBresenham_find_max_intersection(iVec2 P, iVec2 Q, iVec2 Q2)
			{
				if ((P == Q) || (P == Q2)) return 1;
				_bdir linea;
				_bpos posa;
				int64 lena = _init_line(P, Q, linea, posa) + 1;
				_bdir lineb;
				_bpos posb;
				int64 lenb = _init_line(P, Q2, lineb, posb) + 1;
				_lineBresenham_find_max_intersection<checkrange>(linea, posa, lena, lineb, posb, lenb);
			}




			/**
			* Draw the segment [a, + lena[ while avoiding segment [b, b + lenb[ 
			* a and b must share the same start pixel
			**/
			template<bool blend, bool checkrange, bool useop, bool useaa, bool side> void _lineBresenham_avoid(_bdir linea, _bpos posa, int64 lena, _bdir lineb, _bpos posb, int64 lenb, RGBc color, int32 op)
				{
				MTOOLS_ASSERT((posa.x == posb.x) && (posa.y == posb.y));
				if (checkrange)
					{
					iBox2 B(0, _lx - 1, 0, _ly - 1);
					int64 r = _move_inside_box(linea, posa, B); // move the first line into the box. 
					if (r < 0) return; // nothing to draw
					lena -= r;
					_move_line(lineb, posb, r); // move the second line
					lenb -= r;
					lena = std::min<int64>(lena, _lenght_inside_box(linea, posa, B)); // number of pixels still to draw. 
					}
				lena--;
				lenb--;
				int64 l = 0;
				if (linea.x_major)
					{
					if (lineb.x_major)
						{
						while (l <= lena)
							{
							if ((l > lenb) || (posa.x != posb.x) || (posa.y != posb.y)) _update_pixel_bresenham<blend, false, useop, false, useaa, side>(linea, posa, color, op, 0);
							_move_line<true>(linea, posa);
							_move_line<true>(lineb, posb);						
							l++; 
							}
						}
					else
						{
						while (l <= lena)
							{
							if ((l > lenb) || (posa.x != posb.x) || (posa.y != posb.y)) _update_pixel_bresenham<blend, false, useop, false, useaa, side>(linea, posa, color, op, 0);
							_move_line<true>(linea, posa);
							_move_line<false>(lineb, posb);
							l++;
							}
						}
					}
				else
					{
					if (lineb.x_major)
						{
						while (l <= lena)
							{
							if ((l > lenb) || (posa.x != posb.x) || (posa.y != posb.y)) _update_pixel_bresenham<blend, false, useop, false, useaa, side>(linea, posa, color, op, 0);
							_move_line<false>(linea, posa);
							_move_line<true>(lineb, posb);
							l++;
							}
						}
					else
						{
						while (l <= lena)
							{
							if ((l > lenb) || (posa.x != posb.x) || (posa.y != posb.y)) _update_pixel_bresenham<blend, false, useop, false, useaa, side>(linea, posa, color, op, 0);
							_move_line<false>(linea, posa);
							_move_line<false>(lineb, posb);
							l++;
							}
						}
					}
				}


			/**
			 * Draw the segment [P,Q] with the bresenham line algorithm while skipping the pixels
			 * which also belong to the segment [P,P2]. (Always perfect.)
			 * 
			 * stop_before represented the number of pixel at the end of the line which are not drawn
			 * i.e 0 = draw [P,Q], 
			 *     1 = draw [P,Q[   
			 *    >1 = remove more pixels  
			 *    <0 = extend the line adding stop_before additional pixels.
			 **/
			template<bool blend, bool checkrange, bool useop, bool useaa, bool side> MTOOLS_FORCEINLINE void _lineBresenham_avoid(iVec2 P, iVec2 Q, iVec2 P2, RGBc color, int64 stop_before, int32 op)
				{
				if (P == Q) return;
				_bdir linea;
				_bpos posa;
				int64 lena = _init_line(P, Q, linea, posa) + 1 - stop_before;
				_bdir lineb;
				_bpos posb;
				int64 lenb = _init_line(P, P2, lineb, posb) + 1;
				_lineBresenham_avoid<blend, checkrange, useop, useaa, side>(linea, posa, lena, lineb, posb, lenb, color, op);
				}



			/**
			* Draw the segment [a, a + lena[ while avoiding segments [b, b+ lenb[ and [c, c+ lenc[ 
			* a,b,c must share the same starting pixel. 
			**/
			template<bool blend, bool checkrange, bool useop, bool useaa, bool side> void _lineBresenham_avoid(_bdir linea, _bpos posa, int64 lena, _bdir lineb, _bpos posb, int64 lenb, _bdir linec, _bpos posc, int64 lenc, RGBc color, int32 op)
				{
				MTOOLS_ASSERT((posa.x == posb.x) && (posa.y == posb.y));
				MTOOLS_ASSERT((posa.x == posc.x) && (posa.y == posc.y));
				if (checkrange)
					{
					iBox2 B(0, _lx - 1, 0, _ly - 1);
					int64 r = _move_inside_box(linea, posa, B); // move the first line into the box. 
					if (r < 0) return; // nothing to draw
					lena -= r;
					_move_line(lineb, posb, r); // move the second line
					lenb -= r;
					_move_line(linec, posc, r); // move the third line
					lenc -= r;
					lena = std::min<int64>(lena, _lenght_inside_box(linea, posa, B)); // number of pixels still to draw. 
					}
				lena--;
				lenb--;
				lenc--;
				int64 l = 0;
				if (linea.x_major)
					{
					if (lineb.x_major)
						{
						if (linec.x_major)
							{
							while (l <= lena)
								{
								if (((l > lenb) || (posa.x != posb.x) || (posa.y != posb.y)) && ((l > lenc) || (posa.x != posc.x) || (posa.y != posc.y))) _update_pixel_bresenham<blend, false, useop, false, useaa, side>(linea, posa, color, op, 0);
								_move_line<true>(linea, posa); 
								_move_line<true>(lineb, posb); 
								_move_line<true>(linec, posc);
								l++;
								}
							}
						else
							{
							while (l <= lena)
								{
								if (((l > lenb) || (posa.x != posb.x) || (posa.y != posb.y)) && ((l > lenc) || (posa.x != posc.x) || (posa.y != posc.y))) _update_pixel_bresenham<blend, false, useop, false, useaa, side>(linea, posa, color, op, 0);
								_move_line<true>(linea, posa);
								_move_line<true>(lineb, posb);
								_move_line<false>(linec, posc);
								l++;
								}
							}
						}
					else
						{
						if (linec.x_major)
							{
							while (l <= lena)
								{
								if (((l > lenb) || (posa.x != posb.x) || (posa.y != posb.y)) && ((l > lenc) || (posa.x != posc.x) || (posa.y != posc.y))) _update_pixel_bresenham<blend, false, useop, false, useaa, side>(linea, posa, color, op, 0);
								_move_line<true>(linea, posa);
								_move_line<false>(lineb, posb);
								_move_line<true>(linec, posc);
								l++;
								}
							}
						else
							{
							while (l <= lena)
								{
								if (((l > lenb) || (posa.x != posb.x) || (posa.y != posb.y)) && ((l > lenc) || (posa.x != posc.x) || (posa.y != posc.y))) _update_pixel_bresenham<blend, false, useop, false, useaa, side>(linea, posa, color, op, 0);
								_move_line<true>(linea, posa);
								_move_line<false>(lineb, posb);
								_move_line<false>(linec, posc);
								l++;
								}
							}
						}
					}
				else
					{
					if (lineb.x_major)
						{
						if (linec.x_major)
							{
							while (l <= lena)
								{
								if (((l > lenb) || (posa.x != posb.x) || (posa.y != posb.y)) && ((l > lenc) || (posa.x != posc.x) || (posa.y != posc.y))) _update_pixel_bresenham<blend, false, useop, false, useaa, side>(linea, posa, color, op, 0);
								_move_line<false>(linea, posa);
								_move_line<true>(lineb, posb);
								_move_line<true>(linec, posc);
								l++;
								}
							}
						else
							{
							while (l <= lena)
								{
								if (((l > lenb) || (posa.x != posb.x) || (posa.y != posb.y)) && ((l > lenc) || (posa.x != posc.x) || (posa.y != posc.y))) _update_pixel_bresenham<blend, false, useop, false, useaa, side>(linea, posa, color, op, 0);
								_move_line<false>(linea, posa);
								_move_line<true>(lineb, posb);
								_move_line<false>(linec, posc);
								l++;
								}
							}
						}
					else
						{
						if (linec.x_major)
							{
							while (l <= lena)
								{
								if (((l > lenb) || (posa.x != posb.x) || (posa.y != posb.y)) && ((l > lenc) || (posa.x != posc.x) || (posa.y != posc.y))) _update_pixel_bresenham<blend, false, useop, false, useaa, side>(linea, posa, color, op, 0);
								_move_line<false>(linea, posa);
								_move_line<false>(lineb, posb);
								_move_line<true>(linec, posc);
								l++;
								}
							}
						else
							{
							while (l <= lena)
								{
								if (((l > lenb) || (posa.x != posb.x) || (posa.y != posb.y)) && ((l > lenc) || (posa.x != posc.x) || (posa.y != posc.y))) _update_pixel_bresenham<blend, false, useop, false, useaa, side>(linea, posa, color, op, 0);
								_move_line<false>(linea, posa);
								_move_line<false>(lineb, posb);
								_move_line<false>(linec, posc);
								l++;
								}
							}
						}
					}
				}


			/**
			* Draw the segment [P,Q] with the bresenham line algorithm while skipping the pixels
			* which also belong to the segments [P,P2] and [P,P3].  (always perfect.)
			*
			* stop_before represented the number of pixel at the end of the line which are not drawn
			* i.e 0 = draw [P,Q],
			*     1 = draw [P,Q[
			*    >1 = remove som more pixels
			*    <0 = extend the line adding stop_before additional pixels.
			**/
			template<bool blend, bool checkrange, bool useop, bool useaa, bool side> MTOOLS_FORCEINLINE void _lineBresenham_avoid(iVec2 P, iVec2 Q, iVec2 P2, iVec2 P3, RGBc color, int64 stop_before, int32 op)
				{
				if ((P2 == P3)||(P3 == P)) { _lineBresenham_avoid<blend, checkrange, useop, useaa, side>(P, Q, P2, color, stop_before, op); return; }
				if (P2 == P) { _lineBresenham_avoid<blend, checkrange, useop, useaa, side>(P, Q, P3, color, stop_before, op); return; }
				if (P == Q) return;
				_bdir linea;
				_bpos posa;
				int64 lena = _init_line(P, Q, linea, posa) + 1 - stop_before;
				_bdir lineb;
				_bpos posb;
				int64 lenb = _init_line(P, P2, lineb, posb) + 1;
				_bdir linec;
				_bpos posc;
				int64 lenc = _init_line(P, P3, linec, posc) + 1;
				_lineBresenham_avoid<blend, checkrange, useop, useaa, side>(linea, posa, lena, lineb, posb, lenb, linec, posc, lenc, color, op);
				}


			/**
			* Draw the (open) segment ]a, a+ lena[ while avoiding segments [b, b + lenb[ and [c, c+ lenc[
			* 
			* a and b must share the same pixel
			* a + lena and c must share the same pixel
			* TRIANGLE CONDITION: b + lenb and c + lenc must share the same pixel (otherwise, use _lineBresenham_avoid_both_sides() method).
			**/
			template<bool blend, bool checkrange, bool useop, bool useaa, bool side> MTOOLS_FORCEINLINE void _lineBresenham_avoid_both_sides_triangle(_bdir linea, _bpos posa, int64 lena, _bdir lineb, _bpos posb, int64 lenb, _bdir linec, _bpos posc, int64 lenc, RGBc color, int32 op)
				{
				MTOOLS_ASSERT((posa.x == posb.x) && (posa.y == posb.y));
				int64 G = lenb - lenc;
				if (G > lena) { G = lena; } else { if (G < -lena) { G = -lena; } }
				int64 lP = 1 + ((lena + G) >> 1);
				int64 lQ = 1 + ((lena - G) >> 1);
				if (lP + lQ > lena + 1)
					{
					if (lP > lQ) { lP--; } else { lQ--; }
					}
				MTOOLS_ASSERT(lP + lQ == lena + 1);
				_bdir linea2 = linea;
				_bpos posa2 = posa;
				_move_line(linea2, posa2,lena);
				_reverse_line(linea2, posa2);
				MTOOLS_ASSERT((posa2.x == posc.x) && (posa2.y == posc.y));
				_lineBresenham_avoid<blend, checkrange, useop, useaa,  side>(linea, posa, lena + 1 - lQ, lineb, posb, lenb, color, op);
				_lineBresenham_avoid<blend, checkrange, useop, useaa, !side>(linea2, posa2, lena + 1 - lP, linec, posc, lenc, color, op);
				}


			/**
			* Draw the segment [P,Q] with the bresenham line algorithm while skipping the pixels which also
			* belong to the segments [P,R] and [Q,R];
			**/
			template<bool blend, bool checkrange, bool useop, bool useaa, bool side> MTOOLS_FORCEINLINE void _lineBresenham_avoid_both_sides_triangle(iVec2 P, iVec2 Q, iVec2 R, RGBc color, int32 op)
				{
				if ((P==Q)||(P == R)||(Q == R)) {  return; }
				const int64 PQ = _lengthBresenham(P, Q);
				int64 G = _lengthBresenham(P, R) - _lengthBresenham(Q, R);
				if (G > PQ) { G = PQ; } else { if (G < -PQ) { G = -PQ; } }
				int64 lP = 1 + ((PQ + G) >> 1);
				int64 lQ = 1 + ((PQ - G) >> 1);
				if (lP + lQ > PQ + 1)
					{
					if (lP > lQ) { lP--; } else { lQ--; }
					}
				MTOOLS_ASSERT(lP + lQ == PQ + 1);
				_lineBresenham_avoid<blend, checkrange, useop, useaa, side>(P, Q, R, color, lQ, op);
				_lineBresenham_avoid<blend, checkrange, useop, useaa, !side>(Q, P, R, color, lP, op);
				}


			/**
			* Draw the (open) segment ]a, a+ lena[ while avoiding segments [b, b + lenb[ and [c, c+ lenc[
			*
			* a and b must share the same pixel
			* a + lena and c must share the same pixel
			*
			* if the triangle condition is satified: b + lenb' and c + lenc' share the same pixel for some lenb' >= lenb and lenc' >= lenc 
			* then use _lineBresenham_avoid_both_sides_triangle() instead which is faster.
			**/
			template<bool blend, bool checkrange, bool useop, bool useaa, bool side> MTOOLS_FORCEINLINE void _lineBresenham_avoid_both_sides(_bdir linea, _bpos posa, int64 lena, _bdir lineb, _bpos posb, int64 lenb, _bdir linec, _bpos posc, int64 lenc, RGBc color, int32 op)			
				{
				MTOOLS_ASSERT((posa.x == posb.x) && (posa.y == posb.y));
				int64 pl = _lineBresenham_find_max_intersection<checkrange>(linea, posa, lena, lineb, posb, lenb);
				if (pl > lena) { pl = lena; }	else if (pl < 1) pl = 1;

				_bdir linea2 = linea;
				_bpos posa2 = posa;
				_move_line(linea2, posa2, lena);
				_reverse_line(linea2, posa2);
				MTOOLS_ASSERT((posa2.x == posc.x) && (posa2.y == posc.y));
				int64 ql = _lineBresenham_find_max_intersection<checkrange>(linea2, posa2, lena, linec, posc, lenc);
				if (ql > lena) { ql = lena; } else if (ql < 1) ql = 1;

				int64 M = (pl + ((lena + 1) - ql)) >> 1; // much faster and usually good to just take M = L/2; 
				_lineBresenham_avoid<blend, checkrange, useop, useaa, side>(linea, posa, M, lineb, posb, lenb, color, op);
				_lineBresenham_avoid<blend, checkrange, useop, useaa, !side>(linea2, posa2, lena + 1 - M, linec, posc, lenc, color, op);
				}


			/**
			 * Draw the segment [P,Q] with the bresenham line algorithm while skipping the pixels which also
			 * belong to the segments [P,P2] and [Q,Q2].
			 * 
			 * -> If the lines are really close, it is possible that the drawing is not perfect and
			 *    some pixels over an existing line are drawn (but this only happens for 'degenerate' cases).
			 **/
			template<bool blend, bool checkrange, bool useop, bool useaa, bool side> MTOOLS_FORCEINLINE void _lineBresenham_avoid_both_sides(iVec2 P, iVec2 Q, iVec2 P2, iVec2 Q2, RGBc color, int32 op)
				{
				const int64 L = _lengthBresenham(P, Q);				
				int64 pl = _lineBresenham_find_max_intersection<checkrange>(P, Q, P2);
				if (pl > L) { pl = L; } else if (pl < 1) pl = 1;
				int64 ql = _lineBresenham_find_max_intersection<checkrange>(Q, P, Q2);
				if (ql > L) { ql = L; } else if (ql < 1) ql = 1;
				int64 M = (pl + ((L+1) - ql)) >> 1; // much faster and usually good to just take M = L/2; 
				_lineBresenham_avoid<blend, checkrange, useop, useaa, side>(P, Q, P2, color, L + 1 - M, op);
				_lineBresenham_avoid<blend, checkrange, useop, useaa, !side>(Q, P, Q2, color, M, op);
				}


			/**
			* Draw the (open) segment ]a, a+ lena[ while avoiding segments [b, b + lenb[ and [c, c+ lenc[, [d, d+ lend[, [e, e+ lene[
			*
			* a, b and c must share the same pixel
			* a + lena and d and e must share the same pixel
			*
			**/
			template<bool blend, bool checkrange, bool useop, bool useaa, bool side> MTOOLS_FORCEINLINE void _lineBresenham_avoid_both_sides(_bdir linea, _bpos posa, int64 lena, 
																																			 _bdir lineb, _bpos posb, int64 lenb, _bdir linec, _bpos posc, int64 lenc,				                                                                                                       				                                                                                                                             
				                                                                                                                             _bdir lined, _bpos posd, int64 lend, _bdir linee, _bpos pose, int64 lene,				                                                                                                                             
				                                                                                                                             RGBc color, int32 op)
				{
				MTOOLS_ASSERT((posa.x == posb.x) && (posa.y == posb.y));
				MTOOLS_ASSERT((posa.x == posc.x) && (posa.y == posc.y));
				int64 pl2 = _lineBresenham_find_max_intersection<checkrange>(linea, posa, lena, lineb, posb, lenb);
				if (pl2 > lena) { pl2 = lena; } else if (pl2 < 1) pl2 = 1;
				int64 pl3 = _lineBresenham_find_max_intersection<checkrange>(linea, posa, lena, linec, posc, lenc);
				if (pl3 > lena) { pl3 = lena; } else if (pl3 < 1) pl3 = 1;
				int64 pl = std::max<int64>(pl2, pl3);
				_bdir linea2 = linea;
				_bpos posa2 = posa;
				_move_line(linea2, posa2, lena);
				_reverse_line(linea2, posa2);
				MTOOLS_ASSERT((posa2.x == posd.x) && (posa2.y == posd.y));
				MTOOLS_ASSERT((posa2.x == pose.x) && (posa2.y == pose.y));
				int64 ql2 = _lineBresenham_find_max_intersection<checkrange>(linea2, posa2, lena, lined, posd, lend);
				if (ql2 > lena) { ql2 = lena; } else if (ql2 < 1) ql2 = 1;
				int64 ql3 = _lineBresenham_find_max_intersection<checkrange>(linea2, posa2, lena, linee, pose, lene);
				if (ql3 > lena) { ql3 = lena; } else if (ql3 < 1) ql3 = 1;
				int64 ql = std::max<int64>(ql2, ql3);
				int64 M = (pl + ((lena + 1) - ql)) >> 1; // much faster and usually good to just take M = L/2; 
				_lineBresenham_avoid<blend, checkrange, useop, useaa, side>(linea, posa, M, lineb, posb, lenb, linec, posc, lenc, color, op);
				_lineBresenham_avoid<blend, checkrange, useop, useaa, !side>(linea2, posa2, lena + 1 - M, lined, posd, lend, linee, pose, lene, color, op);
				}



			/**
			* Draw the segment [P,Q] with the bresenham line algorithm while skipping the pixels which also
			* belong to the segments [P,P2], [P,P3] and [Q,Q2], [Q,Q3].
			*
			* -> If the lines are really close, it is possible that the drawing is not perfect and
			*    some pixels over an existing line are drawn (but this only happens for 'degenerate' cases).
			**/
			template<bool blend, bool checkrange, bool useop, bool useaa, bool side> inline void _lineBresenham_avoid_both_sides(iVec2 P, iVec2 Q, iVec2 P2, iVec2 P3, iVec2 Q2, iVec2 Q3, RGBc color, int32 op)
				{
				const int64 L = _lengthBresenham(P, Q);
				int64 pl2 = _lineBresenham_find_max_intersection<checkrange>(P, Q, P2);
				if (pl2 > L) { pl2 = L; } else if (pl2 < 1) pl2 = 1;
				int64 pl3 = _lineBresenham_find_max_intersection<checkrange>(P, Q, P3);
				if (pl3 > L) { pl3 = L; } else if (pl3 < 1) pl3 = 1;
				int64 pl = std::max<int64>(pl2, pl3);
				int64 ql2 = _lineBresenham_find_max_intersection<checkrange>(Q, P, Q2);
				if (ql2 > L) { ql2 = L; } else if (ql2 < 1) ql2 = 1;
				int64 ql3 = _lineBresenham_find_max_intersection<checkrange>(Q, P, Q3);
				if (ql3 > L) { ql3 = L; } else if (ql3 < 1) ql3 = 1;
				int64 ql = std::max<int64>(ql2, ql3);
				int64 M = (pl + ((L + 1) - ql)) >> 1; // much faster and usually good to just take M = L/2; 
				_lineBresenham_avoid<blend, checkrange, useop, useaa, side>(P, Q, P2, P3, color, L + 1 - M,op);
				_lineBresenham_avoid<blend, checkrange, useop, useaa, !side>(Q, P, Q2, Q3, color, M,op);	
				}



			/**
			 * Draw the interior of the triangle determined by the 3 Bresenham segments
			 * [P1,P2], [P2,P3], [P3,P1]. 
			 * 
			 * The drawing is perfect (ie only the pixel strictly inside the triangle are drawn). 
			 **/
			template<bool blend, bool checkrange> inline void _draw_triangle_interior(iVec2 P1, iVec2 P2, iVec2 P3, RGBc fillcolor)
				{
				if (P1.Y() > P2.Y()) { mtools::swap(P1, P2); } // reorder by increasing Y value
				if (P1.Y() > P3.Y()) { mtools::swap(P1, P3); } //
				if (P2.Y() > P3.Y()) { mtools::swap(P2, P3); } //
				if (P1.Y() == P3.Y()) return; //flat, nothing to draw. 
				if (P1.Y() == P2.Y())
					{
					_fill_interior_angle<blend, checkrange>(P3, P1, P2, fillcolor, false);
					return;
					}
				if (P2.Y() == P3.Y())
					{
					_fill_interior_angle<blend, checkrange>(P1, P2, P3, fillcolor, false);
					return;
					}
				auto a1 = (P2.X() - P1.X())*(P3.Y() - P2.Y()); if (a1 < 0) a1 = -a1;
				auto a2 = (P3.X() - P2.X())*(P2.Y() - P1.Y()); if (a2 < 0) a2 = -a2;
				if (a1 > a2)
					{
					_fill_interior_angle<blend, checkrange>(P3, P2, P1, fillcolor, false);
					_fill_interior_angle<blend, checkrange>(P1, P2, P3, fillcolor, true);
					}
				else
					{
					_fill_interior_angle<blend, checkrange>(P1, P2, P3, fillcolor, false);
					_fill_interior_angle<blend, checkrange>(P3, P2, P1, fillcolor, true);
					}
				return;
				}



			/**
			* Same as above but with non integer valued lines. 
			**/
			template<bool blend, bool checkrange> inline void _draw_triangle_interior(fVec2 fP1, fVec2 fP2, fVec2 fP3, RGBc fillcolor)
				{
				if (fP1.Y() > fP2.Y()) { mtools::swap(fP1, fP2); } // reorder by increasing Y value
				if (fP1.Y() > fP3.Y()) { mtools::swap(fP1, fP3); } //
				if (fP2.Y() > fP3.Y()) { mtools::swap(fP2, fP3); } //
				int64 y1 = (int64)round(fP1.Y());
				int64 y2 = (int64)round(fP2.Y());
				int64 y3 = (int64)round(fP3.Y());
				if (y1 == y3) return; //flat, nothing to draw. 
				if (y1 == y2)
					{
					_bdir line31, line32;
					_bpos pos31, pos32;
					iVec2 P1, P2, P3;
					_init_line(fP3, fP1, line31, pos31, P3, P1);
					_init_line(fP3, fP2, line32, pos32, P3, P2);
					_fill_interior_angle<blend, checkrange>(P3, P1, P2, line31, pos31, line32, pos32, fillcolor, false);
					return;
					}
				if (y2 == y3)
					{
					_bdir line12, line13;
					_bpos pos12, pos13;
					iVec2 P1, P2, P3;
					_init_line(fP1, fP2, line12, pos12, P1, P2);
					_init_line(fP1, fP3, line13, pos13, P1, P3);
					_fill_interior_angle<blend, checkrange>(P1, P2, P3, line12, pos12, line13, pos13, fillcolor, false);
					return;
					}
				_bdir line32, line12, line31, line13, line23, line21;
				_bpos pos32, pos12, pos31, pos13, pos23, pos21;
				iVec2 P1, P2, P3;
				int64 len12 = _init_line(fP1, fP2, line12, pos12, P1, P2);
				int64 len32 = _init_line(fP3, fP2, line32, pos32, P3, P2);
				int64 len13 = _init_line(fP1, fP3, line13, pos13, P1, P3);
				line31 = line13; pos31 = pos13; _reverse_line(line31, pos31, len13);
				line21 = line12; pos21 = pos12; _reverse_line(line21, pos21, len12);
				line23 = line32; pos23 = pos32; _reverse_line(line23, pos23, len32);

				bool fl3;
				fVec2 vA = (fP3 - fP1), vB = (fP2 - fP1); 
				double det = vA.X()*vB.Y() - vB.X()*vA.Y();
				_move_line_y_dir(line23, pos23, 1);
				_move_line_y_dir(line21, pos21, 1);
				if (det < 0)
					{
					fl3 = (pos23.x < pos21.x) ? true : false;
					}
				else
					{
					fl3 = (pos23.x > pos21.x) ? true : false;
					}
				_fill_interior_angle<blend, checkrange>(P3, P2, P1, line32, pos32, line31, pos31, fillcolor, fl3);
				_fill_interior_angle<blend, checkrange>(P1, P2, P3, line12, pos12, line13, pos13, fillcolor, !fl3);
				return;
				}


			/** sub procedure called by _fill_interior_angle that does the actual filling. */
			template<bool blend, bool checkrange> void _fill_interior_angle_sub(int64 dir, int64 y, int64 ytarget, _bdir & linea, _bpos & posa, _bdir & lineb, _bpos & posb, RGBc color)
				{
				// fix the range. 
				if (dir > 0)
					{
					if (ytarget >= _ly) { ytarget = _ly; }
					if ((ytarget <= 0)||(y >= ytarget)) return;
					if (y < 0)
						{ // move y up to 0
						_move_line_y_dir(linea, posa, -y);
						_move_line_y_dir(lineb, posb, -y);
						y = 0;
						MTOOLS_ASSERT((posa.y == y) && (posb.y == y));
						}
					}
				else
					{
					if (ytarget < 0) { ytarget = -1; }
					if ((ytarget >= _ly - 1) || (y <= ytarget)) return;
					if (y > _ly - 1)
						{ // move y down to ly-1
						_move_line_y_dir(linea, posa, y - _ly + 1);
						_move_line_y_dir(lineb, posb, y - _ly + 1);
						y = _ly - 1;
						MTOOLS_ASSERT((posa.y == y) && (posb.y == y));
						}
					}
				if (linea.x_major)
					{
					if (lineb.x_major)
						{
						if (linea.stepx < 0)
							{
							if (lineb.stepx > 0)
								{
								while (y != ytarget)
									{
									_hline<blend, checkrange>(posa.x + 1, posb.x - 1, y, color);
									_move_line_y_dir<true>(linea, posa);
									_move_line_y_dir<true>(lineb, posb);
									y += dir;
									}
								}
							else
								{
								while (y != ytarget)
									{
									_move_line_y_dir<true>(lineb, posb);
									_hline<blend, checkrange>(posa.x + 1, posb.x, y, color);
									_move_line_y_dir<true>(linea, posa);
									y += dir;
									}
								}
							}
						else
							{
							if (lineb.stepx > 0)
								{
								while (y != ytarget)
									{
									_move_line_y_dir<true>(linea, posa);
									_hline<blend, checkrange>(posa.x, posb.x - 1, y, color);
									_move_line_y_dir<true>(lineb, posb);
									y += dir;
									}
								}
							else
								{
								while (y != ytarget)
									{
									_move_line_y_dir<true>(linea, posa);
									_move_line_y_dir<true>(lineb, posb);
									_hline<blend, checkrange>(posa.x, posb.x, y, color);
									y += dir;
									}
								}
							}
						}
					else
						{
						if (linea.stepx < 0)
							{
							while (y != ytarget)
								{
								_hline<blend, checkrange>(posa.x + 1, posb.x - 1, y, color);
								_move_line_y_dir<true>(linea, posa);
								_move_line_y_dir<false>(lineb, posb);
								y += dir;
								}
							}
						else
							{
							while (y != ytarget)
								{
								_move_line_y_dir<true>(linea, posa);
								_hline<blend, checkrange>(posa.x, posb.x - 1, y, color);
								_move_line_y_dir<false>(lineb, posb);
								y += dir;
								}
							}
						}
					}
				else
					{
					if (lineb.x_major)
						{
						if (lineb.stepx > 0)
							{
							while (y != ytarget)
								{
								_hline<blend, checkrange>(posa.x + 1, posb.x - 1, y, color);
								_move_line_y_dir<true>(lineb, posb);
								_move_line_y_dir<false>(linea, posa);
								y += dir;
								}
							}
						else
							{
							while (y != ytarget)
								{
								_move_line_y_dir<true>(lineb, posb);
								_hline<blend, checkrange>(posa.x + 1, posb.x, y, color);
								_move_line_y_dir<false>(linea, posa);
								y += dir;
								}
							}
						}
					else
						{
						while (y != ytarget)
							{
							_hline<blend, checkrange>(posa.x + 1, posb.x - 1, y, color);
							_move_line_y_dir<false>(lineb, posb);
							_move_line_y_dir<false>(linea, posa);
							y += dir;
							}
						}
					}
				}


			/**
			 * Fill the interior delimited by the angle <Q1,P,Q2>
			 * 
			 * The filling occurs vertically (by scanlines) so the height P.Y()
			 * should be either minimal or maximal among the 3 points
			 * 
			 * Filling is performed until height P1.Y() is reached. Parameter fill_last determined whether
			 * this last line is also filled or not.
			 * 
			 * The method perform exact filling w.r.t. the Bresenham segment [P,Q1] and [P,Q2]. so no pixel
			 * filled overlap these segment and no gap is left in between.
			 * 
			 * Optimized for speed.
			 **/
			template<bool blend, bool checkrange> MTOOLS_FORCEINLINE void _fill_interior_angle(iVec2 P, iVec2 Q1, iVec2 Q2, RGBc color,bool fill_last)
				{
				MTOOLS_ASSERT((P.Y() - Q1.Y())*(P.Y() - Q2.Y()) > 0);
				int64 dir = (P.Y() > Q1.Y()) ? -1 : 1;				// y direction 
				int64 y = P.Y();									// starting height
				int64 ytarget = Q1.Y() + dir*(fill_last ? 1 : 0);	// height to reach	
				if ((Q1.X() - P.X())*abs(Q2.Y() - P.Y())  > (Q2.X() - P.X())*abs(Q1.Y() - P.Y()))
					{ // make sure Q1 is on the left and Q2 on the right. 
					mtools::swap(Q1, Q2);
					}
				_bdir linea;
				_bpos posa;
				_init_line(P, Q1, linea, posa);
				_bdir lineb;
				_bpos posb;
				_init_line(P, Q2, lineb, posb);
				_fill_interior_angle_sub<blend, checkrange>(dir, y, ytarget, linea, posa, lineb, posb, color);
				}


			/** Same as above, but with non-integer valued points */  
			template<bool blend, bool checkrange> inline void _fill_interior_angle(iVec2 P, iVec2 Q1, iVec2 Q2, _bdir linePQ1, _bpos posPQ1, _bdir linePQ2, _bpos posPQ2, RGBc color, bool fill_last)
				{
				MTOOLS_ASSERT((P.Y() - Q1.Y())*(P.Y() - Q2.Y()) > 0);
				int64 dir = (P.Y() > Q1.Y()) ? -1 : 1;
				int64 y = P.Y();
				int64 ytarget = Q1.Y() + dir * (fill_last ? 1 : 0);
				if ((Q1.X() - P.X())*abs(Q2.Y() - P.Y())  > (Q2.X() - P.X())*abs(Q1.Y() - P.Y()))
					{ 
					mtools::swap(linePQ1, linePQ2);
					mtools::swap(posPQ1, posPQ2);
					}
				_fill_interior_angle_sub<blend,checkrange>(dir, y, ytarget, linePQ1, posPQ1, linePQ2, posPQ2, color);
				}				




			/**
			 * Draw an antialiased segment [P1,P2| using Wu algorithm.
			 * 
			 * Set draw_last to true to draw point P2 and to false to draw only the open segment.
			 **/
			template<bool blend, bool checkrange, bool usepen>  MTOOLS_FORCEINLINE void _lineWuAA(iVec2 P1, iVec2 P2, RGBc color, bool draw_last, int32 penwidth)
				{
				int64 & x0 = P1.X(); int64 & y0 = P1.Y();
				int64 & x1 = P2.X(); int64 & y1 = P2.Y();
				_updatePixel<blend, checkrange, false, usepen>(x0, y0,color, 0, penwidth);
				if (draw_last)  _updatePixel<blend, checkrange, false, usepen>(x1, y1,color, 0, penwidth);
				if (y0 > y1) { mtools::swap(y0, y1); mtools::swap(x0, x1); }
				int64 dx = x1 - x0;
				int64 dir;
				if (dx >= 0) { dir = 1; } else { dir = -1; dx = -dx; }
				int64 dy = y1 - y0;
				if (dx == 0) { while (--dy > 0) { y0++; _updatePixel<blend, checkrange, false, usepen>(x0, y0, color, 0, penwidth); } return; }
				if (dy == 0) { while (--dx > 0) { x0 += dir; _updatePixel<blend, checkrange, false, usepen>(x0, y0, color, 0, penwidth); } return; }
				if (dx == dy) { while (--dy > 0) { x0 += dir; y0++; _updatePixel<blend, checkrange, false, usepen>(x0, y0, color, 0, penwidth); } return; }
				uint32 err = 0; // important to be 32 bit, do not change !
				if (dy > dx)
					{
					uint32 inc = (uint32)((dx << 32) / dy);
					while (--dy > 0)
						{
						const uint32 tmp = err;
						err += inc;
						if (err <= tmp) { x0 += dir; } // overflow !
						y0++;
						uint32 mm = (err >> 24) + 1;
						_updatePixel<blend, checkrange, true, usepen>(x0 + dir, y0, color, mm, penwidth);
						_updatePixel<blend, checkrange, true, usepen>(x0, y0, color, 0x100 - mm, penwidth);
						}
					}
				else
					{
					uint32 inc = (uint32)((dy << 32) / dx);
					while (--dx  > 0)
						{
						const uint32 tmp = err;
						err += inc;
						if (err <= tmp) { y0++; } // overflow !
						x0 += dir;
						uint32 mm = (err >> 24) + 1;
						_updatePixel<blend, checkrange, true, usepen>(x0, y0 + 1, color, mm, penwidth);
						_updatePixel<blend, checkrange, true, usepen>(x0, y0, color, 0x100 - mm, penwidth);
						}
					}
				return; 
				}




			/**
			* Draw an antialiased line using Bresenham's algorithm.
			* The end point is drawn.
			* A little slower and 'ticker' than Wu's algorithm
			*  adapted from Alois Zingl  (http://members.chello.at/easyfilter/bresenham.html)
			**/
			template<bool blend, bool checkrange, bool usepen>  inline void _lineBresenhamAA(iVec2 P1, iVec2 P2, RGBc color, bool draw_last, int32 penwidth)
				{
				int64 & x0 = P1.X(); int64 & y0 = P1.Y();
				int64 & x1 = P2.X(); int64 & y1 = P2.Y();
				int64 sx = ((x0 < x1) ? 1 : -1), sy = ((y0 < y1) ? 1 : -1), x2;
				int64 dx = abs(x1 - x0), dy = abs(y1 - y0), err = dx*dx + dy*dy;
				int64 e2 = err == 0 ? 1 : ((int64)(0xffff7fl / sqrt(err)));
				dx *= e2; dy *= e2; err = dx - dy;
				if (draw_last)
					{
					for (; ; )
						{
						_updatePixel<blend, checkrange, true, usepen>(x0, y0, color, 256 - (int32)mtools::convertAlpha_0xFF_to_0x100((uint32)(abs(err - dx + dy) >> 16)), penwidth);
						e2 = err; x2 = x0;
						if (2 * e2 >= -dx)
							{
							if (x0 == x1) break;
							if (e2 + dy < 0xff0000l) _updatePixel<blend, checkrange, true, usepen>(x0, y0 + sy, color, 256 - (int32)mtools::convertAlpha_0xFF_to_0x100((uint32)((e2 + dy) >> 16)), penwidth);
							err -= dy; x0 += sx;
							}
						if (2 * e2 <= dy)
							{
							if (y0 == y1) break;
							if (dx - e2 < 0xff0000l) _updatePixel<blend, checkrange, true, usepen>(x2 + sx, y0, color, 256 - (int32)mtools::convertAlpha_0xFF_to_0x100((uint32)((dx - e2) >> 16)), penwidth);
							err += dx; y0 += sy;
							}
						}
					}				
				else
					{
					for (; ; )
						{
						int64 ssx = x0;
						int64 ssy = y0;
						int32 ssc = 256 - (int32)mtools::convertAlpha_0xFF_to_0x100((uint32)(abs(err - dx + dy) >> 16));
						e2 = err; x2 = x0;
						if (2 * e2 >= -dx)
							{
							if (x0 == x1) break;
							if (e2 + dy < 0xff0000l) _updatePixel<blend, checkrange, true, usepen>(x0, y0 + sy, color, 256 - (int32)mtools::convertAlpha_0xFF_to_0x100((uint32)((e2 + dy) >> 16)), penwidth);
							err -= dy; x0 += sx;
							}
						if (2 * e2 <= dy)
							{
							if (y0 == y1) break;
							if (dx - e2 < 0xff0000l) _updatePixel<blend, checkrange, true, usepen>(x2 + sx, y0, color, 256 - (int32)mtools::convertAlpha_0xFF_to_0x100((uint32)((dx - e2) >> 16)), penwidth);
							err += dx; y0 += sy;
							}
						_updatePixel<blend, checkrange, true, usepen>(ssx, ssy, color, ssc, penwidth);
						}
					}
				}


			/**
			* Draw an tick antialiased line using Bresenham's algorithm.
			* 
			* TODO : NOT VERY GOOD, IMPROVE IT. 
			* 
			*  adapted from Alois Zingl  (http://members.chello.at/easyfilter/bresenham.html) 
			**/
				/*
			template<bool blend, bool checkrange>  inline void _tickLineBresenhamAA(iVec2 P1, iVec2 P2, float wd, RGBc color)
				{
				int64 & x0 = P1.X(); int64 & y0 = P1.Y();
				int64 & x1 = P2.X(); int64 & y1 = P2.Y();
				int64 dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
				int64 dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
				int64 err = dx - dy, e2, x2, y2;
				float ed = dx + dy == 0 ? 1 : sqrt((float)dx*dx + (float)dy*dy);
				int64 op =color.opacityInt();
				if (op == 256)
					{
					for (wd = (wd + 1) / 2; ; )
						{
						color.comp.A = (ui(255 - std::max<float>(0, 255 * (abs(err - dx + dy) / ed - wd + 1))); // BEWARE : does not work anymore with premultiplied alpha
						_updatePixel<blend, checkrange>(x0, y0, color);
						e2 = err; x2 = x0;
						if (2 * e2 >= -dx)
							{
							for (e2 += dy, y2 = y0; e2 < ed*wd && (y1 != y2 || dx > dy); e2 += dx)
								{
								color.comp.A = (uint8)(255 - std::max<float>(0, 255 * (abs(e2) / ed - wd + 1))); // BEWARE : does not work anymore with premultiplied alpha
								_updatePixel<blend, checkrange>(x0, y2 += sy, color);
								}
							if (x0 == x1) break;
							e2 = err; err -= dy; x0 += sx;
							}
						if (2 * e2 <= dy)
							{
							for (e2 = dx - e2; e2 < ed*wd && (x1 != x2 || dx < dy); e2 += dy)
								{
								color.comp.A = (uint8)(255 - std::max<float>(0, 255 * (abs(e2) / ed - wd + 1))); // BEWARE : does not work anymore with premultiplied alpha
								_updatePixel<blend, checkrange>(x2 += sx, y0, color);
								}
							if (y0 == y1) break;
							err += dx; y0 += sy;
							}
						}
					return;
					}
				else
					{
					for (wd = (wd + 1) / 2; ; )
						{
						color.comp.A = (uint8)((((int32)(255 - std::max<float>(0, 255 * (abs(err - dx + dy) / ed - wd + 1)))) * op) >> 8); // BEWARE : does not work anymore with premultiplied alpha
						_updatePixel<blend, checkrange>(x0, y0, color);
						e2 = err; x2 = x0;
						if (2 * e2 >= -dx)
							{
							for (e2 += dy, y2 = y0; e2 < ed*wd && (y1 != y2 || dx > dy); e2 += dx)
								{
								color.comp.A = (uint8)((((int32)(255 - std::max<float>(0, 255 * (abs(e2) / ed - wd + 1))))*op) >> 8); // BEWARE : does not work anymore with premultiplied alpha
								_updatePixel<blend, checkrange>(x0, y2 += sy, color);
								}
							if (x0 == x1) break;
							e2 = err; err -= dy; x0 += sx;
							}
						if (2 * e2 <= dy)
							{
							for (e2 = dx - e2; e2 < ed*wd && (x1 != x2 || dx < dy); e2 += dy)
								{
								color.comp.A = (uint8)((((int32)(255 - std::max<float>(0, 255 * (abs(e2) / ed - wd + 1))))*op) >> 8); // BEWARE : does not work anymore with premultiplied alpha
								_updatePixel<blend, checkrange>(x2 += sx, y0, color);
								}
							if (y0 == y1) break;
							err += dx; y0 += sy;
							}
						}
					return;
					}
				}
			*/


			/**
			 * Plot a limited quadratic Bezier segment, used by _plotQuadBezier.
			 * adapted from Alois Zingl  (http://members.chello.at/easyfilter/bresenham.html)
			 * change: do not draw the endpoint (x2,y2)
			 **/
			template<bool blend, bool checkrange, bool usepen> void _plotQuadBezierSeg(int64 x0, int64 y0, int64 x1, int64 y1, int64 x2, int64 y2, RGBc color, int32 penwidth)
				{           
				if ((x0 == x2) && (y0 == y2)) return;
				int64 sx = x2 - x1, sy = y2 - y1;
				int64 xx = x0 - x1, yy = y0 - y1, xy;
				double dx, dy, err, cur = (double)(xx*sy - yy*sx);
				if (cur == 0)
					{
					_lineBresenham<blend, checkrange, false, usepen, false, false>({ x0, y0 }, { x2, y2 }, color, false, penwidth, 0);
					return;
					}
				bool sw = false;
				if (sx*sx + sy*sy > xx*xx + yy*yy) 
					{
					x2 = x0; x0 = sx + x1; y2 = y0; y0 = sy + y1; cur = -cur;
					sw = true;
					}
				xx += sx; xx *= sx = x0 < x2 ? 1 : -1;
				yy += sy; yy *= sy = y0 < y2 ? 1 : -1;
				xy = 2 * xx*yy; xx *= xx; yy *= yy;
				if (cur*sx*sy < 0) { xx = -xx; yy = -yy; xy = -xy; cur = -cur; }
				dx = 4.0*sy*cur*(x1 - x0) + xx - xy;
				dy = 4.0*sx*cur*(y0 - y1) + yy - xy;
				xx += xx; yy += yy; err = dx + dy + xy;
				if (sw)
					{ // skip first if swaped
					y1 = 2 * err < dx;
					if (2 * err > dy) { x0 += sx; dx -= xy; err += dy += yy; }
					if (y1) { y0 += sy; dy -= xy; err += dx += xx; }
					}
				while (dy < 0 && dx > 0)
					{
					if (x0 == x2 && y0 == y2)
						{ 
						if (sw) _updatePixel<blend, checkrange, false, usepen>(x0, y0, color, 0, penwidth); // write last if swaped
						return;
						}
					_updatePixel<blend, checkrange, false, usepen>(x0, y0, color, 0, penwidth);
					y1 = 2 * err < dx;
					if (2 * err > dy) { x0 += sx; dx -= xy; err += dy += yy; }
					if (y1) { y0 += sy; dy -= xy; err += dx += xx; }
					}
				_lineBresenham<blend, checkrange, false, usepen, false, false>({ x0, y0 }, { x2, y2 }, color, sw, penwidth, 0);
				}


			/**
			* Plot a limited anti-aliased quadratic Bezier segment, used by _plotQuadBezier.
			* adapted from Alois Zingl  (http://members.chello.at/easyfilter/bresenham.html)
			* change: do not draw the endpoint (x2,y2)
			**/
			template<bool blend, bool checkrange, bool usepen> void _plotQuadBezierSegAA(int64 x0, int64 y0, int64 x1, int64 y1, int64 x2, int64 y2, RGBc color, int32 penwidth)
				{
				if ((x0 == x2) && (y0 == y2)) return;
				int64 sx = x2 - x1, sy = y2 - y1;
				int64 xx = x0 - x1, yy = y0 - y1, xy;
				double dx, dy, err, ed, cur = (double)(xx*sy - yy*sx);
				if (cur == 0)
					{
					_lineBresenhamAA<blend, checkrange, usepen>({ x0, y0 }, { x2, y2 }, color, false, penwidth);
					return;
					}
				bool sw = false;
				if (sx*sx + sy*sy > xx*xx + yy*yy)
					{
					x2 = x0; x0 = sx + x1; y2 = y0; y0 = sy + y1; cur = -cur;
					sw = true;
					}
				xx += sx; xx *= sx = x0 < x2 ? 1 : -1;
				yy += sy; yy *= sy = y0 < y2 ? 1 : -1;
				xy = 2 * xx*yy; xx *= xx; yy *= yy;
				if (cur*sx*sy < 0) 
					{
					xx = -xx; yy = -yy; xy = -xy; cur = -cur;
					}
				dx = 4.0*sy*(x1 - x0)*cur + xx - xy;
				dy = 4.0*sx*(y0 - y1)*cur + yy - xy;
				xx += xx; yy += yy; err = dx + dy + xy;
				if (sw)
					{
					cur = fmin(dx + xy, -xy - dy);
					ed = fmax(dx + xy, -xy - dy);
					ed += 2 * ed*cur*cur / (4 * ed*ed + cur*cur);
					x1 = x0; cur = dx - err; y1 = 2 * err + dy < 0;
					if (2 * err + dx > 0) { x0 += sx; dx -= xy; err += dy += yy; }
					if (y1) { y0 += sy; dy -= xy; err += dx += xx; }
					}
				while (dy < dx) 
					{
					cur = fmin(dx + xy, -xy - dy);
					ed = fmax(dx + xy, -xy - dy);
					ed += 2 * ed*cur*cur / (4 * ed*ed + cur*cur);
					if (x0 == x2 || y0 == y2) { break; }
					_updatePixel<blend, checkrange, true, usepen>(x0, y0, color, (int32)(256 - 256*fabs(err - dx - dy - xy)/ed), penwidth);
					x1 = x0; cur = dx - err; y1 = 2 * err + dy < 0;
					if (2 * err + dx > 0) 
						{
						if (err - dy < ed) _updatePixel<blend, checkrange, true, usepen>(x0, y0 + sy, color, (int32)(256 - 256*fabs(err - dy)/ed), penwidth);
						x0 += sx; dx -= xy; err += dy += yy;
						}
					if (y1) 
						{
						if (cur < ed) _updatePixel<blend, checkrange, true, usepen>(x1 + sx, y0, color, (int32)(256 - 256*fabs(cur)/ed), penwidth);
						y0 += sy; dy -= xy; err += dx += xx;
						}
					}
				_lineBresenhamAA<blend, checkrange, usepen>({ x0, y0 }, { x2, y2 }, color, sw, penwidth);
				}


			/**
			 * Plot any quadratic Bezier curve 
		 	 * adapted from Alois Zingl  (http://members.chello.at/easyfilter/bresenham.html) 
			 * change: do not draw always the endpoint (x2,y2)
			 **/
			template<bool blend, bool checkrange, bool antialiasing, bool usepen> void _plotQuadBezier(int64 x0, int64 y0, int64 x1, int64 y1, int64 x2, int64 y2, RGBc color, bool draw_P2, int32 penwidth)
				{  
				if (checkrange)
					{ // check if we can discard the whole curve
					iBox2 mbr({ x0,y0 });
					mbr.swallowPoint({ x1,y1 });
					mbr.swallowPoint({ x2,y2 });
					if ((usepen) && (penwidth > 0)) mbr.enlarge(penwidth);
					if (intersectionRect(mbr, iBox2(0, _lx - 1, 0, _ly - 1)).isEmpty()) return;  // nothing to draw
					}
				if (draw_P2) { _updatePixel<blend, checkrange, false, usepen>(x2, y2, color, 0, penwidth); }
				if ((x0 == x2) && (y0 == y2)) { return; }
				int64 x = x0 - x1, y = y0 - y1;
				double t = (double)(x0 - 2 * x1 + x2), r;
				if (x*(x2 - x1) > 0) 
					{
					if (y*(y2 - y1) > 0)
						if (fabs((y0 - 2 * y1 + y2) / t*x) > abs(y)) 
							{
							x0 = x2; x2 = x + x1; y0 = y2; y2 = y + y1;
							}
					t = (x0 - x1) / t;
					r = (1 - t)*((1 - t)*y0 + 2.0*t*y1) + t*t*y2;
					t = (x0*x2 - x1*x1)*t / (x0 - x1);
					x = (int64)floor(t + 0.5); y = (int64)floor(r + 0.5);
					r = (y1 - y0)*(t - x0) / (x1 - x0) + y0;
					if (antialiasing) { _plotQuadBezierSegAA<blend, checkrange, usepen>(x0, y0, x, (int64)floor(r + 0.5), x, y, color, penwidth); } else { _plotQuadBezierSeg<blend, checkrange, usepen>(x0, y0, x, (int64)floor(r + 0.5), x, y, color, penwidth); }
					r = (y1 - y2)*(t - x2) / (x1 - x2) + y2;
					x0 = x1 = x; y0 = y; y1 = (int64)floor(r + 0.5);
					}
				if ((y0 - y1)*(y2 - y1) > 0) 
					{
					t = (double)(y0 - 2 * y1 + y2); t = (y0 - y1) / t;
					r = (1 - t)*((1 - t)*x0 + 2.0*t*x1) + t*t*x2;
					t = (y0*y2 - y1*y1)*t / (y0 - y1);
					x = (int64)floor(r + 0.5); y = (int64)floor(t + 0.5);
					r = (x1 - x0)*(t - y0) / (y1 - y0) + x0;
					if (antialiasing) { _plotQuadBezierSegAA<blend, checkrange, usepen>(x0, y0, (int64)floor(r + 0.5), y, x, y, color, penwidth); } else { _plotQuadBezierSeg<blend, checkrange, usepen>(x0, y0, (int64)floor(r + 0.5), y, x, y, color, penwidth); }
					r = (x1 - x2)*(t - y2) / (y1 - y2) + x2;
					x0 = x; x1 = (int64)floor(r + 0.5); y0 = y1 = y;
					}
				if (antialiasing) { _plotQuadBezierSegAA<blend, checkrange, usepen>(x0, y0, x1, y1, x2, y2, color, penwidth); } else { _plotQuadBezierSeg<blend, checkrange, usepen>(x0, y0, x1, y1, x2, y2, color, penwidth); }
				}


			/**
			* draw a limited rational Bezier segment, squared weight. Used by _plotQuadRationalBezier().
			* adapted from Alois Zingl  (http://members.chello.at/easyfilter/bresenham.html)
			* change: do not draw the endpoint (x2,y2)
			**/
			template<bool blend, bool checkrange, bool usepen> void _plotQuadRationalBezierSeg(int64 x0, int64 y0, int64 x1, int64 y1, int64 x2, int64 y2, double w, RGBc color, int32 penwidth)
				{
				if ((x0 == x2) && (y0 == y2)) { return; }
				int64 sx = x2 - x1, sy = y2 - y1;
				double dx = (double)(x0 - x2), dy = (double)(y0 - y2), xx = (double)(x0 - x1), yy = (double)(y0 - y1);
				double xy = xx*sy + yy*sx, cur = xx*sy - yy*sx, err;
				if ((cur == 0) || (w <= 0))
					{
					_lineBresenham<blend, checkrange, false, usepen, false, false>({ x0, y0 }, { x2, y2 }, color, false, penwidth, 0);
					return;
					}
				bool sw = false;
				if (sx*sx + sy*sy > xx*xx + yy*yy) 
					{
					x2 = x0; x0 -= (int64)dx; y2 = y0; y0 -= (int64)dy; cur = -cur;
					sw = true;
					}
				xx = 2.0*(4.0*w*sx*xx + dx*dx);
				yy = 2.0*(4.0*w*sy*yy + dy*dy);
				sx = x0 < x2 ? 1 : -1;
				sy = y0 < y2 ? 1 : -1;
				xy = -2.0*sx*sy*(2.0*w*xy + dx*dy);
				if (cur*sx*sy < 0.0) { xx = -xx; yy = -yy; xy = -xy; cur = -cur; }
				dx = 4.0*w*(x1 - x0)*sy*cur + xx / 2.0 + xy;
				dy = 4.0*w*(y0 - y1)*sx*cur + yy / 2.0 + xy;
				if (w < 0.5 && (dy > xy || dx < xy)) 
					{
					cur = (w + 1.0) / 2.0; w = sqrt(w); xy = 1.0 / (w + 1.0);
					sx = (int64)floor((x0 + 2.0*w*x1 + x2)*xy / 2.0 + 0.5);
					sy = (int64)floor((y0 + 2.0*w*y1 + y2)*xy / 2.0 + 0.5);
					dx = floor((w*x1 + x0)*xy + 0.5); dy = floor((y1*w + y0)*xy + 0.5);
					if (sw)
						{
						_plotQuadRationalBezierSeg<blend, checkrange, usepen>(sx,sy, (int64)dx, (int64)dy, x0, y0, cur, color, penwidth);
						dx = floor((w*x1 + x2)*xy + 0.5); dy = floor((y1*w + y2)*xy + 0.5);
						_plotQuadRationalBezierSeg<blend, checkrange, usepen>(x2, y2, (int64)dx, (int64)dy, sx, sy, cur, color, penwidth);
						}
					else
						{
						_plotQuadRationalBezierSeg<blend, checkrange, usepen>(x0, y0, (int64)dx, (int64)dy, sx, sy, cur, color, penwidth);
						dx = floor((w*x1 + x2)*xy + 0.5); dy = floor((y1*w + y2)*xy + 0.5);
						_plotQuadRationalBezierSeg<blend, checkrange, usepen>(sx, sy, (int64)dx, (int64)dy, x2, y2, cur, color, penwidth);
						}
					return;
					}
				err = dx + dy - xy;
				if (sw)
					{
					x1 = 2 * err > dy; y1 = 2 * (err + yy) < -dy;
					if (2 * err < dx || y1) { y0 += sy; dy += xy; err += dx += xx; }
					if (2 * err > dx || x1) { x0 += sx; dx += xy; err += dy += yy; }
					}
				while (dy <= xy && dx >= xy)
					{
					if (x0 == x2 && y0 == y2)
						{
						if (sw) _updatePixel<blend, checkrange, false, usepen>(x0, y0, color,0, penwidth); // write last if swaped
						return;
						}
					_updatePixel<blend, checkrange, false, usepen>(x0, y0, color,0,penwidth);
					x1 = 2 * err > dy; y1 = 2 * (err + yy) < -dy;
					if (2 * err < dx || y1) { y0 += sy; dy += xy; err += dx += xx; }
					if (2 * err > dx || x1) { x0 += sx; dx += xy; err += dy += yy; }
					}
				_lineBresenham<blend, checkrange, false, usepen, false, false>({ x0, y0 }, { x2, y2 }, color, sw, penwidth, 0);
				}



			/**
			* draw an anti-aliased limited rational Bezier segment, squared weight. Used by _plotQuadRationalBezier().
			* adapted from Alois Zingl  (http://members.chello.at/easyfilter/bresenham.html)
			* change: do not draw the endpoint (x2,y2)
			**/
			template<bool blend, bool checkrange, bool usepen> void _plotQuadRationalBezierSegAA(int64 x0, int64 y0, int64 x1, int64 y1, int64 x2, int64 y2, double w, RGBc color, int32 penwidth)
				{
				if ((x0 == x2) && (y0 == y2)) { return; }
				int64 sx = x2 - x1, sy = y2 - y1;
				double dx = (double)(x0 - x2), dy = (double)(y0 - y2), xx = (double)(x0 - x1), yy = (double)(y0 - y1);
				double xy = xx*sy + yy*sx, cur = xx*sy - yy*sx, err, ed;
				bool f;
				if ((cur == 0) || (w <= 0.0))
					{
					_lineBresenhamAA<blend, checkrange, usepen>({ x0, y0 }, { x2, y2 }, color, false, penwidth);
					return;
					}
				bool sw = false;
				if (sx*sx + sy*sy > xx*xx + yy*yy) 
					{
					x2 = x0; x0 -= (int64)dx; y2 = y0; y0 -= (int64)dy; cur = -cur;
					sw = true;
					}
				xx = 2.0*(4.0*w*sx*xx + dx*dx);
				yy = 2.0*(4.0*w*sy*yy + dy*dy);
				sx = x0 < x2 ? 1 : -1;
				sy = y0 < y2 ? 1 : -1;
				xy = -2.0*sx*sy*(2.0*w*xy + dx*dy);
				if (cur*sx*sy < 0) { xx = -xx; yy = -yy; cur = -cur; xy = -xy; }
				dx = 4.0*w*(x1 - x0)*sy*cur + xx / 2.0 + xy;
				dy = 4.0*w*(y0 - y1)*sx*cur + yy / 2.0 + xy;
				if (w < 0.5 && dy > dx) 
					{
					cur = (w + 1.0) / 2.0; w = sqrt(w); xy = 1.0 / (w + 1.0);
					sx = (int64)floor((x0 + 2.0*w*x1 + x2)*xy / 2.0 + 0.5);
					sy = (int64)floor((y0 + 2.0*w*y1 + y2)*xy / 2.0 + 0.5);
					dx = floor((w*x1 + x0)*xy + 0.5); dy = floor((y1*w + y0)*xy + 0.5);
					if (sw)
						{
						_plotQuadRationalBezierSegAA<blend, checkrange, usepen>(sx, sy, (int64)dx, (int64)dy, x0, y0, cur, color, penwidth);
						dx = floor((w*x1 + x2)*xy + 0.5); dy = floor((y1*w + y2)*xy + 0.5);
						_plotQuadRationalBezierSegAA<blend, checkrange, usepen>(x2, y2, (int64)dx, (int64)dy, sx, sy, cur, color, penwidth);
						}
					else
						{
						_plotQuadRationalBezierSegAA<blend, checkrange, usepen>(x0, y0, (int64)dx, (int64)dy, sx, sy, cur, color, penwidth);
						dx = floor((w*x1 + x2)*xy + 0.5); dy = floor((y1*w + y2)*xy + 0.5);
						_plotQuadRationalBezierSegAA<blend, checkrange, usepen>(sx, sy, (int64)dx, (int64)dy, x2, y2, cur, color, penwidth);
						}
					return;
					}
				err = dx + dy - xy;
				if (sw)
					{
					cur = fmin(dx - xy, xy - dy); ed = fmax(dx - xy, xy - dy);
					ed += 2 * ed*cur*cur / (4.*ed*ed + cur*cur);
					x1 = (int64)(256 * fabs(err - dx - dy + xy) / ed);
					f = (2 * err + dy < 0);
					if (2 * err + dx > 0) { x0 += sx; dx += xy; err += dy += yy; }
					if (f) { y0 += sy; dy += xy; err += dx += xx; }
					}
				while (dy < dx)
					{
					cur = fmin(dx - xy, xy - dy); ed = fmax(dx - xy, xy - dy);
					ed += 2 * ed*cur*cur / (4.*ed*ed + cur*cur);
					x1 = (int64)(256 * fabs(err - dx - dy + xy) / ed);
					if ((x0 == x2) && (y0 == y2))
						{
						if (sw) _updatePixel<blend, checkrange, false, usepen>(x0, y0, color, 0, penwidth); // write last if swaped
						return;
						}
					if (x1 <= 256) _updatePixel<blend, checkrange, true, usepen>(x0, y0, color, (int32)(256 - x1), penwidth);
					f = (2 * err + dy < 0);
					if (f)  
						{
						if (y0 == y2) return;
						if (dx - err < ed) _updatePixel<blend, checkrange, true, usepen>(x0 + sx, y0, color, 256 - (int32)(256 * fabs(dx - err) / ed), penwidth);
						}
					if (2 * err + dx > 0) 
						{
						if (x0 == x2) return;
						if (err - dy < ed) _updatePixel<blend, checkrange, true, usepen>(x0, y0 + sy, color, 256 - (int32)(256 * fabs(err - dy) / ed), penwidth);
						x0 += sx; dx += xy; err += dy += yy;
						}
					if (f) { y0 += sy; dy += xy; err += dx += xx; }
					}				
				_lineBresenhamAA<blend, checkrange, usepen>({ x0, y0 }, { x2, y2 }, color, sw, penwidth);
				}




			/**
			* Plot any quadratic rational Bezier curve.
			* adapted from Alois Zingl  (http://members.chello.at/easyfilter/bresenham.html)
			* change: do not draw always the endpoint (x2,y2)
			**/
			template<bool blend, bool checkrange, bool antialiasing, bool usepen> void _plotQuadRationalBezier(int64 x0, int64 y0, int64 x1, int64 y1, int64 x2, int64 y2, double w, RGBc color, bool draw_P2, int32 penwidth)
				{
				if (checkrange)
					{ // check if we can discard the whole curve
					iBox2 mbr({ x0,y0 });
					mbr.swallowPoint({ x1,y1 });
					mbr.swallowPoint({ x2,y2 });
					if ((usepen) && (penwidth > 0)) mbr.enlarge(penwidth);
					if (intersectionRect(mbr, iBox2(0, _lx - 1, 0, _ly - 1)).isEmpty()) return;  // nothing to draw
					}
				if (draw_P2) { _updatePixel<blend, checkrange, false, usepen>(x2, y2, color, 0, penwidth); }
				if ((x0 == x2) && (y0 == y2)) { return; }
				int64 x = x0 - 2 * x1 + x2, y = y0 - 2 * y1 + y2;
				double xx = (double)(x0 - x1), yy = (double)(y0 - y1), ww, t, q;
				if (xx*(x2 - x1) > 0) 
					{
					if (yy*(y2 - y1) > 0)
						if (fabs(xx*y) > fabs(yy*x)) 
							{
							x0 = x2; x2 = (int64)(xx + x1); y0 = y2; y2 = (int64)(yy + y1);
							}
					if (x0 == x2 || w == 1.0) t = (x0 - x1) / (double)x;
					else 
						{
						q = sqrt(4.0*w*w*(x0 - x1)*(x2 - x1) + (x2 - x0)*(x2 - x0));
						if (x1 < x0) q = -q;
						t = (2.0*w*(x0 - x1) - x0 + x2 + q) / (2.0*(1.0 - w)*(x2 - x0));
						}
					q = 1.0 / (2.0*t*(1.0 - t)*(w - 1.0) + 1.0);
					xx = (t*t*(x0 - 2.0*w*x1 + x2) + 2.0*t*(w*x1 - x0) + x0)*q;
					yy = (t*t*(y0 - 2.0*w*y1 + y2) + 2.0*t*(w*y1 - y0) + y0)*q;
					ww = t*(w - 1.0) + 1.0; ww *= ww*q;
					w = ((1.0 - t)*(w - 1.0) + 1.0)*sqrt(q);
					x = (int64)floor(xx + 0.5); y = (int64)floor(yy + 0.5);
					yy = (xx - x0)*(y1 - y0) / (x1 - x0) + y0;
					if (antialiasing) { _plotQuadRationalBezierSegAA<blend, checkrange, usepen>(x0, y0, x, (int64)floor(yy + 0.5), x, y, ww, color, penwidth); } else { _plotQuadRationalBezierSeg<blend, checkrange, usepen>(x0, y0, x, (int64)floor(yy + 0.5), x, y, ww, color, penwidth); }
					yy = (xx - x2)*(y1 - y2) / (x1 - x2) + y2;
					y1 = (int64)floor(yy + 0.5); x0 = x1 = x; y0 = y;
					}
				if ((y0 - y1)*(y2 - y1) > 0) 
					{
					if (y0 == y2 || w == 1.0) t = (y0 - y1) / (y0 - 2.0*y1 + y2);
					else 
						{
						q = sqrt(4.0*w*w*(y0 - y1)*(y2 - y1) + (y2 - y0)*(y2 - y0));
						if (y1 < y0) q = -q;
						t = (2.0*w*(y0 - y1) - y0 + y2 + q) / (2.0*(1.0 - w)*(y2 - y0));
						}
					q = 1.0 / (2.0*t*(1.0 - t)*(w - 1.0) + 1.0);
					xx = (t*t*(x0 - 2.0*w*x1 + x2) + 2.0*t*(w*x1 - x0) + x0)*q;
					yy = (t*t*(y0 - 2.0*w*y1 + y2) + 2.0*t*(w*y1 - y0) + y0)*q;
					ww = t*(w - 1.0) + 1.0; ww *= ww*q;
					w = ((1.0 - t)*(w - 1.0) + 1.0)*sqrt(q);
					x = (int64)floor(xx + 0.5); y = (int64)floor(yy + 0.5);
					xx = (x1 - x0)*(yy - y0) / (y1 - y0) + x0;
					if (antialiasing) { _plotQuadRationalBezierSegAA<blend, checkrange, usepen>(x0, y0, (int64)floor(xx + 0.5), y, x, y, ww, color, penwidth); } else { _plotQuadRationalBezierSeg<blend, checkrange, usepen>(x0, y0, (int64)floor(xx + 0.5), y, x, y, ww, color, penwidth); }
					xx = (x1 - x2)*(yy - y2) / (y1 - y2) + x2;
					x1 = (int64)floor(xx + 0.5); x0 = x; y0 = y1 = y;
					}
				if (antialiasing) { _plotQuadRationalBezierSegAA<blend, checkrange, usepen>(x0, y0, x1, y1, x2, y2, w*w, color, penwidth); } else { _plotQuadRationalBezierSeg<blend, checkrange, usepen>(x0, y0, x1, y1, x2, y2, w*w, color, penwidth); }
				}



			/**
			* Plot a limited cubic Bezier segment.
			* adapted from Alois Zingl  (http://members.chello.at/easyfilter/bresenham.html)
			* change: do not draw always the endpoint (x2,y2)
			**/
			template<bool blend, bool checkrange, bool usepen> void _plotCubicBezierSeg(int64 x0, int64 y0, double x1, double y1, double x2, double y2, int64 x3, int64 y3, RGBc color, int32 penwidth)
				{
				if ((x0 == x3) && (y0 == y3)) return;
				int64 sax3 = x3, say3 = y3;
				int64 f, fx, fy, leg = 1;
				int64 sx = x0 < x3 ? 1 : -1, sy = y0 < y3 ? 1 : -1;
				double xc = -fabs(x0 + x1 - x2 - x3), xa = xc - 4 * sx*(x1 - x2), xb = sx*(x0 - x1 - x2 + x3);
				double yc = -fabs(y0 + y1 - y2 - y3), ya = yc - 4 * sy*(y1 - y2), yb = sy*(y0 - y1 - y2 + y3);
				double ab, ac, bc, cb, xx, xy, yy, dx, dy, ex, *pxy, EP = 0.01;
				if (xa == 0 && ya == 0)
					{
					sx = (int64)floor((3 * x1 - x0 + 1) / 2); sy = (int64)floor((3 * y1 - y0 + 1) / 2);
					_plotQuadBezierSeg<blend, checkrange, usepen>(x0, y0, sx, sy, x3, y3,color, penwidth);
					return;
					}
				x1 = (x1 - x0)*(x1 - x0) + (y1 - y0)*(y1 - y0) + 1;
				x2 = (x2 - x3)*(x2 - x3) + (y2 - y3)*(y2 - y3) + 1;
				do {
					ab = xa*yb - xb*ya; ac = xa*yc - xc*ya; bc = xb*yc - xc*yb;
					ex = ab*(ab + ac - 3 * bc) + ac*ac;
					f = (ex > 0) ? 1 : (int64)sqrt(1 + 1024 / x1);
					ab *= f; ac *= f; bc *= f; ex *= f*f;
					xy = 9 * (ab + ac + bc) / 8; cb = 8 * (xa - ya);
					dx = 27 * (8 * ab*(yb*yb - ya*yc) + ex*(ya + 2 * yb + yc)) / 64 - ya*ya*(xy - ya);
					dy = 27 * (8 * ab*(xb*xb - xa*xc) - ex*(xa + 2 * xb + xc)) / 64 - xa*xa*(xy + xa);
					xx = 3 * (3 * ab*(3 * yb*yb - ya*ya - 2 * ya*yc) - ya*(3 * ac*(ya + yb) + ya*cb)) / 4;
					yy = 3 * (3 * ab*(3 * xb*xb - xa*xa - 2 * xa*xc) - xa*(3 * ac*(xa + xb) + xa*cb)) / 4;
					xy = xa*ya*(6 * ab + 6 * ac - 3 * bc + cb); ac = ya*ya; cb = xa*xa;
					xy = 3 * (xy + 9 * f*(cb*yb*yc - xb*xc*ac) - 18 * xb*yb*ab) / 8;
					if (ex < 0) { dx = -dx; dy = -dy; xx = -xx; yy = -yy; xy = -xy; ac = -ac; cb = -cb; }
					ab = 6 * ya*ac; ac = -6 * xa*ac; bc = 6 * ya*cb; cb = -6 * xa*cb;
					dx += xy; ex = dx + dy; dy += xy; /* error of 1st step */
					for (pxy = &xy, fx = fy = f; x0 != x3 && y0 != y3; )
						{
						if ((x0 != sax3) || (y0 != say3)) _updatePixel<blend, checkrange, false, usepen>(x0, y0, color, 0, penwidth);
						do {
							if (dx > *pxy || dy < *pxy) goto exit;
							y1 = 2 * ex - dy;
							if (2 * ex >= dx) { fx--; ex += dx += xx; dy += xy += ac; yy += bc; xx += ab; }
							if (y1 <= 0) { fy--; ex += dy += yy; dx += xy += bc; xx += ac; yy += cb; }
							}
						while (fx > 0 && fy > 0);
							if (2 * fx <= f) { x0 += sx; fx += f; }
							if (2 * fy <= f) { y0 += sy; fy += f; }
							if (pxy == &xy && dx < 0 && dy > 0) pxy = &EP;
						}
				exit:
					xx = (double)x0; x0 = x3; x3 = (int64)xx; sx = -sx; xb = -xb;
					yy = (double)y0; y0 = y3; y3 = (int64)yy; sy = -sy; yb = -yb; x1 = x2;
					}
				while (leg--);
				if ((x0 == sax3) && (y0 == say3)) { _lineBresenham<blend, checkrange, false, usepen, false, false>({ x3, y3 }, { x0, y0 }, color, false, penwidth, 0); }
				else if ((x3 == sax3) && (y3 == say3)) { _lineBresenham<blend, checkrange, false, usepen, false, false>({ x0, y0 }, { x3, y3 }, color, false, penwidth, 0); }
				else _lineBresenham<blend, checkrange, false, usepen, false, false>({ x0, y0 }, { x3, y3 }, color, true, penwidth, 0);
				}


			/**
			* Plot a limited anti-aliased cubic Bezier segment.
			* adapted from Alois Zingl  (http://members.chello.at/easyfilter/bresenham.html)
			* change: do not draw always the endpoint (x2,y2)
			**/
			template<bool blend, bool checkrange, bool usepen> void _plotCubicBezierSegAA(int64 x0, int64 y0, double x1, double y1, double x2, double y2, int64 x3, int64 y3, RGBc color, int32 penwidth)
				{
				if ((x0 == x3) && (y0 == y3)) return;
				int64 sax3 = x3, say3 = y3;
				int64 f, fx, fy, leg = 1;
				int64 sx = x0 < x3 ? 1 : -1, sy = y0 < y3 ? 1 : -1;
				double xc = -fabs(x0 + x1 - x2 - x3), xa = xc - 4 * sx*(x1 - x2), xb = sx*(x0 - x1 - x2 + x3);
				double yc = -fabs(y0 + y1 - y2 - y3), ya = yc - 4 * sy*(y1 - y2), yb = sy*(y0 - y1 - y2 + y3);
				double ab, ac, bc, ba, xx, xy, yy, dx, dy, ex, px, py, ed, ip, EP = 0.01;
				if (xa == 0 && ya == 0)
					{
					sx = (int64)floor((3 * x1 - x0 + 1) / 2); sy = (int64)floor((3 * y1 - y0 + 1) / 2);
					_plotQuadBezierSegAA<blend, checkrange, usepen>(x0, y0, sx, sy, x3, y3, color, penwidth);
					return;
					}
				x1 = (x1 - x0)*(x1 - x0) + (y1 - y0)*(y1 - y0) + 1;
				x2 = (x2 - x3)*(x2 - x3) + (y2 - y3)*(y2 - y3) + 1;
				do {
					ab = xa*yb - xb*ya; ac = xa*yc - xc*ya; bc = xb*yc - xc*yb;
					ip = 4 * ab*bc - ac * ac; 
					if (ip < 0) ip = -ip;																// BUGFIX : make it positive
					ex = ab*(ab + ac - 3 * bc) + ac*ac;
					f = ((ex > 0) ? 1 : (int64)sqrt(1 + 1024 / x1));
					ab *= f; ac *= f; bc *= f; ex *= f*f;
					xy = 9 * (ab + ac + bc) / 8; ba = 8 * (xa - ya);
					dx = 27 * (8 * ab*(yb*yb - ya*yc) + ex*(ya + 2 * yb + yc)) / 64 - ya*ya*(xy - ya);
					dy = 27 * (8 * ab*(xb*xb - xa*xc) - ex*(xa + 2 * xb + xc)) / 64 - xa*xa*(xy + xa);
					xx = 3 * (3 * ab*(3 * yb*yb - ya*ya - 2 * ya*yc) - ya*(3 * ac*(ya + yb) + ya*ba)) / 4;
					yy = 3 * (3 * ab*(3 * xb*xb - xa*xa - 2 * xa*xc) - xa*(3 * ac*(xa + xb) + xa*ba)) / 4;
					xy = xa*ya*(6 * ab + 6 * ac - 3 * bc + ba); ac = ya*ya; ba = xa*xa;
					xy = 3 * (xy + 9 * f*(ba*yb*yc - xb*xc*ac) - 18 * xb*yb*ab) / 8;
					if (ex < 0) { dx = -dx; dy = -dy; xx = -xx; yy = -yy; xy = -xy; ac = -ac; ba = -ba; }
					ab = 6 * ya*ac; ac = -6 * xa*ac; bc = 6 * ya*ba; ba = -6 * xa*ba;
					dx += xy; ex = dx + dy; dy += xy;
					for (fx = fy = f; x0 != x3 && y0 != y3; )
						{
						y1 = fmin(fabs(xy - dx), fabs(dy - xy));
						ed = fmax(fabs(xy - dx), fabs(dy - xy));
						ed = f*(ed + 2 * ed*y1*y1 / (4 * ed*ed + y1*y1));
						y1 = 256 * fabs(ex - (f - fx + 1)*dx - (f - fy + 1)*dy + f*xy) / ed;
						if (y1 <= 256) { if ((x0 != sax3) || (y0 != say3)) _updatePixel<blend, checkrange, true, usepen>(x0, y0, color, (int32)(256-y1), penwidth); }
						px = fabs(ex - (f - fx + 1)*dx + (fy - 1)*dy);
						py = fabs(ex + (fx - 1)*dx - (f - fy + 1)*dy);
						y2 = (double)y0;
						do {
							if (ip < EP) goto exit;														// BUGFIX. change leg if abs value is too small (I do not know why I do that...)
							if (dx + xx > xy || dy + yy < xy) goto exit;
							y1 = 2 * ex + dx;
							if (2 * ex + dy > 0) { fx--; ex += dx += xx; dy += xy += ac; yy += bc; xx += ab; }
							else if (y1 > 0) goto exit;
							if (y1 <= 0) { fy--; ex += dy += yy; dx += xy += bc; xx += ac; yy += ba; }
							}
						while (fx > 0 && fy > 0);
							if (2 * fy <= f)
								{
								if (py < ed) _updatePixel<blend, checkrange, true, usepen>((int64)(x0 + sx), y0, color, (int32)(256 - 256*py/ed), penwidth);
								y0 += sy; fy += f;
								}
							if (2 * fx <= f)
								{
								if (px < ed) _updatePixel<blend, checkrange, true, usepen>(x0, (int64)(y2 + sy), color, (int32)(256 - 256*px/ed), penwidth);
								x0 += sx; fx += f;
								}
						}
					break;
				exit:
					if (2 * ex < dy && 2 * fy <= f + 2)
						{						
						if (py < ed) _updatePixel<blend, checkrange, true, usepen>((int64)(x0 + sx), y0, color, (int32)(256 - 256*py/ed), penwidth);
						y0 += sy;
						}
					if (2 * ex > dx && 2 * fx <= f + 2)
						{
						if (px < ed) _updatePixel<blend, checkrange, true, usepen>(x0, (int64)(y2 + sy), color, (int32)(256 - 256*px/ed), penwidth);
						x0 += sx;
						}
					xx = (double)x0; x0 = x3; x3 = (int64)xx; sx = -sx; xb = -xb;
					yy = (double)y0; y0 = y3; y3 = (int64)yy; sy = -sy; yb = -yb; x1 = x2;
					}
				while (leg--);					
				if ((x0 == sax3) && (y0 == say3)) {  _lineBresenhamAA<blend, checkrange, usepen>({ x3, y3 }, { x0, y0 }, color, false, penwidth); }
				else if ((x3 == sax3) && (y3 == say3)) { _lineBresenhamAA<blend, checkrange, usepen>({ x0, y0 }, { x3, y3 }, color, false, penwidth); }
				else _lineBresenhamAA<blend, checkrange, usepen>({ x0, y0 }, { x3, y3 }, color, true, penwidth);
				}


			/**
			* Plot any cubic Bezier curve.
			* adapted from Alois Zingl  (http://members.chello.at/easyfilter/bresenham.html)
			* change: do not draw always the endpoint (x2,y2)
			**/
			template<bool blend, bool checkrange, bool antialiasing, bool usepen> void _plotCubicBezier(int64 x0, int64 y0, int64 x1, int64 y1, int64 x2, int64 y2, int64 x3, int64 y3, RGBc color, bool draw_P2, int32 penwidth)
				{
				if (checkrange)
					{ // check if we can discard the whole curve
					iBox2 mbr({ x0,y0 });
					mbr.swallowPoint({ x1,y1 });
					mbr.swallowPoint({ x2,y2 });
					mbr.swallowPoint({ x3,y3 });
					if ((usepen) && (penwidth > 0)) mbr.enlarge(penwidth);
					if (intersectionRect(mbr, iBox2(0, _lx - 1, 0, _ly - 1)).isEmpty()) return;  // nothing to draw
					}
				if (draw_P2) { _updatePixel<blend, checkrange, false, usepen>(x3, y3, color, 0, penwidth); }
				if ((x0 == x3) && (y0 == y3)) return;
				int64 n = 0, i = 0;
				int64 xc = x0 + x1 - x2 - x3, xa = xc - 4 * (x1 - x2);
				int64 xb = x0 - x1 - x2 + x3, xd = xb + 4 * (x1 + x2);
				int64 yc = y0 + y1 - y2 - y3, ya = yc - 4 * (y1 - y2);
				int64 yb = y0 - y1 - y2 + y3, yd = yb + 4 * (y1 + y2);
				double fx0 = (double)x0, fx1, fx2, fx3, fy0 = (double)y0, fy1, fy2, fy3;
				double t1 = (double)(xb*xb - xa*xc), t2, t[6];
				if (xa == 0) { if (abs(xc) < 2 * abs(xb)) t[n++] = xc / (2.0*xb); }
				else if (t1 > 0.0)
					{
					t2 = sqrt(t1);
					t1 = (xb - t2) / xa; if (fabs(t1) < 1.0) t[n++] = t1;
					t1 = (xb + t2) / xa; if (fabs(t1) < 1.0) t[n++] = t1;
					}
				t1 = (double)(yb*yb - ya*yc);
				if (ya == 0) { if (abs(yc) < 2 * abs(yb)) t[n++] = yc / (2.0*yb); }
				else if (t1 > 0.0)
					{
					t2 = sqrt(t1);
					t1 = (yb - t2) / ya; if (fabs(t1) < 1.0) t[n++] = t1;
					t1 = (yb + t2) / ya; if (fabs(t1) < 1.0) t[n++] = t1;
					}
				for (i = 1; i < n; i++) if ((t1 = t[i - 1]) > t[i]) { t[i - 1] = t[i]; t[i] = t1; i = 0; }
				t1 = -1.0; t[n] = 1.0;
				for (i = 0; i <= n; i++)
					{
					t2 = t[i];
					fx1 = (t1*(t1*xb - 2 * xc) - t2*(t1*(t1*xa - 2 * xb) + xc) + xd) / 8 - fx0;
					fy1 = (t1*(t1*yb - 2 * yc) - t2*(t1*(t1*ya - 2 * yb) + yc) + yd) / 8 - fy0;
					fx2 = (t2*(t2*xb - 2 * xc) - t1*(t2*(t2*xa - 2 * xb) + xc) + xd) / 8 - fx0;
					fy2 = (t2*(t2*yb - 2 * yc) - t1*(t2*(t2*ya - 2 * yb) + yc) + yd) / 8 - fy0;
					fx0 -= fx3 = (t2*(t2*(3 * xb - t2*xa) - 3 * xc) + xd) / 8;
					fy0 -= fy3 = (t2*(t2*(3 * yb - t2*ya) - 3 * yc) + yd) / 8;
					x3 = (int64)floor(fx3 + 0.5); y3 = (int64)floor(fy3 + 0.5);
					if (fx0 != 0.0) { fx1 *= fx0 = (x0 - x3) / fx0; fx2 *= fx0; }
					if (fy0 != 0.0) { fy1 *= fy0 = (y0 - y3) / fy0; fy2 *= fy0; }
					if (x0 != x3 || y0 != y3)
						{
						if (antialiasing) { _plotCubicBezierSegAA<blend, checkrange, usepen>(x0, y0, x0 + fx1, y0 + fy1, x0 + fx2, y0 + fy2, x3, y3,color, penwidth); }
						else { _plotCubicBezierSeg<blend, checkrange, usepen>(x0, y0, x0 + fx1, y0 + fy1, x0 + fx2, y0 + fy2, x3, y3,color, penwidth); }
						}
					x0 = x3; y0 = y3; fx0 = fx3; fy0 = fy3; t1 = t2;
					}
				}



			/**
			* plot quadratic spline, destroys input arrays x,y.
			* adapted from Alois Zingl  (http://members.chello.at/easyfilter/bresenham.html)
			* change: do not draw always the endpoint (x2,y2)
			**/
			template<bool blend, bool checkrange, bool antialiasing, bool usepen> void _plotQuadSpline(int64 n, int64 x[], int64 y[], RGBc color, bool draw_last, int32 penwidth)
				{
				if (draw_last) { _updatePixel<blend, checkrange, false, usepen>(x[n], y[n], color, 0, penwidth); }
				const int64 M_MAX = 6;
				double mi = 1, m[M_MAX];
				int64 i, x0, y0, x1, y1, x2 = x[n], y2 = y[n];
				x[1] = x0 = 8 * x[1] - 2 * x[0];
				y[1] = y0 = 8 * y[1] - 2 * y[0];
				for (i = 2; i < n; i++) 
					{
					if (i - 2 < M_MAX) m[i - 2] = mi = 1.0 / (6.0 - mi);
					x[i] = x0 = (int64)floor(8 * x[i] - x0*mi + 0.5);
					y[i] = y0 = (int64)floor(8 * y[i] - y0*mi + 0.5);
					}
				x1 = (int64)(floor((x0 - 2 * x2) / (5.0 - mi) + 0.5)); 
				y1 = (int64)(floor((y0 - 2 * y2) / (5.0 - mi) + 0.5));
				for (i = n - 2; i > 0; i--) 
					{
					if (i <= M_MAX) mi = m[i - 1];
					x0 = (int64)floor((x[i] - x1)*mi + 0.5);
					y0 = (int64)floor((y[i] - y1)*mi + 0.5);
					_plotQuadBezier<blend, checkrange, antialiasing, usepen>((x0 + x1) / 2, (y0 + y1) / 2, x1, y1, x2, y2,color, false, penwidth);
					x2 = (x0 + x1) / 2; x1 = x0;
					y2 = (y0 + y1) / 2; y1 = y0;
					}
				_plotQuadBezier<blend, checkrange, antialiasing, usepen>(x[0], y[0], x1, y1, x2, y2,color, false, penwidth);
				}




			/**
			* plot cubic spline, destroys input arrays x,y.
			* adapted from Alois Zingl  (http://members.chello.at/easyfilter/bresenham.html)
			* change: do not draw always the endpoint (x2,y2)
			**/
			template<bool blend, bool checkrange, bool antialiasing, bool usepen> void _plotCubicSpline(int64 n, int64 x[], int64 y[], RGBc color, bool draw_last, int32 penwidth)
				{
				if (draw_last) { _updatePixel<blend, checkrange,false, usepen>(x[n], y[n], color, 0, penwidth); }
				const int64 M_MAX = 6;
				double mi = 0.25, m[M_MAX];
				int64 x3 = x[n - 1], y3 = y[n - 1], x4 = x[n], y4 = y[n];
				int64 i, x0, y0, x1, y1, x2, y2;
				x[1] = x0 = 12 * x[1] - 3 * x[0];
				y[1] = y0 = 12 * y[1] - 3 * y[0];
				for (i = 2; i < n; i++) 
					{
					if (i - 2 < M_MAX) m[i - 2] = mi = 0.25 / (2.0 - mi);
					x[i] = x0 = (int64)floor(12 * x[i] - 2 * x0*mi + 0.5);
					y[i] = y0 = (int64)floor(12 * y[i] - 2 * y0*mi + 0.5);
					}
				x2 = (int64)(floor((x0 - 3 * x4) / (7 - 4 * mi) + 0.5));
				y2 = (int64)(floor((y0 - 3 * y4) / (7 - 4 * mi) + 0.5));
				_plotCubicBezier<blend, checkrange, antialiasing, usepen>(x3, y3, (x2 + x4) / 2, (y2 + y4) / 2, x4, y4, x4, y4,color, false, penwidth);
				if (n - 3 < M_MAX) mi = m[n - 3];
				x1 = (int64)floor((x[n - 2] - 2 * x2)*mi + 0.5);
				y1 = (int64)floor((y[n - 2] - 2 * y2)*mi + 0.5);
				for (i = n - 3; i > 0; i--) 
					{
					if (i <= M_MAX) mi = m[i - 1];
					x0 = (int64)floor((x[i] - 2 * x1)*mi + 0.5);
					y0 = (int64)floor((y[i] - 2 * y1)*mi + 0.5);
					x4 = (int64)floor((x0 + 4 * x1 + x2 + 3) / 6.0);
					y4 = (int64)floor((y0 + 4 * y1 + y2 + 3) / 6.0);
					_plotCubicBezier<blend, checkrange, antialiasing, usepen>(x4, y4, (int64)floor((2 * x1 + x2) / 3 + 0.5), (int64)floor((2 * y1 + y2) / 3 + 0.5), (int64)floor((x1 + 2 * x2) / 3 + 0.5), (int64)floor((y1 + 2 * y2) / 3 + 0.5), x3, y3, color, false,penwidth);
					x3 = x4; y3 = y4; x2 = x1; y2 = y1; x1 = x0; y1 = y0;
					}
				x0 = x[0]; x4 = (int64)floor((3 * x0 + 7 * x1 + 2 * x2 + 6) / 12.0);
				y0 = y[0]; y4 = (int64)floor((3 * y0 + 7 * y1 + 2 * y2 + 6) / 12.0);
				_plotCubicBezier<blend, checkrange, antialiasing, usepen>(x4, y4, (int64)floor((2 * x1 + x2) / 3 + 0.5), (int64)floor((2 * y1 + y2) / 3 + 0.5), (int64)floor((x1 + 2 * x2) / 3 + 0.5), (int64)floor((y1 + 2 * y2) / 3 + 0.5), x3, y3, color, false,penwidth);
				_plotCubicBezier<blend, checkrange, antialiasing, usepen>(x0, y0, x0, y0, (x0 + x1) / 2, (y0 + y1) / 2, x4, y4, color, false,penwidth);
				}





			/******************************************************************************************************************************************************
			*																				   																      *
			*                                                                  DRAWING SHAPES                                                                     *
			*																																					  *
			*******************************************************************************************************************************************************/


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


			/* blend a region with a given color */
			inline static void _blendRegion(RGBc * pdest, int64 dest_stride, int64 sx, int64 sy, RGBc color)
				{
				for (int64 j = 0; j < sy; j++)
					{
					const int64 offdest = j * dest_stride;
					for (int64 i = 0; i < sx; i++)
						{
						pdest[offdest + i].blend(color);
						}
					}
				}



			/* draw a filled rectangle */
			MTOOLS_FORCEINLINE void _draw_box(int64 x, int64 y, int64 sx, int64 sy, RGBc boxcolor, bool blend)
				{
				if (x < 0) { sx -= x;   x = 0; }
				if (y < 0) { sy -= y;   y = 0; }
				if ((boxcolor.isTransparent()) || (x >= _lx) || (y >= _ly)) return;
				sx -= std::max<int64>(0, (x + sx - _lx));
				sy -= std::max<int64>(0, (y + sy - _ly));
				if ((sx <= 0) || (sy <= 0)) return;
				RGBc * p = _data + _stride*y + x;
				if ((blend) && (!boxcolor.isOpaque()))
					{
					for (int64 j = 0; j < sy; j++)
						{
						for (int64 i = 0; i < sx; i++) { p[i].blend(boxcolor); }
						p += _stride;
						}
					}
				else
					{
					for (int64 j = 0; j < sy; j++)
						{
						for (int64 i = 0; i < sx; i++) { p[i] = boxcolor; }
						p += _stride;
						}
					}
				}



			/****************************************************************************
			/* PRIVATE METHODS FOR DRAWING CIRCLES AND ELLIPSES. 
			*****************************************************************************/



			/** Invert vertical the part chosen when drawing a circle (because y-axis inverted between image and canvas) */
			MTOOLS_FORCEINLINE void _reverseVerticalCirclePart(int & part)
				{
				switch (part)
					{
					case BOX_SPLIT_UP:			{ part = BOX_SPLIT_DOWN; return; }
					case BOX_SPLIT_UP_LEFT:		{ part = BOX_SPLIT_DOWN_LEFT; return; }
					case BOX_SPLIT_UP_RIGHT:	{ part = BOX_SPLIT_DOWN_RIGHT; return; }
					case BOX_SPLIT_DOWN:		{ part = BOX_SPLIT_UP; return; }
					case BOX_SPLIT_DOWN_LEFT:	{ part = BOX_SPLIT_UP_LEFT; return; }
					case BOX_SPLIT_DOWN_RIGHT:	{ part = BOX_SPLIT_UP_RIGHT; return; }
					}
				return;
				}


			/** Compute the integer valued bounding box for an ellipse */
			MTOOLS_FORCEINLINE iBox2 _ellipseBBox(fVec2 center, double rx, double ry)
				{
				return iBox2((int64)(floor(center.X() - rx)), (int64)(ceil(center.X() + rx)), (int64)(floor(center.Y() - ry)), (int64)(ceil(center.Y() + ry)));
				}


			/**
			 * Check if the image intersect an ellipse. 
			 * Return -1 if not intersection.
			 *         0 if intersection
			 *         1 if the image in included in the interior of the ellipse.
			 **/
			MTOOLS_FORCEINLINE int _ellipseIntersection(const iBox2 & iB, fVec2 center, double rx, double ry)
				{
				MTOOLS_ASSERT(rx > 0);
				MTOOLS_ASSERT(ry > 0);
				const fBox2 B( iB.min[0] - 1.5, iB.max[0] + 1.5,
					           iB.min[1] - 1.5, iB.max[1] + 1.5  );
				const double rx2 = rx * rx;
				const double rx2_over_ry2 = rx2 / (ry*ry);

				double u1;
				{
					double XX = B.min[0] - center.X(); XX *= XX;
					double YY = B.min[1] - center.Y(); YY *= YY;
					u1 = rx2 - XX - (YY * rx2_over_ry2);
				}

				double u2;
				{
					double XX = B.max[0] - center.X(); XX *= XX;
					double YY = B.min[1] - center.Y(); YY *= YY;
					u2 = rx2 - XX - (YY * rx2_over_ry2);
				}

				double u3;
				{
					double XX = B.min[0] - center.X(); XX *= XX;
					double YY = B.max[1] - center.Y(); YY *= YY;
					u3 = rx2 - XX - (YY * rx2_over_ry2);
				}

				double u4;
					{
					double XX = B.max[0] - center.X(); XX *= XX;
					double YY = B.max[1] - center.Y(); YY *= YY;
					u4 = rx2 - XX - (YY * rx2_over_ry2);
					}

				if ((u1 > 0) && (u2 > 0) && (u3 > 0) && (u4 > 0)) return 1; // box strictly inside the ellipse
				if ((u1 < 0) && (u2 < 0) && (u3 < 0) && (u4 < 0))
					{ // intersection may be empty. 
					if (center.X() < B.min[0])
						{
						if ((center.Y() < B.min[1]) || (center.Y() > B.max[1])) return -1; // no intersection
						if (center.X() + rx < B.min[0]) return -1; // no intersection
						return 0;
						}
					if (center.X() > B.max[0])
						{
						if ((center.Y() < B.min[1]) || (center.Y() > B.max[1])) return -1; // no intersection
						if (center.X() - rx > B.max[0]) return -1; // no intersection
						return 0;
						}
					if (center.Y() < B.min[1])
						{
						if ((center.X() < B.min[0]) || (center.X() > B.max[0])) return -1; // no intersection
						if (center.Y() + ry < B.min[1]) return -1; // no intersection
						return 0;
						}
					if (center.Y() > B.max[1])
						{
						if ((center.X() < B.min[0]) || (center.X() > B.max[0])) return -1; // no intersection
						if (center.Y() - ry > B.max[1]) return -1; // no intersection
						return 0;
						}
					}
				return 0;
				}


			/**
			* Draw circle. both interior and outline.
			*  adapted from Alois Zingl  (http://members.chello.at/easyfilter/bresenham.html)
			**/
			template<bool blend, bool checkrange, bool outline, bool fill>  inline  void _draw_circle(int64 xm, int64 ym, int64 r, RGBc color, RGBc fillcolor)
				{
				if (r < 0) return;
				if (r > 2)
					{ // circle is large enough to check first if there is something to draw.
					int q = _ellipseIntersection(imageBox(), { (double)xm, (double)ym }, (double)r, (double)r);
					if (q < 0) return; // nothing to draw
					if (q > 0)
						{
						if (fill) { draw_box(imageBox(), fillcolor, true); }
						return;
						}
					}
				switch (r)
				{
				case 0:
					{
					if (fill)
						{
						_updatePixel<blend, checkrange, true, false>(xm, ym, fillcolor, 128, 0);
						_updatePixel<blend, checkrange, true, false>(xm, ym, color, 128, 0);
						}
					else
						{
						_updatePixel<blend, checkrange, false, false>(xm, ym, color, 0, 0);
						}
					return;
					}
				case 1:
					{
					if (fill)
						{
						_updatePixel<blend, checkrange, false, false>(xm, ym, fillcolor, 0, 0);
						}
					_updatePixel<blend, checkrange, false, false>(xm + 1, ym, color, 0, 0);
					_updatePixel<blend, checkrange, false, false>(xm - 1, ym, color, 0, 0);
					_updatePixel<blend, checkrange, false, false>(xm, ym - 1, color, 0, 0);
					_updatePixel<blend, checkrange, false, false>(xm, ym + 1, color, 0, 0);
					return;
					}
				}
				int64 x = -r, y = 0, err = 2 - 2 * r;
				do {
					if (outline)
						{
						_updatePixel<blend, checkrange, false, false>(xm - x, ym + y, color, 0, 0);
						_updatePixel<blend, checkrange, false, false>(xm - y, ym - x, color, 0, 0);
						_updatePixel<blend, checkrange, false, false>(xm + x, ym - y, color, 0, 0);
						_updatePixel<blend, checkrange, false, false>(xm + y, ym + x, color, 0, 0);
						}
					r = err;
					if (r <= y)
						{
						if (fill)
							{
							_hline<blend, checkrange>(xm, xm - x - 1, ym + y, fillcolor);
							_hline<blend, checkrange>(xm + x + 1, xm - 1, ym - y, fillcolor);
							}
						err += ++y * 2 + 1;
						}
					if (r > x || err > y)
						{
						err += ++x * 2 + 1;
						if (fill)
							{
							if (x)
								{
								_hline<blend, checkrange>(xm - y + 1, xm - 1, ym - x, fillcolor);
								_hline<blend, checkrange>(xm, xm + y - 1, ym + x, fillcolor);
								}
							}
						}
					}
				while (x < 0);
				}



			/**
			* Draw an antialiased circle
			* adapted from Alois Zingl  (http://members.chello.at/easyfilter/bresenham.html)
			**/
			template<bool blend, bool checkrange, bool fill> void _draw_circle_AA(int64 xm, int64 ym, int64 r, RGBc color, RGBc colorfill)
			{
				if (r < 0) return;
				if (r > 2)
					{ // circle is large enough to check first if there is something to draw.
					int q = _ellipseIntersection(imageBox(), { (double)xm, (double)ym }, (double)r, (double)r);
					if (q < 0) return; // nothing to draw
					if (q > 0)
						{
						if (fill) { draw_box(imageBox(), colorfill, true); }
						return;
						}
					}
				switch (r)
				{
				case 0:
				{
					if (fill)
					{
						_updatePixel<blend, checkrange, true, false>(xm, ym, colorfill, 128, 0);
						_updatePixel<blend, checkrange, true, false>(xm, ym, color, 128, 0);
					}
					else
					{
						_updatePixel<blend, checkrange, true, false>(xm, ym, color, 256, 0);
					}
					return;
				}
				case 1:
				{
					if (fill)
					{
						_updatePixel<blend, checkrange, true, false>(xm, ym, colorfill, 256, 0);
						_updatePixel<blend, checkrange, true, false>(xm + 1, ym, colorfill, 64, 0);
						_updatePixel<blend, checkrange, true, false>(xm - 1, ym, colorfill, 64, 0);
						_updatePixel<blend, checkrange, true, false>(xm, ym - 1, colorfill, 64, 0);
						_updatePixel<blend, checkrange, true, false>(xm, ym + 1, colorfill, 64, 0);
					}
					_updatePixel<blend, checkrange, true, false>(xm + 1, ym, color, 192, 0);
					_updatePixel<blend, checkrange, true, false>(xm - 1, ym, color, 192, 0);
					_updatePixel<blend, checkrange, true, false>(xm, ym - 1, color, 192, 0);
					_updatePixel<blend, checkrange, true, false>(xm, ym + 1, color, 192, 0);
					_updatePixel<blend, checkrange, true, false>(xm + 1, ym + 1, color, 64, 0);
					_updatePixel<blend, checkrange, true, false>(xm - 1, ym + 1, color, 64, 0);
					_updatePixel<blend, checkrange, true, false>(xm + 1, ym - 1, color, 64, 0);
					_updatePixel<blend, checkrange, true, false>(xm - 1, ym - 1, color, 64, 0);
					return;
				}
				}
				const int64 rad = r;
				const int64 hr = (rad/2 + 1);
				int64 x = -r, y = 0;
				int64 x2, e2, err = 2 - 2 * r;
				int32 i, j;
				r = 1 - err;
				do {
					i = (int32)(256 * abs(err - 2 * (x + y) - 2) / r);
					j = 256 - i;
					if (fill)
					{
						if (y > hr)
						{
							_updatePixel<blend, checkrange, true, false>(xm - x, ym + y, colorfill, i, 0);
							_updatePixel<blend, checkrange, true, false>(xm - y, ym - x, colorfill, i, 0);
							_updatePixel<blend, checkrange, true, false>(xm + x, ym - y, colorfill, i, 0);
							_updatePixel<blend, checkrange, true, false>(xm + y, ym + x, colorfill, i, 0);
						}
					}
					_updatePixel<blend, checkrange, true, false>(xm - x, ym + y, color, j, 0);
					_updatePixel<blend, checkrange, true, false>(xm - y, ym - x, color, j, 0);
					_updatePixel<blend, checkrange, true, false>(xm + x, ym - y, color, j, 0);
					_updatePixel<blend, checkrange, true, false>(xm + y, ym + x, color, j, 0);

					e2 = err; x2 = x;
					if (err + y > 0)
					{
						i = (int32)(256 * (err - 2 * x - 1) / r);
						if (i <= 256)
						{
							j = 256 - i;
							_updatePixel<blend, checkrange, true, false>(xm - x, ym + y + 1, color, j, 0);
							_updatePixel<blend, checkrange, true, false>(xm - y - 1, ym - x, color, j, 0);
							_updatePixel<blend, checkrange, true, false>(xm + x, ym - y - 1, color, j, 0);
							_updatePixel<blend, checkrange, true, false>(xm + y + 1, ym + x, color, j, 0);
						}
						err += ++x * 2 + 1;
					}
					if (e2 + x2 <= 0) {
						i = (int32)(256 * (2 * y + 3 - e2) / r);
						if (i > 256) i = 256;
						j = 256 - i;
						if (fill)
						{
							_updatePixel<blend, checkrange, true, false>(xm - x2 - 1, ym + y, colorfill, i, 0);
							_updatePixel<blend, checkrange, true, false>(xm - y, ym - x2 - 1, colorfill, i, 0);
							_updatePixel<blend, checkrange, true, false>(xm + x2 + 1, ym - y, colorfill, i, 0);
							_updatePixel<blend, checkrange, true, false>(xm + y, ym + x2 + 1, colorfill, i, 0);
						}
						_updatePixel<blend, checkrange, true, false>(xm - x2 - 1, ym + y, color, j, 0);
						_updatePixel<blend, checkrange, true, false>(xm - y, ym - x2 - 1, color, j, 0);
						_updatePixel<blend, checkrange, true, false>(xm + x2 + 1, ym - y, color, j, 0);
						_updatePixel<blend, checkrange, true, false>(xm + y, ym + x2 + 1, color, j, 0);
						if (fill)
						{
							if (y < rad - 1)
							{
								_hline<blend, checkrange>(xm + x2 + 2, xm - x2 - 2, ym + y, colorfill);
								if (y) _hline<blend, checkrange>(xm + x2 + 2, xm - x2 - 2, ym - y, colorfill);
							}
						}
						err += ++y * 2 + 1;
					}
				} while (x < 0);
			}




			/**
			* Draw an ellipse inside a rectangle
			**/
			template<bool blend, bool checkrange, bool outline, bool fill> inline void _draw_ellipse_in_rect(int64 x0, int64 y0, int64 x1, int64 y1, RGBc color, RGBc fillcolor)
				{
				if ((x1 < x0) || (y1 < y0)) return;
				if ((x1 - x0 > 2) || (y1 - y0 > 2))
					{ // circle is large enough to check first if there is something to draw.
					double rx = ((double)(x1 - x0)) / 2;
					double ry = ((double)(y1 - y0)) / 2;
					int q = _ellipseIntersection(imageBox(), fVec2(((double)(x1 + x0)) / 2, ((double)(y1 + y0)) / 2), rx, ry);
					if (q < 0) return; // nothing to draw
					if (q > 0)
						{
						if (fill) { draw_box(imageBox(), fillcolor, true); }
						return;
						}
					}
				if (x1 == x0)
					{
					if (fill)
						{
						for (auto u = y0; u <= y1; u++)
							{
							_updatePixel<blend, checkrange, true, false>(x0, u, color, 128, 0);
							_updatePixel<blend, checkrange, true, false>(x0, u, fillcolor, 128, 0);
							}
						}
					else
						{
						for (auto u = y0; u <= y1; u++)
							{
							_updatePixel<blend, checkrange, false, false>(x0, u, color, 0,  0);
							}
						}
					return;
					}
				if (y1 == y0)
					{
					if (fill)
						{
						for (auto u = x0; u <= x1; u++)
							{
							_updatePixel<blend, checkrange, true, false>(u, y0, color, 128, 0);
							_updatePixel<blend, checkrange, true, false>(u, y0, fillcolor, 128, 0);
							}
						}
					else
						{
						for (auto u = x0; u <= x1; u++)
							{
							_updatePixel<blend, checkrange, false, false>(u, y0, color, 0,  0);
							}
						}
					return;
					}
				bool INCX = ((x1 - x0) & 1);
				bool INCY = ((y1 - y0) & 1);
				int64 a = (x1 - x0) >> 1;
				int64 b = (y1 - y0) >> 1;
				int64 xm = x0 + a;
				int64 ym = y0 + b;
				if (INCX)
					{
					if (INCY)
						{
						_draw_ellipse_sub<blend, checkrange, outline, fill, true, true>(xm, ym, a, b, color, fillcolor);
						}
					else
						{
						_draw_ellipse_sub<blend, checkrange, outline, fill, true, false>(xm, ym, a, b, color, fillcolor);
						}
					}
				else
					{
					if (INCY)
						{
						_draw_ellipse_sub<blend, checkrange, outline, fill, false, true>(xm, ym, a, b, color, fillcolor);
						}
					else
						{
						_draw_ellipse_sub<blend, checkrange, outline, fill, false, false>(xm, ym, a, b, color, fillcolor);
						}
					}
				}


			/**
			* Draw an ellipse
			* set incx = 1 to increse the x-diameter by 1
			* set incy = 1 to increase the y-diameter by 1  -> to make it fit a rectangle (cheat!).
			*
			* adapted from Alois Zingl  (http://members.chello.at/easyfilter/bresenham.html)
			**/
			template<bool blend, bool checkrange, bool outline, bool fill, int64 incx, int64 incy>  inline  void _draw_ellipse_sub(int64 xm, int64 ym, int64 a, int64 b, RGBc color, RGBc fillcolor)
				{
				int64 x = -a, y = 0;
				int64 e2 = b, dx = (1 + 2 * x)*e2*e2;
				int64 dy = x*x, err = dx + dy;
				int64 twoasquare = 2 * a*a, twobsquare = 2 * b*b;
				while (x < -1)
					{
					e2 = 2 * err;
					int64 nx = x;
					if (e2 >= dx) { nx++; err += dx += twobsquare; }
					if (e2 <= dy)
						{
						if (fill)
							{
							_hline<blend, checkrange>(xm + x + 1, xm - x - 1 + incx, ym + y + incy, fillcolor);
							if (y) _hline<blend, checkrange>(xm + x + 1, xm - x - 1 + incx, ym - y, fillcolor);
							}
						y++; err += dy += twoasquare;
						}
					x = nx;
					if (outline)
						{
						_updatePixel<blend, checkrange, false, false>(xm - x + incx, ym + y + incy, color, 0, 0);
						_updatePixel<blend, checkrange, false, false>(xm + x, ym + y + incy, color, 0, 0);
						_updatePixel<blend, checkrange, false, false>(xm + x, ym - y, color, 0, 0);
						_updatePixel<blend, checkrange, false, false>(xm - x + incx, ym - y, color, 0, 0);
						}
					}
				if (fill)
					{
					if (y != b)
						{
						_updatePixel<blend, checkrange, false, false>(xm, ym + y + incy, fillcolor, 0, 0);
						_updatePixel<blend, checkrange, false, false>(xm, ym - y, fillcolor, 0, 0);
						if (incx)
							{
							_updatePixel<blend, checkrange, false, false>(xm + 1, ym + y + incy, fillcolor, 0, 0);
							_updatePixel<blend, checkrange, false, false>(xm + 1, ym - y, fillcolor, 0, 0);
							}
						}
					if (incy)
						{
						_hline<blend, checkrange>(xm - a + 1, xm + a + incx - 1, ym, fillcolor);
						}
					}
				if (outline)
					{
					_updatePixel<blend, checkrange, false, false>(xm - a, ym, color, 0, 0);
					_updatePixel<blend, checkrange, false, false>(xm + a + incx, ym, color, 0, 0);
					_updatePixel<blend, checkrange, false, false>(xm, ym - b, color, 0, 0);
					_updatePixel<blend, checkrange, false, false>(xm, ym + b + incy, color, 0, 0);
					if (incx)
						{
						_updatePixel<blend, checkrange, false, false>(xm + 1, ym - b, color, 0, 0);
						_updatePixel<blend, checkrange, false, false>(xm + 1, ym + b + incy, color, 0, 0);
						}
					if (incy)
						{
						_updatePixel<blend, checkrange, false, false>(xm + a + incx, ym + 1, color, 0, 0);
						_updatePixel<blend, checkrange, false, false>(xm - a, ym + 1, color, 0, 0);
						}
					}
				int64 sy = y;
				while (y++ < b)
					{
					_updatePixel<blend, checkrange, false, false>(xm, ym + y + incy, color, 0, 0);
					_updatePixel<blend, checkrange, false, false>(xm, ym - y, color, 0, 0);
					}
				if (incx)
					{
					y = sy;
					while (y++ < b)
						{
						_updatePixel<blend, checkrange, false, false>(xm + 1, ym + y + incy, color, 0, 0);
						_updatePixel<blend, checkrange, false, false>(xm + 1, ym - y, color, 0, 0);
						}
					}
				}



			/**
			* Draw an antialiased ellipse inside a rectangle
			* adapted from Alois Zingl  (http://members.chello.at/easyfilter/bresenham.html)
			**/
			template<bool blend, bool checkrange> void _draw_ellipse_in_rect_AA(int64 x0, int64 y0, int64 x1, int64 y1, RGBc color)
				{
				if ((x1 < x0) || (y1 < y0)) return;
				if ((x1 - x0 > 2) || (y1 - y0 > 2))
					{ // circle is large enough to check first if there is something to draw.
					double rx = ((double)(x1 - x0)) / 2;
					double ry = ((double)(y1 - y0)) / 2;
					int q = _ellipseIntersection(imageBox(), fVec2(((double)(x1 + x0)) / 2, ((double)(y1 + y0)) / 2), rx, ry);
					if (q < 0) return; // nothing to draw
					if (q > 0) return; // again nothing to draw since there is no filling						
					}
				if (x1 == x0)
					{
					for (auto u = y0; u <= y1; u++)
						{
						_updatePixel<blend, checkrange, false, false>(x0, u, color, 0,  0);
						}
					return;
					}
				if (y1 == y0)
					{
					for (auto u = x0; u <= x1; u++)
						{
						_updatePixel<blend, checkrange, false, false>(u, y0, color, 0,  0);
						}
					return;
					}

				int64 a = abs(x1 - x0), b = abs(y1 - y0), b1 = b & 1;
				double dx = (double)(4 * (a - 1.0)*b*b), dy = (double)(4 * (b1 + 1)*a*a);
				double ed, i, err = b1*a*a - dx + dy;
				bool f;
				if (a == 0 || b == 0) return _lineBresenham<blend, checkrange, false, false, false, false>({ x0, y0 }, { x1, y1 }, color, true, 0, 0);
				if (x0 > x1) { x0 = x1; x1 += a; }
				if (y0 > y1) y0 = y1;
				y0 += (b + 1) / 2; y1 = y0 - b1;
				a = 8 * a*a; b1 = 8 * b*b;
				for (;;)
					{
					i = std::min<double>(dx, dy); ed = std::max<double>(dx, dy);
					if (y0 == y1 + 1 && err > dy && a > b1) ed = 256 * 4. / a;
					else ed = 256 / (ed + 2 * ed*i*i / (4 * ed*ed + i*i));
					i = ed*fabs(err + dx - dy);
					int32 op = (int32)(256 - i);
					_updatePixel<blend, checkrange, true, false>(x0, y0, color, op, 0);
					_updatePixel<blend, checkrange, true, false>(x0, y1, color, op, 0);
					_updatePixel<blend, checkrange, true, false>(x1, y0, color, op, 0);
					_updatePixel<blend, checkrange, true, false>(x1, y1, color, op, 0);
					f = (2 * err + dy >= 0);
					if (f)
						{
						if (x0 >= x1) break;
						i = ed*(err + dx);
						if (i < 256)
							{
							int32 op = (int32)(256 - i);
							_updatePixel<blend, checkrange, true, false>(x0, y0 + 1, color, op, 0);
							_updatePixel<blend, checkrange, true, false>(x0, y1 - 1, color, op, 0);
							_updatePixel<blend, checkrange, true, false>(x1, y0 + 1, color, op, 0);
							_updatePixel<blend, checkrange, true, false>(x1, y1 - 1, color, op, 0);
							}
						}
					if (2 * err <= dx)
						{
						i = ed*(dy - err);
						if (i < 256)
							{
							int32 op = (int32)(256 - i);
							_updatePixel<blend, checkrange, true, false>(x0 + 1, y0, color, op, 0);
							_updatePixel<blend, checkrange, true, false>(x0 + 1, y1, color, op, 0);
							_updatePixel<blend, checkrange, true, false>(x1 - 1, y0, color, op, 0);
							_updatePixel<blend, checkrange, true, false>(x1 - 1, y1, color, op, 0);
							}
						y0++; y1--; err += dy += a;
						}
					if (f) { x0++; x1--; err -= dx -= b1; }
					}
				if (--x0 == x1++)
					while (y0 - y1 < b)
						{
						i = 256 * 4 * fabs(err + dx) / b1;
						int32 op = (int32)(256 - i);
						if (op > 0)
							{
							_updatePixel<blend, checkrange, true, false>(x0, ++y0, color, op, 0);
							_updatePixel<blend, checkrange, true, false>(x1, y0, color, op, 0);
							_updatePixel<blend, checkrange, true, false>(x0, --y1, color, op, 0);
							_updatePixel<blend, checkrange, true, false>(x1, y1, color, op, 0);
							}
						err += dy += a;
						}
				}


			/**
			* Draw an ellipse.
			* Slower but support real-valued parameters and drawing only the part inside a box.
			**/
			template<bool blend, bool outline, bool fill>  inline  void _draw_ellipse2(iBox2 B, fVec2 P, double rx, double ry, RGBc color, RGBc fillcolor)
			{
				if ((rx <= 0) || (ry <= 0)) return; 

				B = intersectionRect(B, iBox2((int64)floor(P.X() - rx - 1),
					(int64)ceil(P.X() + rx + 1),
					(int64)floor(P.Y() - ry - 1),
					(int64)ceil(P.Y() + ry + 1)));
				
				if (B.isEmpty()) return;
				
				MTOOLS_ASSERT(B.isIncludedIn(imageBox()));

				{ //check first if there is something to draw.
					int q = _ellipseIntersection(B, P, rx, ry);
					if (q < 0) return; // nothing to draw
					if (q > 0)
						{
						if (fill) { draw_box(B,fillcolor,true); }
						return;
						}
				}

				const double rx2 = rx * rx;
				const double ry2 = ry * ry;
				const double Rx2 = (rx + 0.5)*(rx + 0.5);
				const double Ry2 = (ry + 0.5)*(ry + 0.5);
				const double Rxy2 = Rx2 * Ry2;

				int64 xmin = B.max[0];
				int64 xmax = B.min[0];

				for (int64 y = B.min[1]; y <= B.max[1]; y++)
				{
					const double dy = (double)(y - P.Y());
					const double absdy = ((dy > 0) ? dy : -dy);
					const double dy2 = (dy*dy);

					if (xmin > xmax)
					{
						if (dy2 > Ry2) continue;  // line is empty. 
						if (P.X() <= (double)B.min[0])
						{
							const double dx = (double)B.min[0] - P.X();
							if ((dx*dx)*Ry2 + (dy2*Rx2) > Rxy2) continue; // line is empty
						}
						else if (P.X() >= (double)B.max[0])
						{
							const double dx = P.X() - (double)B.max[0];
							if ((dx*dx)*Ry2 + (dy2*Rx2) > Rxy2) continue; // line is empty
						}
						xmin = B.min[0]; xmax = B.max[0];
					}
					const double ly = dy2 - absdy + 0.25;
					const double Ly = dy2 + absdy + 0.25;
					const double g1 = rx2 - ly * rx2 / ry2 - 0.25;
					const double g2 = rx2 - Ly * rx2 / ry2 - 0.25;
					double dx = (double)(xmin - P.X());

					while (1)
					{
						const double absdx = ((dx > 0) ? dx : -dx);
						const double dx2 = dx * dx;
						const double lx = dx2 - absdx;
						if ((xmin == B.min[0]) || (lx  > g1)) break;
						xmin--;
						dx--;
					}
					while (1)
					{
						const double absdx = ((dx > 0) ? dx : -dx);
						const double dx2 = dx * dx;
						const double lx = dx2 - absdx;
						const double Lx = dx2 + absdx;
						if ((Lx <= g2) || (xmax < xmin)) break;
						if (outline) { if ((lx  < g2) || (Lx  <  g1)) { _updatePixel<blend, false, false, false>(xmin, y, color, 255, 0); } }
						xmin++;
						dx++;
					}
					dx = (double)(xmax - P.X());
					while (1)
					{
						const double absdx = ((dx > 0) ? dx : -dx);
						const double dx2 = dx * dx;
						const double lx = dx2 - absdx;
						if ((xmax == B.max[0]) || (lx  > g1)) break;
						xmax++;
						dx++;
					}
					while (1)
					{
						const double absdx = ((dx > 0) ? dx : -dx);
						const double dx2 = dx * dx;
						const double lx = dx2 - absdx;
						const double Lx = dx2 + absdx;
						if ((Lx <= g2) || (xmax < xmin)) break;
						if (outline) { if ((lx  < g2) || (Lx  <  g1)) { _updatePixel<blend, false, false, false>(xmax, y, color, 255, 0); } }
						xmax--;
						dx--;
					}
					if (fill) { if (xmin <= xmax) { _hline<blend, false>(xmin, xmax, y, fillcolor); } }
				}
			}


			/**
			* Draw an antialiased ellipse.
			* Slower but support real-valued parameters and drawing only the part inside a box.
			**/
			template<bool blend, bool fill> void _draw_ellipse2_AA(iBox2 B, fVec2 P, double rx, double ry, RGBc color, RGBc fillcolor)
				{
				if ((rx <= 0) || (ry <= 0)) return;
				B = intersectionRect(B, iBox2((int64)floor(P.X() - rx - 1),
					(int64)ceil(P.X() + rx + 1),
					(int64)floor(P.Y() - ry - 1),
					(int64)ceil(P.Y() + ry + 1)));

				if (B.isEmpty()) return;

				MTOOLS_ASSERT(B.isIncludedIn(imageBox()));

				{ //check first if there is something to draw.
					int q = _ellipseIntersection(B, P, rx, ry);
					if (q < 0) return; // nothing to draw
					if (q > 0)
						{
						if (fill) { draw_box(B, fillcolor, true); }
						return;
						}
				}

				const double ex2 = rx * rx;
				const double ey2 = ry * ry;
				const double exy2 = ex2 * ey2;
				const double Rx2 = (rx + 0.5)*(rx + 0.5);
				const double rx2 = (rx - 0.5)*(rx - 0.5);
				const double Ry2 = (ry + 0.5)*(ry + 0.5);
				const double ry2 = (ry - 0.5)*(ry - 0.5);
				const double rxy2 = rx2 * ry2;
				const double Rxy2 = Rx2 * Ry2;
				const double Rx2minus025 = Rx2 - 0.25;
				const double Rx2overRy2 = Rx2 / Ry2;
				const double rx2minus025 = rx2 - 0.25;
				const double rx2overry2 = rx2 / ry2;

				int64 xmin = B.max[0];
				int64 xmax = B.min[0];

				for (int64 y = B.min[1]; y <= B.max[1]; y++)
				{
					const double dy = (double)(y - P.Y());
					const double absdy = ((dy > 0) ? dy : -dy);
					const double dy2 = (dy*dy);

					if (xmin > xmax)
					{
						if (dy2 > Ry2) continue;  // line is empty. 
						if (P.X() <= (double)B.min[0])
						{
							const double dx = (double)B.min[0] - P.X();
							if ((dx*dx)*Ry2 + (dy2*Rx2) > Rxy2) continue; // line is empty
						}
						else if (P.X() >= (double)B.max[0])
						{
							const double dx = P.X() - (double)B.max[0];
							if ((dx*dx)*Ry2 + (dy2*Rx2) > Rxy2) continue; // line is empty
						}
						xmin = B.min[0]; xmax = B.max[0];
					}


					const double v = ex2 * dy2;
					const double vv = ex2 * v;
					const double vminusexy2 = v - exy2;
					const double ly = dy2 - absdy + 0.25;
					const double Ly = dy2 + absdy + 0.25;
					const double g1 = Rx2minus025 - Rx2overRy2 * ly;
					const double g2 = rx2minus025 - rx2overry2 * Ly;
					double dx = (double)(xmin - P.X());
					while (1)
					{
						const double absdx = ((dx > 0) ? dx : -dx);
						const double dx2 = dx * dx;
						const double lx = dx2 - absdx;
						if ((xmin == B.min[0]) || (lx > g1)) break;
						xmin--;
						dx--;
					}
					while (1)
					{
						const double absdx = ((dx > 0) ? dx : -dx);
						const double dx2 = dx * dx;
						const double lx = dx2 - absdx;
						const double Lx = dx2 + absdx;
						if ((Lx < g2) || (xmax < xmin)) break;
						if (lx < g1)
						{
							const double u = ey2 * dx2;
							const double uu = ey2 * u;
							double d = (u + vminusexy2) /sqrt(uu + vv); // d = twice the distance of the point to the ideal ellipse
							double dd =  std::min<double>(2.0, std::abs(d));
							int32 uc = (int32)(128 * dd);
							_updatePixel<blend, false, true, false>(xmin, y, color, 256 - uc, 0);
							if (fill) { if (d < 0) _updatePixel<blend, false, true, false>(xmin, y, fillcolor, uc, 0); }
						}
						xmin++;
						dx++;
					}
					dx = (double)(xmax - P.X());
					while (1)
					{
						const double absdx = ((dx > 0) ? dx : -dx);
						const double dx2 = dx * dx;
						const double lx = dx2 - absdx;
						if ((xmax == B.max[0]) || (lx > g1)) break;
						xmax++;
						dx++;
					}
					while (1)
					{
						const double absdx = ((dx > 0) ? dx : -dx);
						const double dx2 = dx * dx;
						const double lx = dx2 - absdx;
						const double Lx = dx2 + absdx;
						if ((Lx < g2) || (xmax < xmin)) break;
						if (lx < g1)
						{
							const double u = ey2 * dx2;
							const double uu = ey2 * u;
							double d = (u + vminusexy2) /sqrt(uu + vv); // d = twice the distance of the point to the ideal ellipse
							double dd = std::min<double>(2.0, std::abs(d));
							int32 uc = (int32)(128 * dd);
							_updatePixel<blend, false, true, false>(xmax, y, color, 256 - uc, 0);
							if (fill) { if (d < 0) _updatePixel<blend, false, true, false>(xmax, y, fillcolor, uc, 0); }
						}
						xmax--;
						dx--;
					}
					if (fill) { if (xmin <= xmax) { _hline<blend, false>(xmin, xmax, y, fillcolor); } }
					}
				}



			/**
			 * Draw a thick ellipse. 
			 * Support real valued paramter and drawing only the part inside a box.
			 *
			 * (arx,ary) raddi for the interior ring
			 * (Arx,Ary) radii for the exterior ring
			 */
			template<bool blend, bool fill> void _draw_ellipse_thick_AA(iBox2 B, fVec2 P, double arx, double ary, double Arx, double Ary, RGBc color, RGBc fillcolor)
				{
				if ((Arx <= 0) || (Ary <= 0)) return;
				if (arx <= 0) arx = 0;
				if (ary <= 0) ary = 0;
				if (arx >= Arx) arx = Arx;
				if (ary >= Ary) ary = Ary;

				B = intersectionRect(B, iBox2((int64)floor(P.X() - Arx - 1),
					(int64)ceil(P.X() + Arx + 1),
					(int64)floor(P.Y() - Ary - 1),
					(int64)ceil(P.Y() + Ary + 1)));

				if (B.isEmpty()) return;

				MTOOLS_ASSERT(B.isIncludedIn(imageBox()));

				{ //check first if there is something to draw.
					int q = _ellipseIntersection(B, P, Arx, Ary);
					if (q < 0) return; // nothing to draw
					q = _ellipseIntersection(B, P, arx, ary);
					if (q > 0)
						{	
						if (fill) { draw_box(B, fillcolor, true); }
						return;
						}
				}

				if (B.isEmpty()) return;
				// OUTER ELLIPSE
				const double Aex2 = Arx * Arx;
				const double Aey2 = Ary * Ary;
				const double Aexy2 = Aex2 * Aey2;
				const double ARx2 = (Arx + 0.5)*(Arx + 0.5);
				const double Arx2 = (Arx - 0.5)*(Arx - 0.5);
				const double ARy2 = (Ary + 0.5)*(Ary + 0.5);
				const double Ary2 = (Ary - 0.5)*(Ary - 0.5);
				const double Arxy2 = Arx2 * Ary2;
				const double ARxy2 = ARx2 * ARy2;
				const double ARx2minus025 = ARx2 - 0.25;
				const double ARx2overRy2 = ARx2 / ARy2;
				const double Arx2minus025 = Arx2 - 0.25;
				const double Arx2overry2 = Arx2 / Ary2;

				// INNER ELLIPSE
				const double aex2 = arx * arx;
				const double aey2 = ary * ary;
				const double aexy2 = aex2 * aey2;
				const double aRx2 = (arx + 0.5)*(arx + 0.5);
				const double arx2 = (arx - 0.5)*(arx - 0.5);
				const double aRy2 = (ary + 0.5)*(ary + 0.5);
				const double ary2 = (ary - 0.5)*(ary - 0.5);
				const double arxy2 = arx2 * ary2;
				const double aRxy2 = aRx2 * aRy2;
				const double aRx2minus025 = aRx2 - 0.25;
				const double aRx2overRy2 = aRx2 / aRy2;
				const double arx2minus025 = arx2 - 0.25;
				const double arx2overry2 = arx2 / ary2;

				int64 Axmin = B.max[0];
				int64 Axmax = B.min[0];

				int64 axmin = B.max[0];
				int64 axmax = B.min[0];

				for (int64 y = B.min[1]; y <= B.max[1]; y++)
				{
					const double dy = (double)(y - P.Y());
					const double absdy = ((dy > 0) ? dy : -dy);
					const double dy2 = (dy*dy);
					
					if (Axmin > Axmax)
					{ // OUTER FAST DISCARD
						if (dy2 > ARy2) continue;  // line is empty. 
						if (P.X() <= (double)B.min[0])
							{
							const double dx = (double)B.min[0] - P.X();
							if ((dx*dx)*ARy2 + (dy2*ARx2) > ARxy2) continue; // line is empty
							}
						else if (P.X() >= (double)B.max[0])
							{
							const double dx = P.X() - (double)B.max[0];
							if ((dx*dx)*ARy2 + (dy2*ARx2) > ARxy2) continue; // line is empty
							}
						Axmin = B.min[0];
						Axmax = B.max[0];
					}

					{ // OUTER ELLIPSE
						const double v = Aex2 * dy2;
						const double vv = Aex2 * v;
						const double vminusexy2 = v - Aexy2;
						const double ly = dy2 - absdy + 0.25;
						const double Ly = dy2 + absdy + 0.25;
						const double g1 = ARx2minus025 - ARx2overRy2 * ly;
						const double g2 = Arx2minus025 - Arx2overry2 * Ly;
						double dx = (double)(Axmin - P.X());
						while (1)
							{
							const double absdx = ((dx > 0) ? dx : -dx);
							const double dx2 = dx * dx;
							const double lx = dx2 - absdx;
							if ((Axmin == B.min[0]) || (lx > g1)) break;
							Axmin--;
							dx--;
							}
						while (1)
							{
							const double absdx = ((dx > 0) ? dx : -dx);
							const double dx2 = dx * dx;
							const double lx = dx2 - absdx;
							const double Lx = dx2 + absdx;
							if ((Lx < g2) || (Axmax < Axmin)) break;
							if (lx < g1)
								{
								const double u = Aey2 * dx2;
								const double uu = Aey2 * u;
								double d = (u + vminusexy2)/sqrt(uu + vv); // d = twice the distance of the point to the ideal ellipse
								if (d < 0) d = 0;
								if (d < 2) { _updatePixel<blend, false, true, false>(Axmin, y, color, 256 - (int32)(128 * d), 0); }
								}
							Axmin++;
							dx++;
							}

						dx = (double)(Axmax - P.X());
						while (1)
							{
							const double absdx = ((dx > 0) ? dx : -dx);
							const double dx2 = dx * dx;
							const double lx = dx2 - absdx;
							if ((Axmax == B.max[0]) || (lx > g1)) break;
							Axmax++;
							dx++;
							}
						while (1)
							{
							const double absdx = ((dx > 0) ? dx : -dx);
							const double dx2 = dx * dx;
							const double lx = dx2 - absdx;
							const double Lx = dx2 + absdx;
							if ((Lx < g2) || (Axmax < Axmin)) break;
							if (lx < g1)
								{
								const double u = Aey2 * dx2;
								const double uu = Aey2 * u;
								double d = (u + vminusexy2)/ sqrt(uu + vv); // d = twice the distance of the point to the ideal ellipse
								if (d < 0) d = 0;
								if (d < 2) { _updatePixel<blend, false, true, false>(Axmax, y, color, 256 - (int32)(128 * d), 0); }
								}
							Axmax--;
							dx--;
							}
					}
					
					int64 fmin = B.max[0] + 1;
					int64 fmax = B.min[0] - 1;
					int64 mind = B.max[0] + 1;
					int64 maxd = B.min[0] - 1;

					if (axmin > axmax)
						{ // INNER FAST DISCARD
						if (dy2 > aRy2) goto end_loop;  // line is empty. 
						if (P.X() <= (double)B.min[0])
							{
							const double dx = (double)B.min[0] - P.X();
							if ((dx*dx)*aRy2 + (dy2*aRx2) > aRxy2) goto end_loop; // line is empty
							}
						else if (P.X() >= (double)B.max[0])
							{
							const double dx = P.X() - (double)B.max[0];
							if ((dx*dx)*aRy2 + (dy2*aRx2) > aRxy2) goto end_loop; // line is empty
							}
						axmin = B.min[0];
						axmax = B.max[0];
						}

					{ // INNER ELLIPSE
						const double v = aex2 * dy2;
						const double vv = aex2 * v;
						const double vminusexy2 = v - aexy2;
						const double ly = dy2 - absdy + 0.25;
						const double Ly = dy2 + absdy + 0.25;
						const double g1 = aRx2minus025 - aRx2overRy2 * ly;
						const double g2 = arx2minus025 - arx2overry2 * Ly;
						double dx = (double)(axmin - P.X());
						while (1)
							{
							const double absdx = ((dx > 0) ? dx : -dx);
							const double dx2 = dx * dx;
							const double lx = dx2 - absdx;
							if ((axmin == B.min[0]) || (lx > g1)) break;
							axmin--;
							dx--;
							}
						while (1)
						{
							const double absdx = ((dx > 0) ? dx : -dx);
							const double dx2 = dx * dx;
							const double lx = dx2 - absdx;
							const double Lx = dx2 + absdx;
							if ((Lx < g2) || (axmax < axmin)) break;
							if (lx < g1)
								{
								const double u = aey2 * dx2;
								const double uu = aey2 * u;
								double d = -((u + vminusexy2) / sqrt(uu + vv)); // d = twice the distance of the point to the ideal ellipse
								d = std::min<double>(2.0, std::max<double>(0.0, d));
								fmin = std::min<int64>(axmin, fmin);
								mind = axmin;
								const int32 uc = (int32)(128 * d);
								_updatePixel<blend, false, true, false>(axmin, y, color, 256 - uc, 0);
								if (fill) _updatePixel<blend, false, true, false>(axmin, y, fillcolor, uc, 0);
								}
							axmin++;
							dx++;
						}

						dx = (double)(axmax - P.X());
						while (1)
							{
							const double absdx = ((dx > 0) ? dx : -dx);
							const double dx2 = dx * dx;
							const double lx = dx2 - absdx;
							if ((axmax == B.max[0]) || (lx > g1)) break;
							axmax++;
							dx++;
							}
						while (1)
							{
							const double absdx = ((dx > 0) ? dx : -dx);
							const double dx2 = dx * dx;
							const double lx = dx2 - absdx;
							const double Lx = dx2 + absdx;
							if ((Lx < g2) || (axmax < axmin)) break;
							if (lx < g1)
								{
								const double u = aey2 * dx2;
								const double uu = aey2 * u;
								double d = -((u + vminusexy2) /sqrt(uu + vv)); // d = twice the distance of the point to the ideal ellipse
								d = std::min<double>(2.0, std::max<double>(0.0, d));
								fmax = std::max<int64>(axmax, fmax);
								maxd = axmax;
								const int32 uc = (int32)(128 * d);
								_updatePixel<blend, false, true, false>(axmax, y, color, 256 - uc, 0);
								if (fill) _updatePixel<blend, false, true, false>(axmax, y, fillcolor, uc, 0);
								}
							axmax--;
							dx--;
							}
					}

				end_loop:
					if (Axmin <= Axmax)
						{						
						if ((fmin > B.max[0]) && (fmax < B.min[0]))
							{
							if (axmin <= axmax)
								{
								if (fill) { _hline<blend, false>(axmin, axmax, y, fillcolor); }
								}
							else
								{
								_hline<blend, false>(Axmin, Axmax, y, color);
								}
						}
						else
							{
							if (fmin <= B.max[0]) { _hline<blend, false>(Axmin, fmin - 1, y, color); }
							else 
								{ 
								if (axmax == axmin -1)  _hline<blend, false>(Axmin, maxd - 1, y, color); 
								}
							if (fmax >= B.min[0]) { _hline<blend, false>(fmax + 1, Axmax, y, color); }
							else
								{ 
								if (axmax == axmin - 1) _hline<blend, false>(mind + 1, Axmax, y, color);
								}
							if (fill) { _hline<blend, false>(axmin, axmax, y, fillcolor); }
							}							
						}					
				}
			}




			/******************************************************************************************************************************************************
			*																				   																      *
			*                                                                       CAIRO                                                                         *
			* These methods affect: _pcairo_surface, _pcairo_context 		     																				  *
			*******************************************************************************************************************************************************/

#if (MTOOLS_USE_CAIRO)

			/* tell cairo that the data buffer is possibly dirty */
			inline void _cairomarkdirty() const
				{
				if (_pcairo_surface != nullptr)	cairo_surface_mark_dirty((cairo_surface_t *)_pcairo_surface);
				}

			/* flush all cairo operation */
			inline void _cairoflush() const
				{
				if (_pcairo_surface != nullptr)	cairo_surface_flush((cairo_surface_t *)_pcairo_surface);
				}

			/* remove the cairo objects */
			inline void _removecairo() const
				{
				if (_pcairo_context != nullptr) { cairo_destroy((cairo_t *)_pcairo_surface); _pcairo_context = nullptr; }
				if (_pcairo_surface != nullptr) { cairo_surface_destroy((cairo_surface_t *)_pcairo_surface); _pcairo_surface = nullptr; }
				}

			/* create the cairo objects if needed */
			inline bool _createcairo(bool stopOnError) const
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

#else

			inline void _removecairo() const
				{
				_pcairo_context = nullptr;
				_pcairo_surface = nullptr;
				}


#endif


			/*******************************************************************************************************************************************************
			 *																																					   *
			 *                                                              MEMORY MANAGEMENT                                                                      *
			 * These methods affect: _deletepointer and _data                                                                                                      *
			 *******************************************************************************************************************************************************/


			/* return the initial position of the beginning of the memory buffer, or nullptr if there are none. */
			MTOOLS_FORCEINLINE uint32 * _beginOriginalBuffer() const
				{
				if ((_data == nullptr) || (_deletepointer == nullptr)) return nullptr;				
				uint32 * p = ((uint32**)_deletepointer)[1];
				return p + ((p == _deletepointer) ? 4 : 0);
				}


			/* return true is the memory buffer is external */
			MTOOLS_FORCEINLINE bool _isExternalBuffer() const
				{
				if ((_data == nullptr) || (_deletepointer == nullptr)) return false;
				uint32 * p = ((uint32**)_deletepointer)[1];
				return (p != _deletepointer);
				}


			/* allocate memory, updates _data, and _deletepointer */
			MTOOLS_FORCEINLINE void _allocate(int64 ly, int64 stride, RGBc * databuffer)
				{
				size_t memsize = 16 + (size_t)((databuffer == nullptr) ? (4*ly*stride) : 0); // 16 byte + the image buffer size if needed.
				_deletepointer = (uint32*)malloc(memsize);
				if (_deletepointer == nullptr) { MTOOLS_ERROR(std::string("malloc error: cannot allocate ") + mtools::toStringMemSize(memsize)); }
				_deletepointer[0] = 1; // set reference count to 1
				((uint32**)_deletepointer)[1] = (databuffer == nullptr) ? _deletepointer : (uint32*)databuffer; // use to track the beginning of the buffer. 
				_data = (databuffer == nullptr) ? ((RGBc*)(_deletepointer + 4)) : databuffer; // if allocated, buffer start 16 bytes (4 uint32) after the deletepointr.
				}

			/* decrease reference count and deallocate if not buffer not referenced anymore */
			MTOOLS_FORCEINLINE void _deallocate()
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
			MTOOLS_FORCEINLINE void _shallow_copy(uint32 * deletepointer, RGBc * data)
				{
				(*deletepointer)++;
				_deletepointer = deletepointer;
				_data = data;
				}



			/******************************************************************************************************************************************************
			*																				   																      *
			*                                                                       DATA                                                                          *
			*																																					  *
			*******************************************************************************************************************************************************/


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

