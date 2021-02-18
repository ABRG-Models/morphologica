/*
 * Utility functions
 *
 * \author Seb James
 * \author Stuart Wilson
 */
#pragma once

#include <vector>
#include <array>
#include <list>
#include <set>
#include <map>
#include <string>
#include <sys/stat.h>
#include <stdlib.h>
#include <json/json.h>
#include <morph/Process.h>

#include <math.h>
#ifdef __ICC__
# define ARMA_ALLOW_FAKE_GCC 1
#endif
#include <armadillo>
#include <stdlib.h>
#include <stdexcept>

#include <stdio.h>
#ifdef __WIN__
# include <direct.h>
# define GetCurrentDir _getcwd
#else
# include <unistd.h>
# define GetCurrentDir getcwd
#endif

extern "C" {
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
}

/*!
 * Character sets useful when calling util::sanitize().
 *
 * These are ordered so that the most common chars appear earliest.
 */
//@{
#define CHARS_NUMERIC            "0123456789"
#define CHARS_ALPHA              "etaoinshrdlcumwfgypbvkjxqzETAOINSHRDLCUMWFGYPBVKJXQZ"
#define CHARS_ALPHALOWER         "etaoinshrdlcumwfgypbvkjxqz"
#define CHARS_ALPHAUPPER         "ETAOINSHRDLCUMWFGYPBVKJXQZ"
#define CHARS_NUMERIC_ALPHA      "etaoinshrdlcumwfgypbvkjxqz0123456789ETAOINSHRDLCUMWFGYPBVKJXQZ"
#define CHARS_NUMERIC_ALPHALOWER "etaoinshrdlcumwfgypbvkjxqz0123456789"
#define CHARS_NUMERIC_ALPHAUPPER "0123456789ETAOINSHRDLCUMWFGYPBVKJXQZ"
//@}

/*!
 * These are the chars which are acceptable for use in both unix, mac
 * AND windows file names. This doesn guarantee a safe Windows
 * filename, it imposes some extra conditions (no . at end of name,
 * some files such as NUL.txt AUX.txt disallowed).
 */
#define COMMON_FILE_SAFE_CHARS        CHARS_NUMERIC_ALPHA"_-.{}^[]`=,;"

/*!
 * Chars which are safe for IP domainnames
 */
#define IP_DOMAINNAME_SAFE_CHARS      CHARS_NUMERIC_ALPHA"-."

/*!
 * Chars which are safe for IP addresses
 */
#define IP_ADDRESS_SAFE_CHARS         CHARS_NUMERIC"."

namespace morph
{
    //! Callbacks class extends ProcessCallbacks
    class ToolsProcessCallbacks : public ProcessCallbacks
    {
    public:
        ToolsProcessCallbacks (ProcessData* p) { this->parent = p; }
        void startedSignal (std::string msg) {}
        void errorSignal (int err) { this->parent->setErrorNum (err); }
        void processFinishedSignal (std::string msg) { this->parent->setProcessFinishedMsg (msg); }
        void readyReadStandardOutputSignal (void) { this->parent->setStdOutReady (true); }
        void readyReadStandardErrorSignal (void) { this->parent->setStdErrReady (true); }
    private:
        ProcessData* parent;
    };

    //! Allows use of transform and tolower() on strings with GNU compiler
    struct to_lower { char operator() (const char c) const { return tolower(c); } };

    //! Allows use of transform and toupper() on strings with GNU compiler
    struct to_upper { char operator() (const char c) const { return toupper(c); } };

    class Tools
    {
    public:
        /*!
         * Launch git sub-processes to determine info about the current
         * repository. Intended for use with code that will save a Json formatted log of
         * a simulation run.
         *
         * \param root Insert the git tags into this Json object.
         *
         * \param codedir The name of the directory in which significant code is
         * located. If git status detects changes in this directory, then information to
         * this effect will be inserted into \a root.
         *
         * Superceded by morph::Config::insertGitInfo.
         */
        static void insertGitInfo (Json::Value& root, const std::string& codedir)
        {
            ProcessData pD;
            ToolsProcessCallbacks cb(&pD);
            Process p;
            std::string command ("/usr/bin/git");

            std::list<std::string> args1;
            args1.push_back ("git");
            args1.push_back ("rev-parse");
            args1.push_back ("HEAD");

            try {
                p.setCallbacks (&cb);
                p.start (command, args1);
                p.probeProcess ();
                if (!p.waitForStarted()) {
                    throw std::runtime_error ("Process failed to start");
                }
                while (p.running() == true) {
                    p.probeProcess();
                }

                std::stringstream theOutput;
                theOutput << p.readAllStandardOutput();
                std::string line = "";
                int nlines = 0;
                while (getline (theOutput, line, '\n')) {
                    std::cout << "Current git HEAD: " << line << std::endl;
                    if (nlines++ > 0) {
                        break;
                    }
                    root["git_head"] = line; // Should be one line only
                }

            } catch (const std::exception& e) {
                std::cerr << "Exception: " << e.what() << std::endl;
                root["git_head"] = "unknown";
            }

            // Reset Process with arg true to keep callbacks
            p.reset (true);

            std::list<std::string> args2;
            args2.push_back ("git");
            args2.push_back ("status");

            try {
                p.start (command, args2);
                p.probeProcess ();
                if (!p.waitForStarted()) {
                    throw std::runtime_error ("Process failed to start");
                }
                while (p.running() == true) {
                    p.probeProcess();
                }

                std::stringstream theOutput;
                theOutput << p.readAllStandardOutput();
                std::string line = "";
                bool lm = false;
                bool ut = false;
                while (getline (theOutput, line, '\n')) {
                    if (line.find("modified:") != std::string::npos) {
                        if (line.find(codedir) != std::string::npos) {
                            if (!lm) {
                                root["git_modified_sim"] = true;
                                std::cout << "Repository has local modifications in " << codedir << " dir\n";
                            }
                            lm = true;
                        }
                    }
                    if (line.find("Untracked files:") != std::string::npos) {
                        if (line.find(codedir) != std::string::npos) {
                            if (!ut) {
                                root["git_untracked_sim"] = true;
                                std::cout << "Repository has untracked files present in " << codedir << " dir\n";
                            }
                            ut = true;
                        }
                    }
                }

            } catch (const std::exception& e) {
                std::cerr << "Exception: " << e.what() << std::endl;
                root["git_status"] = "unknown";
            }

            // Reset for third call
            p.reset (true);

            // This gets the git branch name
            std::list<std::string> args3;
            args3.push_back ("git");
            args3.push_back ("rev-parse");
            args3.push_back ("--abbrev-ref");
            args3.push_back ("HEAD");

            try {
                p.start (command, args3);
                p.probeProcess ();
                if (!p.waitForStarted()) {
                    throw std::runtime_error ("Process failed to start");
                }
                while (p.running() == true) {
                    p.probeProcess();
                }

                std::stringstream theOutput;
                theOutput << p.readAllStandardOutput();
                std::string line = "";
                int nlines = 0;
                while (getline (theOutput, line, '\n')) {
                    std::cout << "Current git branch: " << line << std::endl;
                    if (nlines++ > 0) {
                        break;
                    }
                    root["git_branch"] = line; // Should be one line only
                }

            } catch (const std::exception& e) {
                std::cerr << "Exception: " << e.what() << std::endl;
                root["git_branch"] = "unknown";
            }
        }

        /*!
         * For mixing up bits of three args; used to generate a good random seed using
         * time() getpid() and clock().
         */
        static unsigned int mix (unsigned int a, unsigned int b, unsigned int c)
        {
            a=a-b;  a=a-c;  a=a^(c >> 13);
            b=b-c;  b=b-a;  b=b^(a << 8);
            c=c-a;  c=c-b;  c=c^(b >> 13);
            a=a-b;  a=a-c;  a=a^(c >> 12);
            b=b-c;  b=b-a;  b=b^(a << 16);
            c=c-a;  c=c-b;  c=c^(b >> 5);
            a=a-b;  a=a-c;  a=a^(c >> 3);
            b=b-c;  b=b-a;  b=b^(a << 10);
            c=c-a;  c=c-b;  c=c^(b >> 15);
            return c;
        }

        /*!
         * Using clock(), time() and getpid() along with the mix utility function,
         * generate a decently random seed for seeding your RNG.
         */
        static unsigned int randomSeed (void)
        {
            unsigned int rsd = morph::Tools::mix(clock(), time(NULL), getpid());
            return rsd;
        }

        /*!
         * Return a random double precision number in the range [0,1], sampled from a
         * uniform distribution.
         *
         * Don't use this! Use c++-11 random number generators, which include a 64 bit
         * Mersenne Twister algorithm as an option!
         */
        static double randDouble (void)
        {
            // FIXME: Have this (and randSingle) instantiate a singleton RandUniform
            // container, and then get instances from that.
            return static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
        }

        /*!
         * Return a random single precision number in the range [0,1], sampled from a
         * uniform distribution.
         *
         * Don't use this! Use c++-11 random number generators, which include a 64 bit
         * Mersenne Twister algorithm as an option!
         */
        static float randSingle (void)
        {
            return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        }

        /*!
         * Return a random floating point number with type F, where F is expected to be
         * either float or double.
         */
        template<typename F>
        static F randF (void);

        static double normalDistributionValue (void)
        {
            return sqrt(-2. * log(randDouble())) * cos(2. * M_PI * randDouble());
        }

