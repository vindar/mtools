/** @file imagewidget.cpp */
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


/*
#if defined(__APPLE__)
#  include <OpenGL/gl3.h> // defines OpenGL 3.0+ functions
#else
#  if defined(WIN32)
#    define GLEW_STATIC 1
#  endif
#  include <GL/glew.h>
#endif
*/

#include <FL/gl.h>

#include <stdio.h>
#include <math.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>


#include "graphics/internal/imagewidget.hpp"


namespace mtools
{

    namespace internals_graphics
    {


        ImageWidgetFL::ImageWidgetFL(int X, int Y, int W, int H, const char *l) : 
			Fl_Window(X, Y, W, H, l), _offbuf((Fl_Offscreen)0), 
			_ox(0), _oy(0), 
			_initdraw(false), _saved_im(nullptr), _saved_im32(nullptr)
        {
        }


        
		ImageWidgetFL::~ImageWidgetFL()
            {
            if (_offbuf != ((Fl_Offscreen)0)) { fl_delete_offscreen((Fl_Offscreen)(_offbuf)); }
            delete _saved_im; _saved_im = nullptr;
            delete _saved_im32; _saved_im32 = nullptr;
            }


        void ImageWidgetFL::setImage(const Image * im)
        {
            std::lock_guard<std::recursive_mutex> lock(_mutim);
            if ((im == nullptr) || (im->isEmpty()))
                {
                if (_offbuf != ((Fl_Offscreen)0)) { fl_delete_offscreen((Fl_Offscreen)(_offbuf)); }
                _offbuf = (Fl_Offscreen)0;
                _ox = 0; _oy = 0;
                redraw();  
                return;
                }
            if (!_initdraw) // prevent FLTK bug on linux by saving the image without displaying it if init is not yet done. 
                {
                delete _saved_im; _saved_im = nullptr;
                delete _saved_im32; _saved_im32 = nullptr;
                _saved_im = new Image(*im, false); // deep copy
                return;
                }
            const int nox = (int)im->width();
            const int noy = (int)im->height();
            if ((nox != _ox) || (noy != _oy))
                {
                if (_offbuf != ((Fl_Offscreen)0)) { fl_delete_offscreen((Fl_Offscreen)(_offbuf)); }
                _offbuf = fl_create_offscreen(nox, noy);
                _ox = nox; _oy = noy;
                MTOOLS_ASSERT(_offbuf != ((Fl_Offscreen)0));
                }
            fl_begin_offscreen((Fl_Offscreen)(_offbuf));
            fl_draw_image(_drawLine_callback, (void*)im, 0, 0, (int)_ox, (int)_oy, 3);
            fl_end_offscreen();
            return;
        }


        void ImageWidgetFL::_drawLine_callback(void * data, int x, int y, int w, uchar *buf)
            {
            const Image * im = (const Image *)data;
			const RGBc * p = im->offset(x, y);
			uchar * d = buf;
            for (int l = 0; l < w; l++) 
				{ 
				*(d++) = p->comp.R;
				*(d++) = p->comp.G;
				*(d++) = p->comp.B;
				p++;
				}
            }




        void ImageWidgetFL::setImage(const ProgressImg * im)
            {
            std::lock_guard<std::recursive_mutex> lock(_mutim);
            if ((im == nullptr) || (im->isEmpty()))
                {
                if (_offbuf != ((Fl_Offscreen)0)) { fl_delete_offscreen((Fl_Offscreen)(_offbuf)); }
                _offbuf = (Fl_Offscreen)0;
                _ox = 0; _oy = 0;
                redraw();
                return;
                }
            if (!_initdraw) // prevent FLTK bug on linux
                {
                delete _saved_im; _saved_im = nullptr;
                delete _saved_im32; _saved_im32 = nullptr;
                _saved_im32 = new ProgressImg(*im); // deep copy
                return;
                }
            const int nox = (int)im->width();
            const int noy = (int)im->height();
            if ((nox != _ox) || (noy != _oy))
                {
                if (_offbuf != ((Fl_Offscreen)0)) { fl_delete_offscreen((Fl_Offscreen)(_offbuf)); }
                _offbuf = fl_create_offscreen(nox, noy);
                _ox = nox; _oy = noy;
                MTOOLS_ASSERT(_offbuf != ((Fl_Offscreen)0));
                }
            fl_begin_offscreen((Fl_Offscreen)(_offbuf));
            fl_draw_image(_drawLine_callback2, (void*)im, 0, 0, (int)_ox, (int)_oy, 3);
            fl_end_offscreen();
            return;
            }


