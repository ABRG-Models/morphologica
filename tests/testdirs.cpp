#include <morph/tools.h>
#include <morph/Random.h>
#include <string>
#include <iostream>

int main()
{
    int rtn = -2;
    morph::RandString rs(8);
    std::string dpath = "testdir" + rs.get();
    morph::tools::createDir (dpath);
    if (morph::tools::dirExists (dpath)) {
        std::cout << "Created directory " << dpath << " successfully.\n";
        rtn = 0;
    } else {
        std::cout << "Failed to create directory " << dpath << ".\n";
        rtn = -1;
    }
    return rtn;
}
