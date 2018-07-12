/* -*-c++-*- */
/*!
 * \file MorphDbg.h
 *
 * \brief The standard morph debugging scheme.
 *
 * Very quick instructions:
 *
 * Add this to your .cpp file in which you want to use DBG() calls:
 *
 *\code
 #define DBGSTREAM std::cout
 #define DEBUG 1
 #include <morph/MorphDbg.h>
 \endcode
 *
 * Then use DBG() like this:
 *
 *\code
 DBG ("It's a bit like cout to use. You can stream numbers like "
      << 2 << " and so on.");
 \endcode
 *
 * Instructions:
 *
 * Include this file, MorphDbg.h, in
 * each .cpp file in your module. Then, if you have DEBUG defined,
 * then you can use DBG() and debuglog() macros in your code.
 *
 * To actually get the DBG messages into a file, any executables built
 * must include these lines:
 *\code
 #include <MorphDbg.h>
 std::ofstream DBGSTREAM;
 \endcode
 *
 * Then, early in the main() function, open
 * the DBGSTREAM ofstream like this:
 *\code
 DBGOPEN ("/tmp/debug.log");
 \endcode
 *
 * The autotools way to have DEBUG defined is to have a configure.ac
 * stanza in your client module, which defines DEBUG in
 * mymodule/mymodule/config.h:
 *\verbatim
 dnl enable/disable debug logging, if specified
 AC_ARG_ENABLE(debug-logging,
 [  --enable-debug-logging  enable debug logging for mymodule [default=no]],
 [case "${enableval}" in
 yes) mymodule_debug=yes ;;
 no)  mymodule_debug=no ;;
 *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug-logging) ;;
 esac], mymodule_debug=no)
 AC_MSG_CHECKING(whether to enable debug logging)
 AC_MSG_RESULT($mymodule_debug)
 if test "$mymodule_debug" = yes; then
 AC_DEFINE(DEBUG, 1, [Define if debug logging is enabled])
 fi
 \endverbatim
 *
 * You then call ./configure with
 * --enable-debug-logging if you want DEBUG defined.
 *
 * Note that util DOESN'T itself need to be compiled with
 * --enable-debug-logging for some other module to generate debug
 * logging using MorphDbg.h.
 *
 * How it works:
 *
 * All debug messages are sent to the DBGSTREAM ofstream. The client
 * code should open this ofstream so that debug messages end up in a
 * file, or sent to stderr, as required.
 *
 * The DBGSTREAM ofstream needs to be declared globally in the same
 * file as the main() function for the executable.
 *
 * Macros are available to open the debug ofstream (DBGOPEN) and to
 * close it (DBGCLOSE). DBG and DBG2 are macros to actually emit a
 * debug message. The argument to DBG() can be used like a stream, so
 * this works:
 *\code
 int i = 1;
 DBG ("The integer is " << i);
 \endcode
 *
 * NB: Where we have previously used the cgicc LOGLN() macro (in our
 * webui stuff) we need to move to the DBG() macro.
 *
 * Extending:
 *
 * If you want to extend the debugging messages, then, as well as
 * including MorphDbg.h, include a new file, moduleDbg.h which adds
 * additional macros something like this:
 *\code
 #define DBGNEWFEATURE(s)  DBGSTREAM << "NEWFEATURE: "
 << __FUNCTION__ << ": "
 << s << endl;
 \endcode
 *
 */

#ifndef _MORPHDBG_H_
#define _MORPHDBG_H_

#ifndef DBGSTREAM
# define DBGSTREAM morphDbg
# include <fstream>
extern std::ofstream DBGSTREAM;
#endif

/*
 * Macro to open the debug file. To be called early on in the main()
 * function.
 */
#ifdef DEBUG
# define DBGOPEN(s) DBGSTREAM.open (s, std::ios::out|std::ios::trunc);  \
    DBGSTREAM << "**Morphologica Debug File**\n";
#else
# define DBGOPEN(s)
#endif

/*
 * Macro to open the debug file, but without first truncating it. To
 * be called early on in the main() function.
 */
#ifdef DEBUG
# define DBGAPPEND(s) DBGSTREAM.open (s, std::ios::out|std::ios::app);  \
    DBGSTREAM << "**Morphologica Debug File**\n";
#else
# define DBGAPPEND(s)
#endif

/*
 * Macro to close the debug file. To be called late in the main()
 * function, just before it exits.
 */
#ifdef DEBUG
# define DBGCLOSE() if (DBGSTREAM.is_open()) { DBGSTREAM.close(); }
#else
# define DBGCLOSE()
#endif

#ifdef DEBUG
# define DBG(s)  DBGSTREAM << "DBG: " << __FUNCTION__ << ": " << s << std::endl;
# define debuglog(type, format, ...) syslog(type, format, __VA_ARGS__)
#else
# define DBG(s)
# define debuglog(type, format, ...)
#endif

#ifdef DEBUG2
# define DBG2(s)  DBGSTREAM << "DBG2: " << __FUNCTION__ << ": " << s << std::endl;
# define debuglog2(type, format, ...) syslog(type, format, __VA_ARGS__)
#else
# define DBG2(s)
# define debuglog2(type, format, ...)
#endif

#endif // _MORPHDBG_H_
