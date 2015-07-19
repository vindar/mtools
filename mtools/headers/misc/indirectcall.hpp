/** @file indirectcall.hpp */
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


#include "../misc/error.hpp"

#include <utility>
#include <functional>
#include <atomic>


namespace mtools
{


    namespace internals_indirectcall
        {

        template<int ...> struct seq {};                                            // recursive template used for unpacking the arguments.
        template<int N, int ...S> struct gens : gens < N - 1, N - 1, S... > {};     // 
        template<int ...S> struct gens < 0, S... > { typedef seq<S...> type; };     // 

        }


    /**
     * Non-template base classe for indirect calls. The interface contain the method `virtual void
     * call()` which execute the registered call associated with the object. Using polymorhism
     * enables the receiver of an object derived from IndirectCall to execute the call without
     * knowing its specifics (i.e. call to global/member function/procedure).
     *
     * @sa  IndirectFun,IndirectProc, IndirectMemberFun, IndirectMemberProc
     **/
    class IndirectCall
    {
        public:
        virtual void call() { MTOOLS_INSURE(false); }; ///< make the registered call (may be invoqued more than once)
    };


    /**
     * The non-template base class for IndirectConstructor. Polymorphism enables the receiver of an
     * object with base class IndirectCtor to construct/destroy it without knowing its specifics.
     **/
    class IndirectCtor
    {
        public:
        virtual void construct() { MTOOLS_INSURE(false); };                       ///< construct the object
        virtual void* adress() const { MTOOLS_INSURE(false); return nullptr; };   ///< return a pointer ot the object or nullptr if not yet constructed. 
    };


    /**
     * The non-template base class for IndirectDestructor.
     **/
    class IndirectDtor
    {
        public:
        virtual void destroy() { MTOOLS_INSURE(false); };                        ///< destroy the object
        virtual void* adress() const { MTOOLS_INSURE(false); return nullptr; };   ///< return a pointer ot the object or nullptr if already destructed
    };



    /**
     * Create a proxy object which can be used at a later time to call a member function of a given
     * object with a predetermined set of parameters. The member function to call may take no imput
     * argument but it must return something. In order to call a function which does not return
     * anything, use the IndirectMemberProc class instead.
     * 
     * @code{.cpp}
     * 
     * struct Foo {
     *     double f(int a, char b) { ++c;  return (a + b + c); }
     *     static int c;
     *    };
     * 
     * int Foo::c = 0;
     * 
     * int main()
     *    {
     *    Foo obj;
     *    mtools::IndirectMemberFun<Foo, double,int,char> proxy(obj, &Foo::f,123,'A'); // registers the call "obj.f(123,'a')" in proxy
     *    mtools::cout << proxy.result() << "\n"; // pointer to the result is null prior to the evaluation
     *    proxy.call();   // make the call (this could be done in another thread)
     *    mtools::cout << (*proxy.result()) << "\n"; // query the result
     *    proxy.call();   // another call, discard the previous result
     *    mtools::cout << (*proxy.result()) << "\n"; // query the new result
     *    return 0;
     *    }
     * 
     * @endcode.
     *
     * @tparam  T           Type of objet.
     * @tparam  R           Return type of the member function (must not be void)
     * @tparam  ...Params   List of the input parameters types of the function.
     *
     * @sa  IndirectFun,IndirectProc, IndirectMemberFun, IndirectMemberProc, IndirectCall
     **/
    template<typename T, typename R, typename ...Params> class IndirectMemberFun : public IndirectCall
        {

        typedef R(T::*pF)(Params...); ///< typedef for a pointer to to the member function to call

        public:

        /**
         * Constructor. Register the call 
         *
         * @param [in,out]  o   The object to use for the call
         * @param   f           The member function to call.
         * @param   ...args     The arguments to pass to f.
         **/
        template<typename ...Prms> IndirectMemberFun(T & o, pF f, Prms&&... args) : _obj(o), _fun(f), _params(std::forward<Prms>(args)...), _res((R*)nullptr) {}


        /**
         * Destructor.
         **/
        ~IndirectMemberFun() { reset(); }


        /**
         * Copy Constructor.
         **/
        IndirectMemberFun(const IndirectMemberFun & proxy) : _obj(proxy._obj), _fun(proxy._fun), _params(proxy._params), _res((R*)nullptr) {}
        IndirectMemberFun(IndirectMemberFun && proxy) : _obj(proxy._obj), _fun(proxy._fun), _params(std::forward<std::tuple<Params...> >(proxy._params)), _res((R*)nullptr) {}

        

        /**
         * Perform the registered call, the result may be queried via result(). The previous result, if
         * present is discarded.
         **/
        virtual void call() { reset(); _res = new R(_call(typename internals_indirectcall::gens<sizeof...(Params)>::type())); }

      
        /**
         * Return the result obtained from the last call to call().
         *
         * @return  A pointer to the last result or nullptr if no previous call() were made.
         **/
        R * result() const { return (R*)_res; }


        /**
         * Delete the previous result and set the pointer to nullptr (useful for checking if the next
         * computation is completed).
         **/
        void reset() { delete(R*)_res; _res = (R*)nullptr; }

    private:    

        IndirectMemberFun & operator=(const IndirectMemberFun &) = delete;    // no assignement

        /* recursive unpacking of the arguments */
        template<int ...S> R _call(internals_indirectcall::seq<S...>) const { return((_obj.*_fun)(std::get<S>(_params) ...)); }
    
        T & _obj;                           // the object
        pF _fun;                            // the method to call
        std::tuple<Params...> _params;      // the parameters of the method
        std::atomic<R*> _res;               // the result
    };


