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

    class Tools {
    public:


        static vector<double> getJetColor (double gray);

        /*!
         * Floating point array version of getJetColor()
         */
        static array<float, 3> getJetColorF (double gray);

        static vector<double> getGrayScaleColor (double gray);
        static vector<double> HSVtoRGB (double, double, double);
#if 0
        // This was confusingly named to return a double, rather than
        // a float. I've created randDouble() which returns a double
        // precision random number.
        static double randFloat (void);
#endif
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
