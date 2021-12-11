#include <fstream>
#include <iostream>
#include <string>

#include "morph/ConfigNL.h"

int main()
{
    std::string jsonfile ("./testConfig.json");

#if 0
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
#else
    morph::ConfigNL initial("./testConfig.json");
    initial.set ("testbool", true);
    initial.set ("testint", 27);
    initial.set ("testfloat", 7.63f);
    initial.setArray ("testlist", std::vector<int>({ 1, 2, 45, 5 }));
    initial.setArray ("testlist2", std::vector<std::string>({ std::string("one"), std::string("two") }));
    initial.write();
#endif
    int rtn = -1;
    {
        morph::ConfigNL config(jsonfile);

        const bool testbool = config.getBool ("testbool", false);
        std::cout << "testbool from JSON: " << (testbool ? "true" : "false") << " (expect: true)\n";
        const int testint = config.getInt ("testint", 3);
        std::cout << "testint from JSON: " << testint << " (expect: 27)\n";
        const float testfloat = config.getFloat ("testfloat", 9.8f);
        std::cout << "testfloat from JSON: " << testfloat << " (expect: 7.63)\n";

        auto ar = config.getArray ("testlist");
        for (auto a : ar) {
            std::cout << "array: " << a << std::endl;
        }
        auto ar2 = config.getArray ("testlist2");
        for (auto a : ar2) {
            std::cout << "array: " << a << std::endl;
        }

        if (testbool == true && testint == 27 && testfloat == 7.63f) {
            std::cout << "All test values were read correctly from the JSON.\n";
            rtn = 0;
        }
    }
    return rtn;
}