        void ImageWidgetFL::_drawLine_callback2(void * data, int x, int y, int w, uchar *buf)
            {
			const ProgressImg * im = (const ProgressImg *)data;
			const uint32  nb = *(im->normData()) + 1; // all pixels have the same normaliszation
			const RGBc64 * p = im->imData(x, y);
			uchar * d = buf;
			switch (nb)
				{
				case 1   : { for (int l = 0; l < w; l++) { *(d++) = (uchar)(p->comp.R);      *(d++) = (uchar)(p->comp.G);      *(d++) = (uchar)(p->comp.B);       p++; } break; }
				case 2   : { for (int l = 0; l < w; l++) { *(d++) = (uchar)(p->comp.R >> 1); *(d++) = (uchar)(p->comp.G >> 1); *(d++) = (uchar)(p->comp.B >> 1);  p++; } break; }
				case 4   : { for (int l = 0; l < w; l++) { *(d++) = (uchar)(p->comp.R >> 2); *(d++) = (uchar)(p->comp.G >> 2); *(d++) = (uchar)(p->comp.B >> 2);  p++; } break; }
				case 8   : { for (int l = 0; l < w; l++) { *(d++) = (uchar)(p->comp.R >> 3); *(d++) = (uchar)(p->comp.G >> 3); *(d++) = (uchar)(p->comp.B >> 3);  p++; } break; }
				case 16  : { for (int l = 0; l < w; l++) { *(d++) = (uchar)(p->comp.R >> 4); *(d++) = (uchar)(p->comp.G >> 4); *(d++) = (uchar)(p->comp.B >> 4);  p++; } break; }
				case 32  : { for (int l = 0; l < w; l++) { *(d++) = (uchar)(p->comp.R >> 5); *(d++) = (uchar)(p->comp.G >> 5); *(d++) = (uchar)(p->comp.B >> 5);  p++; } break; }
				case 64  : { for (int l = 0; l < w; l++) { *(d++) = (uchar)(p->comp.R >> 6); *(d++) = (uchar)(p->comp.G >> 6); *(d++) = (uchar)(p->comp.B >> 6);  p++; } break; }
				case 128 : { for (int l = 0; l < w; l++) { *(d++) = (uchar)(p->comp.R >> 7); *(d++) = (uchar)(p->comp.G >> 7); *(d++) = (uchar)(p->comp.B >> 7);  p++; } break; }
				case 256 : { for (int l = 0; l < w; l++) { *(d++) = (uchar)(p->comp.R >> 8); *(d++) = (uchar)(p->comp.G >> 8); *(d++) = (uchar)(p->comp.B >> 8);  p++; } break; }
				default: 
					{ 
					for (int l = 0; l < w; l++) { *(d++) = (uchar)(p->comp.R / nb); *(d++) = (uchar)(p->comp.G / nb); *(d++) = (uchar)(p->comp.B / nb);  p++; }
					break; 
					}
				}
            }



        void ImageWidgetFL::partDraw(const iBox2 & r)
            {
            if (!_initdraw) { draw(); return; }
            else
                {
                std::lock_guard<std::recursive_mutex> lock(_mutim);
                iBox2 rr = mtools::intersectionRect(r, iBox2(0, _ox - 1, 0, _oy - 1));
                if ((_offbuf == ((Fl_Offscreen)0))||(rr.lx() < 0) || (rr.ly() < 0)) return;
                fl_copy_offscreen((int)rr.min[0], (int)rr.min[1], (int)rr.lx() + 1, (int)rr.ly() + 1, (Fl_Offscreen)_offbuf, (int)rr.min[0], (int)rr.min[1]);
                }
            }


        void ImageWidgetFL::draw()
            {
            if ((!_initdraw) || (w() > ((int)_ox)) || (h() > ((int)_oy))) { Fl_Window::draw(); } // first time or base widget showing : redraw it.
                {
                std::lock_guard<std::recursive_mutex> lock(_mutim);

                if (!_initdraw)
                    { 
                    _initdraw = true;
                    if (_saved_im != nullptr) { setImage(_saved_im); delete _saved_im; _saved_im = nullptr; }
                    if (_saved_im32 != nullptr) { setImage(_saved_im32); delete _saved_im32; _saved_im32 = nullptr; }
                    }
                int lx = ((int)_ox > w()) ? w() : (int)_ox;
                int ly = ((int)_oy > h()) ? h() : (int)_oy;
                if ((_offbuf == ((Fl_Offscreen)0)) || (_ox <= 0) || (_oy <= 0)) { return; }

				fl_copy_offscreen(0, 0, lx, ly,(Fl_Offscreen)_offbuf, 0, 0);
                }
            }


