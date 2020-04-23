/*!
 * \file Process.h
 *
 * \brief A class for execing processes
 *
 * Process is a class to fork and exec processes.
 *
 * For an example of how to use this class (along with ProcessData),
 * see tests/testProcess.cpp
 *
 * Author: Seb James <seb.james@sheffield.ac.uk>
 */
#pragma once

#include <list>
#include <vector>
#include <string>
#include <fstream>
#include <cstdlib>
extern "C" {
#include <sys/poll.h>
}
#define PROCESS_MAIN_APP 0
#define PROCESS_FAILURE -1

// Possible errors to be generated
#define PROCESSNOERROR       0
#define PROCESSFAILEDTOSTART 1
#define PROCESSCRASHED       2
#define PROCESSTIMEDOUT      3
#define PROCESSWRITEERR      4
#define PROCESSREADERR       5
#define PROCESSUNKNOWN       6
#define PROCESSNOMOREPIPES   7
#define PROCESSFORKFAILED    8

namespace morph {

        /*!
         * \brief A set of virtual callbacks for use with the Process
         * class.
         *
         * A set of virtual callbacks. These should be derived in the
         * client code. They're called by Process via the
         * ProcessCallbacks* callbacks member variable.
         */
        class ProcessCallbacks
        {
        public:
                ProcessCallbacks() {}
                virtual ~ProcessCallbacks() {}
                virtual void startedSignal (std::string) {}
                virtual void errorSignal (int) {}
                virtual void processFinishedSignal (std::string) {}
                virtual void readyReadStandardOutputSignal (void) {}
                virtual void readyReadStandardErrorSignal (void) {}
        };

        /*!
         * \brief A C++ class to exec processes without use of
         * system().
         *
         * Process is a simple replacement for the Qt class QProcess.
         */
        class Process
        {
        public:
                /*!
                 * Just one constructor. Allocates memory for pollfd*
                 * p.
                 */
                Process();

                /*!
                 * Destructor. Deallocates memory for pollfd* p
                 */
                ~Process();

        private:
                /*!
                 * Close any open file descriptors.
                 */
                void closeAllFileDescriptors (void);

        public:
                /*!
                 * Reset the process ready to be used again. If this
                 * process is still running, return false and don't do
                 * the reset. Otherwise, reset member attributes and
                 * return true.
                 *
                 * \param keepCallbacks If true, then the stored
                 * pointer to a callback class should NOT be
                 * reset. The default behaviour is to reset the
                 * callbacks.
                 */
                bool reset (bool keepCallbacks = false);

                /*!
                 * Write \arg input to the stdin of the process.
                 */
                void writeIn (const std::string& input) const;

                /*!
                 * When Process::start() is called, pause useconds
                 * before forking and exec-ing the program. This is a
                 * setter for this->pauseBeforeStart.
                 */
                void setPauseBeforeStart (const unsigned int useconds);

                /*!
                 * fork and exec the process.
                 */
                int start (const std::string& program, const std::list<std::string>& args);

                /*!
                 * Send a TERM signal to the process.
                 */
                void terminate (void);

                /*!
                 * poll to see if there is data on stderr or stdout
                 * and to see if the process has exited.
                 *
                 * This must be called on a scheduled basis. It checks
                 * for any stdout/stderr data and also checks whether
                 * the process is still running.
                 */
                void probeProcess (void);

                /*!
                 * If the process is running - if pid > 0, return
                 * true. Otherwise return false.
                 */
                bool running (void) const;

                /*!
                 * Accessors
                 */
                //@{
                pid_t getPid (void) const;
                int getError (void) const;
                void setError (const int e);

                /*!
                 * Setter for the callbacks.
                 */
                void setCallbacks (ProcessCallbacks * cb);
                //@}

                /*!
                 * Slots
                 */
                //@{
                std::string readAllStandardOutput (void) const;
                std::string readAllStandardError (void) const;

                /*!
                 * Wait for the process to get itself going. Do this
                 * by looking at pid.  If no pid after a while,
                 * return false.
                 */
                bool waitForStarted (void);
                //@}
        private:
                /*!
                 * The name of the program to execute
                 */
                std::string progName;

                /*!
                 * The environment and arguments of the program to execute
                 */
                std::list<std::string> environment;

                /*!
                 * Number of micro seconds to pause (via a usleep()
                 * call) before forking and execing the program
                 * following the call to Process::start().
                 */
                unsigned int pauseBeforeStart;

                /*!
                 * Holds a Process error, defined above. PROCESSNOERROR, etc.
                 */
                int error;

                /*!
                 * Process ID of the program
                 */
                pid_t pid;

                /*!
                 * Set to true if the fact that the program has been
                 * started has been signalled using the callback
                 * callbacks->startedSignal
                 */
                bool signalledStart;

                /*!
                 * stdin parent to child. Initialised as zeros. If
                 * these are non-zero at destruction, close() is
                 * called.
                 */
                int parentToChild[2];

                /*!
                 * stdout child to parent. Initialised as zeros. If
                 * these are non-zero at destruction, close() is
                 * called.
                 */
                int childToParent[2];

                /*!
                 * stderr child to parent. Initialised as zeros. If
                 * these are non-zero at destruction, close() is
                 * called.
                 */
                int childErrToParent[2];

                /*!
                 * Used in the poll() call in probeProcess()
                 */
                struct pollfd * p;

                /*!
                 * Pointer to a callback object
                 */
                ProcessCallbacks * callbacks;
        };


        /*!
         * \brief A class used as a parent to a callback object.
         *
         * This class is used as a callback object parent when a
         * process is used within a static function, and as such no
         * parent object exists.
         *
         * Example of this usage can be found in testProcess.cpp (was WmlnetapuiUtilities.cpp)
         */
        class ProcessData
        {
        public:
                /* \name Constructor and Destructor */
                ProcessData (void);
                ~ProcessData (void);

                /* \name Public Class Functions */
                //@{
                /*!
                 * \brief Set the process finished message for a process
                 */
                void setProcessFinishedMsg (const std::string& message);

                /*!
                 * \brief Set the error num for a process
                 */
                void setErrorNum (const int err);

                /*!
                 * \brief Set stdOutReady
                 */
                void setStdOutReady (const bool ready);

                /*!
                 * \brief Set stdErrReady
                 */
                void setStdErrReady (const bool ready);

                /*!
                 * \brief Getters
                 */
                std::string getProcessFinishedMsg (void) const;
                int getErrorNum (void) const;
                bool getStdOutReady (void) const;
                bool getStdErrReady (void) const;
                //@}
        private:
                /* \name Private Class Attributes */
                //@{
                /*!
                 * \brief Holds the name of the process that finished
                 */
                std::string processFinishedMessage;

                /*!
                 * \brief Holds a process error number
                 */
                int errorNum;

                /*!
                 * \brief determines if std out is ready to be read from
                 */
                bool stdOutReady;

                /*!
                 * \brief determines if std err is ready to be read from
                 */
                bool stdErrReady;
                //@}
        };

} // namespace morph
