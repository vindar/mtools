#include "stdafx_test.h"

#include "mtools.hpp"
using namespace mtools;


MT2004_64 gen;
double sinus(double x) { return sin(x); }

void stupidTest()
    {
    cout << "Hello World\n";
    int64 v = cout.ask("Give me a number", 0);
    watch("your number", v);
    cout << "Terminate the program gracefully by closing the plotter window\n";
    cout << "or forcefully by closing the cout or watch window\n";
    Plotter2D PL;
    auto P1 = makePlot2DFun(sinus, -10, 10, "sinus");
    PL[P1];
    PL.autorangeXY();
    PL.plot();

    //mtools::exit(5);


    while (PL.shown())
        {
        v += Unif_1(gen);
        }
    }















using mtools::metaprog::yes;
using mtools::metaprog::no;

namespace mtools
    {

    template<typename T> struct img : public cimg_library::CImg<T>
        {
        RGBc toRGBc() const 
            {
            mtools::cout << "IMG_UC to RGB\n";
            return RGBc(); 
            }
        };

        using img_uc = img<unsigned char>;


    }


/**
 * GetImage method selector.
 * Detect if a type (or function) contain a method compatible with getImage().
 * The method can be called via call().
 **/
template<typename T> class GetImageSelector
    {
    static void * dumptr;

    template<typename U> static decltype((*(U*)(0)).getImage(iVec2{ 0,0 }, dumptr)) vers1(int);
    template<typename> static no vers1(...);
    static const bool version1 = std::is_same<std::decay<decltype(vers1<T>(0))>, std::decay<mtools::img_uc*> >::value;

    template<typename U> static decltype((*(U*)(0)).getImage(iVec2{ 0,0 })) vers2(int);
    template<typename> static no vers2(...);
    static const bool version2 = std::is_same<std::decay<decltype(vers2<T>(0))>, std::decay<mtools::img_uc*> >::value;

    template<typename U> static decltype((*(U*)(0)).getImage(0, 0, dumptr)) vers3(int);
    template<typename> static no vers3(...);
    static const bool version3 = std::is_same<std::decay<decltype(vers3<T>(0))>, std::decay<mtools::img_uc*> >::value;

    template<typename U> static decltype((*(U*)(0)).getImage(0, 0)) vers4(int);
    template<typename> static no vers4(...);
    static const bool version4 = std::is_same<std::decay<decltype(vers4<T>(0))>, std::decay<mtools::img_uc*> >::value;

    template<typename U> static decltype((*(U*)(0))(iVec2{ 0,0 }, dumptr)) vers5(int);
    template<typename> static no vers5(...);
    static const bool version5 = std::is_same<std::decay<decltype(vers5<T>(0))>, std::decay<mtools::img_uc*> >::value;

    template<typename U> static decltype((*(U*)(0))(iVec2{ 0,0 })) vers6(int);
    template<typename> static no vers6(...);
    static const bool version6 = std::is_same<std::decay<decltype(vers6<T>(0))>, std::decay<mtools::img_uc*> >::value;

    template<typename U> static decltype((*(U*)(0))(0, 0, dumptr)) vers7(int);
    template<typename> static no vers7(...);
    static const bool version7 = std::is_same<std::decay<decltype(vers7<T>(0))>, std::decay<mtools::img_uc*> >::value;

    template<typename U> static decltype((*(U*)(0))(0, 0)) vers8(int);
    template<typename> static no vers8(...);
    static const bool version8 = std::is_same<std::decay<decltype(vers8<T>(0))>, std::decay<mtools::img_uc*> >::value;

    static mtools::img_uc * call1(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<true> D) { return obj.getImage(pos, data); }
    static mtools::img_uc * call2(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<true> D) { return obj.getImage(pos); }
    static mtools::img_uc * call3(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<true> D) { return obj.getImage(pos.X(), pos.Y(), data); }
    static mtools::img_uc * call4(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<true> D) { return obj.getImage(pos.X(), pos.Y()); }
    static mtools::img_uc * call5(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<true> D) { return obj(pos, data); }
    static mtools::img_uc * call6(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<true> D) { return obj(pos); }
    static mtools::img_uc * call7(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<true> D) { return obj(pos.X(), pos.Y(), data); }
    static mtools::img_uc * call8(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<true> D) { return obj(pos.X(), pos.Y()); }

    static mtools::img_uc * call1(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<false> D) { return call2(obj, pos, data, mtools::metaprog::dummy<version2>()); }
    static mtools::img_uc * call2(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<false> D) { return call3(obj, pos, data, mtools::metaprog::dummy<version3>()); }
    static mtools::img_uc * call3(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<false> D) { return call4(obj, pos, data, mtools::metaprog::dummy<version4>()); }
    static mtools::img_uc * call4(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<false> D) { return call5(obj, pos, data, mtools::metaprog::dummy<version5>()); }
    static mtools::img_uc * call5(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<false> D) { return call6(obj, pos, data, mtools::metaprog::dummy<version6>()); }
    static mtools::img_uc * call6(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<false> D) { return call7(obj, pos, data, mtools::metaprog::dummy<version7>()); }
    static mtools::img_uc * call7(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<false> D) { return call8(obj, pos, data, mtools::metaprog::dummy<version8>()); }
    static mtools::img_uc * call8(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<false> D) { MTOOLS_ERROR("No getImage() found"); return nullptr; }

    public:

        static const bool has_getImage = version1 | version2 | version3 | version4 | version5 | version6 | version7 | version8;

        static img_uc * call(T & obj, const iVec2 & pos, void * data) { return call1(obj, pos, data, mtools::metaprog::dummy<version1>()); }
    };


