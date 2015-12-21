

#include "stdafx_test.h"

#include "mtools.hpp"


#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Float_Input.H>
//
// Demonstrate creating a table of widgets without Fl_Table
//                                                   --erco Mar 8 2005
#define COLS 3
#define ROWS 10
class RateTable : public Fl_Scroll {
    void *w[ROWS][COLS];        // widget pointers
public:
    RateTable(int X, int Y, int W, int H, const char*L = 0) : Fl_Scroll(X, Y, W, H, L)
        {
        static const char *header[COLS] = {
            "",    "name", "value"
            };
        Fl_Scroll::type(Fl_Scroll::VERTICAL);
        int cellw = 80;
        int cellh = 25;
        int xx = X, yy = Y;
        Fl_Tile *tile = new Fl_Tile(X, Y, cellw*COLS + 100, cellh*ROWS);
        // Create widgets
        for (int r = 0; r<ROWS; r++) {
            for (int c = 0; c<COLS; c++) {
                if (r == 0) {
                    Fl_Box *box = new Fl_Box(xx, yy, cellw, cellh, header[c]);
                    box->box(FL_BORDER_BOX);
                    w[r][c] = (void*)box;
                    }
                else if (c == 0) {
                    auto *in = new Fl_Button(xx, yy, cellw, cellh,"option");
                    in->box(FL_UP_BOX);
                    //in->value("");
                    w[r][c] = (void*)in;
                    }
                else {
                    auto *in = new Fl_Box(xx, yy, cellw, cellh,"yop\nazer");
                    in->box(FL_BORDER_BOX);

                    in->color(mtools::RGBc::c_White);
                    //in->value("0.00");
                    //if (c == 1) in->deactivate();
                    w[r][c] = (void*)in;
                    }
                xx += cellw;
                }
            xx = X;
            yy += cellh;
            }
        auto *box = new Fl_Box(X + 80*COLS, Y, 100, cellh*ROWS,"");
        box->box(FL_FLAT_BOX);
        //box->resizable(box);


        Fl_Box * rb = new Fl_Box(tile->x() + 80, tile->y() + 20, tile->w() - 120, tile->h() - 40);

        tile->resizable(*rb);
        
        tile->end();
        end();
  
        resizable(tile);
        }
    };








void inFLTK()
    {

    /*

    WatchWindow W;



    W.spy("name",n,[true]);       // spy on type, edit is disallowed
    W.spy()

    W.spyfun(fout, [fin]);

    */




    
    Fl_Double_Window *  win = new Fl_Double_Window(720, 486);
    RateTable *  rate = new RateTable(10, 10, 720 - 20, 486 - 20);
    win->resizable(rate);
    win->show();

    }









