/*
 * Utility functions
 */

#ifndef _TOOLS_H_
#define _TOOLS_H_

#include <vector>
#include <array>
#include <list>
#include <set>
#include <map>
#include <string>

using std::array;
using std::vector;
using std::list;
using std::set;
using std::map;
using std::string;
using std::istream;
using std::ostream;

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
    /*!
     * Allows the use of transform and tolower() on strings with
     * GNU compiler
     */
    class to_lower
    {
    public:
        /*!
         * Apply lower case operation to the char c.
         */
        char operator() (const char c) const {
            return tolower(c);
        }
    };

    /*!
     * Allows the use of transform and toupper() on strings with
     * GNU compiler
     */
    class to_upper
    {
    public:
        /*!
         * Apply upper case operation to the char c.
         */
        char operator() (const char c) const {
            return toupper(c);
        }
    };

    class Tools
    {
    public:
        static vector<double> getJetColor (double gray);

        /*!
         * Floating point array version of getJetColor()
         */
        static array<float, 3> getJetColorF (double gray);

        static vector<double> getGrayScaleColor (double gray);

        static array<float,3> HSVtoRGB (double, double, double);

        /*!
         * For mixing up bits of three args; used to generate a good
         * random seed using time() getpid() and clock().
         */
        static unsigned int mix (unsigned int a, unsigned int b, unsigned int c);

        /*!
         * Using clock(), time() and getpid() along with the mix
         * utility function, generate a decently random seed for
         * seeding your RNG.
         */
        static unsigned int randomSeed (void);

        /*!
         * Return a random double precision number in the range [0,1], sampled
         * from a uniform distribution.
         */
        static double randDouble (void);

        /*!
         * Return a random single precision number in the range [0,1],
         * sampled from a uniform distribution.
         */
        static float randSingle (void);

        /*!
         * Return a random floating point number with type F, where F
         * is expected to be either float or double.
         */
        template<typename F>
        static F randF (void);

        static double normalDistributionValue (void);

        static double wrapAngle(double);
        static vector <vector <float> > rotateCloud (vector <vector <float> >, double, double, double);
#ifdef SPHERE_ATTEMPT
        static vector <vector <float> > sphere (int, double);
#endif
        static vector<vector<int> > graph(vector<vector<int> >);
        /*!
         * return indices of descending value in unsorted
         */
        static vector<int> sort (vector<double> unsorted);

        /*!
         * This removes all carriage return characters ('\\r'
         * 0xd) from input. It will convert all DOS style
         * newlines, which consist of '\\r''\\n' character duplets,
         * to UNIX style newlines ('\\n'). A side effect is that
         * any lone '\\r' characters which are present will be
         * removed, whether or not they are followed by a '\\n'
         * character.
         */
        static int ensureUnixNewlines (string& input);

        /*!
         * Get the working directory
         */
        static string getPwd (void);

        /*!
         * If the last character of input is a carriage return
         * ('\\r' 0xd), then it is erased from input.
         */
        static int stripTrailingCarriageReturn (string& input);

        /*!
         * Erase trailing spaces from input. Return the
         * number of spaces removed.
         */
        //@{
        static int stripTrailingSpaces (string& input);
        //@}

        /*!
         * Erase trailing chars c from input. Return the
         * number of chars removed.
         */
        //@{
        static int stripTrailingChars (string& input, const char c = ' ');
        //@}

        /*!
         * Erase trailing whitespace from input. Return the
         * number of whitespace characters removed.
         */
        static int stripTrailingWhitespace (string& input);

        /*!
         * Erase leading spaces from input. Return the
         * number of spaces removed.
         */
        //@{
        static int stripLeadingSpaces (string& input);
        //@}

        /*!
         * Erase any leading character c from input. Return the
         * number of chars removed.
         */
        //@{
        static int stripLeadingChars (string& input, const char c = ' ');
        //@}

        /*!
         * Erase leading whitespace from input. Return the
         * number of whitespace characters removed.
         */
        static int stripLeadingWhitespace (string& input);

        /*!
         * Erase leading and trailing whitespace from
         * input. Return the number of whitespace characters
         * removed.
         */
        static int stripWhitespace (string& input);

        /*!
         * Return true if input contains only space, tab, newline
         * chars.
         */
        static bool containsOnlyWhitespace (string& input);

        /*!
         * Strip any occurrences of the characters in charList
         * from input.
         */
        //@{
        static int stripChars (string& input, const string& charList);
        static int stripChars (string& input, const char charList);
        //@}

        /*!
         * Convert any C-style hex character sequence into its
         * corresponding character.
         *
         * E.g. "\x41" becomes "A" "\x1b" becomes an escape
         * char, etc.
         *
         * \return The number of hex sequences replaced in
         * \param input.
         */
        static int convertCHexCharSequences (string& input);

        /*!
         * Do a search and replace, search for searchTerm,
         * replacing with replaceTerm. if replaceAll is true,
         * replace all occurrences of searchTerm, otherwise
         * just replace the first occurrence of searchTerm
         * with replaceTerm.
         *
         * \return the number of terms replaced.
         */
        static int searchReplace (const string& searchTerm,
                                  const string& replaceTerm,
                                  string& data,
                                  const bool replaceAll = true);

        /*!
         * Return the number of instances of the character c
         * in line.
         */
        static unsigned int countChars (const string& line, const char c);

        /*!
         * Take the string str and condition it, so that it
         * makes a valid XML tag, by replacing disallowed
         * characters with '_' and making sure it doesn't
         * start with a numeral.
         */
        static void conditionAsXmlTag (string& str);

        /*!
         * split csv into a vector
         */
        static vector<string> csvToVector (const string& csvList,
                                           const char separator = ',',
                                           const bool ignoreTrailingEmptyVal = true);

        /*!
         * split csv into a list
         */
        static list<string> csvToList (const string& csvList,
                                       const char separator = ',');
        /*!
         * split csv into a set
         */
        static set<string> csvToSet (const string& csvList,
                                     const char separator = ',');

        /*!
         * Output a vector of strings as a csv string.
         */
        static string vectorToCsv (const vector<string>& vecList,
                                   const char separator = ',');

        /*!
         * Output a list of strings as a csv string.
         */
        static string listToCsv (const list<string>& listList,
                                 const char separator = ',');

        /*!
         * Output a set of strings as a csv string.
         */
        static string setToCsv (const set<string>& listList,
                                const char separator = ',');

        /*!
         * Split a comma-separated key/value pair list into a map.
         */
        static map<string, string> csvToMap (const string& csvList,
                                             const char relationship = '=',
                                             const char separator = ',');
        /*!
         * Split a string of values into a vector using the
         * separator string (not char) passed in as
         * "separator". If ignoreTrailingEmptyVal is true,
         * then a trailing separator with nothing after it
         * will NOT cause an additional empty value in the
         * returned vector.
         *
         * Similar to util::splitString but
         * FASTER. PREFER THIS OVER splitString.
         */
        static vector<string> stringToVector (const string& s, const string& separator,
                                              const bool ignoreTrailingEmptyVal = true);

        /*!
         * File and directory access methods
         */
        //@{

        /*!
         * Stat a file, return true if the file exists and is
         * any kind of file except a directory.
         */
        static bool fileExists (const string& path);

        /*!
         * Stat a file, return true if the file exists and is
         * a regular file.  If file is a hanging symlink,
         * fileExists returns false, if file is a symlink
         * pointing to a regular file, fileExists returns
         * true.
         */
        static bool regfileExists (const string& path);

        /*!
         * Like regfileExists, but also checks if the file has
         * the "executable by user" bit set (chmod u+x).
         */
        static bool userExefileExists (const string& path);

        /*!
         * Like regfileExists, but for block devices
         */
        static bool blockdevExists (const string& path);

        /*!
         * Like regfileExists, but for sockets
         */
        static bool socketExists (const string& path);

        /*!
         * Like regfileExists, but for fifos
         */
        static bool fifoExists (const string& path);

        /*!
         * Like regfileExists, but for char devices
         */
        static bool chardevExists (const string& path);

        /*!
         * Like lnkExists, but for char devices
         */
        static bool linkExists (const string& path);

        /*!
         * Stat a directory, return true if the directory
         * exists.
         */
        static bool dirExists (const string& path);

        /*!
         * Create the directory and any parent directories
         * which need to be created.
         *
         * Makes use of mkdir() and acts like the system
         * command mkdir -p path.
         *
         * If uid/gid is set to >-1, then chown each
         * directory. This means that ownership is set for the
         * directories in the path even if the directories do
         * not need to be created.
         *
         * \param path The path (relative or absolute) to the
         * directory which should be created.
         *
         * \param mode the permissions mode which should be
         * set on the directory. This is applied even if the
         * directory was not created.
         *
         * \param uid The user id to apply to the
         * directory. This is applied even if the directory
         * was not created. This is NOT applied if it is set
         * to -1.
         *
         * \param gid The group id to apply to the
         * directory. This is applied even if the directory
         * was not created. This is NOT applied if it is set
         * to -1.
         */
        static void createDir (const string& path,
                               const mode_t mode = 0775,
                               const int uid = -1, const int gid = -1);

        /*!
         * Attempt to rmdir path.
         */
        static void removeDir (const string& path);

        /*!
         * Set the permissions for the provided file
         */
        static void setPermissions (const string& filepath,
                                    const mode_t mode);

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
        static bool checkAccess (const string& filepath,
                                 const string& accessType);

        /*!
         * Set the ownership for the provided file
         */
        static void setOwnership (const string& filepath,
                                  const int uid = -1,
                                  const int gid = -1);

        /*!
         * Touch the file.
         */
        static void touchFile (const string& path);

        /*!
         * Copy a file. If from/to is a string or a char*,
         * then these are the filepaths. Some versions allow
         * you to copy the file contents into an output
         * stream. Throw exception on failure.
         *
         * The "from" file is expected to be a regular file -
         * an exception will be thrown if this is not the
         * case.
         */
        //@{
        static void copyFile (const string& from, const string& to);
        static void copyFile (const string& from, ostream& to);
        static void copyFile (FILE* from, const string& to);
        static void copyFile (istream& from, const string& to);
        static void copyFile (const string& from, FILE* to);
        //@}

        /*!
         * Copy from one file pointer to another. Both are
         * expected to be open, neither is closed after the
         * copy.
         */
        static void copyFile (FILE* from, FILE* to);

        /*!
         * Copy a file from an input stream into a string.
         */
        static void copyFileToString (istream& from, string& to);

        /*!
         * Append the file from to the filestream appendTo
         */
        //@{
        static void appendFile (const string& from, ostream& appendTo);
        static void appendFile (istream& from, ostream& appendTo);
        static void appendFile (istream& from, const string& appendTo);
        static void appendFile (const string& from, const string& appendTo);
        //@}

        /*!
         * Make a copy of \param bytes bytes of the file at
         * \param original to the file \param truncated.
         */
        static void truncateFile (const string& original,
                                  const string& truncated,
                                  const unsigned int bytes);

        /*!
         * Move a file. Throw exception on failure.
         */
        static void moveFile (const string& from, const string& to);

        /*!
         * Call unlink() on the given file path fpath. If
         * unlinking fails, throw a descriptive error based on
         * the errno which was set on unlink's return.
         */
        static void unlinkFile (const string& fpath);

        /*!
         * Unlink files in dirPath which are older than
         * olerThanSeconds and which contain filePart.
         */
        static void clearoutDir (const string& dirPath,
                                 const unsigned int olderThanSeconds = 0,
                                 const string& filePart = "");

        /*!
         * This reads the contents of a directory tree, making
         * up a list of the contents in the vector vec. If the
         * directory tree has subdirectories, these are
         * reflected in the vector entries. So, a directory
         * structure might lead to the following entries in vec:
         *
         * file2
         * file1
         * dir2/file2
         * dir2/file1
         * dir1/file1
         *
         * Note that the order of the files is REVERSED from
         * what you might expect. This is the way that
         * readdir() seems to work. If it's important to
         * iterate through the vector<string>& vec, then use a
         * reverse_iterator.
         *
         * The base directory path baseDirPath should have NO
         * TRAILING '/'. The subDirPath should have NO INITIAL
         * '/' character.
         *
         * The subDirPath argument is present because this is
         * a recursive function.
         *
         * If olderThanSeconds is passed in with a non-zero
         * value, then only files older than olderThanSeconds
         * will be returned.
         */
        static void readDirectoryTree (vector<string>& vec,
                                       const string& baseDirPath,
                                       const string& subDirPath,
                                       const unsigned int olderThanSeconds = 0);

        /*!
         * A simple wrapper around the more complex version,
         * for the user to call.
         *
         * If olderThanSeconds is passed in with a non-zero
         * value, then only files older than olderThanSeconds
         * will be returned.
         */
        static void readDirectoryTree (vector<string>& vec,
                                       const string& dirPath,
                                       const unsigned int olderThanSeconds = 0);

        /*!
         * Get a list of only the immediate subdirectories in
         * dirPath.
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
        static void readDirectoryDirs (set<string>& dset,
                                       const string& dirPath);

        /*!
         * Return empty subdirectories in
         * dirPath/subDir. Recursive partner to
         * readDirectoryEmptyDirs(set<string>&, const string&).
         *
         * The base directory path baseDirPath should have NO
         * TRAILING '/'. The subDirPath should have NO INITIAL
         * '/' character.
         */
        static void readDirectoryEmptyDirs (set<string>& dset,
                                            const string& baseDirPath,
                                            const string& subDir = "");

        /*!
         * Attempts to remove all the unused directories in a tree.
         *
         * May throw exceptions.
         */
        static void removeUnusedDirs (set<string>& dset,
                                      const string& dirPath);

        /*!
         * Recursively remove all empty directories in baseDirPath(/subDir)
         *
         * Removed directories are inserted into dset, so you
         * know what you got rid of.
         *
         * This won't remove baseDirPath itself, even if that is empty.
         *
         * This does one "pass" - it removes all empty
         * end-of-directories in a tree. If you want to remove
         * all "unused" directories in a tree, use removeUnusedDirs()
         */
        static void removeEmptySubDirs (set<string>& dset,
                                        const string& baseDirPath,
                                        const string& subDir = "");

        /*!
         * Return a datestamp - st_mtime; the file
         * modification time for the given file.
         */
        static string fileModDatestamp (const string& filename);

        /*!
         * Check whether the specified files differ.
         */
        static bool filesDiffer (const string& first, const string& second);
        //@}

        /*!
         * Date and time utility functions
         */
        //@{
        /*!
         * Return the current year.
         */
        static unsigned int yearNow (void);

        /*!
         * Return the current month (1==Jan, 12==Dec).
         */
        static unsigned int monthNow (void);

        /*!
         * Return the current date.
         */
        static unsigned int dateNow (void);

        /*!
         * Given the month as an int, where 1==Jan, 12==Dec,
         * return the month as a string. If shortFormat is true,
         * return "Jan", "Dec", etc., otherwise "January",
         * "December" etc.
         */
        static std::string monthStr (const int month, const bool shortFormat=false);

        /*!
         * Give the number n, return the suitable (english)
         * suffix. E.g. "st" for 1, "nd" for 22 etc.
         */
        static std::string suffix (const int n);

        /*!
         * Convert a date of form 2009-02-16 to the unix epoch
         * number. The fifth character of the string is
         * examined, and if it is not a numeral, it is used as
         * the separator. If the fifth character IS a numeral,
         * then the date format is read in as YYYYMMDD.
         */
        static time_t dateToNum (const std::string& dateStr);

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
        static time_t dateTimeToNum (const std::string& dateTimeStr);

        /*!
         * Convert a unix epoch number to a date/time of form
         * 2009-02-16 02:03:01, using dateSeparator to delimit
         * the date and timeSeparator to delimit the time.
         */
        static std::string numToDateTime (const time_t epochSeconds,
                                          const char dateSeparator = '\0',
                                          const char timeSeparator = '\0');

        /*!
         * Convert a unix epoch number to a date of form
         * 2009-02-16, using separator to delimit the date.
         */
        static std::string numToDate (const time_t epochSeconds,
                                      const char separator = '\0');

        /*!
         * Return the current time in neat string format.
         */
        static std::string timeNow (void);
        //@}

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
        //@{
        template <typename ST>
        static vector<ST> splitStringWithEncs (const ST& s,
                                               const ST& separatorChars = ST(";, "),
                                               const ST& enclosureChars = ST("\"'"),
                                               const typename ST::value_type& escapeChar = typename ST::value_type(0));
        //@}

    }; // class Tools

} // morph namespace

/*!
 * Templated random number function.
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
//@{
template <typename ST>
vector<ST>
morph::Tools::splitStringWithEncs (const ST& s,
                                   const ST& separatorChars,
                                   const ST& enclosureChars,
                                   const typename ST::value_type& escapeChar) // or '\0'
{
    // Run through the string, searching for separator and
    // enclosure chars and finding tokens based on those.

    vector<ST> theVec;
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
//@}

#endif // _TOOLS_H_
