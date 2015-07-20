# mtools: My Maths Toolkit

- Author:   [Arvind singh](mailto:arvind.singh@normalesup.org)
- Licence:  [GPLv3](http://www.gnu.org/licenses/gpl-3.0.html)


### About

This repository contain a collection of C++ classes I use when performing simulations and displaying graphics. It consists mostly of code that I accumulated over the years which I finally decided to put inside a library. I find it quite practical so I am sharing it in the hope that you may also find it useful.

I cleaned up and rewrote several part of the library in order to take advantage of some of the new C++11 features. Therefore, you will need a C++ compiler with decent C++11 support to build the library.   

Using the library is straightforward. Every class/function is located under the `mtools` namespace. All you need to do is link your program against the binary of the library. Then, make sure mtools's header directory is in the include path and use '#include "mtools.hpp"' (this header includes all the other required headers of the library).

The library itself is divided in several parts:

  - **/randomgen/** : Some classical random generators such as Mersenne Twister, Xorgen... and some methods to generate specific probability distributions such as stable laws,  distribution of SRW...
  - **/math/** : Not much in there yet. Just some classes for rectangles, vectors, matrices and some very elementary linear algebra.
  - **/io/** : Contains stuff related to input/output:
    - `IArchive`/`OArchive`. Classes used for serialization of objects in the spirit of boost::serialization but much simpler, without forward serialization of pointer type but with the possibility of including comments to improve the readidability of the archives.
    - `Console`. Class which displays a text console that can also log into a file. The console can be used for simple text input/output. A global `mtools::cout` object is defined and can (or not) be used as a substitute for std::cout/std::cin.
    - `LogFile`. A very simple class for logging into a file.
  - **/container/** : This directory contain template classes for holding objects (a little bit like the stl containers).
    - `Grid_basic`/`Grid_factor`. Template classes which simulate an infinite d-dimensional grid Z^d where each vertex of the grid holds an object of the template type. The grid is dynamically constructed when objects are read/accessed and the internal structure is similar to that of an octree with some additional refinements. The whole grid can copied and saved/read from a file. The Grid_factor version also permits 'factorization' of objects (i.e. the same object can be re-used for distinct positions in the grid). This class is particularly useful when simulating multi-dimensional processes such as random walks, growing domains, percolation processes, interacting particle processes etc... Check it out, it is really neat !
    - Other outdated container classes such as `RWtreegraph`, `randomurn` ...  I plan to rewrite them from scratch at some point...
  - **/graphics/** This is the biggest (and best!) part of the library. It define a set of class which are use to plot 2D grpahics. Basically, anything that can be embeded in Z^2 can be plotted/manipulated and zoom in&out interactively. The main class is the Ploter2D class wich display the plotter window and enable the user interaction. It is posible to move the center/zomm in/out using the arrow key and or the mouse. It is also posible to set specific options for each drawing and chosoe the drawing order of the plotted object as well as their transparency and refresh rate of the drawing. It is also possible to save the view as a image file  
  The main class is Plotter2D which creeate a plotter that can print pretty much anything that has a
    - `Plotter2D` : this is the main class. It displays the plotter windows 
  - **/misc/** Contains general purpose code. Some interesting headers:
    - 'timefct.hpp' : Contain methods dealing with time. In particular, class `ProgressBar` display a progress bar window.
    - 'memory.hpp' : Class `SingleAllocator` define a simple but very fast memory allocator.
    - 'metaprog.hpp' : Contain several meta-programming tricks for detecting existence of member functions/operators.
    - 'error.hpp' : Contain functions and macros for dealing with errors.
    - 'stringfct' : contain various functions for manipulating string. In particular, the `toString()` method used to convert any object into a string.


### Documentation
I try to keep the documentation fairly up to date. Information about a class or a method is located in the corresponding header file above its declaration. Comments are in doxygen format: you can use the 'Doxyfile' file to build the documentation for the whole library using doxygen. It is also worth looking into the /examples/ directory which contain sample code demonstrating how to use the library.


### Building the library
The library is cross-platform (at least, it work under Linux and Windows) but it depends on some other libraries such has CImg and FLTK. See the file 'INSTALL.TXT' for detailed instructions on how to build and use the library depending on your platform.