		void ImageWidgetFL::set_color(RGBc col)
			{
			fl_color(col.comp.R, col.comp.G, col.comp.B);
			}


		void ImageWidgetFL::draw_line(iVec2 P1, iVec2 P2)
			{
			fl_line((int)P1.X(), (int)P1.Y(), (int)P2.X(), (int)P2.Y());
			}


		void ImageWidgetFL::draw_rect(iBox2 B)
			{
			fl_line((int)B.min[0], (int)B.min[1], (int)B.max[0], (int)B.min[1]);
			fl_line((int)B.min[0], (int)B.max[1], (int)B.max[0], (int)B.max[1]);
			fl_line((int)B.min[0], (int)B.min[1], (int)B.min[0], (int)B.max[1]);
			fl_line((int)B.max[0], (int)B.min[1], (int)B.max[0], (int)B.max[1]);
			}


		void ImageWidgetFL::draw_text(const std::string & text, iBox2 B, int fontsize, int text_offx, int text_offy, RGBc color, RGBc bkcolor)
			{
			if (!bkcolor.isTransparent())
				{
				set_color(bkcolor);
				fl_rectf((int)B.min[0], (int)B.min[1], (int)(B.lx() + 1), (int)(B.ly() + 1));
				}
			set_color(color);
			fl_font(FL_HELVETICA, fontsize);
			fl_draw(text.c_str(), (int)(B.min[0] + text_offx), (int)(B.max[1] - fl_descent() - text_offy));
			}









	// for windows, we may need to define GL_BGRA if we do not load OpenGL 3
	#ifdef GL_BGRA_EXT
	#ifndef GL_BGRA
	#define GL_BGRA GL_BGRA_EXT
	#endif
	#endif

			ImageWidgetGL::ImageWidgetGL(int X, int Y, int W, int H, const char *l) 
			                  : Fl_Gl_Window(X, Y, W, H, l),  _ox(0), _oy(0), _tex_lx(0), _tex_ly(0), _texID{0,0}, _current_tex(0), _init_done(false), _tmp_im(), _missed_draw(false)
					{ 
					//mode(FL_RGB8 | FL_DOUBLE); // set the mode
					//mode(FL_RGB8 | FL_DOUBLE | FL_OPENGL3); // set the mode
					}


			ImageWidgetGL::~ImageWidgetGL()
					{
					_ox = 0; 
					_oy = 0; 
					_tex_lx = 0; 
					_tex_ly = 0; 
					glDeleteTextures(1, (GLuint*)&(_texID[0])); _texID[0] = 0;
					glDeleteTextures(1, (GLuint*)&(_texID[1])); _texID[1] = 0;
					}

			// OPENGL 3
			/*
			int ImageWidgetGL::handle(int event) 
				{
				static int first = 1;
				if (first && event == FL_SHOW && shown()) 
					{
					first = 0;
					make_current();
					#ifndef __APPLE__
					GLenum err = glewInit(); // defines pters to functions of OpenGL V 1.2 and above
					if (err) Fl::warning("glewInit() failed returning %u", err);
					#endif
					}
				return Fl_Gl_Window::handle(event);
				}
			*/			



			void ImageWidgetGL::setImage(const Image * im)
				{
				// run in fltk thread
				if (!mtools::isFltkThread())
					{
					mtools::IndirectMemberProc<ImageWidgetGL, const Image *> ID((*this), &ImageWidgetGL::setImage, im);
					mtools::runInFltkThread(ID);
					return;
					}

				if  ((context() == nullptr) || (((bool)_init_done) == false) ) // not opengl context yet (maybe because we are minimized) or not init yet.
					{ 
					if (im != (&_tmp_im)) { _tmp_im = im->get_standalone(); }	// save the image
					_missed_draw = true;										// set the flag to draw next time draw() is called
					return;
					}

				// ok, we are in fltk thread, context is valid so we can do opengl stuffs
				make_current(); // select the opengl context for this window. 

				if (im->isEmpty())
					{ // empty image so we draw nothing
					_ox = 0;
					_oy = 0;
					}
				else
					{ // non empty image
					_ox = (int)im->width();		// new image size
					_oy = (int)im->height();		//

					int new_tex_lx = mtools::pow2roundup(_ox);	// new texture size
					int new_tex_ly = mtools::pow2roundup(_oy);	// 

					if ((new_tex_lx != _tex_lx) || (new_tex_ly != _tex_ly))
						{ // we must (re)-create new textures with the correct size
						_tex_lx = new_tex_lx;	// save new texture size
						_tex_ly = new_tex_ly;	//

						glBindTexture(GL_TEXTURE_2D, _texID[0]);
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, new_tex_lx, new_tex_ly, 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);	// allocate memory but copy nothing
						glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
						glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
						MTOOLS_INSURE(glGetError() == GL_NO_ERROR);

						glBindTexture(GL_TEXTURE_2D, _texID[1]);
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, new_tex_lx, new_tex_ly, 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);	// allocate memory but copy nothing
						glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
						glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
						MTOOLS_INSURE(glGetError() == GL_NO_ERROR);
						}
					// update the part of the texture that contain the image

