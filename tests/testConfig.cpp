#include <fstream>
#include <iostream>
#include <string>

#include "morph/Config.h"

int main()
{
    std::string jsonfile ("./testConfig.json");

    std::ofstream f;
    f.open ("./testConfig.json", std::ios::out | std::ios::trunc);
    if (!f.is_open()) {
        std::cerr << "Failed to open a file to write the config JSON into\n";
        return -1;
    }
    f << "{\n"
      << "\"testbool\" : true,\n"
      << "\"testint\" : 27,\n"
      << "\"testfloat\" : 7.63\n"
      << "}\n";
    f.close();

    int rtn = -1;
    {
        morph::Config config(jsonfile);
        if (config.ready) {
            const bool testbool = config.getBool ("testbool", false);
            std::cout << "testbool from JSON: " << (testbool ? "true" : "false") << " (expect: true)\n";
            const int testint = config.getInt ("testint", 3);
            std::cout << "testint from JSON: " << testint << " (expect: 27)\n";
            const float testfloat = config.getFloat ("testfloat", 9.8f);
            std::cout << "testfloat from JSON: " << testfloat << " (expect: 7.63)\n";

            if (testbool == true && testint == 27 && testfloat == 7.63f) {
                std::cout << "All test values were read correctly from the JSON.\n";
                rtn = 0;
            }
        }
    }
    return rtn;
}