    /**
     * Create a proxy object which can be used at a later time to call a member procedure of a given
     * object with a predetermined set of parameters. The member function may take any combination
     * of input arguments but must return void. In order to call a function which returns something,
     * use the IndirectMemberFun class instead.
     * 
     * @code{.cpp}
     * 
     * struct Foo {
     *     void f(int a) { mtools::cout << a + c << "\n"; c++; }
     *     static int c;
     *    };
     * 
     * int Foo::c = 0;
     * 
     * int main()
     *    {
     *    Foo obj;
     *    mtools::IndirectMemberProc<Foo, int> proxy(obj, &Foo::f,17); // registers the call "obj.f(17)" in proxy
     *    proxy.call();   // make the call (this could be done in another thread)
     *    proxy.call();   // another call
     *    return 0;
     *    }
     * 
     * @endcode.
     *
     * @tparam  T           Type of objet.
     * @tparam  ...Params   List of the input parameters types of the function.
     * @tparam  R           Return type of the member function (must not be void)
     *
     * @sa  IndirectFun,IndirectProc, IndirectMemberFun, IndirectMemberProc, IndirectCall
     **/
    template<typename T, typename ...Params> class IndirectMemberProc : public IndirectCall
        {

        typedef void (T::*pF)(Params...); ///< typedef for a pointer to to the member procedure to call

        public:

        /**
         * Constructor. Register the call 
         *
         * @param [in,out]  o   The object to use for the call
         * @param   f           The member procedure to call.
         * @param   ...args     The arguments to pass to f.
         **/
        template<typename ...Prms> IndirectMemberProc(T & o, pF f, Prms&&... args) : _obj(o), _fun(f), _params(std::forward<Prms>(args)...), _sta(false) {}


        /**
         * Destructor.
         **/
        ~IndirectMemberProc() { reset(); }


        /**
         * Copy Constructor. 
         **/
        IndirectMemberProc(const IndirectMemberProc & proxy) : _obj(proxy._obj), _fun(proxy._fun), _params(proxy._params), _sta(false) {}
        IndirectMemberProc(IndirectMemberProc && proxy) : _obj(proxy._obj), _fun(proxy._fun), _params(std::forward<std::tuple<Params...> >(proxy._params)), _sta(false) {}


        
        /**
         * Perform the registered call to the procedure
         **/
        virtual void call() { _sta = false; _call(typename internals_indirectcall::gens<sizeof...(Params)>::type()); _sta = true; }


        /**
         * Check if computation is completed. The status flag may be set back to false before a new
         * computation by using the reset() method.
         *
         * @return  true is computation is finished, false otherwise.
         **/
        bool status() const { return _sta; }


        /**
         * Reset the status flag to false.
         **/
        void reset() { _sta = false; }

    private:    

        IndirectMemberProc & operator=(IndirectMemberProc &) = delete;    // no assignement

        /* recursive unpacking of the arguments */
        template<int ...S> void _call(internals_indirectcall::seq<S...>) const { (_obj.*_fun)(std::get<S>(_params) ...); }
    
        T &     _obj;                       // the object
        pF      _fun;                       // the method to call
        std::tuple<Params...> _params;      // the parameters of the method
        std::atomic<bool> _sta;             // the result
    };


