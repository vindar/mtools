

#include "stdafx_test.h"

#include "mtools.hpp"

using namespace mtools;

	
int main(int argc, char* argv[])
{

    cout << "This is a test...\n";
    //cout.getKey();

    int k = 5;
    std::string ss;
    for (int i = 0;i < 400;i++)
        {
        ss += "-";
        ss += toString(k);
        k = (k + 1)*(k - 1);
        cout << ss;
        }

	return 0;
}
