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
#include <format>
#include <cstdlib>
#include <cstring>
#include <morph/tools.h>

// For readDirectoryTree
extern "C" {
# include <dirent.h>
# include <sys/stat.h>
}

namespace morph {
    namespace tools {
        // Read a directory tree. Declarations
        void readDirectoryTree (std::vector<std::string>& vec, const std::string& baseDirPath,
                                const std::string& subDirPath, const unsigned int olderThanSeconds = 0);
        void readDirectoryTree (std::vector<std::string>& vec, const std::string& dirPath,
                                const unsigned int olderThanSeconds = 0);
    } // tools
} // morph

enum class ctabletype {
    Crameri,
    CET,
    unknown
};

int main()
{
    ctabletype tt = ctabletype::unknown;
    // First get a directory listing and make sure we're in a compatible folder of colour maps
    if (morph::tools::fileExists ("+README_ScientificColourMaps.pdf")) {
        // Hint: Obtain ScientificColourMaps8.zip from https://www.fabiocrameri.ch/colourmaps/
        tt = ctabletype::Crameri;
    } else if (morph::tools::fileExists ("CET-C1.csv")) {
        // Hint: Obtain CETperceptual_csv_0_1.zip from https://colorcet.com/download/index.html
        tt = ctabletype::CET;
    } else {
        std::cerr << "Run this program from within the Crameri (ScientificColourMaps8) OR "
                  << "CET (CETperceptual_csv_0_1) colour table directories\n";
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
        // Upper-case first character (actually, no)
        // std::transform (name_upperfirst.begin(), name_upperfirst.begin()+1, name_upperfirst.begin(), morph::to_upper());
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
            hpp << std::format("            {{ {:.7f}f, {:.7f}f, {:.7f}f }}{}",
                               std::atof(tokens[0].c_str()), std::atof(tokens[1].c_str()), std::atof(tokens[2].c_str()),
                               (cline++ < lastline ? "," : "")) << std::endl;
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

namespace morph {
    namespace tools {

        /*!
         * This reads the contents of a directory tree, making up a list of the contents
         * in the vector vec. If the directory tree has subdirectories, these are
         * reflected in the vector entries. So, a directory structure might lead to the
         * following entries in vec:
         *
         * file2
         * file1
         * dir2/file2
         * dir2/file1
         * dir1/file1
         *
         * Note that the order of the files is REVERSED from what you might expect. This
         * is the way that readdir() seems to work. If it's important to iterate through
         * the vector<string>& vec, then use a reverse_iterator.
         *
         * The base directory path baseDirPath should have NO TRAILING '/'. The
         * subDirPath should have NO INITIAL '/' character.
         *
         * The subDirPath argument is present because this is a recursive function.
         *
         * If olderThanSeconds is passed in with a non-zero value, then only files older
         * than olderThanSeconds will be returned.
         */
        void readDirectoryTree (std::vector<std::string>& vec,
                                const std::string& baseDirPath,
                                const std::string& subDirPath,
                                const unsigned int olderThanSeconds)
        {
            DIR* d;
            struct dirent *ep;
            std::string::size_type entry_len = 0u;

            std::string dirPath (baseDirPath);
            std::string sd (subDirPath);
            if (!sd.empty()) {
                dirPath += "/" + sd;
            }

            if (!(d = opendir (dirPath.c_str()))) {
                std::string msg = "Failed to open directory " + dirPath;
                throw std::runtime_error (msg);
            }

            struct stat buf;
            while ((ep = readdir (d))) {

                unsigned char fileType;
                std::string fileName = dirPath + "/" + (std::string)ep->d_name;

                if (ep->d_type == DT_LNK) {
                    // Is it a link to a directory or a file?
                    struct stat * buf = NULL;
                    buf = static_cast<struct stat*>(malloc (sizeof (struct stat)));
                    if (!buf) { // Malloc error.
                        throw std::runtime_error ("Failed to malloc buf; could not stat link " + fileName);
                    }
                    std::memset (buf, 0, sizeof(struct stat));
                    if (stat (fileName.c_str(), buf)) {
                        throw std::runtime_error ("Failed to stat link " + fileName);
                    } else {
                        if (S_ISREG(buf->st_mode)) {
                            fileType = DT_REG;
                        } else if (S_ISDIR(buf->st_mode)) {
                            fileType = DT_DIR;
                        } else {
                            fileType = DT_UNKNOWN;
                        }
                    }
                    if (buf) { free (buf); }
                } else {
                    fileType = ep->d_type;
                }

                if (fileType == DT_DIR) {

                    // Skip "." and ".." directories
                    if ( ((entry_len = std::strlen (ep->d_name)) > 0 && ep->d_name[0] == '.') &&
                         (ep->d_name[1] == '\0' || ep->d_name[1] == '.') ) {
                        continue;
                    }

                    // For all other directories, recurse.
                    std::string newPath;
                    if (sd.size() == 0) {
                        newPath = ep->d_name;
                    } else {
                        newPath = sd + "/" + ep->d_name;
                    }
                    tools::readDirectoryTree (vec, baseDirPath, newPath.c_str(), olderThanSeconds);
                } else {
                    // Non-directories are simply added to the vector
                    std::string newEntry;
                    if (sd.size() == 0) {
                        newEntry = ep->d_name;
                    } else {
                        newEntry = sd + "/" + ep->d_name;
                    }

                    // If we have to check the file age, do so here before the vec.push_back()
                    if (olderThanSeconds > 0) {
                        // Stat the file
                        std::memset (&buf, 0, sizeof (struct stat));

                        if (stat (fileName.c_str(), &buf)) {
                            // no file to stat
                            continue;
                        }

                        if (static_cast<unsigned int>(time(NULL)) - buf.st_mtime
                            <= olderThanSeconds) {
                            // The age of the last modification is less than
                            // olderThanSeconds, so skip (we're only returning the OLDER
                            // files)
                            continue;
                        } //else DBG ("File " << fileName << " is older than " << olderThanSeconds << " s");
                    }
                    vec.push_back (newEntry);
                }
            }

            (void) closedir (d);
        }

        /*!
         * A simple wrapper around the more complex version, for the user to call.
         *
         * If olderThanSeconds is passed in with a non-zero value, then only files older
         * than olderThanSeconds will be returned.
         */
        void readDirectoryTree (std::vector<std::string>& vec,
                                const std::string& dirPath,
                                const unsigned int olderThanSeconds)
        {
            tools::readDirectoryTree (vec, dirPath, "", olderThanSeconds);
        }

    } //
} //