    /**
     * Create a proxy object which can be used at a later time to call a global function with a
     * predetermined set of parameters. The function to call may take no imput argument but it must
     * return something. In order to call a global function which does not return anything, use the
     * IndirectProc class instead.
     * 
     * @code{.cpp}
     * 
     * int c = 0;
     * double fun(int a, char b) { ++c; return(a + b + c); }
     * 
     * int main()
     *    {
     *    mtools::IndirectFun<double, int, char> proxy(&fun, 123, 'A'); // registers the call 
     *    mtools::cout << proxy.result() << "\n"; // pointer to the result is null prior to the evaluation
     *    proxy.call();   // make the call (this could be done in another thread)
     *    mtools::cout << (*proxy.result()) << "\n"; // query the result
     *    proxy.call();   // another call, discard the previous result
     *    mtools::cout << (*proxy.result()) << "\n"; // query the new result
     *    return 0;
     *    }
     * 
     * @endcode.
     *
     * @tparam  R           Return type of the function (must not be void)
     * @tparam  ...Params   List of the input parameters types of the function.
     *
     * @sa  IndirectFun,IndirectProc, IndirectMemberFun, IndirectMemberProc, IndirectCall
     **/
    template<typename R, typename ...Params> class IndirectFun : public IndirectCall
        {

        typedef R(*pF)(Params...); ///< typedef for a pointer to to the function to call

        public:

        /**
         * Constructor. Register the call.
         *
         * @param   f       The member function to call.
         * @param   ...args The arguments to pass to f.
         **/
        template<typename ...Prms> IndirectFun(pF f, Prms&&... args) : _fun(f), _params(std::forward<Prms>(args)...), _res((R*)nullptr) {}


        /**
         * Destructor.
         **/
        ~IndirectFun() { reset(); }


        /**
         * Copy Constructor.
         **/
        IndirectFun(const IndirectFun & proxy) : _fun(proxy._fun), _params(proxy._params), _res((R*)nullptr) {}
        IndirectFun(IndirectFun && proxy) : _fun(proxy._fun), _params(std::forward<std::tuple<Params...> >(proxy._params)), _res((R*)nullptr) {}

        

        /**
         * Perform the registered call, the result may be queried via result(). The previous result, if
         * present is discarded.
         **/
        virtual void call() { reset(); _res = new R(_call(typename internals_indirectcall::gens<sizeof...(Params)>::type())); }

      
        /**
         * Return the result obtained from the last call to call().
         *
         * @return  A pointer to the last result or nullptr if no previous call() were made.
         **/
        R * result() const { return (R*)_res; }


        /**
         * Delete the previous result and set the pointer to nullptr (useful for checking if the next
         * computation is completed).
         **/
        void reset() { delete(R*)_res; _res = (R*)nullptr; }

    private:    

        IndirectFun & operator=(const IndirectFun &) = delete;    // no assignement

        /* recursive unpacking of the arguments */
        template<int ...S> R _call(internals_indirectcall::seq<S...>) const { return((*_fun)(std::get<S>(_params) ...)); }
    
        pF _fun;                            // the method to call
        std::tuple<Params...> _params;      // the parameters of the method
        std::atomic<R*> _res;               // the result
    };


