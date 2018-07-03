/* -*-c++-*- */
/*!
 * \file util.h
 *
 * \brief Declares the class util
 *
 * util contains some generally useful static member functions.
 *
 *  This code is based on wml::futil
 *  (github.com/sebjameswml/futil.git). I removed one or two methods
 *  which depended on Glibmm.
 *
 *  WML futil is Copyright William Matthew Ltd. 2010.
 *
 *  Authors: Seb James <sjames@wmltd.co.uk>
 *           Tamora James <tjames@wmltd.co.uk>
 *           Mark Richardson <mrichardson@wmltd.co.uk>
 *
 *  WML futil and morph::util is free software: you can redistribute
 *  it and/or modify it under the terms of the GNU Lesser General
 *  Public License as published by the Free Software Foundation,
 *  either version 3 of the License, or (at your option) any later
 *  version.
 *
 *  WML futil is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with WML futil.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _UTIL_H_
#define _UTIL_H_

#ifdef __GNUG__
# pragma interface
#endif

extern "C" {
#include <sys/stat.h>
#include <sys/types.h>
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

/*!
 * CUPS would appear to be happy with any character under the sun
 * being in the title. However, we'll restrict it to these:
 */
#define WMLCUPS_TITLE_SAFE_CHARS      CHARS_NUMERIC_ALPHA"_{}^[]`=,;"

/*!
 * Internally, cups is happy with a very wide range of characters:
 */
#define CUPS_QUEUENAME_SAFE_CHARS     CHARS_NUMERIC_ALPHA"!\"$\%&'()*+,-.:;<=>?@[\\]^_{|}~"

/*!
 * Excel doesn't like [ ] * \ / ? :
 */
#define EXCEL_SHEETNAME_SAFE_CHARS    CHARS_NUMERIC_ALPHA"_;!\"'£$\%^&()=+#~@;<>{}|.,"

/*!
 * To make our life a bit easier, we'll disallow a number of queuename
 * characters which would otherwise be acceptable.
 *
 * There is both the HTML environment and the jQuery environment which
 * can be confused by strange queue names. Queue names are frequently
 * used (X)HTML tags so that disallows most of the additional
 * characters allowed by CUPS. The '.' char _should_ be ok, but
 * appears to upset the jQuery despite this.
 */
#define WMLCUPS_QUEUENAME_SAFE_CHARS  CHARS_NUMERIC_ALPHA"-_"

/*!
 * Cups addresses are reckoned to be allowed to contain the same chars
 * as IP domain names.
 */
#define CUPS_ADDRESS_SAFE_CHARS       IP_DOMAINNAME_SAFE_CHARS

/*!
 * The Cups destination queue can only be numbers or letters.
 */
#define CUPS_DESTQUEUEPORT_SAFE_CHARS CHARS_NUMERIC_ALPHA

/*!
 * Unreserved characters for URI percent encoding.
 */
#define URI_UNRESERVED_CHARS      CHARS_NUMERIC_ALPHA"-._~"

/*
 * util code is available to C++ programs only.
 */
#ifdef __cplusplus

#include <vector>
#include <list>
#include <set>
#include <map>
#include <istream>
#include <ostream>
#include <iostream>
#include <fstream>

namespace morph {

    /*!
     * \brief The type of script file being passed
     */
    enum SCRIPT_TYPE {SCRIPT_JAVASCRIPT, SCRIPT_CSS};

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

    /*!
     * \brief A collection of utility functions
     *
     * The util class provides numerous static utility
     * functions.
     */
    class util
    {
    public:
        /*!
         * Constructor
         */
        util();

        /*!
         * This removes all carriage return characters ('\\r'
         * 0xd) from input. It will convert all DOS style
         * newlines, which consist of '\\r''\\n' character duplets,
         * to UNIX style newlines ('\\n'). A side effect is that
         * any lone '\\r' characters which are present will be
         * removed, whether or not they are followed by a '\\n'
         * character.
         */
        static int ensureUnixNewlines (std::string& input);

        /*!
         * If the last character of input is a carriage return
         * ('\\r' 0xd), then it is erased from input.
         */
        static int stripTrailingCarriageReturn (std::string& input);

        /*!
         * Erase trailing spaces from input. Return the
         * number of spaces removed.
         */
        //@{
        static int stripTrailingSpaces (std::string& input);
        //@}

        /*!
         * Erase trailing chars c from input. Return the
         * number of chars removed.
         */
        //@{
        static int stripTrailingChars (std::string& input, const char c = ' ');
        //@}

        /*!
         * Erase trailing whitespace from input. Return the
         * number of whitespace characters removed.
         */
        static int stripTrailingWhitespace (std::string& input);