/** Pure virtual base class for a watchable object */
class WatchObj
    {
    public:

        WatchObj(int rate) : _rate(rate) {}

        virtual ~WatchObj() { }

        virtual std::string get() const { MTOOLS_ERROR("pure virtual method WatchObj::get(), access forbidden"); return std::string(); }

        virtual size_t set(const std::string & value) { MTOOLS_ERROR("pure virtual method WatchObj::set(), access forbidden"); return 0; }

        virtual bool writable() const { MTOOLS_ERROR("pure virtual method WatchObj::writable(), access forbidden"); return false; }

        int refreshRate() const { return _rate; }

        int refreshRate(int newrate) { _rate = newrate; return _rate; }

    private:
        int _rate;   // refresh rate (number of time per minutes)

    };


    /**
     * A watch object containing a variable of type T
     **/
    template<typename T, bool allowWrite> class WatchObjVar : public WatchObj
        {
        public:

        /** Constructor **/
        WatchObjVar(const T & val, int rate) : WatchObj(rate), _p(const_cast<T*>(&val))  {}
    
        /** Destructor. */
        virtual ~WatchObjVar() override {}

        /** Gets the string representing the object  **/
        virtual std::string get() const override { return mtools::toString(*_p); }

        /**
         * Sets the given value and return the number of character read from the input.
         * Do nothing and return 0 if if writable is false or if the object cannot
         * be set using mtools::fromString().
         **/
        virtual size_t set(const std::string & str) override {  return _set(str, mtools::metaprog::dummy<_writable>()); }

        /**
         * Return true if the object is writable and false otherwise.
         **/
        virtual bool writable() const override { return _writable; }

    public: 

        size_t _set(const std::string & str, mtools::metaprog::dummy<true> dum) { return mtools::fromString(str, *_p); } // set value using fromString()
        size_t _set(const std::string & str, mtools::metaprog::dummy<false> dum) { return 0; } // do nothing

        T * _p;      // pointer to the watched object
        static const bool _writable = (allowWrite && mtools::metaprog::has_from_istream<T>::value);  // true if the object is writable and false otherwise
        };



    /**
    * A watch object containing a variable of type T, use a function/functor for output
    **/
    template<typename T, typename OutFun, bool allowWrite> class WatchObjVarOut : public WatchObj
        {
        public:

            /** Constructor **/
            WatchObjVarOut(const T & val, OutFun & outfun, int rate) : WatchObj(rate), _p(const_cast<T*>(&val)), _outfun(&outfun) {}

            /** Destructor. */
            ~WatchObjVarOut() override {}

            /** Gets the string representing the object  **/
            std::string get() const override { return mtools::toString((*_outfun)(*_p)); }

            /**
            * Sets the given value and return the number of character read from the input.
            * Do nothing and return 0 if if writable is false or if the object cannot
            * be set using mtools::fromString().
            **/
            virtual size_t set(const std::string & str) override { return _set(str, mtools::metaprog::dummy<_writable>()); }

            /**
            * Return true if the object is writable and false otherwise.
            **/
            virtual bool writable() const override { return _writable; }

        public:

            size_t _set(const std::string & str, mtools::metaprog::dummy<true> dum) { return mtools::fromString(str, *_p); } // set value using fromString()
            size_t _set(const std::string & str, mtools::metaprog::dummy<false> dum) { return 0; } // do nothing

            T * _p;                 // pointer to the watched object
            OutFun * _outfun;       // pointer to the output functor
            static const bool _writable = (allowWrite && mtools::metaprog::has_from_istream<T>::value);  // true if the object is writable and false otherwise
        };


    /**
    * A watch object containing a variable of type T, use functions/functors for output and input
    **/
    template<typename T, typename OutFun, typename InFun, bool allowWrite> class WatchObjVarOutIn : public WatchObj
        {
        public:

            /** Constructor **/
            WatchObjVarOutIn(const T & val, OutFun & outfun, InFun & infun, int rate) : WatchObj(rate), _p(const_cast<T*>(&val)), _outfun(&outfun), _infun(&infun) {}

            /** Destructor. */
            ~WatchObjVarOutIn() override {}

            /** Gets the string representing the object  **/
            std::string get() const override { return mtools::toString((*_outfun)(*_p)); }

            /**
            * Sets the given value (if writable, otherwise do nothing), then return 0.
            **/
            virtual size_t set(const std::string & str) override { return _set(str, mtools::metaprog::dummy<allowWrite>()); }

            /**
            * Return true if the object is writable and false otherwise (ie if allowWrite was set to false)
            **/
            virtual bool writable() const override { return allowWrite; }

        public:

            size_t _set(const std::string & str, mtools::metaprog::dummy<true> dum) { (*_infun)(str, *_p); return 0; } // set value using infun
            size_t _set(const std::string & str, mtools::metaprog::dummy<false> dum) { return 0; } // do nothing

            T * _p;                 // pointer to the watched object
            OutFun * _outfun;       // pointer to the output functor
            InFun *  _infun;        // pointer to the output functor
        };



    class FltkWatchWin
        {

        public:


        /** Default constructor. */
        FltkWatchWin()
            {

            }


        /** Destructor. */
        ~FltkWatchWin()
            {

            }


        /**
         * Removes a variable from the spy window.
         **/
        void remove(const std::string & name)
            {
            mtools::cout << "REMOVING " << name << "....\n\n";
            return;
            }


        /**
        * Change the refresh rate of a variable
        **/
        void refreshRate(const std::string & name, int newrate)
            {
            return;
            }


        /**
         * Add a variable to the spy window.
         **/
        void add(const std::string & name, WatchObj * obj)
            {
            return;
            }


        };