        /*!
         * return indices of descending value in unsorted
         */
        static std::vector<int> sort (std::vector<double> unsorted)
        {
            std::vector<int> unsortID;
            for(int i=0;i<static_cast<int>(unsorted.size());i++){
                unsortID.push_back(i);
            }
            std::vector<int> sortID;
            std::vector<double> sorted;

            while(unsorted.size()){
                double maxVal = 0.;
                int maxInd = 0;
                for(int i=0;i<static_cast<int>(unsorted.size());i++){
                    if(unsorted[i]>maxVal){
                        maxVal = unsorted[i];
                        maxInd = i;
                    }
                }
                sorted.push_back(unsorted[maxInd]);
                sortID.push_back(unsortID[maxInd]);
                unsorted.erase(unsorted.begin()+maxInd);
                unsortID.erase(unsortID.begin()+maxInd);
            }
            return sortID;
        }

        /*!
         * This removes all carriage return characters ('\\r' 0xd) from input. It will
         * convert all DOS style newlines, which consist of '\\r''\\n' character
         * duplets, to UNIX style newlines ('\\n'). A side effect is that any lone '\\r'
         * characters which are present will be removed, whether or not they are
         * followed by a '\\n' character.
         */
        static int ensureUnixNewlines (std::string& input)
        {
            int num = 0;

            for (unsigned int i=0; i<input.size(); i++) {
                if (input[i] == '\r') {
                    input.erase(i,1);
                    num++;
                }
            }

            return num; // The number of \r characters we found in the string.
        }

        /*!
         * Get the working directory
         */
        static std::string getPwd (void)
        {
            char b[FILENAME_MAX];
            GetCurrentDir (b, FILENAME_MAX);
            return std::string(b);
        }

        /*!
         * If the last character of input is a carriage return ('\\r' 0xd), then it is
         * erased from input.
         */
        static int stripTrailingCarriageReturn (std::string& input)
        {
            if (input[input.size()-1] == '\r') {
                input.erase(input.size()-1, 1);
                return 1;
            }
            return 0;
        }

        /*!
         * Erase trailing spaces from input. Return the number of spaces removed.
         */
        static int stripTrailingSpaces (std::string& input)
        {
            return Tools::stripTrailingChars (input);
        }

        /*!
         * Erase trailing chars c from input. Return the number of chars removed.
         */
        static int stripTrailingChars (std::string& input, const char c = ' ')
        {
            int i = 0;
            while (input.size()>0 && input[input.size()-1] == c) {
                input.erase (input.size()-1, 1);
                i++;
            }
            return i;
        }

        /*!
         * Erase trailing whitespace from input. Return the number of whitespace
         * characters removed.
         */
        static int stripTrailingWhitespace (std::string& input)
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
         * Erase leading spaces from input. Return the number of spaces removed.
         */
        static int stripLeadingSpaces (std::string& input)
        {
            return Tools::stripLeadingChars (input);
        }

        /*!
         * Erase any leading character c from input. Return the number of chars removed.
         */
        static int stripLeadingChars (std::string& input, const char c = ' ')
        {
            int i = 0;
            while (input.size()>0 && input[0] == c) {
                input.erase (0, 1);
                i++;
            }
            return i;
        }

        /*!
         * Erase leading whitespace from input. Return the number of whitespace
         * characters removed.
         */
        static int stripLeadingWhitespace (std::string& input)
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
        static int stripWhitespace (std::string& input)
        {
            int n = Tools::stripLeadingWhitespace (input);
            n += Tools::stripTrailingWhitespace (input);
            return n;
        }

        /*!
         * Return true if input contains only space, tab, newline chars.
         */
        static bool containsOnlyWhitespace (std::string& input)
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
         * Strip any occurrences of the characters in charList from input.
         */
        static int stripChars (std::string& input, const std::string& charList)
        {
            int rtn(0);
            std::string::size_type pos(0);
            while ((pos = input.find_last_of (charList)) != std::string::npos) {
                input.erase (pos, 1);
                ++rtn;
            }
            return rtn;
        }

        /*!
         * Strip any occurrences of the characters in charList from input.
         */
        static int stripChars (std::string& input, const char charList)
        {
            int rtn(0);
            std::string::size_type pos(0);
            while ((pos = input.find_last_of (charList)) != std::string::npos) {
                input.erase (pos, 1);
                ++rtn;
            }
            return rtn;
        }

        /*!
         * Convert any C-style hex character sequence into its corresponding character.
         *
         * E.g. "\x41" becomes "A" "\x1b" becomes an escape char, etc.
         *
         * \return The number of hex sequences replaced in \param input.
         */
        static int convertCHexCharSequences (std::string& input)
        {
            // This converts a string containing C style hex sequences
            // like "\x41\x42\x43" into the corresponding characters
            // ("ABC" for the example).

            std::string::iterator readPos = input.begin();
            std::string::iterator writePos = input.begin();
            std::string::size_type newSize = 0;
            char n1 = '\0', n2 = '\0'; // two Ns in "1xNN"
            char c = 0;
            int count = 0;

            while (readPos != input.end()) {

                c = *readPos;

                if (*readPos == '\\') {
                    // We have a possible hex escape sequence.
                    ++readPos;
                    if (readPos != input.end() && *readPos == 'x') {
                        // We have a hex escape sequence. Read in next two chars
                        ++readPos;
                        if (readPos != input.end()) {
                            n1 = *readPos;
                            ++readPos;
                            if (readPos != input.end()) {
                                n2 = *readPos;
                                ++count;
                                // Now create the replacement for c.
                                c = 0;
                                switch (n1) {
                                case '0':
                                    // c |= 0 << 4;
                                    break;
                                case '1':
                                    c |= 1 << 4;
                                    break;
                                case '2':
                                    c |= 2 << 4;
                                    break;
                                case '3':
                                    c |= 3 << 4;
                                    break;
                                case '4':
                                    c |= 4 << 4;
                                    break;
                                case '5':
                                    c |= 5 << 4;
                                    break;
                                case '6':
                                    c |= 6 << 4;
                                    break;
                                case '7':
                                    c |= 7 << 4;
                                    break;
                                case '8':
                                    c |= 8 << 4;
                                    break;
                                case '9':
                                    c |= 9 << 4;
                                    break;
                                case 'a':
                                case 'A':
                                    c |= 10 << 4;
                                    break;
                                case 'b':
                                case 'B':
                                    c |= 11 << 4;
                                    break;
                                case 'c':
                                case 'C':
                                    c |= 12 << 4;
                                    break;
                                case 'd':
                                case 'D':
                                    c |= 13 << 4;
                                    break;
                                case 'e':
                                case 'E':
                                    c |= 14 << 4;
                                    break;
                                case 'f':
                                case 'F':
                                    c |= 15 << 4;
                                    break;
                                default:
                                    break;
                                }

                                switch (n2) {
                                case '0':
                                    // c |= 0;
                                    break;
                                case '1':
                                    c |= 1;
                                    break;
                                case '2':
                                    c |= 2;
                                    break;
                                case '3':
                                    c |= 3;
                                    break;
                                case '4':
                                    c |= 4;
                                    break;
                                case '5':
                                    c |= 5;
                                    break;
                                case '6':
                                    c |= 6;
                                    break;
                                case '7':
                                    c |= 7;
                                    break;
                                case '8':
                                    c |= 8;
                                    break;
                                case '9':
                                    c |= 9;
                                    break;
                                case 'a':
                                case 'A':
                                    c |= 10;
                                    break;
                                case 'b':
                                case 'B':
                                    c |= 11;
                                    break;
                                case 'c':
                                case 'C':
                                    c |= 12;
                                    break;
                                case 'd':
                                case 'D':
                                    c |= 13;
                                    break;
                                case 'e':
                                case 'E':
                                    c |= 14;
                                    break;
                                case 'f':
                                case 'F':
                                    c |= 15;
                                    break;
                                default:
                                    break;
                                }

                            } else {
                                // Nothing following "\xN", step back 3.
                                --readPos;
                                --readPos;
                                --readPos;
                            }
                        } else {
                            // Nothing following "\x", step back 2.
                            --readPos;
                            --readPos;
                        }

                    } else {
                        // Not an escape sequence, just a '\' character. Step back 1.
                        --readPos;
                    }

                } else {
                    // We already set writePos to readPos and c to *readPos.
                }

                // if need to write
                *writePos = c;
                ++writePos;
                ++newSize;

                ++readPos;
            }

            // Terminate the now possibly shorter string:
            input.resize (newSize);

            return count;
        }