    /**
     * Create a proxy object which can be used at a later time to call a global procedure with a
     * predetermined set of parameters. The function may take any combination of input arguments but
     * must return void. In order to call a function which returns something, use the IndirectFun
     * class instead.
     * 
     * @code{.cpp}
     * 
     * int c = 0;
     * void fun(int a) { ++c; mtools::cout << a + c << "\n"; }
     *
     * int main()
     *    {
     *    mtools::IndirectProc<int> proxy(&fun, 123); // registers the call
     *    proxy.call();   // make the call (this could be done in another thread)
     *    proxy.call();   // another call
     *    return 0;
     *    }
     *    
     * @endcode.
     *
     * @tparam  T           Type of objet.
     * @tparam  ...Params   List of the input parameters types of the function.
     * @tparam  R           Return type of the member function (must not be void)
     *
     * @sa  IndirectFun,IndirectProc, IndirectMemberFun, IndirectMemberProc, IndirectCall
     **/
    template<typename ...Params> class IndirectProc : public IndirectCall
        {

        typedef void (*pF)(Params...); ///< typedef for a pointer to to the member procedure to call

        public:

        /**
         * Constructor. Register the call 
         *
         * @param   f           The member procedure to call.
         * @param   ...args     The arguments to pass to f.
         **/
        template<typename ...Prms> IndirectProc(pF f, Prms &&... args) : _fun(f), _params(std::forward<Prms>(args)...), _sta(false) {}


        /**
         * Destructor.
         **/
        ~IndirectProc() { reset(); }


        /**
         * Copy Constructor. 
         **/
        IndirectProc(const IndirectProc & proxy) : _fun(proxy._fun), _params(proxy._params), _sta(false) {}
        IndirectProc(IndirectProc && proxy) : _fun(proxy._fun), _params(std::forward<std::tuple<Params...> >(proxy._params)), _sta(false) {}

        
        /**
         * Perform the registered call to the procedure
         **/
        virtual void call() { _sta = false; _call(typename internals_indirectcall::gens<sizeof...(Params)>::type()); _sta = true; }


        /**
         * Check if computation is completed. The status flag may be set back to false before a new
         * computation by using the reset() method.
         *
         * @return  true is computation is finished, false otherwise.
         **/
        bool status() const { return _sta; }


        /**
         * Reset the status flag to false.
         **/
        void reset() { _sta = false; }

    private:    

        IndirectProc & operator=(IndirectProc &) = delete;    // no assignement

        /* recursive unpacking of the arguments */
        template<int ...S> void _call(internals_indirectcall::seq<S...>) const { (*_fun)(std::get<S>(_params) ...); }
    
        pF      _fun;                        // the method to call
        std::tuple<Params...> _params;       // the parameters of the method
        std::atomic<bool> _sta;              // the result
    };