					_current_tex = 1 - _current_tex; // swap texture

					glBindTexture(GL_TEXTURE_2D, _texID[_current_tex]);
					glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint)im->stride());
					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _ox, _oy, GL_BGRA, GL_UNSIGNED_BYTE, im->data()); // copy image to texture
									
					MTOOLS_INSURE(glGetError() == GL_NO_ERROR);
					}			
				redraw();	// schedule redraw soon		
				}




			void ImageWidgetGL::setImage(const ProgressImg * im)
				{
				// run in fltk thread
  				if (!mtools::isFltkThread())
					{
					mtools::IndirectMemberProc<ImageWidgetGL, const ProgressImg *> ID((*this), &ImageWidgetGL::setImage, im);
					mtools::runInFltkThread(ID);
					return;
					}		
				if (im->isEmpty()) { setImage(); return; }

				const size_t lx = im->width();
				const size_t ly = im->height();

				_tmp_im.resizeRaw(lx,ly,false);		// resize if needed

				const uint32  nb = *(im->normData()) + 1; // all pixels have the same normaliszation
				RGBc * pdst = _tmp_im.data();
				const RGBc64 * psrc = im->imData();
				const size_t stride_dst = lx; 
				const size_t stride_src = _tmp_im.stride();

				if (nb == 1)
					{
					for (size_t j = 0; j < ly; j++)
						{
						for (size_t i = 0; i < lx; i++) { pdst[i] = psrc[i]; pdst[i].comp.A = 255; } pdst += stride_dst; psrc += stride_src;
						}
					}
				else
					{
					for (size_t j = 0; j < ly; j++)
						{
						for (size_t i = 0; i < lx; i++) { pdst[i].fromRGBc64(psrc[i], nb); pdst[i].comp.A = 255; } pdst += stride_dst; psrc += stride_src;
						}
					}
				setImage(&_tmp_im);
				}

		

			void ImageWidgetGL::partDraw(const iBox2 & r)
				{
				MTOOLS_ERROR("Not implemented, should not have been called !");
				}



			void ImageWidgetGL::draw()
				{
				if (!valid())
					{ // windows is not valid (size may have changed)
					valid(1);								
					init(); 

					const int ww = w();
					const int hh = h();
					MTOOLS_INSURE((ww > 0) && (hh > 0));

					// set viewport
					glViewport(0, 0, ww, hh); // Viewport
					glMatrixMode(GL_PROJECTION);
					glLoadIdentity();
					glMatrixMode(GL_MODELVIEW);
					glLoadIdentity();
					MTOOLS_INSURE(glGetError() == GL_NO_ERROR);
					}

				if (_missed_draw)
					{ // we missed a drawing so we do it now 
					setImage(&_tmp_im);
					_missed_draw = false;
					}

				// Clear the screen to Gray
				// glClearColor(0.5, 0.5, 0.5, 1.0);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				// size of windows and image
				const double W = (double)w();			// screen size
				const double H = (double)h();			//
				const double OX = (double)_ox;			// image_size
				const double OY = (double)_oy;			//

				// quits if there is no image to draw.
				if ((OX == 0) || (OY == 0)) return;	

				// load current texture
				glBindTexture(GL_TEXTURE_2D, _texID[_current_tex]);
				glEnable(GL_TEXTURE_2D);
				glDisable(GL_DEPTH_TEST);

				// compute coordinates
				GLfloat tx, ty;
				GLfloat vx, vy;
				if (OX <= W)
					{
					tx = 1.0; 
					vx = -1.0f + (GLfloat)(2*(OX/W));
					}
				else
					{
					tx = (GLfloat)(W/OX);
					vx = 1.0f;
					}
				if (OY <= H)
					{
					ty = 1.0f; 
					vy = -1.0f + (GLfloat)(2*(OY/H)); 
					}
				else
					{
					ty= (GLfloat)(H/OY);
					vy = 1.0f;
					}
				tx *= (GLfloat)(OX / _tex_lx);	
				ty *= (GLfloat)(OY / _tex_ly);
				
				// blit on screen				
				glLoadIdentity();				
				glBegin(GL_QUADS);
				glTexCoord2f(0.0f, ty);		glVertex2f(-1.0,    -vy);	// top left
				glTexCoord2f(tx, ty);		glVertex2f(vx,      -vy);	// top right
				glTexCoord2f(tx, 0.0f);		glVertex2f(vx,     1.0);	// bottom right
				glTexCoord2f(0.0f, 0.0f);	glVertex2f(-1.0,   1.0);    // bottom left
				glEnd();		
				glDisable(GL_TEXTURE_2D);			

				MTOOLS_INSURE(glGetError() == GL_NO_ERROR);	// check for errors
				}




			void ImageWidgetGL::init()
				{
				if (((bool)_init_done) == false)
					{ // initialization, performed only once
					glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);   // use actual texture colors
					glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
					glGenTextures(2, (GLuint*)_texID);							// generate 2 textures
					_tex_lx = 0; _tex_ly = 0;									// no buffer assigned yet
					MTOOLS_INSURE(glGetError() == GL_NO_ERROR);					// check everything is ok.
					_init_done = true;
					}
				}



		void ImageWidgetGL::set_color(RGBc col)
			{
			glColor3f(col.comp.R/255.0f, col.comp.G/255.0f, col.comp.B/255.0f);
			}


		void ImageWidgetGL::draw_line(iVec2 P1, iVec2 P2)
			{
			const double ww = (double)w();
			const double hh = (double)h();
			const fBox2 srcB(-0.5,ww-0.5,-0.5,hh-0.5);
			const fBox2 dstB(-1.0,1.0,-1.0,1.0);
			fVec2 fP1 = boxTransform<true>((fVec2)P1, srcB, dstB);
			fVec2 fP2 = boxTransform<true>((fVec2)P2, srcB, dstB);
			glBegin(GL_LINES);
			glVertex2f((GLfloat)fP1.X(), (GLfloat)fP1.Y());
			glVertex2f((GLfloat)fP2.X(), (GLfloat)fP2.Y());
			glEnd();
			}


		void ImageWidgetGL::draw_rect(iBox2 B)
			{
			const double ww = (double)w();
			const double hh = (double)h();
			const fBox2 srcB(-0.5,ww-0.5,-0.5,hh-0.5);
			const fBox2 dstB(-1.0,1.0,-1.0,1.0);
			fBox2 fB = boxTransform<true>((fBox2)B, srcB, dstB);
			glBegin(GL_LINES);
			glVertex2f((GLfloat)fB.min[0], (GLfloat)fB.min[1]); glVertex2f((GLfloat)fB.max[0], (GLfloat)fB.min[1]);
			glVertex2f((GLfloat)fB.min[0], (GLfloat)fB.max[1]); glVertex2f((GLfloat)fB.max[0], (GLfloat)fB.max[1]);
			glVertex2f((GLfloat)fB.min[0], (GLfloat)fB.min[1]); glVertex2f((GLfloat)fB.min[0], (GLfloat)fB.max[1]);
			glVertex2f((GLfloat)fB.max[0], (GLfloat)fB.min[1]); glVertex2f((GLfloat)fB.max[0], (GLfloat)fB.max[1]);
			glEnd();
			}


		void ImageWidgetGL::draw_text(std::string text, iBox2 B, int fontsize, int text_offx, int text_offy, RGBc color, RGBc bkcolor)
			{
			const double ww = (double)w();
			const double hh = (double)h();
			const fBox2 srcB(-0.0,ww-1,-0.0,hh-1.0);
			const fBox2 dstB(-1.0,1.0,-1.0,1.0);
			fBox2 fB = boxTransform<true>((fBox2)B, srcB, dstB);
			const float offx = (GLfloat)((2*text_offx)/ww);
			const float offy = (GLfloat)((2*text_offy)/hh);
			const float ffo = (GLfloat)((2*fontsize) / hh);

			if (!bkcolor.isTransparent())
				{
				set_color(bkcolor);
				glRectf((GLfloat)fB.min[0], (GLfloat)fB.min[1], (GLfloat)fB.max[0], (GLfloat)fB.max[1]);
				}

			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);   // use actual texture colors

			gl_font(1, fontsize);
			set_color(color);
			gl_draw(text.c_str(), text.size(), (GLfloat)(fB.min[0]) + offx, (GLfloat)(fB.min[1]) + offy + ffo/2);

			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);   // use actual texture colors
			return;
			}




    }
}
/* end of file */

