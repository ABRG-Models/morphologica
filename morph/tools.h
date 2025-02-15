/*
 * Utility functions
 *
 * \author Seb James
 * \author Stuart Wilson
 */
#pragma once

#include <vector>
#include <utility>
#include <algorithm>
#include <string>
#include <string_view>
#include <sstream>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <filesystem>
#include <ctime>

/*!
 * Character sets
 *
 * These are ordered so that the most common chars appear earliest. Until C++20 (and constexpr
 * std::string) I didn't find a better way to express these (as string_view can't be
 * concatenated). They're used to define std::string_views below.
 */
#define CHARS_NUMERIC            "0123456789"
#define CHARS_ALPHA              "etaoinshrdlcumwfgypbvkjxqzETAOINSHRDLCUMWFGYPBVKJXQZ"
#define CHARS_ALPHALOWER         "etaoinshrdlcumwfgypbvkjxqz"
#define CHARS_ALPHAUPPER         "ETAOINSHRDLCUMWFGYPBVKJXQZ"
#define CHARS_NUMERIC_ALPHA      "etaoinshrdlcumwfgypbvkjxqz0123456789ETAOINSHRDLCUMWFGYPBVKJXQZ"
#define CHARS_NUMERIC_ALPHALOWER "etaoinshrdlcumwfgypbvkjxqz0123456789"
#define CHARS_NUMERIC_ALPHAUPPER "0123456789ETAOINSHRDLCUMWFGYPBVKJXQZ"

namespace morph
{
    /*!
     * Chars which are safe for IP domainnames. Allow numeric and alpha chars, the underscore and the
     * hyphen. colon is strictly allowed, but best avoided.
     */
    static constexpr std::string_view chars_xml_safe {CHARS_NUMERIC_ALPHA"_-"};

    /*!
     * These are the chars which are acceptable for use in both unix, mac AND windows file
     * names. This doesn't guarantee a safe Windows filename, as Windows imposes some extra
     * conditions (no '.' at end of name, some files such as NUL.txt AUX.txt disallowed).
     */
    static constexpr std::string_view chars_common_file_safe {CHARS_NUMERIC_ALPHA"_-.{}^[]`=,;"};

    /*!
     * Chars which are safe for IP domainnames
     */
    static constexpr std::string_view chars_ip_domainname_safe {CHARS_NUMERIC_ALPHA"-."};

    /*!
     * Chars which are safe for IP addresses
     */
    static constexpr std::string_view chars_ip_address_safe {CHARS_NUMERIC"."};

    //! Allows use of transform and tolower() on strings with GNU compiler
    struct to_lower { char operator() (const char c) const { return tolower(c); } };

    //! Allows use of transform and toupper() on strings with GNU compiler
    struct to_upper { char operator() (const char c) const { return toupper(c); } };

    namespace tools
    {
        /*!
         * If the last character of input is a carriage return ('\\r' 0xd), then it is
         * erased from input.
         */
        int stripTrailingCarriageReturn (std::string& input)
        {
            if (input[input.size()-1] == '\r') {
                input.erase(input.size()-1, 1);
                return 1;
            }
            return 0;
        }

        /*!
         * Erase trailing chars c from input. Return the number of chars removed.
         */
        int stripTrailingChars (std::string& input, const char c = ' ')
        {
            int i = 0;
            while (input.size()>0 && input[input.size()-1] == c) {
                input.erase (input.size()-1, 1);
                i++;
            }
            return i;
        }

        /*!
         * Erase trailing spaces from input. Return the number of spaces removed.
         */
        int stripTrailingSpaces (std::string& input) { return tools::stripTrailingChars (input); }

        /*!
         * Erase trailing whitespace from input. Return the number of whitespace
         * characters removed.
         */
        int stripTrailingWhitespace (std::string& input)
        {
            char c;
            std::string::size_type len = input.size(), pos = len;
            while (pos > 0 &&
                   ((c = input[pos-1]) == ' '
                    || c == '\t'
                    || c == '\n'
                    || c == '\r')) {
                pos--;
            }
            input.erase (pos);
            return (len - pos);
        }