        /*!
         * Erase leading spaces from input. Return the
         * number of spaces removed.
         */
        //@{
        static int stripLeadingSpaces (std::string& input);
        //@}

        /*!
         * Erase any leading character c from input. Return the
         * number of chars removed.
         */
        //@{
        static int stripLeadingChars (std::string& input, const char c = ' ');
        //@}

        /*!
         * Erase leading whitespace from input. Return the
         * number of whitespace characters removed.
         */
        static int stripLeadingWhitespace (std::string& input);

        /*!
         * Erase leading and trailing whitespace from
         * input. Return the number of whitespace characters
         * removed.
         */
        static int stripWhitespace (std::string& input);

        /*!
         * Strip any occurrences of the characters in charList
         * from input.
         */
        //@{
        static int stripChars (std::string& input, const std::string& charList);
        static int stripChars (std::string& input, const char charList);
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
        static int convertCHexCharSequences (std::string& input);

        /*!
         * Do a search and replace, search for searchTerm,
         * replacing with replaceTerm. if replaceAll is true,
         * replace all occurrences of searchTerm, otherwise
         * just replace the first occurrence of searchTerm
         * with replaceTerm.
         *
         * \return the number of terms replaced.
         */
        static int searchReplace (const std::string& searchTerm,
                                  const std::string& replaceTerm,
                                  std::string& data,
                                  const bool replaceAll = true);

        /*!
         * Do a search and replace in the file fileName,
         * search for searchTerm, replacing with
         * replaceTerm. if replaceAll is true, replace all
         * occurrences of searchTerm, otherwise just replace
         * the first occurrence of searchTerm with
         * replaceTerm.
         */
        static int searchReplaceInFile (const std::string& searchTerm,
                                        const std::string& replaceTerm,
                                        const std::string& fileName,
                                        const bool replaceAll = true);

        /*!
         * Search for searchTerm in fileName. Any lines
         * containing searchTerm should be deleted. If
         * deleteEndOfLine is true, also remove the EOL NL or
         * CRNL sequence, otherwise, leave the EOL in place.
         */
        static int deleteLinesContaining (const std::string& searchTerm,
                                          const std::string& fileName,
                                          const bool deleteEndOfLine = true);

        /*!
         * Return the number of instances of the character c
         * in line.
         */
        static unsigned int countChars (const std::string& line, const char c);

        /*!
         * Take the string str and condition it, so that it
         * makes a valid XML tag, by replacing disallowed
         * characters with '_' and making sure it doesn't
         * start with a numeral.
         */
        static void conditionAsXmlTag (std::string& str);

        /*!
         * \brief Return the amount of RAM installed on the system.
         *
         * Returns RAM in bytes.
         */
        static unsigned long long int getMemory (void);

        /*!
         * \brief Return the amount of RAM used as cache.
         *
         * Returns cached RAM in bytes.
         */
        static unsigned long long int getCachedMemory (void);

        /*!
         * \brief Return the amount of RAM used as buffers.
         *
         * Returns buffer RAM in bytes.
         */
        static unsigned long long int getBufferedMemory (void);

        /*!
         * \brief Return the amount of active RAM.
         *
         * Returns active RAM in bytes.
         */
        static unsigned long long int getActiveMemory (void);

        /*!
         * \brief Return the amount of inactive RAM.
         *
         * Returns inactive RAM in bytes.
         */
        static unsigned long long int getInactiveMemory (void);

        /*!
         * Returns the system load average.
         */
        static float getLoadAverage (void);

        /*!
         * Get the uptime for the system in seconds.
         */
        static float getUptime (void);

        /*!
         * Return the amount of free space on the filesystem
         * on which dirPath resides. The returned value is a
         * human readable amount, with the KB/MB/GB suffix
         * being automatically selected.
         */
        static std::string freeSpace (const std::string& dirPath);

        /*!
         * Return the amount of free space in KBytes on the
         * filesystem on which dirPath resides.
         */
        static unsigned long long int freeSpaceKBytes (const std::string& dirPath);

        /*!
         * Return the amount of space in KBytes used by the
         * absolute file paths listed in \param fileList.
         */
        static unsigned long long int KBytesUsedBy (const std::vector<std::string>& fileList);

        /*!
         * Return the amount of space in KBytes used by the
         * relative file paths listed in \param fileList,
         * which all start from the directory \param dirPath.
         */
        static unsigned long long int KBytesUsedBy (const std::vector<std::string>& fileList,
                                         const std::string& dirPath);