class WatchWindow
    {

    public:


    static const int DEFAULT_REFRESHRATE = 60; // default refresh rate = every second.


     /** Default constructor. */
     WatchWindow() : _fltkobj(nullptr) {}

     /** Destructor. */
     ~WatchWindow() { clear(); }


     /**
     * Removes a variable from the spy window.
     *
     * @param   name    name identifier of the variable to remove.
     **/
     void remove(const std::string & name)
         {
         createIfNeeded();
         mtools::IndirectMemberProc<FltkWatchWin, const std::string &> proxy((*_fltkobj), &FltkWatchWin::remove, name);
         mtools::runInFLTKThread(proxy);
         }


     /**
     *  Remove all variables inside the watch window (makes the window disappear from the screen)
     **/
     void clear()
         {
         if (_fltkobj != nullptr) { mtools::deleteInFLTKThread(_fltkobj); _fltkobj = nullptr; }
         }


     /**
     * Set the refresh rate for a given variable.
     *
     * @param   name    name identifier
     * @param   newrate The new refresh rate (number of times per seconds). Set to 0 to disable refresh.
     **/
     void refreshRate(const std::string & name, int newrate)
         {
         createIfNeeded();
         mtools::IndirectMemberProc<FltkWatchWin, const std::string &, int> proxy((*_fltkobj), &FltkWatchWin::refreshRate, name, newrate);
         mtools::runInFLTKThread(proxy);
         }

    /**
     * Attach a variable to the window.
     *
     * @tparam  allowWrite  true to authorize writing to the variable.
     * @param   name        name identifier.
     * @param [in,out]  val the variable to spy on.
     **/
    template<bool allowWrite = true, typename T> void spy(const std::string & name, T & val)
        {
        createIfNeeded();
        WatchObjVar<T, allowWrite> * p = new WatchObjVar<T, allowWrite>(val, DEFAULT_REFRESHRATE);
        mtools::IndirectMemberProc<FltkWatchWin, const std::string &, int> proxy((*_fltkobj), &FltkWatchWin::add, name, p);
        mtools::runInFLTKThread(proxy);
        }




    /**
     * Attach a variable to the window. The value to display is obtained by calling 'outfun(val)'
     *
     * @tparam  allowWrite      true to authorize writing to the variable.
     * @param   name            name identifier.
     * @param [in,out]  val     the variable to spy on.
     * @param [in,out]  outfun  a function/functor called before displaying the value of the
     *                          variable. The value displayed in the watch window is 'outfun(val)',
     *                          (converted to string if necessary using mtools::toString).
     **/
    template<bool allowWrite = true, typename T, typename OutFun> void spy(const std::string & name, T & val, OutFun & outfun)
        {
        createIfNeeded();
        WatchObjVarOut<T, OutFun, allowWrite> * p = new WatchObjVarOut<T, OutFun, allowWrite>(val, outfun, DEFAULT_REFRESHRATE);
        mtools::IndirectMemberProc<FltkWatchWin, const std::string &, int> proxy((*_fltkobj), &FltkWatchWin::add, name, p);
        mtools::runInFLTKThread(proxy);
        }


    /**
     * Attach a variable to the window. The value to display is obtained by calling 'outfun(val)'.
     * Uses a custom function/functor for setting the variable value.
     *
     * @tparam  allowWrite  true to authorize writing to the variable.
     * @tparam  T           Generic type parameter.
     * @tparam  Fun         Type of the fun.
     * @tparam  Fun         Type of the fun.
     * @param   name            name identifier.
     * @param [in,out]  val     the variable to spy on.
     * @param [in,out]  outfun  a function/functor called before displaying the value of the
     *                          variable. The value displayed in the watch window is 'outfun(val)',
     *                          (converted to string if necessary using mtools::toString).
     * @param [in,out]  infun   a function/functor used to set the variable value 'val' from a
     *                          std::string 'str' by calling of the form 'infun(str,val)'. Return
     *                          value of infun, if any is discarded.
     **/
    template<bool allowWrite = true, typename T, typename OutFun, typename InFun> void spy(const std::string & name, T & val, OutFun & outfun, InFun & infun)
        {
        createIfNeeded();
        WatchObjVarOutIn<T, OutFun, InFun, allowWrite> * p = new WatchObjVarOutIn<T, OutFun, InFun, allowWrite>(val, outfun, infun, DEFAULT_REFRESHRATE);
        mtools::IndirectMemberProc<FltkWatchWin, const std::string &, int> proxy((*_fltkobj), &FltkWatchWin::add, name, p);
        mtools::runInFLTKThread(proxy);
        }


    /** Same as spy(name,val) **/
    template<bool allowWrite = true, typename T> void operator()(const std::string & name, T & val) { spy<allowWrite,T>(name, val); }

    /** Same as spy(name,val,outfun) **/
    template<bool allowWrite = true, typename T, typename OutFun> void operator()(const std::string & name, T & val, OutFun & outfun) { spy<allowWrite,T, OutFun>(name, val,outfun); }

    /** Same as spy(name,val,outfun,infun) **/
    template<bool allowWrite = true, typename T, typename OutFun, typename InFun> void operator()(const std::string & name, T & val, OutFun & outfun, InFun & infun) { spy<allowWrite, T, OutFun, InFun>(name, val, outfun, infun); }


    private:

        /** Creates the fltk object if it does not exist yet */
        void createIfNeeded() { if (_fltkobj == nullptr) { _fltkobj = mtools::newInFLTKThread<FltkWatchWin>(); } }

        WatchWindow(const WatchWindow &) = delete;              // no copy
        WatchWindow & operator=(const WatchWindow &) = delete;  //

        FltkWatchWin* _fltkobj;  // the object running inside the fltk thread.
    };










class foo
    {
    public:
        int r;
    };



struct ttcc
    {
    double operator()(int v) { return v + 1; }
    };

int ttff(int v) { return v + 2; }



int main(int argc, char* argv[])
{

    int n = 12345;
    double x = 156.889;
    foo FF;
    mtools::iRect R;
    ttcc z;


    WatchWindow watch;

    watch.remove("hello");

    watch("n",n);
    watch("n1", n,z);
    watch("n2", n,ttff, mtools::fromString<int>);
    watch("x", x);
    watch("FF", FF);
    watch("R", R);



    mtools::cout.getKey();


    mtools::cout << "This is a test...\n";
    mtools::cout.getKey();

   

	return 0;
}
