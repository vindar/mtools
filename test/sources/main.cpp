

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
    
    Fl_Double_Window *  win = new Fl_Double_Window(720, 486);
    RateTable *  rate = new RateTable(10, 10, 720 - 20, 486 - 20);
    win->resizable(rate);
    win->show();

    }






int main(int argc, char* argv[])
{

    mtools::cout << "test\n";

    mtools::IndirectProc<> proxy(&inFLTK); // registers the call
    runInFLTKThread(proxy);

    mtools::cout << "This is a test...\n";
    mtools::cout.getKey();

   

	return 0;
}