        /*!
         * Return the fractional amount of space on the
         * filesystem on which dirPath resides.
         */
        static float freeSpaceFraction (const std::string& dirPath);

        /*!
         * Return the total amount of space in KBytes on the
         * filesystem on which dirPath resides.
         */
        static unsigned long long int totalSpaceKBytes (const std::string dirPath);

        /*!
         * Stat a file, return true if the file exists and is
         * any kind of file except a directory.
         */
        static bool fileExists (const std::string& path);

        /*!
         * Stat a file, return true if the file exists and is
         * a regular file.  If file is a hanging symlink,
         * fileExists returns false, if file is a symlink
         * pointing to a regular file, fileExists returns
         * true.
         */
        static bool regfileExists (const std::string& path);

        /*!
         * Like regfileExists, but also checks if the file has
         * the "executable by user" bit set (chmod u+x).
         */
        static bool userExefileExists (const std::string& path);

        /*!
         * Like regfileExists, but for block devices
         */
        static bool blockdevExists (const std::string& path);

        /*!
         * Like regfileExists, but for sockets
         */
        static bool socketExists (const std::string& path);

        /*!
         * Like regfileExists, but for fifos
         */
        static bool fifoExists (const std::string& path);

        /*!
         * Like regfileExists, but for char devices
         */
        static bool chardevExists (const std::string& path);

        /*!
         * Like lnkExists, but for char devices
         */
        static bool linkExists (const std::string& path);

        /*!
         * Stat a directory, return true if the directory
         * exists.
         */
        static bool dirExists (const std::string& path);

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
        static void createDir (const std::string& path,
                               const mode_t mode = 0775,
                               const int uid = -1, const int gid = -1);

        /*!
         * Attempt to rmdir path.
         */
        static void removeDir (const std::string& path);