        /*!
         * Do a search and replace, search for searchTerm, replacing with
         * replaceTerm. if replaceAll is true, replace all occurrences of searchTerm,
         * otherwise just replace the first occurrence of searchTerm with replaceTerm.
         *
         * \return the number of terms replaced.
         */
        static int searchReplace (const std::string& searchTerm,
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
                        // This is a move backwards along the
                        // string far enough that we don't
                        // match a substring of the last
                        // replaceTerm in the next search.
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

        /*!
         * Return the number of instances of the character c
         * in line.
         */
        static unsigned int countChars (const std::string& line, const char c)
        {
            unsigned int count(0);
            std::string::const_iterator i = line.begin();
            while (i != line.end()) {
                if (*i++ == c) { ++count; }
            }
            return count;
        }

        /*!
         * Take the string str and condition it, so that it makes a valid XML tag, by
         * replacing disallowed characters with '_' and making sure it doesn't start
         * with a numeral.
         */
        static void conditionAsXmlTag (std::string& str)
        {
            // 1) Replace chars which are disallowed in an XML tag
            std::string::size_type ptr = std::string::npos;

            // We allow numeric and alpha chars, the underscore and the
            // hyphen. colon strictly allowed, but best avoided.
            while ((ptr = str.find_last_not_of (CHARS_NUMERIC_ALPHA"_-", ptr)) != std::string::npos) {
                // Replace the char with an underscore:
                str[ptr] = '_';
                ptr--;
            }

            // 2) Check first 3 chars don't spell xml (in any case)
            std::string firstThree = str.substr (0,3);
            transform (firstThree.begin(), firstThree.end(),
                       firstThree.begin(), morph::to_lower());
            if (firstThree == "xml") {
                // Prepend 'A'
                std::string newStr("_");
                newStr += str;
                str = newStr;
            }

            // 3) Prepend an '_' if initial char begins with a numeral or hyphen
            if (str[0] > 0x29 && str[0] < 0x3a) {
                // Prepend '_'
                std::string newStr("_");
                newStr += str;
                str = newStr;
            }
        }

        /*!
         * split csv into a vector
         */
        static std::vector<std::string> csvToVector (const std::string& csvList,
                                                     const char separator = ',',
                                                     const bool ignoreTrailingEmptyVal = true);

        /*!
         * split csv into a list
         */
        static std::list<std::string> csvToList (const std::string& csvList,
                                                 const char separator = ',');
        /*!
         * split csv into a set
         */
        static std::set<std::string> csvToSet (const std::string& csvList,
                                               const char separator = ',');

        /*!
         * Output a vector of strings as a csv string.
         */
        static std::string vectorToCsv (const std::vector<std::string>& vecList,
                                        const char separator = ',');

        /*!
         * Output a list of strings as a csv string.
         */
        static std::string listToCsv (const std::list<std::string>& listList,
                                      const char separator = ',');

        /*!
         * Output a set of strings as a csv string.
         */
        static std::string setToCsv (const std::set<std::string>& listList,
                                     const char separator = ',');

        /*!
         * Split a comma-separated key/value pair list into a map.
         */
        static std::map<std::string, std::string> csvToMap (const std::string& csvList,
                                                            const char relationship = '=',
                                                            const char separator = ',');
        /*!
         * Split a string of values into a vector using the separator string (not char)
         * passed in as "separator". If ignoreTrailingEmptyVal is true, then a trailing
         * separator with nothing after it will NOT cause an additional empty value in
         * the returned vector.
         *
         * Similar to util::splitString but FASTER. PREFER THIS OVER splitString.
         */
        static std::vector<std::string> stringToVector (const std::string& s,
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
            while (a < s.size()
                   && (b = s.find (separator, a)) != std::string::npos) {
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
        static bool fileExists (const std::string& path)
        {
            struct stat * buf = NULL;

            buf = static_cast<struct stat*> (malloc (sizeof (struct stat)));
            if (buf == NULL) {
                //cerr << "Memory allocation error in futil::fileExists";
                return false;
            }
            memset (buf, 0, sizeof (struct stat));

            if (stat (path.c_str(), buf)) {
                free (buf);
                return false;
            }

            if (S_ISREG (buf->st_mode)
                || S_ISBLK (buf->st_mode)
                || S_ISSOCK (buf->st_mode)
                || S_ISFIFO (buf->st_mode)
                || S_ISLNK (buf->st_mode)
                || S_ISCHR (buf->st_mode)) {
                free (buf);
                return true;
            }

            free (buf);
            return false;
        }

        /*!
         * Stat a file, return true if the file exists and is a regular file.  If file
         * is a hanging symlink, fileExists returns false, if file is a symlink pointing
         * to a regular file, fileExists returns true.
         */
        static bool regfileExists (const std::string& path)
        {
            struct stat * buf = NULL;

            buf = static_cast<struct stat*> (malloc (sizeof (struct stat)));
            memset (buf, 0, sizeof (struct stat));

            if (stat (path.c_str(), buf)) {
                free (buf);
                return false;
            }

            if (S_ISREG (buf->st_mode)) {
                free (buf);
                return true;
            }

            free (buf);
            return false;
        }

        /*!
         * Like regfileExists, but also checks if the file has the "executable by user"
         * bit set (chmod u+x).
         */
        static bool userExefileExists (const std::string& path)
        {
            struct stat * buf = NULL;

            buf = static_cast<struct stat*> (malloc (sizeof (struct stat)));
            memset (buf, 0, sizeof (struct stat));

            if (stat (path.c_str(), buf)) {
                free (buf);
                return false;
            }

            if (S_ISREG (buf->st_mode) && (S_IXUSR & buf->st_mode) ) {
                free (buf);
                return true;
            }

            free (buf);
            return false;
        }

        /*!
         * Like regfileExists, but for block devices
         */
        static bool blockdevExists (const std::string& path)
        {
            struct stat * buf = NULL;

            buf = static_cast<struct stat*> (malloc (sizeof (struct stat)));
            memset (buf, 0, sizeof (struct stat));

            if (stat (path.c_str(), buf)) {
                free (buf);
                return false;
            }

            if (S_ISBLK (buf->st_mode)) {
                free (buf);
                return true;
            }

            free (buf);
            return false;
        }

        /*!
         * Like regfileExists, but for sockets
         */
        static bool socketExists (const std::string& path)
        {
            struct stat * buf = NULL;

            buf = static_cast<struct stat*> (malloc (sizeof (struct stat)));
            memset (buf, 0, sizeof (struct stat));

            if (stat (path.c_str(), buf)) {
                free (buf);
                return false;
            }

            if (S_ISSOCK (buf->st_mode)) {
                free (buf);
                return true;
            }

            free (buf);
            return false;
        }

        /*!
         * Like regfileExists, but for fifos
         */
        static bool fifoExists (const std::string& path)
        {
            struct stat * buf = NULL;

            buf = static_cast<struct stat*> (malloc (sizeof (struct stat)));
            memset (buf, 0, sizeof (struct stat));

            if (stat (path.c_str(), buf)) {
                free (buf);
                return false;
            }

            if (S_ISFIFO (buf->st_mode)) {
                free (buf);
                return true;
            }

            free (buf);
            return false;
        }

        /*!
         * Like regfileExists, but for char devices
         */
        static bool chardevExists (const std::string& path)
        {
            struct stat * buf = NULL;

            buf = static_cast<struct stat*> (malloc (sizeof (struct stat)));
            memset (buf, 0, sizeof (struct stat));

            if (stat (path.c_str(), buf)) {
                free (buf);
                return false;
            }

            if (S_ISCHR (buf->st_mode)) {
                free (buf);
                return true;
            }

            free (buf);
            return false;
        }

        /*!
         * Does a link exist?
         */
        static bool linkExists (const std::string& path)
        {
            struct stat * buf = NULL;

            buf = static_cast<struct stat*> (malloc (sizeof (struct stat)));
            memset (buf, 0, sizeof (struct stat));

            if (stat (path.c_str(), buf)) {
                free (buf);
                return false;
            }

            if (S_ISLNK (buf->st_mode)) {
                free (buf);
                return true;
            }

            free (buf);
            return false;
        }

        /*!
         * Stat a directory, return true if the directory exists.
         */
        static bool dirExists (const std::string& path)
        {
            DIR* d;
            if (!(d = opendir (path.c_str()))) {
                // Dir doesn't exist.
                return false;
            } else {
                // Dir does exist.
                (void) closedir (d);
                return true;
            }
        }

        /*!
         * Create the directory and any parent directories which need to be created.
         *
         * Makes use of mkdir() and acts like the system command mkdir -p path.
         *
         * If uid/gid is set to >-1, then chown each directory. This means that
         * ownership is set for the directories in the path even if the directories do
         * not need to be created.
         *
         * \param path The path (relative or absolute) to the directory which should be
         * created.
         *
         * \param mode the permissions mode which should be set on the directory. This
         * is applied even if the directory was not created.
         *
         * \param uid The user id to apply to the directory. This is applied even if the
         * directory was not created. This is NOT applied if it is set to -1.
         *
         * \param gid The group id to apply to the directory. This is applied even if
         * the directory was not created. This is NOT applied if it is set to -1.
         */
        static void createDir (const std::string& path,
                               const mode_t mode = 0775,
                               const int uid = -1, const int gid = -1)
        {
            if (path.empty()) { return; }

            // Set to true if we are provided with an absolute filepath
            bool pathIsAbsolute(false);

            // Set umask to 0000 to stop it interfering with mode
            int oldUmask = umask (0000);
            std::string::size_type pos, lastPos = path.size()-1;
            std::vector<std::string> dirs;
            if ((pos = path.find_last_of ('/', lastPos)) == std::string::npos) {
                // Path is single directory.
                dirs.push_back (path);
            } else {
                // Definitely DO have a '/' in the path somewhere:
                if (path[0] == '/') {
                    pathIsAbsolute = true;
                    while ((pos = path.find_last_of ('/', lastPos)) != 0) {
                        dirs.push_back (path.substr(pos+1, lastPos-pos));
                        lastPos = pos-1;
                    }
                    dirs.push_back (path.substr(1, lastPos));
                } else {
                    // Non absolute...
                    while ((pos = path.find_last_of ('/', lastPos)) != 0) {
                        dirs.push_back (path.substr(pos+1, lastPos-pos));
                        lastPos = pos-1;
                        if (pos == std::string::npos) {
                            break;
                        }
                    }
                }
            }

            std::vector<std::string>::reverse_iterator i = dirs.rbegin();
            std::string prePath("");
            bool first(true);
            while (i != dirs.rend()) {
                if (first && !pathIsAbsolute) {
                    prePath += "./" + *i;
                    first = false;
                } else {
                    prePath += "/" + *i;
                }
                int rtn = mkdir (prePath.c_str(), mode);
                if (rtn) {
                    int e = errno;
                    std::stringstream emsg;
                    emsg << "createDir(): mkdir() set error: ";
                    switch (e) {
                    case EACCES:
                        emsg << "Permission is denied";
                        break;
                    case EEXIST:
                        // Path exists, though maybe not as a directory.
                        // Set mode/ownership before moving on:
                        if (uid>-1 && gid>-1) {
                            chown (prePath.c_str(), static_cast<uid_t>(uid), static_cast<gid_t>(gid));
                            chmod (prePath.c_str(), mode);
                        }
                        i++;
                        continue;
                        break;
                    case EFAULT:
                        emsg << "Bad address";
                        break;
                    case ELOOP:
                        emsg << "Too many symlinks in " << prePath;
                        break;
                    case ENAMETOOLONG:
                        emsg << "File name (" << prePath << ") too long";
                        break;
                    case ENOENT:
                        emsg << "Path '" << prePath << "' invalid (part or all of it)";
                        break;
                    case ENOMEM:
                        emsg << "Out of kernel memory";
                        break;
                    case ENOSPC:
                        emsg << "Out of storage space/quota exceeded.";
                        break;
                    case ENOTDIR:
                        emsg << "component of the path '" << prePath << "' is not a directory";
                        break;
                    case EPERM:
                        emsg << "file system doesn't support directory creation";
                        break;
                    case EROFS:
                        emsg << "path '" << prePath << "' refers to location on read only filesystem";
                        break;
                    default:
                        emsg << "unknown error";
                        break;
                    }
                    throw std::runtime_error (emsg.str());
                }
                if (uid>-1 && gid>-1) {
                    chown (prePath.c_str(), static_cast<uid_t>(uid), static_cast<gid_t>(gid));
                }
                i++;
            }

            // Reset umask
            umask (oldUmask);
        }

        /*!
         * Attempt to rmdir path.
         */
        static void removeDir (const std::string& path)
        {
            int rtn = rmdir (path.c_str());
            if (rtn) {
                int e = errno;
                std::stringstream emsg;
                emsg << "setPermissions(): chmod() set error: ";
                switch (e) {
                case EACCES:
                    emsg << "Permission is denied";
                    break;
                case EBUSY:
                    emsg << "Path in use";
                    break;
                case EFAULT:
                    emsg << "Bad address";
                    break;
                case EINVAL:
                    emsg << "Path has . as last component";
                    break;
                case ELOOP:
                    emsg << "Too many symlinks";
                    break;
                case ENAMETOOLONG:
                    emsg << "File name too long";
                    break;
                case ENOENT:
                    emsg << "Path invalid (part or all of it)";
                    break;
                case ENOMEM:
                    emsg << "Out of kernel memory";
                    break;
                case ENOTDIR:
                    emsg << "component of the path is not a directory";
                    break;
                case EPERM:
                    emsg << "file system doesn't support directory creation";
                    break;
                case EROFS:
                    emsg << "path refers to location on read only filesystem";
                    break;
                default:
                    emsg << "unknown error";
                    break;
                }
                throw std::runtime_error (emsg.str());
            }
        }

        /*!
         * Set the permissions for the provided file
         */
        static void setPermissions (const std::string& filepath, const mode_t mode)
        {
            int rtn = chmod (filepath.c_str(), mode);
            if (rtn) {
                int e = errno;
                std::stringstream emsg;
                emsg << "setPermissions(): chmod() set error: ";
                switch (e) {
                case EACCES:
                    emsg << "Permission is denied";
                    break;
                case EFAULT:
                    emsg << "Bad address";
                    break;
                case ELOOP:
                    emsg << "Too many symlinks";
                    break;
                case ENAMETOOLONG:
                    emsg << "File name too long";
                    break;
                case ENOENT:
                    emsg << "Path invalid (part or all of it)";
                    break;
                case ENOMEM:
                    emsg << "Out of kernel memory";
                    break;
                case ENOTDIR:
                    emsg << "component of the path is not a directory";
                    break;
                case EPERM:
                    emsg << "file system doesn't support directory creation";
                    break;
                case EROFS:
                    emsg << "path refers to location on read only filesystem";
                    break;
                case EBADF:
                    emsg << "file descriptor is not valid";
                    break;
                case EIO:
                    emsg << "an i/o error occurred";
                    break;
                default:
                    emsg << "unknown error";
                    break;
                }
                throw std::runtime_error (emsg.str());
            }
        }

        /*!
         * Check read/write access for the specified file.
         *
         * Checks whether read/write access, as indicated by
         * the accessType string, is available for the
         * specified file.
         *
         * \param filepath File to check.
         * \param accessType Indicates which access type(s) to
         * check. r=read, w=write.
         */
        static bool checkAccess (const std::string& filepath, const std::string& accessType)
        {
            if (accessType.find("r") != std::string::npos) {
                std::ifstream in;
                in.open (filepath.c_str(), std::ios::in);
                if (!in.is_open()) {
                    return false;
                }
                in.close();
            }
            if (accessType.find("w") != std::string::npos) {
                std::ofstream out;
                out.open (filepath.c_str(), std::ios::out);
                if (!out.is_open()) {
                    return false;
                }
                out.close();
            }
            return true;
        }

        /*!
         * Set the ownership for the provided file
         */
        static void setOwnership (const std::string& filepath,
                                  const int uid = -1,
                                  const int gid = -1)
        {
            int rtn = chown (filepath.c_str(), uid, gid);
            if (rtn) {
                int e = errno;
                std::stringstream emsg;
                emsg << "setOwnership(): chown() set error: ";
                switch (e) {
                case EACCES:
                    emsg << "Permission is denied";
                    break;
                case EFAULT:
                    emsg << "Bad address";
                    break;
                case ELOOP:
                    emsg << "Too many symlinks";
                    break;
                case ENAMETOOLONG:
                    emsg << "File name too long";
                    break;
                case ENOENT:
                    emsg << "Path invalid (part or all of it)";
                    break;
                case ENOMEM:
                    emsg << "Out of kernel memory";
                    break;
                case ENOTDIR:
                    emsg << "component of the path is not a directory";
                    break;
                case EPERM:
                    emsg << "file system doesn't support directory creation";
                    break;
                case EROFS:
                    emsg << "path refers to location on read only filesystem";
                    break;
                case EBADF:
                    emsg << "file descriptor is not valid";
                    break;
                case EIO:
                    emsg << "an i/o error occurred";
                    break;
                default:
                    emsg << "unknown error";
                    break;
                }
                throw std::runtime_error (emsg.str());
            }
        }

        /*!
         * Touch the file.
         */
        static void touchFile (const std::string& path)
        {
            std::ofstream f;
            f.open (path.c_str(), std::ios::out|std::ios::app);
            if (!f.is_open()) {
                f.open (path.c_str(), std::ios::out|std::ios::trunc);
                if (!f.is_open()) {
                    std::string emsg = "Failed to create file '" + path + "'";
                    throw std::runtime_error (emsg);
                } else {
                    f.close();
                }
            } else {
                // File is open, was already there
                f.close();
            }
        }

        /*!
         * Copy a file. If from/to is a string or a char*, then these are the
         * filepaths. Some versions allow you to copy the file contents into an output
         * stream. Throw exception on failure.
         *
         * The "from" file is expected to be a regular file - an exception will be
         * thrown if this is not the case.
         */
#define COPYFILE_BUFFERSIZE    32768
#define COPYFILE_BUFFERSIZE_MM 32767 // MM: Minus Minus
        //@{
        static void copyFile (const std::string& from, const std::string& to)
        {
            std::ofstream out;

            out.open (to.c_str(), std::ios::out|std::ios::trunc);
            if (!out.is_open()) {
                std::string emsg = "Tools::copyFile(): Couldn't open TO file '" + to + "'";
                throw std::runtime_error (emsg);
            }

            Tools::copyFile (from, out);

            out.close();
        }
        static void copyFile (const std::string& from, std::ostream& to)
        {
            std::ifstream in;

            // Test that "from" is a regular file
            if (!Tools::regfileExists (from)) {
                std::stringstream ee;
                ee << "Tools::copyFile(): FROM file '"
                   << from << "' is not a regular file";
                throw std::runtime_error (ee.str());
            }

            in.open (from.c_str(), std::ios::in);
            if (!in.is_open()) {
                throw std::runtime_error ("Tools::copyFile(): Couldn't open FROM file");
            }

            if (!to) {
                throw std::runtime_error ("Tools::copyFile(): Error occurred in TO stream");
            }

            char buf[64];
            while (!in.eof()) {
                in.read (buf, 63);
                // Find out how many were read
                unsigned int bytes = in.gcount();
                // and write that many to the output stream
                to.write (buf, bytes);
            }

            // Make sure output buffer is flushed.
            to.flush();

            // Finally, close the input.
            in.close();
        }
        static void copyFile (FILE* from, const std::string& to)
        {
            FILE * ofp = NULL;
            long pos;
            int bytes=0, output=0;
            char inputBuffer[COPYFILE_BUFFERSIZE];

            // Get posn of from as we will return the file pointer there when done
            pos = ftell (from);

            ofp = fopen (to.c_str(), "w");
            if (!ofp) {
                throw std::runtime_error ("Tools::copyFile(): Can't open output for writing");
            }
            while ((bytes = fread (inputBuffer, 1, COPYFILE_BUFFERSIZE_MM, from)) > 0) {
                output = fwrite (inputBuffer, 1, bytes, ofp);
                if (output != bytes) {
                    fseek (from, pos, SEEK_SET); /* reset input */
                    throw std::runtime_error ("Tools::copyFile(): Error writing data");
                }
            }
            fclose (ofp); /* close output */
            fseek (from, pos, SEEK_SET); /* reset input */
        }
        static void copyFile (std::istream& from, const std::string& to)
        {
            char buf[64];
            std::ofstream f;
            f.open (to.c_str(), std::ios::out|std::ios::trunc);
            if (!f.is_open()) {
                std::stringstream ee;
                ee << "Failed to open output file '" << to << "'";
                throw std::runtime_error (ee.str());
            }
            while (!from.eof()) {
                from.read (buf, 63);
                f.write (buf, from.gcount());
            }
        }
        static void copyFile (const std::string& from, FILE* to)
        {
            FILE* ifp = fopen (from.c_str(), "r");
            Tools::copyFile (ifp, to);
            fclose (ifp);
        }
        //@}

        /*!
         * Copy from one file pointer to another. Both are
         * expected to be open, neither is closed after the
         * copy.
         */
        static void copyFile (FILE* from, FILE* to)
        {
            long pos;
            int bytes=0, output=0;
            char inputBuffer[COPYFILE_BUFFERSIZE];

            // Get posn of from as we will return the file pointer there when done
            pos = ftell (from);

            if (!to) {
                throw std::runtime_error ("Tools::copyFile(): output is not open for writing");
            }
            while ((bytes = fread (inputBuffer, 1, COPYFILE_BUFFERSIZE_MM, from)) > 0) {
                output = fwrite (inputBuffer, 1, bytes, to);
                if (output != bytes) {
                    fseek (from, pos, SEEK_SET); /* reset input */
                    throw std::runtime_error ("Tools::copyFile(): Error writing data");
                }
            }
            fseek (from, pos, SEEK_SET); /* reset input */
        }

        /*!
         * Copy a file from an input stream into a string.
         */
        static void copyFileToString (std::istream& from, std::string& to)
        {
            char buf[64];
            while (!from.eof()) {
                from.read (buf, 63);
                to.append (buf, from.gcount());
            }
        }

        static void copyStringToFile (const std::string& fromstr, const std::string& to)
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

        //! Append the file from to the filestream appendTo
        static void appendFile (const std::string& from, std::ostream& appendTo)
        {
            if (!appendTo.good()) {
                throw std::runtime_error ("Can't append to appendTo, it's not good()");
            }
            std::ifstream in;
            in.open (from.c_str(), std::ios::in);
            if (!in.is_open()) {
                throw std::runtime_error ("Tools::appendFile(): Couldn't open FROM file");
            }

            char buf[64];
            while (!in.eof() && appendTo.good()) {
                in.read (buf, 63);
                appendTo.write (buf, in.gcount());
            }

            in.close();
        }

        //! Append the content of the filestream from to the filestream appendTo
        static void appendFile (std::istream& from, std::ostream& appendTo)
        {
            if (!appendTo.good()) {
                throw std::runtime_error ("Can't append to appendTo, it's not good()");
            }

            char buf[64];
            buf[63] = '\0';
            while (!from.eof() && appendTo.good()) {
                from.read (buf, 63);
                appendTo.write (buf, from.gcount());
            }
        }

        //! Append the content of the filestream from to the file at appendTo
        static void appendFile (std::istream& from, const std::string& appendTo)
        {
            std::ofstream f;
            f.open (appendTo.c_str(), std::ios::out|std::ios::app);
            if (!f.is_open()) {
                std::stringstream ee;
                ee << "Failed to open output file '" << appendTo << "'";
                throw std::runtime_error (ee.str());
            }

            char buf[64];
            buf[63] = '\0';
            while (!from.eof() && f.good()) {
                from.read (buf, 63);
                f.write (buf, from.gcount());
            }
        }

        //! Append the file at path from to the file at path appendTo
        static void appendFile (const std::string& from, const std::string& appendTo)
        {
            std::ifstream fin;
            fin.open (from.c_str(), std::ios::in);
            if (!fin.is_open()) {
                std::stringstream ee;
                ee << "Failed to open input file '" << from << "'";
                throw std::runtime_error (ee.str());
            }

            std::ofstream f;
            f.open (appendTo.c_str(), std::ios::out|std::ios::app);
            if (!f.is_open()) {
                std::stringstream ee;
                ee << "Failed to open output file '" << appendTo << "'";
                throw std::runtime_error (ee.str());
            }

            char buf[64];
            buf[63] = '\0';
            while (!fin.eof() && f.good()) {
                fin.read (buf, 63);
                f.write (buf, fin.gcount());
            }
        }

        /*!
         * Make a copy of \param bytes bytes of the file at \param original to the file
         * \param truncated.
         */
        static void truncateFile (const std::string& original,
                                  const std::string& truncated,
                                  const unsigned int bytes)
        {
            std::ofstream out;

            out.open (truncated.c_str(), std::ios::out|std::ios::trunc);
            if (!out.is_open()) {
                std::string emsg = "Tools::copyFile(): Couldn't open TRUNCATED file '" + truncated + "'";
                throw std::runtime_error (emsg);
            }

            std::ifstream in;

            // Test that "original" is a regular file
            if (!Tools::regfileExists (original)) {
                std::stringstream ee;
                ee << "Tools::truncateFile(): ORIGINAL file '"
                   << original << "' is not a regular file";
                throw std::runtime_error (ee.str());
            }

            in.open (original.c_str(), std::ios::in);
            if (!in.is_open()) {
                throw std::runtime_error ("Tools::truncateFile(): Couldn't open ORIGINAL file");
            }

            if (!out) {
                throw std::runtime_error ("Tools::truncateFile(): Error occurred in TRUNCATED stream");
            }

            unsigned int loops(0);
            unsigned int maxLoops = bytes / 63;
            unsigned int remaining = bytes % 63;
            char buf[64];
            while (!in.eof() && loops < maxLoops) {
                in.read (buf, 63);
                // Find out how many were read
                unsigned int bytesCopied = in.gcount();
                // and write that many to the output stream
                out.write (buf, bytesCopied);
                ++loops;
            }
            // Copy remaining
            if (!in.eof()) {
                in.read (buf, remaining);
                // Find out how many were read
                unsigned int bytesCopied = in.gcount();
                if (bytesCopied != remaining) {
                    throw std::runtime_error ("copy error bytesCopied != remaining");
                }
                // and write that many to the output stream
                out.write (buf, bytesCopied);
            }

            // Make sure output buffer is flushed.
            out.flush();

            // Finally, close the input and output
            in.close();
            out.close();
        }

        /*!
         * Move a file. Throw exception on failure.
         */
        static void moveFile (const std::string& from, const std::string& to)
        {
            Tools::copyFile (from, to);
            Tools::unlinkFile (from);
        }

        /*!
         * Call unlink() on the given file path fpath. If unlinking fails, throw a
         * descriptive error based on the errno which was set on unlink's return.
         */
        static void unlinkFile (const std::string& fpath)
        {
            int rtn = unlink (fpath.c_str());
            if (rtn) {
                int theError = errno;
                std::string emsg;
                switch (theError) {
                case EPERM:
                case EACCES:
                    emsg = "Write access to '" + fpath + "' is not allowed due to permissions";
                    break;
                case EBUSY:
                    emsg = "'" + fpath + "' cannot be removed as it is in use by another process";
                    break;
                case EFAULT:
                    emsg = "'" + fpath + "' points outside your accessible address space";
                    break;
                case EIO:
                    emsg = "I/O error occurred reading '" + fpath + "'";
                    break;
                case EISDIR:
                    emsg = "'" + fpath + "' is a directory";
                    break;
                case ELOOP:
                    emsg = "Too many symlinks encountered in '" + fpath + "'";
                    break;
                case ENAMETOOLONG:
                    emsg = "'" + fpath + "' is too long a name";
                    break;
                case ENOENT:
                    emsg = "'" + fpath + "' does not exist or is a dangling symlink";
                    break;
                case ENOMEM:
                    emsg = "In sufficient kernel memory to open '" + fpath + "'";
                    break;
                case ENOTDIR:
                    emsg = "'" + fpath + "' contains a component that is not a directory";
                    break;
                case EROFS:
                    emsg = "'" + fpath + "' is on a read-only filesystem";
                    break;
                default:
                    emsg = "Unknown error unlinking file '" + fpath + "'";
                    break;
                }

                throw std::runtime_error (emsg);
            }
        }

        /*!
         * Unlink files in dirPath which are older than olerThanSeconds and which
         * contain filePart.
         */
        static void clearoutDir (const std::string& dirPath,
                                 const unsigned int olderThanSeconds = 0,
                                 const std::string& filePart = "")
        {
            std::vector<std::string> files;
            try {
                Tools::readDirectoryTree (files, dirPath, olderThanSeconds);
            } catch (const std::exception& e) {
                //DBG ("Failed to read dir tree: " << e.what());
                return;
            }
            std::vector<std::string>::iterator i = files.begin();
            while (i != files.end()) {
                std::string fpath = dirPath + "/" + *i;
                try {
                    if (filePart.empty()) {
                        Tools::unlinkFile (fpath);
                    } else {
                        // Must find filePart to unlink
                        if (i->find (filePart, 0) != std::string::npos) {
                            Tools::unlinkFile (fpath);
                        } // else do nothing
                    }

                } catch (const std::exception& e) {
                    //DBG ("Failed to unlink " << *i << ": " << e.what());
                }
                ++i;
            }
        }

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
        static void readDirectoryTree (std::vector<std::string>& vec,
                                       const std::string& baseDirPath,
                                       const std::string& subDirPath,
                                       const unsigned int olderThanSeconds = 0)
        {
            DIR* d;
            struct dirent *ep;
            size_t entry_len = 0;

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
                    buf = (struct stat*) malloc (sizeof (struct stat));
                    if (!buf) { // Malloc error.
                        throw std::runtime_error ("Failed to malloc buf; "
                                                  "could not stat link " + fileName);
                    }
                    memset (buf, 0, sizeof(struct stat));
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
                    Tools::readDirectoryTree (vec, baseDirPath,
                                              newPath.c_str(), olderThanSeconds);
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
                        memset (&buf, 0, sizeof (struct stat));

                        if (stat (fileName.c_str(), &buf)) {
                            // no file to stat
                            //DBG ("stat() error for '" << fileName << "'");
                            continue;
                        }

                        if (static_cast<unsigned int>(time(NULL)) - buf.st_mtime
                            <= olderThanSeconds) {
                            // The age of the last modification is less
                            // than olderThanSeconds, so skip
                            // (we're only returning the OLDER
                            // files)
                            //DBG ("File " << fileName << " is too new to include, continuing");
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
        static void readDirectoryTree (std::vector<std::string>& vec,
                                       const std::string& dirPath,
                                       const unsigned int olderThanSeconds = 0)
        {
            Tools::readDirectoryTree (vec, dirPath, "", olderThanSeconds);
        }

        /*!
         * Get a list of only the immediate subdirectories in dirPath.
         *
         * For example, if you have a structure like:
         *
         * file1
         * file2
         * dir1/file1
         * dir2/file1
         * dir2/file2
         * dir2/aDirectory
         *
         * The set dset would be filled only with dir2, dir1.
         */
        static void readDirectoryDirs (std::set<std::string>& dset,
                                       const std::string& dirPath)
        {
            DIR* d;
            struct dirent *ep;
            size_t entry_len = 0;

            if (!(d = opendir (dirPath.c_str()))) {
                std::string msg = "Failed to open directory " + dirPath;
                throw std::runtime_error (msg);
            }

            while ((ep = readdir (d))) {

                if (ep->d_type == DT_DIR) {

                    // Skip "." and ".." directories
                    if ( ((entry_len = std::strlen (ep->d_name)) > 0 && ep->d_name[0] == '.') &&
                         (ep->d_name[1] == '\0' || ep->d_name[1] == '.') ) {
                        continue;
                    }

                    // All other directories are added to vec
                    dset.insert (ep->d_name);
                }
            }

            (void) closedir (d);
        }

        /*!
         * Return empty subdirectories in dirPath/subDir. Recursive partner to
         * readDirectoryEmptyDirs(set<std::string>&, const std::string&).
         *
         * The base directory path baseDirPath should have NO TRAILING '/'. The
         * subDirPath should have NO INITIAL '/' character.
         */
        static void readDirectoryEmptyDirs (std::set<std::string>& dset,
                                            const std::string& baseDirPath,
                                            const std::string& subDir = "")
        {
            DIR* d;
            struct dirent *ep;
            size_t entry_len = 0;

            std::string dirPath (baseDirPath);
            if (!subDir.empty()) {
                dirPath += "/" + subDir;
            }

            if (!(d = opendir (dirPath.c_str()))) {
                std::string msg = "Failed to open directory " + dirPath;
                throw std::runtime_error (msg);
            }

            unsigned int levelDirCount = 0;
            while ((ep = readdir (d))) {

                if (ep->d_type == DT_DIR) {
                    // Skip "." and ".." directories
                    if ( ((entry_len = std::strlen (ep->d_name)) > 0 && ep->d_name[0] == '.') &&
                         (ep->d_name[1] == '\0' || ep->d_name[1] == '.') ) {
                        continue;
                    }

                    ++levelDirCount;
                    // Because we found a directory, this current
                    // directory ain't empty - recurse with a new
                    // directory in the subDir path:
                    std::string newSubDir;
                    if (subDir.empty()) {
                        newSubDir = (const char*)ep->d_name;
                    } else {
                        newSubDir = subDir + "/" + (const char*)ep->d_name;
                    }
                    Tools::readDirectoryEmptyDirs (dset, baseDirPath, newSubDir);
                }
            }

            if (levelDirCount == 0) {
                // No directories found here, check for files
                std::vector<std::string> foundfiles;
                Tools::readDirectoryTree (foundfiles, dirPath);
                if (foundfiles.empty()) {
                    dset.insert (subDir);
                } // else DBG ("NOT adding " << subDir << " as " << dirPath << " contains " << foundfiles.size() << " files");
            }

            (void) closedir (d);
        }

        /*!
         * Attempts to remove all the unused directories in a tree.
         *
         * May throw exceptions.
         */
        static void removeUnusedDirs (std::set<std::string>& dset, const std::string& dirPath)
        {
            std::set<std::string> onepass;
            do {
                onepass.clear();
                Tools::removeEmptySubDirs (onepass, dirPath);
                dset.insert (onepass.begin(), onepass.end());
            } while (!onepass.empty());
        }

        /*!
         * Recursively remove all empty directories in baseDirPath(/subDir)
         *
         * Removed directories are inserted into dset, so you know what you got rid of.
         *
         * This won't remove baseDirPath itself, even if that is empty.
         *
         * This does one "pass" - it removes all empty end-of-directories in a tree. If
         * you want to remove all "unused" directories in a tree, use removeUnusedDirs()
         */
        static void removeEmptySubDirs (std::set<std::string>& dset,
                                        const std::string& baseDirPath,
                                        const std::string& subDir = "")
        {
            DIR* d;
            struct dirent *ep;
            size_t entry_len = 0;

            std::string dirPath (baseDirPath);
            if (!subDir.empty()) {
                dirPath += "/" + subDir;
            }

            if (!(d = opendir (dirPath.c_str()))) {
                std::string msg = "Failed to open directory " + dirPath;
                throw std::runtime_error (msg);
            }

            unsigned int levelDirCount = 0;
            while ((ep = readdir (d))) {

                if (ep->d_type == DT_DIR) {
                    // Skip "." and ".." directories
                    if ( ((entry_len = std::strlen (ep->d_name)) > 0 && ep->d_name[0] == '.') &&
                         (ep->d_name[1] == '\0' || ep->d_name[1] == '.') ) {
                        continue;
                    }

                    ++levelDirCount;
                    // Because we found a directory, this current
                    // directory ain't empty - recurse with a new
                    // directory in the subDir path:
                    std::string newSubDir;
                    if (subDir.empty()) {
                        newSubDir = (const char*)ep->d_name;
                    } else {
                        newSubDir = subDir + "/" + (const char*)ep->d_name;
                    }
                    Tools::removeEmptySubDirs (dset, baseDirPath, newSubDir);
                }
            }

            if (levelDirCount == 0) {
                // No directories found here, check for files
                std::vector<std::string> foundfiles;
                Tools::readDirectoryTree (foundfiles, dirPath);

                if (foundfiles.empty()) {
                    if (!subDir.empty()) {
                        Tools::removeDir (dirPath);
                        dset.insert (subDir);
                    }
                } // else "NOT Removing " << dirPath << " which contains " << foundfiles.size() << " files";
            }

            (void) closedir (d);
        }

        /*!
         * Return a datestamp - st_mtime; the file modification time for the given file.
         */
        static std::string fileModDatestamp (const std::string& filename)
        {
            struct stat * buf = NULL;
            std::stringstream datestamp;

            buf = (struct stat*) malloc (sizeof (struct stat));
            if (!buf) { // Malloc error.
                std::cout << "malloc error\n";
            }
            memset (buf, 0, sizeof(struct stat));
            if (stat (filename.c_str(), buf)) {
                datestamp << 0;
            } else {
                datestamp << buf->st_mtime;
            }
            if (buf) { free (buf); }

            std::string dstr = datestamp.str();
            return dstr;
        }

        /*!
         * Check whether the specified files differ.
         */
        static bool filesDiffer (const std::string& first, const std::string& second)
        {
            if (!(Tools::regfileExists (first) && Tools::regfileExists (second))) {
                throw std::runtime_error ("Error: expecting two regular files");
            }
            std::string diffcmd = "diff " + first + " " + second + " >/dev/null 2>&1";
            // diff returns zero if files are identical, non-zero if files differ.
            return (system (diffcmd.c_str()) != 0);
        }

        /*!
         * Given a path like /path/to/file in str, remove all the preceding /path/to/
         * stuff to leave just the filename.
         */
        static void stripUnixPath (std::string& unixPath)
        {
            std::string::size_type pos (unixPath.find_last_of ('/'));
            if (pos != std::string::npos) { unixPath = unixPath.substr (++pos); }
        }

        /*!
         * Given a path like /path/to/file in str, remove the final filename, leaving
         * just the path, "/path/to".
         */
        static void stripUnixFile (std::string& unixPath)
        {
            std::string::size_type pos (unixPath.find_last_of ('/'));
            if (pos != std::string::npos) { unixPath = unixPath.substr (0, pos); }
        }

        /*!
         * Given a path like /path/to/file.ext or just file.ext in str, remove the file
         * suffix.
         */
        static void stripFileSuffix (std::string& unixPath)
        {
            std::string::size_type pos (unixPath.rfind('.'));
            if (pos != std::string::npos) {
                // We have a '.' character
                std::string tmp (unixPath.substr (0, pos));
                if (!tmp.empty()) {
                    unixPath = tmp;
                }
            }
        }

        /*
         * Date and time utility functions
         */

        /*!
         * Return the current year.
         */
        static unsigned int yearNow (void)
        {
            time_t curtime; // Current time
            struct tm * t;
            curtime = time(NULL);
            t = localtime (&curtime);
            unsigned int theYear = static_cast<unsigned int>(t->tm_year+1900);
            return theYear;
        }

        /*!
         * Return the current month (1==Jan, 12==Dec).
         */
        static unsigned int monthNow (void)
        {
            time_t curtime; // Current time
            struct tm * t;
            curtime = time(NULL);
            t = localtime (&curtime);
            unsigned int theMonth = static_cast<unsigned int>(t->tm_mon+1);
            return theMonth;
        }

        /*!
         * Return the current 'day of month' (the tm_mday field of a struct tm).
         */
        static unsigned int dateNow (void)
        {
            time_t curtime; // Current time
            struct tm * t;
            curtime = time(NULL);
            t = localtime (&curtime);
            unsigned int theDate = static_cast<unsigned int>(t->tm_mday);
            return theDate;
        }

        /*!
         * Given the month as an int, where 1==Jan, 12==Dec,
         * return the month as a string. If shortFormat is true,
         * return "Jan", "Dec", etc., otherwise "January",
         * "December" etc.
         */
        static std::string monthStr (const int month, const bool shortFormat=false)
        {
            std::string rtn("");

            if (shortFormat == true) {
                switch (month) {
                case 1:
                    rtn = "Jan";
                    break;
                case 2:
                    rtn = "Feb";
                    break;
                case 3:
                    rtn = "Mar";
                    break;
                case 4:
                    rtn = "Apr";
                    break;
                case 5:
                    rtn = "May";
                    break;
                case 6:
                    rtn = "Jun";
                    break;
                case 7:
                    rtn = "Jul";
                    break;
                case 8:
                    rtn = "Aug";
                    break;
                case 9:
                    rtn = "Sep";
                    break;
                case 10:
                    rtn = "Oct";
                    break;
                case 11:
                    rtn = "Nov";
                    break;
                case 12:
                    rtn = "Dec";
                    break;
                default:
                    rtn = "unk";
                    break;
                }
            } else {
                switch (month) {
                case 1:
                    rtn = "January";
                    break;
                case 2:
                    rtn = "February";
                    break;
                case 3:
                    rtn = "March";
                    break;
                case 4:
                    rtn = "April";
                    break;
                case 5:
                    rtn = "May";
                    break;
                case 6:
                    rtn = "June";
                    break;
                case 7:
                    rtn = "July";
                    break;
                case 8:
                    rtn = "August";
                    break;
                case 9:
                    rtn = "September";
                    break;
                case 10:
                    rtn = "October";
                    break;
                case 11:
                    rtn = "November";
                    break;
                case 12:
                    rtn = "December";
                    break;
                default:
                    rtn = "unknown";
                    break;
                }
            }

            return rtn;
        }

        /*!
         * Give the number n, return the suitable (english)
         * suffix. E.g. "st" for 1, "nd" for 22 etc.
         */
        static std::string suffix (const int n)
        {
            std::string suf("th"); // Most numbers end in "th" (in English)
            int leastSig = n%10;     // Right most, least significant numeral
            int leastSigTwo = n%100; // Right most pair of numerals

            switch (leastSig) {
            case 1:
                if (leastSigTwo != 11) {
                    suf = "st";
                }
                break;
            case 2:
                if (leastSigTwo != 12) {
                    suf = "nd";
                }
                break;
            case 3:
                if (leastSigTwo != 13) {
                    suf = "rd";
                }
                break;
            default:
                break;
            }

            return suf;
        }

        /*!
         * Convert a date of form 2009-02-16 to the unix epoch
         * number. The fifth character of the string is
         * examined, and if it is not a numeral, it is used as
         * the separator. If the fifth character IS a numeral,
         * then the date format is read in as YYYYMMDD.
         */
        static time_t dateToNum (const std::string& dateStr)
        {
            char separator = '\0';

            if (dateStr.empty()) { return -2; }

            if (dateStr.size() < 8) { return -3; }

            bool bigEndian (true);

            if (dateStr[2] < '0' || dateStr[2] > '9') {
                separator = dateStr[2];
                bigEndian = false;
            } else if (dateStr[4] < '0' || dateStr[4] > '9') {
                separator = dateStr[4];
            }

            if (separator != '\0' && dateStr.size() < 10) { return -4; }

            std::string year;
            std::string month;
            std::string day;
            unsigned int yearN=0, monthN=0, dayN=0;

            if (bigEndian) {
                year = dateStr.substr (0,4);

                if (separator == '\0') {
                    month = dateStr.substr (4,2);
                    day = dateStr.substr (6,2);
                } else {
                    month = dateStr.substr (5,2);
                    day = dateStr.substr (8,2);
                }

            } else {
                day = dateStr.substr (0,2);

                if (separator == '\0') {
                    month = dateStr.substr (2,2);
                    year = dateStr.substr (4,4);
                } else {
                    month = dateStr.substr (3,2);
                    year = dateStr.substr (6,4);
                }
            }

            std::stringstream yearss, monthss, dayss;
            yearss << year;
            yearss.width(4);
            yearss.fill ('0');
            yearss >> yearN;

            monthss << month;
            monthss.width(2);
            monthss.fill ('0');
            monthss >> monthN;

            dayss << day;
            dayss.width(2);
            dayss.fill ('0');
            dayss >> dayN;

            struct tm * t;
            t = (struct tm*) malloc (sizeof (struct tm));
            t->tm_year = yearN-1900;
            t->tm_mon = monthN-1;
            t->tm_mday = dayN;
            t->tm_hour = 0;
            t->tm_min = 0;
            t->tm_sec = 0;
            t->tm_isdst = -1;
            time_t rtnTime = mktime (t);
            if (rtnTime == -1) { throw std::runtime_error ("mktime() returned -1"); }
            free (t);

            return rtnTime;
        }

        /*!
         * Convert a date/time of form 2009-02-16 14:34:34 to
         * the unix epoch number. The fifth character of the
         * string is examined, and if it is not a numeral, it
         * is used as the date separator. If the fifth
         * character IS a numeral, then the date format is
         * read in as YYYYMMDD.
         *
         * The 3rd char after the space is read in and used as
         * time separator
         */
        static time_t dateTimeToNum (const std::string& dateTimeStr)
        {
            char dateSeparator = '\0';
            char timeSeparator = '\0';

            if (dateTimeStr.empty()) { return -2; }

            if (dateTimeStr.size() < 8) { return -3; }

            if (dateTimeStr[4] < '0' || dateTimeStr[4] > '9') {
                dateSeparator = dateTimeStr[4];
                if (dateTimeStr.size() < 10) { return -4; }
            }

            std::string year;
            std::string month;
            std::string day;
            unsigned int yearN=0, monthN=0, dayN=0;

            year = dateTimeStr.substr (0,4);

            if (dateSeparator == '\0') {
                month = dateTimeStr.substr (4,2);
                day = dateTimeStr.substr (6,2);
            } else {
                month = dateTimeStr.substr (5,2);
                day = dateTimeStr.substr (8,2);
            }

            std::stringstream yearss, monthss, dayss;
            yearss << year;
            yearss.width(4);
            yearss.fill ('0');
            yearss >> yearN;

            monthss << month;
            monthss.width(2);
            monthss.fill ('0');
            monthss >> monthN;

            dayss << day;
            dayss.width(2);
            dayss.fill ('0');
            dayss >> dayN;

            std::string hour;
            std::string min;
            std::string sec;
            unsigned int hourN=0, minN=0, secN=0;

            std::string::size_type spacePos = dateTimeStr.find (" ", 0);
            if (spacePos != std::string::npos) {
                if (dateTimeStr[spacePos+3] < '0' || dateTimeStr[spacePos+3] > '9') {
                    timeSeparator = dateTimeStr[spacePos+3];
                }

                hour = dateTimeStr.substr (spacePos+1, 2);

                if (timeSeparator != '\0') {
                    min = dateTimeStr.substr (spacePos+4, 2);
                    sec = dateTimeStr.substr (spacePos+7, 2);
                } else {
                    min = dateTimeStr.substr (spacePos+3, 2);
                    sec = dateTimeStr.substr (spacePos+5, 2);
                }

                std::stringstream hourss, minss, secss;
                hourss << hour;
                hourss.width(2);
                hourss.fill ('0');
                hourss >> hourN;

                minss << min;
                minss.width(2);
                minss.fill ('0');
                minss >> minN;

                secss << sec;
                secss.width(2);
                secss.fill ('0');
                secss >> secN;
            }

            struct tm * t;
            t = (struct tm*) malloc (sizeof (struct tm));
            t->tm_year = yearN-1900;
            t->tm_mon = monthN-1;
            t->tm_mday = dayN;
            t->tm_hour = hourN;
            t->tm_min = minN;
            t->tm_sec = secN;
            t->tm_isdst = -1;
            time_t rtnTime = mktime (t);
            if (rtnTime == -1) { throw std::runtime_error ("mktime() returned -1"); }
            free (t);

            return rtnTime;
        }

        /*!
         * Convert a unix epoch number to a date/time of form
         * 2009-02-16 02:03:01, using dateSeparator to delimit
         * the date and timeSeparator to delimit the time.
         */
        static std::string numToDateTime (const time_t epochSeconds,
                                          const char dateSeparator = '\0',
                                          const char timeSeparator = '\0')
        {
            if (epochSeconds == 0) {
                return "unknown";
            }

            struct tm * t;
            time_t es = epochSeconds;
            t = (struct tm*) malloc (sizeof (struct tm));
            t = localtime_r (&es, t);
            int theDay = t->tm_mday;
            int theMonth = t->tm_mon+1;
            int theYear = t->tm_year+1900;
            int theHour = t->tm_hour;
            int theMin = t->tm_min;
            int theSec = t->tm_sec;
            free (t);

            std::stringstream rtn;

            // Date part
            rtn.width(4);
            rtn.fill('0');
            rtn << theYear;
            if (dateSeparator != '\0') { rtn << dateSeparator; }
            rtn.width(2);
            rtn.fill('0');
            rtn << theMonth;
            if (dateSeparator != '\0') { rtn << dateSeparator; }
            rtn.width(2);
            rtn.fill('0');
            rtn << theDay;

            rtn << " ";

            // Time part
            rtn.width(2);
            rtn.fill('0');
            rtn << theHour;
            if (timeSeparator != '\0') { rtn << timeSeparator; }
            rtn.width(2);
            rtn.fill('0');
            rtn << theMin;
            if (timeSeparator != '\0') { rtn << timeSeparator; }
            rtn.width(2);
            rtn.fill('0');
            rtn << theSec;

            return rtn.str();
        }

        /*!
         * Convert a unix epoch number to a date of form
         * 2009-02-16, using separator to delimit the date.
         */
        static std::string numToDate (const time_t epochSeconds, const char separator = '\0')
        {
            struct tm * t;
            time_t es = epochSeconds;
            t = (struct tm*) malloc (sizeof (struct tm));
            t = localtime_r (&es, t);
            int theDay = t->tm_mday;
            int theMonth = t->tm_mon+1;
            int theYear = t->tm_year+1900;
            free (t);

            std::stringstream rtn;
            if (separator == '\0') {
                rtn.width(4);
                rtn.fill('0');
                rtn << theYear;
                rtn.width(2);
                rtn.fill('0');
                rtn << theMonth;
                rtn.width(2);
                rtn.fill('0');
                rtn << theDay;
            } else {
                rtn.width(4);
                rtn.fill('0');
                rtn << theYear << separator;
                rtn.width(2);
                rtn.fill('0');
                rtn << theMonth << separator;
                rtn.width(2);
                rtn.fill('0');
                rtn << theDay;
            }

            return rtn.str();
        }

        /*!
         * Return the current time in neat string format.
         */
        static std::string timeNow (void)
        {
            time_t curtime;
            struct tm *loctime;
            curtime = time (NULL);
            loctime = localtime (&curtime);
            return asctime(loctime);
        }

        /*!
         * This splits up a "search style" string into tokens.
         *
         * \param s The string to split up
         *
         * \param separatorChars The chars used only to
         * separate tokens (" ,;")
         *
         * \param enclosureChars The characters used to
         * enclose a multi-word token ("\"\'")
         *
         * \param the escape character. If not set to \0, then
         * this is the character used to escape the enclosure
         * chars.
         */
        template <typename ST>
        static std::vector<ST> splitStringWithEncs (const ST& s,
                                                    const ST& separatorChars = ST(";, "),
                                                    const ST& enclosureChars = ST("\"'"),
                                                    const typename ST::value_type& escapeChar = typename ST::value_type(0));

    }; // class Tools

} // morph namespace

/*!
 * Templated random number function.
 *
 * Don't use this! Use c++-11 random number generators, which include a 64 bit Mersenne
 * Twister algorithm as an option!
 */
template <typename F>
F
morph::Tools::randF (void)
{
    return static_cast<F>(rand()) / static_cast<F>(RAND_MAX);
}

/*!
 * Templated function splitStringWithEncs implementation.
 */
template <typename ST>
std::vector<ST>
morph::Tools::splitStringWithEncs (const ST& s,
                                   const ST& separatorChars,
                                   const ST& enclosureChars,
                                   const typename ST::value_type& escapeChar) // or '\0'
{
    // Run through the string, searching for separator and
    // enclosure chars and finding tokens based on those.

    std::vector<ST> theVec;
    ST entry("");
    typename ST::size_type a=0, b=0, c=0;
    ST sepsAndEncsAndEsc = separatorChars + enclosureChars;
    sepsAndEncsAndEsc += escapeChar;

    typename ST::size_type sz = s.size();
    while (a < sz) {

        // If true, then the thing we're searching for is an enclosure
        // char, otherwise, it's a separator char.
        bool nextIsEnc(false);
        typename ST::value_type currentEncChar = 0;

        if (a == 0) { // First field.
            if (escapeChar && s[a] == escapeChar) {
                // First char is an escape char - skip this and next
                ++a; ++a;
                continue;
            } else if ((enclosureChars.find_first_of (static_cast<typename ST::value_type>(s[a]), 0)) != ST::npos) {
                // First char is an enclosure char.
                nextIsEnc = true;
                currentEncChar = s[a];
                ++a; // Skip the enclosure char
            } else if ((separatorChars.find_first_of (static_cast<typename ST::value_type>(s[a]), 0)) != ST::npos) {
                // First char is a ',' This special case means that we insert an entry for the current ',' and step past it.
                //DBG2 ("First char special case, insert entry.");
                theVec.push_back ("");
                ++a;

            } // else first char is a normal char or a separator.

        } else { // Not first field

            if ((a = s.find_first_of (sepsAndEncsAndEsc, a)) == ST::npos) {
                //DBG ("No enclosure, separator or escape chars in string");
                theVec.push_back (s);
                return theVec;
            }

            else if (escapeChar && s[a] == escapeChar) {
                // it's an escape char - skip this and next
                ++a; ++a;
                continue;
            } else if ((enclosureChars.find_first_of (static_cast<typename ST::value_type>(s[a]), 0)) != ST::npos) {
                // it's an enclosure char.
                nextIsEnc = true;
                currentEncChar = s[a];
                ++a; // Skip the enclosure char
            } else if ((separatorChars.find_first_of (static_cast<typename ST::value_type>(s[a]), 0)) != ST::npos) {
                // It's a field separator
                //DBG2 ("Field separator found at position " << a << " skipping...");
                ++a; // Skip the separator
                if (a >= sz) {
                    // Special case - a trailing separator character - add an empty
                    // value to the return vector of tokens.
                    //DBG2 ("Adding trailing empty field due to trailing separator");
                    theVec.push_back ("");
                } else {
                    // a < sz, so now check if we've hit an escape char
                    if ((enclosureChars.find_first_of (static_cast<typename ST::value_type>(s[a]), 0)) != ST::npos) {
                        // Enclosure char following sep char
                        nextIsEnc = true;
                        currentEncChar = s[a];
                        ++a; // Skip the enclosure char
                    }
                }
            } else {
                //throw std::runtime_error ("Tools::splitStringWithEncs: Unexpected case");
            }
        }

        // Check we didn't over-run
        if (a >= sz) { break; }

        // Now get the token
        typename ST::size_type range = ST::npos;
        if (nextIsEnc) {
            //DBG2 ("Searching for next instances of enc chars: >" << enclosureChars << "< ");
            c = a;
            while ((b = s.find_first_of (currentEncChar, c)) != ST::npos) {
                // FIXME: Check we didn't find an escaped enclosureChar.
                if (escapeChar) {
                    c = b; --c;
                    if (s[c] == escapeChar) {
                        // Skip b which is an escaped enclosure char
                        c = b; ++c;
                        continue;
                    }
                }
                range = b - a;
                break;
            }
        } else {
            //DBG2 ("Searching for next instances of sep chars: >" << separatorChars << "< from position " << a);
            if ((b = s.find_first_of (separatorChars, a)) != ST::npos) {
                // Check it wasn't an escaped separator:
                if (escapeChar) {
                    c = b; --c;
                    if (c >= 0 && c != ST::npos && c < sz && s[c] == escapeChar) {
                        //DBG2 ("Found escaped separator character");
                        c = b; ++c;
                        continue;
                    }
                }
                range = b - a;
                //DBG2 ("On finding a separator char at position " << b
                //<< " (starting from position " << a << "), have set range to " << range);
            }
        }

        entry = s.substr (a, range);
        Tools::stripChars (entry, escapeChar);
        //DBG2 ("Adding entry '" << entry << "' to vector");
        theVec.push_back (entry);

        if (range != ST::npos) { // end of field was not end of string
            if (nextIsEnc) {
                //DBG2 ("Adding " << range + 1 << " to a (" << a << ") as nextIsEnc...");
                a += range + 1; // +1 to take us past the closing enclosure.
            } else {
                //DBG2 ("Adding " << range << " to a (" << a << ")...");
                a += range; // in new scheme, we want to find the separator, so this
                // places us ON the separator.
            }
        } else {
            a = range;
        }
        //DBG2 ("...a is now " << a);
    }

    return theVec;
}
