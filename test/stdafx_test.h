
/** pre-compiled header */

//#pragma message("Compiling precompiled headers for stdafx_test.h\n")

//
// STL
//
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <cwchar>

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

#if defined (_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 4312 )
#pragma warning( disable : 4319 )
#endif


//
// FLTK
//
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Text_Display.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Color_Chooser.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/filename.H>
#include <FL/fl_draw.H>

// and misc fltk headers
#include "zlib.h"       // fltk zlib
#include "png.h"        // fltk libpng
#include "jpeglib.h"    // fltk libjpeg


#if defined (_MSC_VER)
#pragma warning( pop )
#endif

//
// MTOOLS
//
#include "mtools.hpp"