        /*!
         * Set the permissions for the provided file
         */
        static void setPermissions (const std::string& filepath,
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
        static bool checkAccess (const std::string& filepath,
                                 const std::string& accessType);

        /*!
         * Set the ownership for the provided file
         */
        static void setOwnership (const std::string& filepath,
                                  const int uid = -1,
                                  const int gid = -1);

        /*!
         * Touch the file.
         */
        static void touchFile (const std::string& path);

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
        static void copyFile (const std::string& from, const std::string& to);
        static void copyFile (const std::string& from, std::ostream& to);
        static void copyFile (FILE* from, const std::string& to);
        static void copyFile (std::istream& from, const std::string& to);
        static void copyFile (const std::string& from, FILE* to);
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
        static void copyFileToString (std::istream& from, std::string& to);

        /*!
         * Append the file from to the filestream appendTo
         */
        //@{
        static void appendFile (const std::string& from, std::ostream& appendTo);
        static void appendFile (std::istream& from, std::ostream& appendTo);
        static void appendFile (std::istream& from, const std::string& appendTo);
        static void appendFile (const std::string& from, const std::string& appendTo);
        //@}

        /*!
         * Make a copy of \param bytes bytes of the file at
         * \param original to the file \param truncated.
         */
        static void truncateFile (const std::string& original,
                                  const std::string& truncated,
                                  const unsigned int bytes);

        /*!
         * Calls basic_string::getline to copy from istrm into
         * line, setting inputComplete to true if eof is
         * reached and optionally copying to copystrm if it is
         * open.
         */
        static bool getlineWithCopy (std::istream* istrm,
                                     std::string& line,
                                     std::ofstream& copystrm,
                                     bool& inputComplete,
                                     const char eolChar = '\n');

        /*!
         * Move a file. Throw exception on failure.
         */
        static void moveFile (const std::string& from, const std::string& to);

        /*!
         * Call unlink() on the given file path fpath. If
         * unlinking fails, throw a descriptive error based on
         * the errno which was set on unlink's return.
         */
        static void unlinkFile (const std::string& fpath);

        /*!
         * Unlink files in dirPath which are older than
         * olerThanSeconds and which contain filePart.
         */
        static void clearoutDir (const std::string& dirPath,
                                 const unsigned int olderThanSeconds = 0,
                                 const std::string& filePart = "");

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
        static void readDirectoryTree (std::vector<std::string>& vec,
                                       const std::string& baseDirPath,
                                       const std::string& subDirPath,
                                       const unsigned int olderThanSeconds = 0);

        /*!
         * A simple wrapper around the more complex version,
         * for the user to call.
         *
         * If olderThanSeconds is passed in with a non-zero
         * value, then only files older than olderThanSeconds
         * will be returned.
         */
        static void readDirectoryTree (std::vector<std::string>& vec,
                                       const std::string& dirPath,
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
        static void readDirectoryDirs (std::set<std::string>& dset,
                                       const std::string& dirPath);

        /*!
         * Return empty subdirectories in
         * dirPath/subDir. Recursive partner to
         * readDirectoryEmptyDirs(set<string>&, const string&).
         *
         * The base directory path baseDirPath should have NO
         * TRAILING '/'. The subDirPath should have NO INITIAL
         * '/' character.
         */
        static void readDirectoryEmptyDirs (std::set<std::string>& dset,
                                            const std::string& baseDirPath,
                                            const std::string& subDir = "");

        /*!
         * Attempts to remove all the unused directories in a tree.
         *
         * May throw exceptions.
         */
        static void removeUnusedDirs (std::set<std::string>& dset,
                                      const std::string& dirPath);

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
        static void removeEmptySubDirs (std::set<std::string>& dset,
                                        const std::string& baseDirPath,
                                        const std::string& subDir = "");

        /*!
         * Return a datestamp - st_mtime; the file
         * modification time for the given file.
         */
        static std::string fileModDatestamp (const std::string& filename);

        /*!
         * Check whether the specified files differ.
         */
        static bool filesDiffer (const std::string& first, const std::string& second);

#ifdef FIGURED_OUT_CPP_FILE
        /*!
         * get an flock on the given file.
         */
        static void getLock (const int fd);

        /*!
         * Get flock on a C++ stream.
         */
        static void getLock (const std::fstream& f);

        /*!
         * release an flock on the locked fd.
         */
        static void releaseLock (const int fd);

        /*!
         * release an flock on the locked C++ stream.
         */
        static void releaseLock (const std::fstream& f);
#endif

        /*!
         * Replace '\' with '\\' so that str is suitable to be
         * placed in a shell script.
         */
        static void backSlashEscape (std::string& str);

        /*!
         * Escape shell special chars in str with '\' chars so
         * it is suitable to be placed in a shell script.
         */
        static void slashEscape (std::string& str);

        /*!
         * Escape characters which should be escaped in sql
         * strings. Pass in forPatternMatching to escape % and
         *   chars which are treated in a special way by sql
         * like LIKE '%blah_%'
         */
        static void sqlEscape (std::string& str, const bool forPatternMatching);

        /*!
         * As above, but returns the escaped string.
         */
        static std::string sqlEscapeRtn (const std::string& str, const bool forPatternMatching);

        /*!
         * Escape characters to produce string suitable as XML
         * content.
         *
         * Replaces characters that may cause problems in XML
         * content: &, <, >, ', ", and, optionally, characters
         * outside the ASCII character set.
         */
        static std::string xmlEscape (const std::string& s, const bool replaceNonAscii = true);

        /*!
         * Given a path like C:\\path\\to\\file in str, remove
         * all the preceding C:\\ stuff to leave just the
         * filename.
         */
        static void stripDosPath (std::string& dosPath);

        /*!
         * Given a path like /path/to/file in str, remove all
         * the preceding /path/to/ stuff to leave just the
         * filename.
         */
        static void stripUnixPath (std::string& unixPath);

        /*!
         * Given a path like /path/to/file in str, remove the
         * final filename, leaving just the path, "/path/to".
         */
        static void stripUnixFile (std::string& unixPath);

        /*!
         * Given a path like /path/to/file.ext or just
         * file.ext in str, remove the file suffix.
         */
        static void stripFileSuffix (std::string& unixPath);

        /*!
         * Generate a uuid random string as a temporary random
         * filename. Pass in the path, and a prefix to
         * identify the way the file is to be used. For
         * example, with prefixPath "/tmp/xml-", the resulting
         * file could be:
         * /tmp/xml-814b3393-e55a-449e-b16b-b5241497b532
         *
         * If numChars is non-zero, it indicates the number of
         * characters of the UUID to include. For example,
         * with prefixPath "/tmp/xml-" and numChars equal to
         * 8, the resulting file could be:
         * /tmp/xml-814b3393
         */
        static std::string generateRandomFilename (const std::string& prefixPath,
                                                   const unsigned int numChars = 0);

        /*!
         * Return a portion of a random UUID string. numChars
         * to be between 0 and 36. Characters are in the range
         * 0-9 and a-f (i.e. hex).
         */
        static std::string uuidPortion (const unsigned int numChars);

        /*!
         * Return a random string of characters.
         *
         * \param numChars The number of characters to return
         *
         * \param includeUppercase Include upper case alphas
         * in returned string
         *
         * \param includeLowercase Include lower case alphas
         * in returned string
         *
         * \param includeNumerals Include decimal numerals in
         * the returned string
         *
         * \param allowSimilars Allow the string to include
         * the "similar characters": 1,l,I,O,0; all of which
         * are somewhat ambiguous.
         *
         * \return the random string.
         */
        static std::string randomString (const unsigned int numChars,
                                         const bool includeUppercase = true,
                                         const bool includeLowercase = true,
                                         const bool includeNumerals = true,
                                         const bool allowSimilars = true);

        /*!
         * \brief Generate MD5 checksum for the specified string.
         */
        static std::string generateMd5sum (const std::string& s);

        /*!
         * This could be a template. Return true if v contains
         * i.
         */
        static bool vectorContains (const std::vector<unsigned int>& v, const unsigned int i);

        /*!
         * Return position in v of first string which is equal
         * to s, or -1 if it's not present.
         */
        static int strVectorContains (const std::vector<std::string>& v, const std::string& s);

        /*!
         * Return position in v of first string which is a
         * substring of s, or -1 if none is present.
         */
        static int strVectorMatches (const std::vector<std::string>& v, const std::string& s);

        /*!
         * Return position in v of first entry which _doesn't_
         * match s. Return -1 if all members of v match s.
         */
        static int firstNotMatching (const std::vector<std::string>& v, const std::string& s);

        /*!
         * This could be a template. Return true if l contains
         * i.
         */
        static bool listContains (const std::list<unsigned int>& l, const unsigned int i);

        /*!
         * This could be a template. Return true if l contains
         * i.
         */
        static bool listContains (const std::list<std::string>& l, const std::string& s);

        /*!
         * Return true if the given pid is loaded, unless its
         * state, as given in /proc/[pid]/status, is either
         * zombie (Z) or dead (X).
         *
         * The second line of /proc/[pid]/status, if the file
         * exists, provides information about the state of the
         * specified pid. State may be one of the following:
         * "R (running)", "S (sleeping)", "D (disk sleep)", "T
         * (stopped)", "T (tracing stop)", "Z (zombie)", or "X
         * (dead)".
         */
        static bool pidLoaded (const int pid);

        /*!
         * Get the command line for the given pid out of
         * /proc/pid/cmdline.
         */
        static std::string pidCmdline (const int pid);

        /*!
         * Populate a vector of directories in /proc
         */
        static void readProcDirs (std::vector<std::string>& vec,
                                  const std::string& baseDirPath,
                                  const std::string& subDirPath);

        /*!
         * \brief Get the first PID whose program name matches the
         * argument programName.
         *
         * From all running processes, work out if any of the
         * process named programName is running. Return the
         * pid of the running process. If there is none,
         * return -1;
         *
         * programName is just the executable file name,
         * without the path, e.g. tail not /usr/bin/tail and
         * so on. If the program name is more than 15 chars,
         * use only the first 15 chars to look for a match.
         *
         * Opens all subdirectories in /proc which have only
         * numerals in their name within these, gets the first
         * line: Name: whatever If the name is programName,
         * then get the Pid line, and return that.
         */
        static int getPid (const std::string& programName);

        /*!
         * Terminate then Kill the pid. Program name is used
         * to ensure that the signal had required effect. ONLY
         * RELIABLE FOR PROCESSES THAT ARE EXPECTED TO RUN AS
         * SINGLE INSTANCES. Set pid to -1 if killed. Return
         * -1 on error, 0 on success. At 20110105, unused in
         * WML code.
         */
        static int termKill (const std::string& programName, int& pid);

        /*!
         * Get the mac address of the platform from eth0
         *
         * @return The mac address as a string (something like
         * "aa:bb:cc:00:0a:0b")
         */
        static std::string getMacAddr (void);

        /*!
         * Get the mac address of the platform from the
         * network device @param netdev
         *
         * @return The mac address as a string (something like
         * "aa:bb:cc:00:0a:0b")
         */
        static std::string getMacAddr (const std::string& netdev);

        /*!
         * Get the mac address of the platform from eth0
         *
         * @param[out] mac A buffer of 2 * 32 bit unsigned
         * ints in which to place the mac address
         */
        static void getMacAddr (unsigned int* mac);

        /*!
         * Get the mac address of the platform from the
         * network device @param netdev.
         *
         * @param[out] mac A buffer of 2 * 32 bit unsigned
         * ints in which to place the mac address
         */
        static void getMacAddr (unsigned int* mac, const std::string& netdev);

        /*!
         * Get the mac address from the string @macStr
         *
         * @param[in] macStr A string representing the Mac address
         * in the form "aa:bb:cc:11:22:33".
         *
         * @param[out] mac A buffer of 2 * 32 bit unsigned
         * ints in which to place the mac address. mac[0]
         * contains the lowest 4 nibbles of the mac address;
         * in this example, it would contain
         * 0xcc112233. mac[1] contains the top two nibbles -
         * here it would be 0xaabb.
         */
        static void strToMacAddr (const std::string& macStr, unsigned int* mac);

        /*!
         * Convert @mac, a buffer of 2 * 32 bit unsigned ints
         * into a string representation in the form
         * "aa:bb:cc:aa:bb:cc".
         */
        static std::string macAddrToStr (const unsigned int* mac);

        /*!
         * Return the list of all possible alias addresses in
         * a vector of strings.
         */
        static std::vector<std::string> getAllAliases (void);

        /*!
         * Read /etc/hostname to obtain the hostname of the
         * running system.
         */
        static std::string readHostname (void);

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

        /*!
         * Read the lineNum'th line of the syslog file and
         * obtain the month, returning it. (1==Jan, 12==Dec).
         */
        static unsigned int getMonthFromLog (const std::string& filePath,
                                             const unsigned int lineNum);

        /*!
         * Read a script file and output to rScript. This is a
         * function to use when you want to read a script file
         * and output it into the html, so it adds the relevant
         * opening and closing tags.
         */
        static void getScript (const SCRIPT_TYPE script,
                               std::stringstream& rScript,
                               const std::string& scriptFile,
                               const bool inlineOutput = true);

        /*!
         * Read a javascript file and output to
         * rJavascript. This is a function to use when you
         * want to read a javascript file and output it into
         * the html, so it adds \<script type="text/javascript"\>
         * at the start and \</script\> at the end.
         */
        static void getJavascript (std::stringstream& rJavascript,
                                   const std::string& jsFile,
                                   const bool inlineOutput = true);

        /*!
         * Read a CSS file and output to
         * rCSS. This is a function to use when you
         * want to read a CSS file and output it into
         * the html, so it adds \<style type="text/css"\>
         * at the start and \</style\> at the end.
         */
        static void getCSS (std::stringstream& rCSS,
                            const std::string& cssFile,
                            const bool inlineOutput = true);

        /*!
         * Take an ascii string and represent the characters
         * as unicode utf8.
         */
        static void unicodeize (std::string& str);

        static std::string unicodePointToUTF8 (unsigned long& unicodePoint);

        static void numericCharRefsToUTF8 (std::string& s);

        /*!
         * If str contains only numerals 0 to 9, return true,
         * else return false.
         */
        static bool containsOnlyNumerals (const std::string& str);

        /*!
         * If str contains only whitespace characters (tab,
         * NL, CR, space), return true, otherwise return false.
         */
        static bool containsOnlyWhitespace (const std::string& str);

        /*!
         * A general purpose sanitization function. This is a
         * "allow only a list of characters" sort of
         * sanitiser, so you call it with a different list of
         * characters in allowed, depending on whether you're
         * sanitising for html or sql. This function could
         * throw errors or simply erase disallowed chars. Try
         * former behaviour, but offer a bool to switch. If
         * eraseForbidden is true, then the function will
         * remove forbidden chars from the string rather than
         * throwing a runtime error.
         */
        static void sanitize (std::string& str,
                              const std::string& allowed,
                              const bool eraseForbidden = false);

        /*!
         * Modification of sanitize in which the offending
         * characters are replaced with replaceChar.
         */
        static void sanitizeReplace (std::string& str,
                                     const std::string& allowed,
                                     const char replaceChar = '_');

        /*!
         * Read filePath and output to stdout. Useful for
         * outputting static html.
         */
        static void coutFile (const std::string& filePath);

        /*!
         * Get the size of the file in bytes.
         */
        static int fileSize (const std::string& filePath);

        /*!
         * Increment the count stored in the file given by
         * filePath. Return the new value of the count, or -1
         * on error.
         */
        static int incFileCount (const std::string& filePath);

        /*!
         * Increment the count stored in the file given by
         * filePath. Return the new value of the count, or -1
         * on error.
         */
        static int decFileCount (const std::string& filePath);

        /*!
         * Zero the file count in filePath, ENSURING that the
         * file access time is reset to the current time. For
         * this reason, the file is first unlinked, then
         * re-created, then "0" is inserted into it. Return
         * the new value of the count (0), or -1 on error.
         */
        static int zeroFileCount (const std::string& filePath);

        /*!
         * split csv into a vector
         */
        static std::vector<std::string> csvToVector (const std::string& csvList,
                                                     const char separator = ',',
                                                     const bool ignoreTrailingEmptyVal = true);

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
        static std::vector<std::string> stringToVector (const std::string& s, const std::string& separator,
                                                        const bool ignoreTrailingEmptyVal = true);

#ifdef _DEPRECATED_
        /*!
         * Split a string into a set of strings using the
         * delimiter specified.
         *
         * Similar to util::stringToVector but
         * SLOW. Perhaps because it runs recursively?
         *
         * Try not to use this - PREFER stringToVector OVER
         * this function.
         *
         * \param tokens The container into which the tokens
         * will be placed
         *
         * \param stringToSplit The input string to be split
         * up
         *
         * \param delim The delimiter parameter - a single
         * char or a multi-character token.
         */
        static void splitString (std::vector<std::string>& tokens,
                                 const std::string& stringToSplit,
                                 const std::string& delim);
#endif
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
        template <typename stringType>
        static std::vector<stringType> splitStringWithEncs (const stringType& s,
                                                            const stringType& separatorChars = stringType(";, "),
                                                            const stringType& enclosureChars = stringType("\"'"),
                                                            const typename stringType::value_type& escapeChar = typename stringType::value_type(0));
        //@}

        /*!
         * Highlight matching portions of search terms in <tag></tag> tags
         *
         * If term is "sjames" and searchTermsUC contains
         * "JAMES", "1234" and tag contains "b", then term
         * will be modified to contain "s<b>james</b>".
         *
         * \param term The search term which may need to be
         * highlighted. This will NOT be modified by this
         * function (by adding the tags) - instead a modified
         * version is returned.
         *
         * \param searchTermsUC All the terms that user had in
         * his search which must already be in upper case.
         *
         * \param tag The tag which will be used to highlight
         * the matching portion of term.
         *
         * \todo Highlight repeated search terms/occurrence
         * of all search terms in the search string.
         */
        static std::string htmlHighlightTerm (const std::string& term,
                                              const std::vector<std::string>& searchTermsUC,
                                              const std::string& tag);


        /*!
         * Utility version of
         * htmlHighlightTerm(string&,vector<string>&,string&),
         * with search terms passed in as a string, and a
         * default tag value.
         */
        static std::string htmlHighlightTerm (const std::string& term,
                                              const std::string& searchTerms,
                                              const std::string& tag = "strong");

        /*!
         * Split line into substring of a given maximum
         * length, splitting at spaces or at the specified
         * character if possible.
         */
        static std::vector<std::string> wrapLine (const std::string& line,
                                                  const unsigned int maxLength,
                                                  const char wrapAfter = '\0');

        /*!
         * Turn the passed in vector of string values into a
         * string separated by the separator string (not char)
         * passed in as "separator".
         */
        static std::string vectorToString (const std::vector<std::string>& s,
                                           const std::string& separator);


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
         * A wrapper around the iconv() call from glibc.
         *
         * @param fromEncoding The iconv-style encoding
         * specifier from which the string needs to be
         * transcoded. You can get a list of possible values
         * using iconv --list on any normal system. Examples
         * might be UTF-8, UTF-16BE, ISO8859-1
         *
         * @param toEncoding The iconv-style encoding specifer
         * into which the string should be converted.
         *
         * @param fromString The string in its existing format.
         *
         * @param toString A container into which the
         * transcoded string will be placed.
         */
        static void doIconv (const std::string& fromEncoding,
                             const std::string& toEncoding,
                             const std::string& fromString,
                             std::string& toString);

        /*!
         * Opens (or re-opens) the referenced filestream in the
         * correct state for overwriting.
         */
        static void openFilestreamForOverwrite (std::fstream& f,
                                                const std::string& filepath);

        /*!
         * Opens (or re-opens) the referenced filestream in the
         * correct state for appending.
         */
        static void openFilestreamForAppend (std::fstream& f,
                                             const std::string& filepath);

        /*!
         * Close the filestream if necessary
         */
        static void closeFilestream (std::fstream& f);

        /*!
         * Clear the filestream flags if necessary
         */
        static void clearFilestreamFlags (std::fstream& f);

        /*!
         * Determine if ip_string is a valid IP
         * address. Return true if so, false if not.
         *
         * This function copied from
         * wmlnetapui/wmlnetapui/SystemPage.cpp
         */
        static bool valid_ip (const std::string& ip_string);

        /*!
         * Determine if mac_string is a valid MAC address, of
         * the form 01:23:45:67:89:ab. Return true if so,
         * false if not.
         */
        static bool valid_mac (const std::string& mac_string);

        /*!
         * Encodes reserved characters for representation in a
         * URI.
         *
         * Implemented in a fairly straightforward way using
         * information on reserved/unreserved characters found
         * here:
         * http://labs.apache.org/webarch/uri/rev-2002/rfc2396bis.html#characters
         *
         * Later, updated based on testing encoding/decoding
         * arbitrary PDF files, with information from the
         * Wikipedia page "Percent Encoding" and brought into
         * util from wml::fwebui.
         */
        static void encodeURIComponent (std::string& s);

        /*!
         * This does the opposite of the encodeURIComponent()
         * function in javascript.
         */
        static void decodeURIComponent (std::string& s);

        /*!
         * Currently Linux only. Consult the /proc directory
         * to find the number of files open for the passed-in
         * process id pid. If pid is 0, then use the pid of
         * the current process.
         */
        static unsigned int filesOpen (pid_t pid = 0);
    };

} // namespace wml

/*!
 * Templated function splitStringWithEncs implementation.
 */
//@{
template <typename stringType/*, typename charType*/>
std::vector<stringType>
morph::util::splitStringWithEncs (const stringType& s,
                                  const stringType& separatorChars,
                                  const stringType& enclosureChars,
                                  const typename stringType::value_type/*charType*/& escapeChar) // or '\0'
{
    //DBG2 ("Called for string >" << s << "<");
    // Run through the string, searching for separator and
    // enclosure chars and finding tokens based on those.

    std::vector<stringType> theVec;
    stringType entry("");
    typename stringType::size_type a=0, b=0, c=0;
    stringType sepsAndEncsAndEsc = separatorChars + enclosureChars;
    sepsAndEncsAndEsc += escapeChar;

    typename stringType::size_type sz = s.size();
    while (a < sz) {

        // If true, then the thing we're searching for is an enclosure
        // char, otherwise, it's a separator char.
        bool nextIsEnc(false);
        typename stringType::value_type currentEncChar = 0;

        if (a == 0) { // First field.
            if (escapeChar && s[a] == escapeChar) {
                // First char is an escape char - skip this and next
                ++a; ++a;
                continue;
            } else if ((enclosureChars.find_first_of (static_cast<typename stringType::value_type>(s[a]), 0)) != stringType::npos) {
                // First char is an enclosure char.
                nextIsEnc = true;
                currentEncChar = s[a];
                ++a; // Skip the enclosure char
            } else if ((separatorChars.find_first_of (static_cast<typename stringType::value_type>(s[a]), 0)) != stringType::npos) {
                // First char is a ',' This special case means that we insert an entry for the current ',' and step past it.
                //DBG2 ("First char special case, insert entry.");
                theVec.push_back ("");
                ++a;

            } // else first char is a normal char or a separator.

        } else { // Not first field

            if ((a = s.find_first_of (sepsAndEncsAndEsc, a)) == stringType::npos) {
                //DBG ("No enclosure, separator or escape chars in string");
                theVec.push_back (s);
                return theVec;
            }

            else if (escapeChar && s[a] == escapeChar) {
                // it's an escape char - skip this and next
                ++a; ++a;
                continue;
            } else if ((enclosureChars.find_first_of (static_cast<typename stringType::value_type>(s[a]), 0)) != stringType::npos) {
                // it's an enclosure char.
                nextIsEnc = true;
                currentEncChar = s[a];
                ++a; // Skip the enclosure char
            } else if ((separatorChars.find_first_of (static_cast<typename stringType::value_type>(s[a]), 0)) != stringType::npos) {
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
                    if ((enclosureChars.find_first_of (static_cast<typename stringType::value_type>(s[a]), 0)) != stringType::npos) {
                        // Enclosure char following sep char
                        nextIsEnc = true;
                        currentEncChar = s[a];
                        ++a; // Skip the enclosure char
                    }
                }
            } else {
                //throw std::runtime_error ("util::splitStringWithEncs: Unexpected case");
            }
        }

        // Check we didn't over-run
        if (a >= sz) { break; }

        // Now get the token
        typename stringType::size_type range = stringType::npos;
        if (nextIsEnc) {
            //DBG2 ("Searching for next instances of enc chars: >" << enclosureChars << "< ");
            c = a;
            while ((b = s.find_first_of (currentEncChar, c)) != stringType::npos) {
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
            if ((b = s.find_first_of (separatorChars, a)) != stringType::npos) {
                // Check it wasn't an escaped separator:
                if (escapeChar) {
                    c = b; --c;
                    if (c >= 0 && c != stringType::npos && c < sz && s[c] == escapeChar) {
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
        util::stripChars (entry, escapeChar);
        //DBG2 ("Adding entry '" << entry << "' to vector");
        theVec.push_back (entry);

        if (range != stringType::npos) { // end of field was not end of string
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

#endif // __cplusplus

#endif // _UTIL_H_