        /*!
         * Erase any leading character c from input. Return the number of chars removed.
         */
        int stripLeadingChars (std::string& input, const char c = ' ')
        {
            int i = 0;
            while (input.size()>0 && input[0] == c) {
                input.erase (0, 1);
                i++;
            }
            return i;
        }

        /*!
         * Erase leading spaces from input. Return the number of spaces removed.
         */
        int stripLeadingSpaces (std::string& input)
        {
            return tools::stripLeadingChars (input);
        }

        /*!
         * Erase leading whitespace from input. Return the number of whitespace
         * characters removed.
         */
        int stripLeadingWhitespace (std::string& input)
        {
            char c;
            std::string::size_type pos = 0;
            while (pos<input.size() &&
                   ((c = input[pos]) == ' '
                    || c == '\t'
                    || c == '\n'
                    || c == '\r')) {
                pos++;
            }
            input.erase (0, pos);
            return pos;
        }

        /*!
         * Erase leading and trailing whitespace from input. Return the number of
         * whitespace characters removed.
         */
        int stripWhitespace (std::string& input)
        {
            int n = tools::stripLeadingWhitespace (input);
            n += tools::stripTrailingWhitespace (input);
            return n;
        }

        /*!
         * Return true if input contains only space, tab, newline chars.
         */
        bool containsOnlyWhitespace (std::string& input)
        {
            bool rtn = true;
            for (std::string::size_type i = 0; i < input.size(); ++i) {
                if (input[i] == ' ' || input[i] == '\t' || input[i] == '\n' || input[i] == '\r') {
                    // continue.
                } else {
                    rtn = false;
                    break;
                }
            }
            return rtn;
        }

        /*!
         * Do a search and replace, search for searchTerm, replacing with
         * replaceTerm. if replaceAll is true, replace all occurrences of searchTerm,
         * otherwise just replace the first occurrence of searchTerm with replaceTerm.
         *
         * \return the number of terms replaced.
         */
        int searchReplace (const std::string& searchTerm,
                           const std::string& replaceTerm,
                           std::string& data,
                           const bool replaceAll = true)
        {
            int count = 0;
            std::string::size_type pos = 0;
            std::string::size_type ptr = std::string::npos;
            std::string::size_type stl = searchTerm.size();
            if (replaceAll) {
                pos = data.size();
                while ((ptr = data.rfind (searchTerm, pos)) != std::string::npos) {
                    data.erase (ptr, stl);
                    data.insert (ptr, replaceTerm);
                    count++;
                    if (ptr >= stl) {
                        // This is a move backwards along the string far enough that we
                        // don't match a substring of the last replaceTerm in the next
                        // search.
                        pos = ptr - stl;
                    } else {
                        break;
                    }
                }
            } else {
                // Replace first only
                if ((ptr = data.find (searchTerm, pos)) != std::string::npos) {
                    data.erase (ptr, stl);
                    data.insert (ptr, replaceTerm);
                    count++;
                }
            }

            return count;
        }

        //! Convert str to lower case
        void toLowerCase (std::string& str)
        {
            std::transform (str.begin(), str.end(), str.begin(), morph::to_lower());
        }

        //! Convert str to upper case
        void toUpperCase (std::string& str)
        {
            std::transform (str.begin(), str.end(), str.begin(), morph::to_upper());
        }

        /*!
         * Remove filename-forbidden characters from str (including directory specifiers
         * '\' and '/'.
         */
        void conditionAsFilename (std::string& str)
        {
            std::string::size_type ptr = std::string::npos;
            while ((ptr = str.find_last_not_of (morph::chars_common_file_safe, ptr)) != std::string::npos) {
                str[ptr] = '_'; // Replacement character
                ptr--;
            }
        }

        /*!
         * Split a string of values into a vector using the separator string (not char)
         * passed in as "separator". If ignoreTrailingEmptyVal is true, then a trailing
         * separator with nothing after it will NOT cause an additional empty value in
         * the returned vector. See also splitStringWithEncs
         */
        std::vector<std::string> stringToVector (const std::string& s,
                                                 const std::string& separator,
                                                 const bool ignoreTrailingEmptyVal = true)
        {
            if (separator.empty()) {
                throw std::runtime_error ("Can't split the string; the separator is empty.");
            }
            std::vector<std::string> theVec;
            std::string entry("");
            std::string::size_type sepLen = separator.size();
            std::string::size_type a=0, b=0;
            while (a < s.size() && (b = s.find (separator, a)) != std::string::npos) {
                entry = s.substr (a, b-a);
                theVec.push_back (entry);
                a=b+sepLen;
            }
            // Last one has no separator
            if (a < s.size()) {
                b = s.size();
                entry = s.substr (a, b-a);
                theVec.push_back (entry);
            } else {
                if (!ignoreTrailingEmptyVal) {
                    theVec.push_back ("");
                }
            }

            return theVec;
        }

