/*
 * This example shows you how to write simulation data out using HdfData
 *
 * Author: Seb James
 * Date: September 2021
 */

#include <morph/HdfData.h>
#include <vector>
#include <deque>
#include <iostream>

int main()
{
    // Write data into the file test.h5
    std::vector<double> vd = { 10.0, 12.0, 13.0, 14.0 };
    {
        morph::HdfData data("test.h5"); // Default file access is FileAccess::TruncateWrite
        data.add_contained_vals ("/testvectordouble", vd);
    } // data closes when out of scope

    // Read data from test.h5 into the container vdread.
    std::deque<double> vdread;
    {
        morph::HdfData data("test.h5", morph::FileAccess::ReadOnly);
        data.read_contained_vals ("/testvectordouble", vdread);
    }

    for (auto d : vdread) {
        std::cout << "Read the number " << d << " from test.h5...\n";
    }

    return 0;
}
