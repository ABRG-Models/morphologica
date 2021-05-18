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

    morph::Config config(jsonfile);
    if (config.ready) {

        // Single values are easy to read
        const bool testbool = config.getBool ("testbool", false);
        std::cout << "\ntestbool from JSON: " << (testbool ? "true" : "false") << " (expect: true)\n";
        const int testint = config.getInt ("testint", 3);
        std::cout << "\ntestint from JSON: " << testint << " (expect: 27)\n";
        const float testfloat = config.getFloat ("testfloat", 9.8f);
        std::cout << "\ntestfloat from JSON: " << testfloat << " (expect: 7.63)\n";

        // A simple array
        const Json::Value testarray = config.getArray("testarray");
        std::cout << "\nValues of the simple array \"testarray\":\n   [   ";
        for (unsigned int j = 0; j < testarray.size(); ++j) {
            std::cout << testarray[j] << "   ";
        }
        std::cout << "]\n";

        // A more complicated array in which each array element is itself a JSON object
        const Json::Value testarray_of_objects = config.getArray("testarray_of_objects");
        std::cout << "\nValues of the array of objects \"testarray_of_objects\":\n";
        for (unsigned int j = 0; j < testarray_of_objects.size(); ++j) {
            Json::Value v = testarray_of_objects[j];
            // Each object has a "desc", an "x" and a "y" item:
            std::string desc = v.get ("desc", "unknown").asString();
            float x = v.get ("x", -1.0f).asFloat();
            double y = v.get ("y", -1.0f).asDouble();
            std::cout << "   " << desc << ": (x=" << x << ", y=" << y << ")\n";
        }

    } else {
        std::cout << "Something was wrong with the JSON file.\n";
    }

    return 0;
}
