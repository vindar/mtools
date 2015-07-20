# mtools: My Maths Toolkit

- Author:   [Arvind singh](mailto:arvind.singh@normalesup.org)
- Licence:  [GPLv3](http://www.gnu.org/licenses/gpl-3.0.html)


### About

This repository contain a collection of C++ classes I created for performing simulations and displaying graphics. It consists mostly of code that I accumulated over the years that I finally decided to gather inside a library. I find it quite practical so I am sharing it in the hope that you may also find it useful.

I cleaned up and rewrote several part of the library in order to take advantage of some of the new C++11 features. You will need a C++ compiler with decent C++11 support to build the library.  

Using the library is straightforward. Every class/function is located under the `mtools` namespace. All you need to do is link your program against the binary of the library. Then, make sure mtools's header directory is in the include path and use '#include "mtools.hpp"' (this header includes all other required headers of the library).

The library itself is divided in several parts:

  - **randomgen/** : Some classical random generators such as Mersenne Twister, Xorgen... and some methods to generate specific probability distributions such as stable laws,  distribution of SRW...
  - **math/** : Not much in there yet. Classes for rectangles, vectors, matrices and some very elementary linear algebra.
  - **io/** : Contains stuff related to input/output:
    - `IArchive`/`OArchive`. Classes used for serialization of objects in the spirit of boost::serialization but much simpler, without forward serialization of pointer type but with the possibility of including comments to improve the readability of the archive.
    - `Console`. Class which displays a text console that can also log into a file. The console can be used for simple text input/output. A global `mtools::cout` object is defined and can be used as a substitute for std::cout/std::cin.
    - `LogFile`. A very simple class for logging into a file.
  - **container/** : This directory contain template classes for holding objects (a little bit like the stl containers).
    - `Grid_basic`/`Grid_factor`. Template classes which simulate an infinite d-dimensional grid Z^d where each vertex of the grid holds an object of the template type. The grid is dynamically constructed when objects are read/accessed and the internal structure is similar to that of an octree with some additional refinements. The whole grid can copied and saved/read from a file. The Grid_factor version also permits 'factorization' of objects (i.e. the same object can be re-used for distinct positions in the grid). I find this class particularly useful when simulating multi-dimensional processes such as random walks, growing domains, percolation processes, interacting particle processes etc... Check it out, it is really neat!
    - Other outdated container classes such as `RWtreegraph`, `randomurn` ...  I plan to rewrite them from scratch at some point...
  - **graphics/** This is the biggest (and best!) part of the library. It defines a set of classes used to plot two-dimensional graphics. Basically, anything that can be embedded in R^2 can be displayed and manipulated interactively. The main class is the `Plotter2D` class which displays the plotter window and manage user commands. Interaction with the plotter performed via the keyboard (arrow keys, page up/down and other shortcuts) and  the mouse (left click&drag for zooming, right click for centering and mouse wheel). It is possible to manage specific options for each o the drawn objects, enable or disable them, choose their drawing order, their transparencies, set the refresh rate, draw an axes system, draw a grid, save the image to a file, etc... Plenty of stuffs really ! Each interactive command can also be set programmatically by invoking the corresponding methods. The plotter can draws a superposition of all the 'drawable' objects that have been inserted using the add() or the operator[] methods. For an object to be drawable, it must implement the `Plotter2DObj` interface which is, itself, a variation of the `Drawable2DObject` interface with additional methods for creating a drop-down plot-specific option menu. You can create you own class derived from  `Plotter2DObj` if you wish so but, in most case, you can fall back on using one of the predefined sub-classes:
    - `Plot2DLattice`: Encapsulate a two-dimensional integer lattice. You must provide a function which take an integer position (i,j) and return a color. Optionally, you can also indicate another function which take a position and return a CImg image associated with the site. In this case, the site images are used when zooming on a small portion of the lattice (switches between drawing modes can be done in real time using the plot drop-down menu). Check the 'OERRW_2D', 'IDLA_2D' or 'LERRW_2D' demos in examples/ directory to see how to use this class.
    - `Plot2DPlane`: Encapsulate a 2D plane. You must provide a function which take an real-valued position (x,y) and return a color. Look at 'MandelbrotDemo' to see how to use this class.
    - `Plot2DCImg`: Encapsulate a CImg image. Look at 'demoImage' to to see how to use this class.
    - `Plot2DFun` : Draw the graph of a function. Template<T,U> class which take any function of the form T f(U) where T and U are convertible to double. Several drawing options are available in the plot drop-down menu. Look at 'demoBM' to see how to use this class.
    - `Plot2DArray` : Encapsulate a one-dimensional C-array. Template<T> class which take a T* pointer and its associated bufer size. Type T must be convertible to double. Several options such as the interpolation method are available in the plot drop-down menu.
    - `Plot2DVector` : Similar to `Plot2Darray` but takes an std::vector<T> instead of a raw pointer as parameter. This permits the vector size to change even while being plotted. If is possible to decide how changing the vector size affects plot range. Look at 'demoBM' to see how to use this class.
    - `Plot2DGrid` and `Plot2DAxes` : Define an object that draws a custom grid/axes coordinate. Normally, you do not need to create instances of these classes since you can always add/remove a grid/axes objects directly from the Plotter2D class. 
  The easiest way to create objects of the types described above is to use the factory global functions 'makePlot2Dxxx()'. Check the demo in examples/ sub-directory to find out how to do it. There are many other options for the Plotter2D and its associated class which are (should be) described in the header files/Doxygen documentation...  
  - **misc/** Contains general purpose code. Some header files you might want to have a look at:
    - 'timefct.hpp' : Contain methods dealing with time. In particular, class `ProgressBar` display a progress bar window.
    - 'memory.hpp' : Class `SingleAllocator` define a simple but very fast memory allocator.
    - 'metaprog.hpp' : Contain several meta-programming tricks for detecting existence of member functions/operators.
    - 'error.hpp' : Contain functions and macros for dealing with errors.
    - 'stringfct' : contain various functions for manipulating string. In particular, the `toString()` method used to convert any object into a string.


### Documentation
I try to keep the documentation fairly up to date. Information about a class or a method is located in the corresponding header file above its declaration. Comments are in doxygen format: you can use the 'Doxyfile' file to build the documentation for the whole library using doxygen. It is also worth looking into the /examples/ directory which contain sample code demonstrating how to use the library.


### Building the library
The library is cross-platform (at least, it work under Linux and Windows) but it depends on some other libraries such has CImg and FLTK. See the file '[INSTALL.TXT](https://github.com/vindar/mtools/blob/master/INSTALL.TXT)' for detailed instructions on how to build and use the library depending on your platform.

