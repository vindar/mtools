# mtools : my maths toolkit

- Author:   [Arvind singh](mailto:arvind.singh@normalesup.org)
- Licence:  [GPLv3](http://www.gnu.org/licenses/gpl-3.0.html)


### About

This is the collection of C++ classes I created and use to perform simulations and display graphics. It consist mostly of code that I accumated over the years until I finally decided to put everything inside a library. I find it quite useful when creating simulations (mostly of exotic stochastic processes) and I hope you will too. 

Recently, I cleaned up and  rewrote several part of the library (in particular the Plotter class) in order to take advantage of the new C++11 features. Therefore, you need a C++ compiler with decent C++11 support to build the library.    

The whole library is located under the `mtools` namespace. The 'mtools.hpp' header include all the other required headers of the library so it is the only one that that need to be included when using mtools. 

The library is divided in several parts:

  - **/randomgen/** : Some classical random generators such as Mersenne Twister, Xorgen... and some methods to generate specific probability distributions such as stable laws,  distribution of SRW... 
  - **/math/** : Not much in there yet. Just some classes for rectangles, vectors, matrices and some very elementary linear algebra.
  - **/io/** : Contain stuff related to input/output:
    - `IArchive`/`OArchive`. Classes used for serialization of objects in the spirit of boost::serialization but much simpler (no forward serialization of pointer type) and with the possibility of adding comments. 
    - `Console`. Class which display a text console that can also log into a file. The console can be used for simple text input/output. A global `mtools::cout` is defined and can be used as a replacement for std::cout.
    - `LogFile`. A very simple class for logging into a file. 
  - **/container/** : This directory contain template classes for holding objects (a little bit like the stl container). 
    - `Grid_basic`/`Grid_factor`. Template classe which simulate an infinite d-dimensional grid Z^d where each vertex of the grid holds a object of the template type.  The grid is dynamiccally constructed when object are read/accessed and the internal structure is similar to that of an octree with some addtionnal refinements. The whole grid can copied and save to/read from a file. The Grid_factor version also permits 'factorization' of objects (i.e. the smae object can be re-used for distinct positions in the grid). This class is particularly useful when simulation multi-dimensional processes such as random walks, growing domains, percolation processes, interacting particle processes etc... Check it out, it is really neat !
    - Other outdated container classes such as `RWtreegraph`, `randomurn` ...  I plan to rewrite them from scratch at some point...
  - **/graphics/** This is the biggest (and best!) part of the library.
  - **/misc/** Contain general purpose code. Most interesting are the following files.
    - 'timefct.hpp' : Contain methods dealing with time. class `ProgressBar` display a progress bar window.
    - 'memory.hpp' : Class `SingleAllocator` define a simple but very fast memory allocator. 
    - 'metaprog.hpp' : Contain several meta-programming tricks for detecting existence of memeber function/operators.
    - 'error.hpp' : Contain functions and macros for dealing with errors.
    - 'stringfct' : contain various function for manipulating string. In particular, the `toString()` method to convert an object into a string.


### Documentation
I try to keep the documentation (fairly) detailled and up to date. Informations about a class or a method is located in the corresponding header file above the declaration. The comments are in doxygen format and you can use the 'Doxyfile' file to build the documentation for the whole library using doxygen.


### Building and using the library. 
The library is cross-platform (at least, it work under Linux and Windows) but it also depends on some other libraries : CImg and FLTK. See the file 'INSTALL.TXT' for detailled instructions on how to build it depending on the platform. 

Have fun !