/**
* GetColor method selector.
* Detect if a type (or function) contain a method compatible with getColor().
* The method is called via call().
**/
template<typename T> class GetColorSelector
    {
        static void * dumptr;

        template<typename U> static decltype((*(U*)(0)).getColor(iVec2{0,0}, dumptr)) vers1(int);
        template<typename> static no vers1(...);
        static const bool version1 = std::is_same<std::decay<decltype(vers1<T>(0))>, std::decay<mtools::RGBc> >::value;

        template<typename U> static decltype((*(U*)(0)).getColor(iVec2{0,0})) vers2(int);
        template<typename> static no vers2(...);
        static const bool version2 = std::is_same<std::decay<decltype(vers2<T>(0))>, std::decay<mtools::RGBc> >::value;

        template<typename U> static decltype((*(U*)(0)).getColor(0,0, dumptr)) vers3(int);
        template<typename> static no vers3(...);
        static const bool version3 = std::is_same<std::decay<decltype(vers3<T>(0))>, std::decay<mtools::RGBc> >::value;

        template<typename U> static decltype((*(U*)(0)).getColor(0,0)) vers4(int);
        template<typename> static no vers4(...);
        static const bool version4 = std::is_same<std::decay<decltype(vers4<T>(0))>, std::decay<mtools::RGBc> >::value;

        template<typename U> static decltype((*(U*)(0))(iVec2{0,0}, dumptr)) vers5(int);
        template<typename> static no vers5(...);
        static const bool version5 = std::is_same<std::decay<decltype(vers5<T>(0))>, std::decay<mtools::RGBc> >::value;

        template<typename U> static decltype((*(U*)(0))(iVec2{0,0})) vers6(int);
        template<typename> static no vers6(...);
        static const bool version6 = std::is_same<std::decay<decltype(vers6<T>(0))>, std::decay<mtools::RGBc> >::value;

        template<typename U> static decltype((*(U*)(0))(0,0, dumptr)) vers7(int);
        template<typename> static no vers7(...);
        static const bool version7 = std::is_same<std::decay<decltype(vers7<T>(0))>, std::decay<mtools::RGBc> >::value;

        template<typename U> static decltype((*(U*)(0))(0,0)) vers8(int);
        template<typename> static no vers8(...);
        static const bool version8 = std::is_same<std::decay<decltype(vers8<T>(0))>, std::decay<mtools::RGBc> >::value;

        static mtools::RGBc call1(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<true> D) { return obj.getColor(pos, data); }
        static mtools::RGBc call2(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<true> D) { return obj.getColor(pos); }
        static mtools::RGBc call3(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<true> D) { return obj.getColor(pos.X(), pos.Y(), data); }
        static mtools::RGBc call4(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<true> D) { return obj.getColor(pos.X(),pos.Y()); }
        static mtools::RGBc call5(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<true> D) { return obj(pos, data); }
        static mtools::RGBc call6(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<true> D) { return obj(pos); }
        static mtools::RGBc call7(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<true> D) { return obj(pos.X(), pos.Y(), data); }
        static mtools::RGBc call8(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<true> D) { return obj(pos.X(), pos.Y()); }
        static mtools::RGBc call9(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<true> D) { return (GetImageSelector<T>::call(obj, pos, data))->toRGBc(); }

        static mtools::RGBc call1(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<false> D) { return call2(obj, pos, data, mtools::metaprog::dummy<version2>()); }
        static mtools::RGBc call2(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<false> D) { return call3(obj, pos, data, mtools::metaprog::dummy<version3>()); }
        static mtools::RGBc call3(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<false> D) { return call4(obj, pos, data, mtools::metaprog::dummy<version4>()); }
        static mtools::RGBc call4(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<false> D) { return call5(obj, pos, data, mtools::metaprog::dummy<version5>()); }
        static mtools::RGBc call5(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<false> D) { return call6(obj, pos, data, mtools::metaprog::dummy<version6>()); }
        static mtools::RGBc call6(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<false> D) { return call7(obj, pos, data, mtools::metaprog::dummy<version7>()); }
        static mtools::RGBc call7(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<false> D) { return call8(obj, pos, data, mtools::metaprog::dummy<version8>()); }
        static mtools::RGBc call8(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<false> D) { return call9(obj, pos, data, mtools::metaprog::dummy<GetImageSelector<T>::has_getImage>()); }
        static mtools::RGBc call9(T & obj, const iVec2 & pos, void * data, mtools::metaprog::dummy<false> D) { static_assert(has_getColor, "No method found for getColor()"); return RGBc(); }

    public:

        static const bool has_getColor = version1 | version2 | version3 | version4 | version5 | version6 | version7 | version8;

        static RGBc call(T & obj, const iVec2 & pos, void * data) { return call1(obj, pos, data, mtools::metaprog::dummy<version1>()); }


    };




