

#include "stdafx_test.h"

#include "mtools.hpp"
using namespace mtools;





int main(int argc, char* argv[])
{

    mtools::parseCommandLine(argc, argv);
 
    volatile mtools::int64 n = 0;
    volatile mtools::int64 m = 1;

    const int c = 0;
    const volatile std::string sss("a");


    std::string s = arg("-str"); //(arg('r'));
    
    s = (std::string)arg("-str");

    cout << "n=" << m << "\n";

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

    Chronometer();


    OArchive OA("a");

    OA & n;
    OA & m;
    OA & c;
    OA & sss;

    IArchive IA("a");

    IA & n;
    IA & m;
    IA & c;
    IA & sss;


   while(n < nmax)
        {
        n++;
        m = 2*m  + 7;

        //watch[n];
        }
    mtools::cout << "res = " << s << "\n";
    mtools::cout << "Done in " << mtools::Chronometer() << "\n";;
    mtools::cout.getKey();

   
   
	return 0;
}
