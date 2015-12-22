

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

using namespace mtools;

int main(int argc, char* argv[])
{
 
volatile mtools::int64 n = 0;
mtools::int64 m = 1;


    watch("n", n); cout.getKey();

    watch("n2", n); cout.getKey();

    watch.move(1000, 0); cout.getKey();

    watch.clear(); cout.getKey();

    watch("n3", n); cout.getKey();

    watch("n4", n); cout.getKey();

    watch.move(0, 500);
    
    watch.remove("n3"); cout.getKey();
    watch.remove("n4"); cout.getKey();

    watch.move(200, 200);

    watch("n5", n); cout.getKey();

    int64 nmax = 10000000000;

   while(n < nmax)
        {
        n++;
        m = 2*m  + 7;

        //watch[n];
        }
        
    mtools::cout << "res = " << m << "\n";
    mtools::cout << "Done in " << mtools::Chronometer() << "\n";;
    mtools::cout.getKey();

   

	return 0;
}