struct AzA
    {

    /*
    RGBc getColor(iVec2  pos, void* & blop)
        {
        mtools::cout << "CALLED 1\n";
        return RGBc::c_Black;
        }
        
        
    RGBc getColor(iVec2  pos)
        {
        mtools::cout << "CALLED 2\n";
        return RGBc::c_Black;
        }

        RGBc getColor(int64 x,int64 y, void* & blop)
        {
        mtools::cout << "CALLED 3\n";
        return RGBc::c_Black;
        }



    RGBc getColor(const int64 & x,int64 y)
        {
        mtools::cout << "CALLED 4\n";
        return RGBc::c_Black;
        }
        

    
    RGBc operator()(iVec2  pos, void* & blop)
        {
        mtools::cout << "CALLED 5\n";
        return RGBc::c_Black;
        }
        


    RGBc operator()(iVec2  pos)
        {
        mtools::cout << "CALLED 6\n";
        return RGBc::c_Black;
        }


    RGBc operator()(int64 x, int64 y, void* & blop)
        {
        mtools::cout << "CALLED 7\n";
        return RGBc::c_Black;
        }


    RGBc operator()(const int64 & x, int64 y)
        {
        mtools::cout << "CALLED 8\n";
        return RGBc::c_Black;
        }
       */

    img_uc * operator()(const int64 & x, int64 y)
        {
        mtools::cout << "CALLED \n";
        return &img_uc();
        }

    };



RGBc fcoul(iVec2 pos,void * & p)
    {
    mtools::cout << "CALLED f 5\n";

    //return 0;

    return RGBc::c_Black;
    }




int main(int argc, char *argv[])
    {
    MTOOLS_SWAP_THREADS(argc, argv);
    parseCommandLine(argc, argv, false, false);


    mtools::cout << GetColorSelector<AzA>::call(AzA(), iVec2{ 1, 2 }, nullptr) << "\n";

    //cout << GetColorSelector<decltype(fcoul)>::call(fcoul, iVec2{ 1, 2 }, nullptr);
    mtools::cout.getKey();

    //stupidTest();

    return 0;
    }

/* end of file main.cpp */