        /*
         * File and directory access methods
         */

        /*!
         * Stat a file, return true if the file exists and is any kind of file except a
         * directory.
         */
        bool fileExists (const std::string& path)
        {
            if (std::filesystem::exists(path)) {
                if (std::filesystem::is_regular_file (path)
                    || std::filesystem::is_block_file (path)
                    || std::filesystem::is_socket (path)
                    || std::filesystem::is_fifo (path)
                    || std::filesystem::is_symlink (path)
                    || std::filesystem::is_character_file (path)) {
                    return true;
                }
            }
            return false;
        }

        // Redundant, but still used functions wrapping std::filesystem functions
        bool dirExists (const std::string& path) { return std::filesystem::is_directory (path); }
        bool regfileExists (const std::string& path) { return std::filesystem::is_regular_file (path); }
        void createDir (const std::string& path) { std::filesystem::create_directories (path); }
        void removeDir (const std::string& path) { std::filesystem::remove (path); }
        std::string getPwd() { return std::filesystem::current_path(); }
        void unlinkFile (const std::string& fpath) { std::filesystem::remove (fpath); }

        /*!
         * Copy a file from an input stream into a string.
         */
        void copyFileToString (std::istream& from, std::string& to)
        {
            char buf[64];
            while (!from.eof()) {
                from.read (buf, 63);
                to.append (buf, from.gcount());
            }
        }

        /*!
         * Copy a string fromstr to a file named to
         */
        void copyStringToFile (const std::string& fromstr, const std::string& to)
        {
            std::ofstream out;
            out.open (to.c_str(), std::ios::out|std::ios::trunc);
            if (!out.is_open()) {
                std::stringstream ee;
                ee << "Failed to open file '" << to << "' for writing";
                throw std::runtime_error (ee.str());
            }
            out << fromstr;
            out.close();
        }

        /*!
         * Given a path like /path/to/file in str, remove all the preceding /path/to/
         * stuff to leave just the filename.
         */
        void stripUnixPath (std::string& unixPath)
        {
            std::string::size_type pos (unixPath.find_last_of ('/'));
            if (pos != std::string::npos) { unixPath = unixPath.substr (++pos); }
        }

        /*!
         * Given a path like /path/to/file in str, remove the final filename, leaving
         * just the path, "/path/to".
         */
        void stripUnixFile (std::string& unixPath)
        {
            std::string::size_type pos (unixPath.find_last_of ('/'));
            if (pos != std::string::npos) { unixPath = unixPath.substr (0, pos); }
        }

        /*!
         * Given a path to a file, identify the path and return it in the first element
         * of a pair of strings, returning the filename as the second element. The
         * unixPath could then be reconstructed as rt.first + string("/") + rtn.second.
         */
        std::pair<std::string, std::string> getUnixPathAndFile (const std::string& unixPath)
        {
            std::string fpath(unixPath);
            std::string fname(unixPath);
            morph::tools::stripUnixFile (fpath);
            morph::tools::stripUnixPath (fname);
            return std::make_pair (fpath, fname);
        }

        /*!
         * Given a path like /path/to/file.ext or just file.ext in str, remove the file
         * suffix.
         */
        void stripFileSuffix (std::string& unixPath)
        {
            std::string::size_type pos (unixPath.rfind('.'));
            if (pos != std::string::npos) {
                // We have a '.' character
                std::string tmp (unixPath.substr (0, pos));
                if (!tmp.empty()) { unixPath = tmp; }
            }
        }

        /*!
         * Return the current time in neat string format.
         */
        std::string timeNow()
        {
            std::time_t curtime = std::time (nullptr);
            return std::asctime(std::localtime (&curtime));
        }

    } // namespace tools
} // namespace morph
