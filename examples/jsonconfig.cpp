/*
 * This example shows you how to read configuration information stored in a JSON file
 * using morph::Config.
 *
 * Author: Seb James
 * Date: May 2021
 */

#include <fstream>
#include <iostream>
#include <string>

#include "morph/Config.h"

int main()
{
    // First, create an example JSON file
    std::string jsonfile ("./exampleConfig.json");
    std::ofstream f;
    f.open ("./exampleConfig.json", std::ios::out | std::ios::trunc);
    if (!f.is_open()) {
        std::cerr << "Failed to open a file to write the example config JSON into\n";
        return -1;
    }
    f << "{\n"
      << "\"testbool\" : true,\n"
      << "\"testint\" : 27,\n"
      << "\"testfloat\" : 7.63,\n"
      << "\"testarray\" : [ 1.0, 2.1, 3.2 ],\n"
      << "\"testarray_of_objects\" : [ { \"desc\" : \"Point 1\", \"x\" : 1, \"y\" : 2},\n"
      << "                             { \"desc\" : \"Point 2\",  \"x\" : 2, \"y\" : 4} ]\n"
      << "}\n";
    f.close();

    // Now read from the example
    morph::Config config(jsonfile);
    if (config.ready) {

        // Single values are easy to read. The first arg matches the name in the example
        // JSON, the second arg is the default to return if "testbool" is absent.
        const bool testbool = config.getBool ("testbool", false);
        std::cout << "\ntestbool from JSON: " << (testbool ? "true" : "false") << " (expect: true)\n";

        // Get an integer from the config, defaulting to 3 if there's no "testint":
        const int testint = config.getInt ("testint", 3);
        std::cout << "\ntestint from JSON: " << testint << " (expect: 27)\n";

        // Get floating point numbers with getFloat() or getDouble():
        const float testfloat = config.getFloat ("testfloat", 9.8f);
        std::cout << "\ntestfloat from JSON: " << testfloat << " (expect: 7.63)\n";

        // A simple array of values:
        const auto testarray = config.get("testarray");
        std::cout << "\nValues of the simple array \"testarray\":\n   [   ";
        for (unsigned int j = 0; j < testarray.size(); ++j) {
            std::cout << testarray[j] << "   ";
        }
        std::cout << "]\n";

        // A more complicated array in which each array element is itself a JSON object
        const auto testarray_of_objects = config.get("testarray_of_objects");
        std::cout << "\nValues of the array of objects \"testarray_of_objects\":\n";
        for (unsigned int j = 0; j < testarray_of_objects.size(); ++j) {
            auto v = testarray_of_objects[j]; // v.type is nlohmann::basic_json<>::value_t
            // Each object has a "desc", an "x" and a "y" item:
            std::string desc = v["desc"];
            float x = v["x"];
            double y = v["y"];
            std::cout << "Array index " << j << ":  " << desc << ": (x=" << x << ", y=" << y << ")\n";
        }

        // Another way to iterate:
        for (auto& [key, obj] : testarray_of_objects.items()) {
            // Each obj is a json of objects
            std::cout << "Array index " << key << ":  " << std::string(obj["desc"]) << ": (x=" << obj["x"] << ", y=" << obj["y"] << ")\n";
        }

    } else {
        std::cout << "Something was wrong with the JSON file.\n";
    }

    return 0;
}
