

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

volatile mtools::int64 n = 0;
volatile mtools::int64 m = 1;


int main(int argc, char* argv[])
{


    double x = 156.889;
    foo FF;
    mtools::iRect R;
//    ttcc z;

    mtools::WatchWindow watch;

    mtools::cout << "n";

    watch("n", n);//mtools::cout.getKey();
    watch("m", m);//mtools::cout.getKey();

    watch("n2", n);//mtools::cout.getKey();
    watch("n3", x);//mtools::cout.getKey();
    watch("n4", FF);//mtools::cout.getKey();
    watch("n5", R);//mtools::cout.getKey();
    watch("n6", n);//mtools::cout.getKey();
    
    watch("n7", n);//mtools::cout.getKey();
    watch("n8", n);//mtools::cout.getKey();


    watch("n9", n);//mtools::cout.getKey();
    watch("n10", n);//mtools::cout.getKey();
    watch("n11", n);//mtools::cout.getKey();
    watch("n12", n);//mtools::cout.getKey();
    watch("n13", n);//mtools::cout.getKey();
    watch("n14", n);//mtools::cout.getKey();
    watch("n15", n);//mtools::cout.getKey();
    watch("n16", n);//mtools::cout.getKey();
    watch("n17", n);//mtools::cout.getKey();

    
    watch.refreshRate("n2", 600);
    watch.refreshRate("n3", 600);
    watch.refreshRate("n4", 600);
    watch.refreshRate("n5", 600);
    watch.refreshRate("n6", 600);
    watch.refreshRate("n7", 600);
    watch.refreshRate("n8", 600);
    watch.refreshRate("n9", 600);
    watch.refreshRate("n10", 600);
    watch.refreshRate("n11", 600);
    watch.refreshRate("n12", 600);
    watch.refreshRate("n13", 600);
    watch.refreshRate("n14", 600);

    while (1)
        {
        n++;

        m = 2*m;

      //  mtools::cout.getKey();

        }
        

    mtools::cout << "This is a test...\n";
    mtools::cout.getKey();

   

	return 0;
}
