// Test a few morph::tools functions
#include "morph/tools.h"

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

    std::cout << input_str << " conditionAsXmlTag:   " << for_xml << std::endl;
    std::string expected_xml = "lkajwef7436473723_____BLAH";
    if (expected_xml != for_xml) { --rtn; }

    return rtn;
}
