/***********************************************
 * Project : eDLA_2D
 * date : Tue Dec 15 17:09:44 2015
 ***********************************************/

#pragma message("Compiling pre-compiled headers for stdafx.h\n")
#pragma once

// STL
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cmath>  
#include <cwchar>
#include <locale>

#include <utility>
#include <type_traits>
#include <typeinfo>
#include <string>
#include <algorithm>
#include <limits>
#include <initializer_list>
#include <chrono>

#include <ostream>
#include <fstream>
#include <iostream>

#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>

#include <array>
#include <vector>
#include <deque>
#include <forward_list>
#include <list>
#include <stack>
#include <queue>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>


// *** Library mtools ***
// Uncomment the following line to disable mtools console and use stdout instead
// #define MTOOLS_BASIC_CONSOLE
#include "mtools.hpp"

// *** Library fltk ***
#if defined (_MSC_VER) 
#pragma warning( push )
#pragma warning( disable : 4312 )
#pragma warning( disable : 4319 )
#endif
#include "FL/Fl.H"
#include "FL/Fl_Window.H"
#include "FL/Fl_Double_Window.H"
#include "FL/Fl_Box.H"
#include "FL/Fl_Input.H"
#include "FL/Fl_Button.H"
#include "FL/Fl_Check_Button.H"
#include "FL/Fl_Round_Button.H"
#include "FL/Fl_Toggle_Button.H"
#include "FL/Fl_Value_Slider.H"
#include "FL/Fl_Scroll.H"
#include "FL/Fl_Progress.H"
#include "FL/Fl_Pack.H"
#include "FL/Fl_Text_Display.H"
#include "FL/fl_ask.H"
#include "FL/Fl_Color_Chooser.H"
#include "FL/Fl_File_Chooser.H"
#include "FL/filename.H"
#include "FL/fl_draw.H"
#if defined (_MSC_VER) 
#pragma warning( pop )
#endif

// *** Library fltk ***
#include "zlib.h"       // fltk zlib
#include "png.h"        // fltk libpng
#include "jpeglib.h"    // fltk libjpeg



/* end of file stdafx.h */
