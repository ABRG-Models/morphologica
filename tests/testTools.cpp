// Test a few morph::tools functions
#include "morph/tools.h"

#include <cstring>
// For readDirectoryTree
extern "C" {
# include <dirent.h>
# include <sys/stat.h>
}

int main()
{
    int rtn = 0;

    std::cout << "pwd: " << morph::tools::getPwd() << std::endl;

    std::string input_str = "lkajwef7436473723$&\"'.BLAH";
    std::string for_filename = input_str;
    morph::tools::conditionAsFilename (for_filename);

    std::cout << input_str << " conditionAsFilename: " << for_filename << std::endl;
    std::string expected_filename = "lkajwef7436473723____.BLAH";
    if (expected_filename != for_filename) { --rtn; }

    std::string path = ".";
    std::vector<std::string> svec{};
    morph::tools::readDirectoryTree (svec, path);
    std::cout << "Found " << svec.size() << " regular files in the current directory" << std::endl;

    return rtn;
}
