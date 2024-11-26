/*
 * Process csv or txt files that contain colourmap tables into header-ready
 * code. Written to process Fabio Crameri's tables initially.
 *
 * Author: Seb James
 * Date: Oct 2024
 */
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <morph/tools.h>

int main (int argc, char** argv)
{
    std::string fpath ("unknown");
    if (argc > 1) { fpath = std::string (argv[1]); }
    //std::cout << "Processing file " << fpath << std::endl;

    // Obtain name from fpath
    std::string name = fpath;
    morph::tools::stripUnixPath (name);
    morph::tools::stripFileSuffix (name);
    if (name.empty()) {
        std::cerr << "No name.\n";
        return -1;
    }
    //std::cout << "Extracted name: " << name << std::endl;

    std::ifstream ifile (fpath, std::ios::in);
    if (!ifile.is_open()) {
        std::cerr << "Failed opening file " << fpath << "\n";
        return -1;
    }

    unsigned int nlines = 0;
    bool commas = false;
    for (std::string line; std::getline(ifile, line);) {
        // Check first line for commas
        if (nlines == 0) {
            if (line.find (",") != std::string::npos) {
                commas = true;
            }
        }
        ++nlines;
    }
    // reset ifile
    ifile.clear();
    ifile.seekg(0);

    std::cout << "\n    constexpr std::array<std::array<float, 3>, " << nlines << "> cm_" << name << " = {{\n";
    unsigned int cline = 0;
    unsigned int lastline = nlines - 1;
    for (std::string line; std::getline(ifile, line);) {
        // Process line into csv
        std::vector<std::string> tokens = morph::tools::stringToVector (line, (commas ? "," : " "));
        if (tokens.size() != 3) {
            std::cerr << "text format error: != 3 values in line '" << line << "', (got "<< tokens.size() << ")\n";
            return -1;
        }
        std::cout << "            {"
                  << tokens[0] << "," << tokens[1] << "," << tokens[2] << (cline++ < lastline ? "},\n" : "}\n");
    }
    std::cout << "        }}; // cm_" << name << "\n";

    return 0;
}