    /**
     * Create an indirect constructor which can be used at a later time to construct on the heap an
     * object of type T with specified parameter set.
     * 
     * @code{.cpp}
     * 
     * class gg
     *     {
     *     public:
     *     gg(int v) { mtools::cout << "ctor with v = " << v << "\n"; }
     *     ~gg() { mtools::cout << "dtor"; }
     *     };
     * 
     * int main()
     *     {
     *     mtools::IndirectConstructor<gg,int> proxyC(5);  // register the constructor
     *     proxyC.construct();  // create the object
     *     mtools::IndirectDestructor<gg> proxyD((gg*)proxyC.adress());   // register the destructor
     *     proxyD.destroy(); // delete the object
     *     return 0;
     *     }
     * 
     * @endcode.
     *
     * @tparam  T           Type of objet to construct.
     * @tparam  ...Params   List of parameter to call the constructor with.
     *
     * @sa  IndirectFun,IndirectProc, IndirectMemberFun, IndirectMemberProc, IndirectCall,
     *      IndirectDestructor
     **/
    template<typename T, typename ...Params> class IndirectConstructor : public IndirectCtor
        {

        public:

        /**
         * Constructor. Register the parameters which shall be used to construct the object of type T.
         *
         * @param   args    The arguments to pass to the constructor.
         **/
        template<typename ...Prms> IndirectConstructor(Prms&&... args) : _params(std::forward<Prms>(args)...), _res((T*)nullptr) {}


        /**
         * Destructor. Does NOT destroy the object created !
         **/
        ~IndirectConstructor() {  }


        /**
         * Copy Constructor.
         **/
        IndirectConstructor(const IndirectConstructor & proxy) : _params(proxy._params), _res((T*)nullptr) { }
        IndirectConstructor(IndirectConstructor && proxy) : _params(std::forward<std::tuple<Params...> >(proxy._params)), _res((T*)nullptr) { }


        /**
         * Construct the object. the adress of the object may be queried via adress().
         * Should be called only once !
         **/
        virtual void construct() { MTOOLS_ASSERT((T*)_res == nullptr);  _res = _call(typename internals_indirectcall::gens<sizeof...(Params)>::type()); }

      
        /**
         * The adress of the constructed object.
         *
         * @return  A pointer to the adress of the constructed object. nullptr if not yet constructed.
         **/
        virtual void * adress() const { return((void*)_res); }


    private:    

        IndirectConstructor & operator=(const IndirectConstructor &) = delete;    // no assignement

        /* recursive unpacking of the arguments */
        template<int ...S> T* _call(internals_indirectcall::seq<S...>) const { return new T(std::get<S>(_params) ...); }
            
    
        std::tuple<Params...> _params;  // parameters of the constructor
        std::atomic<T*> _res;           // pointer to the constructed objet
    };


     /**
      * Create an indirect destructor which can be used at a later time to delete an object of a
      * given type.
      * 
      * @code{.cpp}
      * 
      * class gg
      *     {
      *     public:
      *     gg(int v) { mtools::cout << "ctor with v = " << v << "\n"; }
      *     ~gg() { mtools::cout << "dtor"; }
      *     };
      * 
      * int main()
      *     {
      *     mtools::IndirectConstructor<gg,int> proxyC(5);  // register the constructor
      *     proxyC.construct();  // create the object
      *     mtools::IndirectDestructor<gg> proxyD((gg*)proxyC.adress());   // register the destructor
      *     proxyD.destroy(); // delete the object
      *     return 0;
      *     }
      * 
      * @endcode.
      *
      * @tparam T   Type of objet to destroy.
      *
      * @sa IndirectFun,IndirectProc, IndirectMemberFun, IndirectMemberProc, IndirectCall,
      *     IndirectConstructor
      **/
     template<typename T> class IndirectDestructor : public IndirectDtor
        { 
        public:

        /**
         * Constructor.
         *
         * @param [in,out]  p   The adress of the object to be deleted.
         **/
        IndirectDestructor(T * p) : _adr(p) {}


        /**
         * Destructor. Does NOT destroy the object !
         **/
        ~IndirectDestructor() { }


        /**
         * Copy Constructor.
         **/
        IndirectDestructor(const IndirectDestructor & proxy) : _adr(proxy._adr) { }

      
        /**
         * The adress of the object.
         *
         * @return  A pointer to the adress of the constructed object. nullptr if already destructed.
         **/
        virtual void * adress() const { return((void*)_adr); }


        /**
         * Delete the T object and set the adress to nullptr.
         **/
        virtual void destroy() { MTOOLS_ASSERT((T*)_adr != nullptr);  delete((T*)_adr); _adr = (T*)nullptr; }

    private:    

        IndirectDestructor & operator=(const IndirectDestructor &) = delete;    // no assignement

        std::atomic<T*> _adr;           // pointer to the objet to destroy
    };



    
}


/* end of file */




