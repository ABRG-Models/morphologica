/*
 * Process all of Fabio Crameri's OR the CET tables into a header file and also output text for
 * ColourMap.h's various functions
 *
 * Author: Seb James
 * Date: Oct 2024
 */
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <morph/tools.h>

enum class ctabletype {
    Crameri,
    CET,
    unknown
};

int main ()
{
    ctabletype tt = ctabletype::unknown;
    // First get a directory listing and make sure we're in the Crameri folder
    if (morph::tools::fileExists ("+README_ScientificColourMaps.pdf")) {
        tt = ctabletype::Crameri;
    } else if (morph::tools::fileExists ("CET-C1.csv")) {
        tt = ctabletype::CET;
    } else {
        std::cerr << "Run this program within the Crameri OR CET colour table directories\n";
        return -1;
    }

    // Get directories
    std::vector<std::string> dirs;
    std::vector<std::string> table_files;
    std::string basedir = "./";
    std::string subdir = "";
    morph::tools::readDirectoryTree (dirs, basedir, subdir);
    if (tt == ctabletype::Crameri) {
        for (auto dir : dirs) {
            std::cerr << "Got file " << dir << std::endl;
            if (dir.find (".txt") != std::string::npos
                && dir.find ("DiscretePalettes") == std::string::npos
                && dir.find ("CategoricalPalettes") == std::string::npos) {
                table_files.push_back (dir);
            }
        }
        std::cerr << table_files.size() << " Crameri maps to process\n";
    } else if (tt == ctabletype::CET) {
        for (auto dir : dirs) {
            std::cerr << "Got file " << dir << std::endl;
            if (dir.find (".csv") != std::string::npos) {
                table_files.push_back (dir);
            }
        }
    }

    std::ofstream hpp ((tt == ctabletype::Crameri ? "colourmaps_crameri.h" : "colourmaps_cet.h"), std::ios::out|std::ios::trunc);
    std::ofstream cpp_content0 ("colourmap_enum.cpp", std::ios::out|std::ios::trunc);
    std::ofstream cpp_content1 ("colourmap_colourMapTypeToStr.cpp", std::ios::out|std::ios::trunc);
    std::ofstream cpp_content2 ("colourmap_strToColourMapType.cpp", std::ios::out|std::ios::trunc);
    std::ofstream cpp_content3 ("colourmap_convert_switch.cpp", std::ios::out|std::ios::trunc);
    std::ofstream cpp_content4 ("colourmap_example.cpp", std::ios::out|std::ios::trunc);

    cpp_content0 << "// Section for ColourMapType enum\n";
    cpp_content1 << "// Section for morph::ColourMap::colourMapTypeToStr\n";
    cpp_content2 << "// Section for morph::ColourMap::strToColourMapType\n";
    cpp_content3 << "// Section for morph::ColourMap::convert switch\n";
    cpp_content4 << "// Section for examples/colourmaps_crameri.cpp\n";
    if (tt == ctabletype::Crameri) {
        hpp << "// Scientific Colour Maps from Fabio Crameri (see https://zenodo.org/records/8409685)\n"
            << "// Converted into C++ lookup tables for morphologica by Seb James\n\n"
            << "#pragma once\n\n"
            << "#include <array>\n\n"
            << "namespace morph {\n"
            << "  namespace crameri {\n";
    } else {
        hpp << "// CET Colour maps from https://colorcet.com/gallery.html\n"
            << "// Converted into C++ lookup tables for morphologica by Seb James\n\n"
            << "#pragma once\n\n"
            << "#include <array>\n\n"
            << "namespace morph {\n"
            << "  namespace cet {\n";
    }

    std::string nspacename = (tt == ctabletype::Crameri ? "crameri" : "cet");

    for (auto fpath : table_files) {
        std::cerr << "Got table file " << fpath << std::endl;

        // Obtain name from fpath
        std::string name = fpath;
        morph::tools::stripUnixPath (name);
        morph::tools::stripFileSuffix (name);
        if (name.empty()) {
            std::cerr << "No name.\n";
            return -1;
        }
        std::string name_lower = name;
        morph::tools::toLowerCase (name_lower);

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

        std::string name_upperfirst = name;
        // Upper-case first character
        std::transform (name_upperfirst.begin(), name_upperfirst.begin()+1, name_upperfirst.begin(), morph::to_upper());
        // Replace non-allowed chars with _
        std::string::size_type ptr = std::string::npos;
        while ((ptr = name_upperfirst.find_last_not_of (CHARS_NUMERIC_ALPHA"_", ptr)) != std::string::npos) {
            name_upperfirst[ptr] = '_';
            ptr--;
        }

        hpp << "\n    constexpr std::array<std::array<float, 3>, " << nlines << "> cm_" << name_upperfirst << " = {{\n";
        unsigned int cline = 0;
        unsigned int lastline = nlines - 1;
        for (std::string line; std::getline(ifile, line);) {
            // Process line into csv
            std::vector<std::string> tokens = morph::tools::stringToVector (line, (commas ? "," : " "));
            if (tokens.size() != 3) {
                std::cerr << "text format error: != 3 values in line '" << line << "', (got "<< tokens.size() << ")\n";
                return -1;
            }
            hpp << "            {"
                << tokens[0] << "," << tokens[1] << "," << tokens[2] << (cline++ < lastline ? "},\n" : "}\n");
        }
        hpp << "        }}; // cm_" << name_upperfirst << "\n";

        // Content output.
        cpp_content0 << "        " << name_upperfirst << ",\n";

        cpp_content1 << "            case morph::ColourMapType::" << name_upperfirst << ":\n"
                     << "            {\n"
                     << "                s = \"" << name_upperfirst << "\";\n"
                     << "                break;\n"
                     << "            }\n";

        cpp_content2 << "            } else if (_s == \"" << name_lower << "\") {\n"
                     << "                cmt = morph::ColourMapType::" << name_upperfirst << ";\n";

        cpp_content3 << "            case ColourMapType::"<< name_upperfirst << ":\n"
                     << "            {\n"
                     << "                size_t datum_i = static_cast<size_t>( std::abs (std::round (datum * static_cast<float>(morph::" << nspacename << "::cm_"<< name_upperfirst << ".size()-1))));\n"
                     << "                c = morph::" << nspacename << "::cm_" << name_upperfirst << "[datum_i];\n"
                     << "                break;\n"
                     << "            }\n";

        cpp_content4 << "    cmap_types.push_back (morph::ColourMapType::" << name_upperfirst << ");\n";
    }
    hpp << "  } // namespace " << nspacename << "\n";
    hpp << "} // namespace morph\n";

    hpp.close();
    cpp_content0.close();
    cpp_content1.close();
    cpp_content2.close();
    cpp_content3.close();
    cpp_content4.close();

    return 0;
}
