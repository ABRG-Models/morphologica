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
#include <iostream>
extern "C" {
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <signal.h>
}
#include <morph/MorphDbg.h>

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

// Used when dealing with pipes
#define PROCESS_READING_END 0
#define PROCESS_WRITING_END 1
#define PROCESS_STDIN  0
#define PROCESS_STDOUT 1
#define PROCESS_STDERR 2

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
        virtual void readyReadStandardOutputSignal() {}
        virtual void readyReadStandardErrorSignal() {}
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
        //! Just one constructor. Allocates memory for pollfd* p.
        Process() :
            progName("unknown"),
            pauseBeforeStart(0),
            error (PROCESSNOERROR),
            pid(0),
            signalledStart(false)
        {
            // Set up the polling structs
            this->p = static_cast<struct pollfd*>(malloc (2*sizeof (struct pollfd)));

            // Initialise the callbacks to 0
            this->callbacks = nullptr;

            // Initialise pipes to 0
            this->parentToChild[PROCESS_WRITING_END] = 0;
            this->parentToChild[PROCESS_READING_END] = 0;
            this->childToParent[PROCESS_READING_END] = 0;
            this->childToParent[PROCESS_WRITING_END] = 0;
            this->childErrToParent[PROCESS_READING_END] = 0;
            this->childErrToParent[PROCESS_WRITING_END] = 0;
        }

        //! Destructor. Deallocates memory for pollfd* p
        ~Process()
        {
            // Close pipes
            this->closeAllFileDescriptors();
            // Free polling struct
            free (this->p);
        }

    private:
        //! Close any open file descriptors.
        void closeAllFileDescriptors()
        {
            if (this->parentToChild[PROCESS_WRITING_END] > 0) {
                DBG ("close parentToChild[PROCESS_WRITING_END]...");
                if (close (this->parentToChild[PROCESS_WRITING_END])) {
                    // Normally succeeds
                    DBG ("Failed to close parentToChild[PROCESS_WRITING_END]");
                }
                this->parentToChild[PROCESS_WRITING_END] = 0;
            }

            if (this->parentToChild[PROCESS_READING_END] > 0) {
                DBG ("Unexpectedly closing parentToChild[PROCESS_READING_END]...");
                if (close (this->parentToChild[PROCESS_READING_END])) {
                    // Normally already closed in Process::start
                    DBG ("Failed to close parentToChild[PROCESS_READING_END]");
                }
                this->parentToChild[PROCESS_READING_END] = 0;
            }

            if (this->childToParent[PROCESS_READING_END] > 0) {
                DBG ("close childToParent[PROCESS_READING_END]...");
                if (close (this->childToParent[PROCESS_READING_END])) {
                    // Normally succeeds
                    DBG ("Failed to close childToParent[PROCESS_READING_END]");
                }
                this->childToParent[PROCESS_READING_END] = 0;
            }

            if (this->childToParent[PROCESS_WRITING_END] > 0) {
                DBG ("Unexpectedly closing childToParent[PROCESS_WRITING_END]...");
                if (close (this->childToParent[PROCESS_WRITING_END])) {
                    // Normally already closed in Process::start
                    DBG ("Failed to close childToParent[PROCESS_WRITING_END]");
                }
                this->childToParent[PROCESS_WRITING_END] = 0;
            }

            if (this->childErrToParent[PROCESS_READING_END] > 0) {
                DBG ("close childErrToParent[PROCESS_READING_END]...");
                if (close (this->childErrToParent[PROCESS_READING_END])) {
                    // Normally succeeds
                    DBG ("Failed to close childErrToParent[PROCESS_READING_END]");
                }
                this->childErrToParent[PROCESS_READING_END] = 0;
            }

            if (this->childErrToParent[PROCESS_WRITING_END] > 0) {
                DBG ("Unexpectedly closing childErrToParent[PROCESS_READING_END]...");
                if (!close (this->childErrToParent[PROCESS_WRITING_END])) {
                    // Normally already closed in Process::start
                    DBG ("Failed to close childErrToParent[PROCESS_READING_END]");
                }
                this->childErrToParent[PROCESS_WRITING_END] = 0;
            }
        }

    public:
        /*!
         * Reset the process ready to be used again. If this process is still running,
         * return false and don't do the reset. Otherwise, reset member attributes and
         * return true.
         *
         * \param keepCallbacks If true, then the stored pointer to a callback class
         * should NOT be reset. The default behaviour is to reset the callbacks.
         */
        bool reset (bool keepCallbacks = false)
        {
            if (this->running()) { return false; }
            if (!keepCallbacks && this->callbacks != nullptr) {
                DBG ("Resetting callbacks pointer");
                this->callbacks = nullptr;
            }
            this->signalledStart = false;
            this->pauseBeforeStart = 0;
            this->error = PROCESSNOERROR;
            this->progName = "unknown";
            this->environment.clear();
            // Ensure all file descriptors are closed.
            this->closeAllFileDescriptors();
            return true;
        }

        //! Write \arg input to the stdin of the process.
        void writeIn (const std::string& input) const
        {
            write (this->parentToChild[PROCESS_WRITING_END], input.c_str(), input.size());
        }

        /*!
         * When Process::start() is called, pause useconds before forking and exec-ing
         * the program. This is a setter for this->pauseBeforeStart.
         */
        void setPauseBeforeStart (const unsigned int usecs) { this->pauseBeforeStart = usecs; }

        /*!
         * fork and exec the process using execv, which takes stdin via a fifo and
         * returns output also via a fifo.
         */
        int start (const std::string& program, const std::list<std::string>& args)
        {
            char** argarray;
            std::list<std::string> myargs = args;
            std::list<std::string>::iterator i;
            unsigned int j = 0;
            int theError;

            // NB: The first item in the args list should be the program name.
            this->progName = program;

            // Set up our pipes. These may run out. Typically you get 1024
            // which means that if you call Process::start more than 341
            // times you'll run out, unless in your client program you
            // call setrlimit() to change RLIMIT_NOFILE.
            if (pipe(this->parentToChild) == -1
                || pipe(this->childToParent) == -1
                || pipe(this->childErrToParent) == -1) {
                DBG ("Failed to set up pipes, return PROCESS_FAILURE" << std::flush);
                this->error = PROCESSNOMOREPIPES;
                return PROCESS_FAILURE;
            }

            this->pid = fork();
            switch (this->pid) {
            case -1:
                DBG ("fork() returned -1, return PROCESS_FAILURE" << std::flush);
                this->error = PROCESSFORKFAILED;
                return PROCESS_FAILURE;
            case 0:
                // This is the CHILD process

                // Close unwanted ends of the pipes
                close (this->parentToChild[PROCESS_WRITING_END]);
                this->parentToChild[PROCESS_WRITING_END] = 0;
                close (this->childToParent[PROCESS_READING_END]);
                this->childToParent[PROCESS_READING_END] = 0;
                close (this->childErrToParent[PROCESS_READING_END]);
                this->childErrToParent[PROCESS_READING_END] = 0;

                // Now all we have to do is make the writing file
                // descriptors 0,1 or 2 and they will be used instead
                // of stdout, stderr and stdin.
                if ((dup2 (this->parentToChild[PROCESS_READING_END], PROCESS_STDIN)) == -1  ||
                    (dup2 (this->childToParent[PROCESS_WRITING_END], PROCESS_STDOUT)) == -1 ||
                    (dup2 (this->childErrToParent[PROCESS_WRITING_END], PROCESS_STDERR)) == -1) {
                    theError = errno;
                    std::cout << "ERROR! Couldn't get access to stdin/out/err! errno was " << theError << std::endl;
                    return PROCESS_FAILURE;
                }

                // Allocate memory for the program arguments
                // 1+ to allow space for NULL terminating pointer
                argarray = static_cast<char**>(malloc ((1+args.size()) * sizeof (char*)));
                for (i=myargs.begin(); i!=myargs.end(); i++) {
                    argarray[j] = static_cast<char*>(malloc ( (1+(*i).size()) * sizeof (char) ));
                    snprintf (argarray[j++], 1+(*i).size(), "%s", (*i).c_str());
                    DBG (*i);
                }
                argarray[j] = NULL;

                // Pause here, in the child process, prior to the exec call
                if (this->pauseBeforeStart) {
                    usleep (this->pauseBeforeStart);
                }

                DBG ("About to execute '" + program + "' with those arguments..");

                execv (program.c_str(), argarray);

                // If process returns, error occurred
                theError = errno;
                // This'll get picked up by parseResponse
                std::cout << "Process error: " << this->pid << " crashed. errno:" << theError << std::endl;

                // This won't get picked up by the parent process.
                this->error = PROCESSCRASHED;

                // In this case, we close the pipes to signal to the parent that we crashed
                close (this->parentToChild[PROCESS_READING_END]);
                this->parentToChild[PROCESS_READING_END] = 0;
                close (this->childToParent[PROCESS_WRITING_END]);
                this->childToParent[PROCESS_WRITING_END] = 0;
                close (this->childErrToParent[PROCESS_WRITING_END]);
                this->childErrToParent[PROCESS_WRITING_END] = 0;

                // Child should exit now.
                _exit(-1);

            default:
                // This is the PARENT process

                // Close unwanted ends of the pipes
                close (this->parentToChild[PROCESS_READING_END]);
                this->parentToChild[PROCESS_READING_END] = 0;
                close (this->childToParent[PROCESS_WRITING_END]);
                this->childToParent[PROCESS_WRITING_END] = 0;
                close (this->childErrToParent[PROCESS_WRITING_END]);
                this->childErrToParent[PROCESS_WRITING_END] = 0;

                // Write to this->parentToChild[PROCESS_WRITING_END] to write to stdin of the child
                // Read from this->childToParent[PROCESS_READING_END] to read from stdout of child
                // Read from this->childErrToParent[PROCESS_READING_END] to read from stderr of child

                break;
            }
            return PROCESS_MAIN_APP;
        }

        //! Send a TERM signal to the process.
        void terminate()
        {
            kill (this->pid, 15); // 15 is TERM
            // Now check if the process has gone and kill it with signal 9 (KILL)
            this->pid = 0;
            this->error = PROCESSNOERROR;
            this->signalledStart = false;
        }

        /*!
         * poll to see if there is data on stderr or stdout and to see if the process
         * has exited.
         *
         * This must be called on a scheduled basis. It checks for any stdout/stderr
         * data and also checks whether the process is still running.
         */
        void probeProcess()
        {
            // Has the process started?
            if (!this->signalledStart) {
                if (this->pid > 0) {
                    if (this->callbacks != nullptr) {
                        this->callbacks->startedSignal (this->progName);
                    }
                    this->signalledStart = true;
                    DBG ("Process::probeProcess set signalledStart and signalled the start...");
                }
            }

            // Check for error condition
            if (this->error > 0) {
                if (this->callbacks != nullptr) {
                    this->callbacks->errorSignal (this->error);
                }
                DBG ("have error in probeProcess, returning");
                return;
            }

            if (this->pid == 0) { return; } // Not yet started.

            // Why can't these 4 lines go in contructor? Because
            // this->childToParent etc aren't set up until Process::start().
            this->p[0].fd = this->childToParent[PROCESS_READING_END];
            this->p[0].events = POLLIN | POLLPRI;
            this->p[1].fd = this->childErrToParent[PROCESS_READING_END];
            this->p[1].events = POLLIN | POLLPRI;

            // Poll to determine if data is available
            this->p[0].revents = 0;
            this->p[1].revents = 0;

            // Poll for data
            int prtn = poll (this->p, 2, 0);
            if (prtn > 0) {
                DBG2 ("poll returned " << prtn);
            } else if (prtn == 0) {
                // No revents from poll
            } else {
                DBG ("error from poll() call");
            }

            if (this->p[0].revents & POLLNVAL || this->p[1].revents & POLLNVAL) {
                DBG ("Process::probeProcess: pipes closed, process must have crashed");
                this->error = PROCESSCRASHED;
                if (this->callbacks != nullptr) {
                    this->callbacks->errorSignal (this->error);
                }
                return;
            }

            if (this->p[0].revents & POLLIN || this->p[0].revents & POLLPRI) {
                if (this->callbacks != nullptr) {
                    this->callbacks->readyReadStandardOutputSignal();
                }
            }
            if (this->p[1].revents & POLLIN || this->p[1].revents & POLLPRI) {
                if (this->callbacks != nullptr) {
                    this->callbacks->readyReadStandardErrorSignal();
                }
            }

            // Is the process running? We check last, so that we get any
            // messages on stdout/stderr that we may wish to process, such
            // as error messages from nxssh key authentication.
            int theError;
            if (this->signalledStart == true) {
                int rtn = 0;
                if ((rtn = waitpid (this->pid, nullptr, WNOHANG)) == this->pid) {
                    if (this->callbacks != nullptr) {
                        this->callbacks->processFinishedSignal (this->progName);
                    }
                    this->pid = 0;
                    DBG ("Process finished, returning");
                    return;
                } else if (rtn == -1) {
                    theError = errno;
                    if (theError != 10) { // We ignore errno 10 "no child" as this commonly occurs
                        std::cerr << "waitpid returned errno: " << theError;
                    }
                } // else rtn == 0
            }
        }

        //! If the process is running - if pid > 0, return true. Otherwise return false.
        bool running() const { return this->pid > 0 ? true : false; }

        // Accessors
        pid_t getPid() const { return this->pid; }
        int getError() const { return this->error; }
        void setError (const int e) { this->error = e; }

        //! Setter for the callbacks.
        void setCallbacks (ProcessCallbacks * cb) { this->callbacks = cb; }

        // Slots

        // Read stdout pipe, without blocking.
        std::string readAllStandardOutput() const
        {
            DBG ("Called");
            std::string s;
            int bytes = 0;
            char c;
            struct pollfd p;

            p.fd = this->childToParent[PROCESS_READING_END];
            p.events = POLLIN | POLLPRI;
            // We know we have at least one character to read, so seed revents
            p.revents = POLLIN;
            while (p.revents & POLLIN || p.revents & POLLPRI) {
                // This read of 1 byte should never block
                if ((bytes = read (this->childToParent[PROCESS_READING_END], &c, 1)) == 1) {
                    s.append (1, c);
                }
                p.revents = 0;
                poll (&p, 1, 0);
            }
            return s;
        }

        //! Read stderr pipe without blocking
        std::string readAllStandardError() const
        {
            std::string s;
            int bytes = 0;
            char c;
            struct pollfd p;

            p.fd = this->childErrToParent[PROCESS_READING_END];
            p.events = POLLIN | POLLPRI;
            // We know we have at least one character to read, so seed revents
            p.revents = POLLIN;
            while (p.revents & POLLIN || p.revents & POLLPRI) {
                // This read of 1 byte should never block because a poll() call tells us there is data
                if ((bytes = read (this->childErrToParent[PROCESS_READING_END], &c, 1)) == 1) {
                    s.append (1, c);
                }
                p.revents = 0;
                poll (&p, 1, 0);
            }
            return s;
        }

        /*!
         * Wait for the process to get itself going. Do this by looking at pid.  If no
         * pid after a while, return false.
         */
#define START_SLEEP_PERIOD 10 // useconds
#define START_SLEEP_TOTAL 100000000 // useconds
        bool waitForStarted()
        {
            unsigned int i=0;
            // Wait for a total of START_SLEEP_TOTAL plus pauseBeforeStart useconds
            while (this->pid == 0
                   && i < ((START_SLEEP_TOTAL+this->pauseBeforeStart)/START_SLEEP_PERIOD)) {
                usleep (START_SLEEP_PERIOD);
                i++;
            }
            if (this->pid>0) {
                DBG ("The process started!");
                if (this->callbacks != nullptr) {
                    this->callbacks->startedSignal (this->progName);
                }
                this->signalledStart = true;
                return true;
            } else {
                this->error = PROCESSFAILEDTOSTART;
                if (this->callbacks != nullptr) {
                    this->callbacks->errorSignal (this->error);
                }
            }
            return false;
        }

    private:
        //! The name of the program to execute
        std::string progName;

        //! The environment and arguments of the program to execute
        std::list<std::string> environment;

        /*!
         * Number of micro seconds to pause (via a usleep() call) before forking and
         * execing the program following the call to Process::start().
         */
        unsigned int pauseBeforeStart;

        //! Holds a Process error, defined above. PROCESSNOERROR, etc.
        int error;

        //! Process ID of the program
        pid_t pid;

        /*!
         * Set to true if the fact that the program has been started has been signalled
         * using the callback callbacks->startedSignal
         */
        bool signalledStart;

        /*!
         * stdin parent to child. Initialised as zeros. If these are non-zero at
         * destruction, close() is called.
         */
        int parentToChild[2];

        /*!
         * stdout child to parent. Initialised as zeros. If these are non-zero at
         * destruction, close() is called.
         */
        int childToParent[2];

        /*!
         * stderr child to parent. Initialised as zeros. If these are non-zero at
         * destruction, close() is called.
         */
        int childErrToParent[2];

        //! Used in the poll() call in probeProcess()
        struct pollfd * p;

        //! Pointer to a callback object
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
        ProcessData()
        {
            this->errorNum = -1;
            this->stdOutReady = false;
            this->stdErrReady = false;
        }

        ~ProcessData() {};

        //! \brief Set the process finished message for a process
        void setProcessStartedMsg (const std::string& message) { this->processStartedMessage = message; }
        //! \brief Set the process finished message for a process
        void setProcessFinishedMsg (const std::string& message) { this->processFinishedMessage = message; }
        //! \brief Set the error num for a process
        void setErrorNum (const int err) { this->errorNum = err; }
        //! \brief Set stdOutReady
        void setStdOutReady (const bool ready) { this->stdOutReady = ready; }
        //! \brief Set stdErrReady
        void setStdErrReady (const bool ready) { this->stdErrReady = ready; }

        // Getters
        std::string getProcessFinishedMsg() const { return this->processFinishedMessage; }
        int getErrorNum() const { return this->errorNum; }
        bool getStdOutReady() const { return this->stdOutReady; }
        bool getStdErrReady() const { return this->stdErrReady; }

    private:
        //! \brief Holds a message that the process started
        std::string processStartedMessage;
        //! \brief Holds the name of the process that finished
        std::string processFinishedMessage;
        //! \brief Holds a process error number
        int errorNum;
        //! \brief determines if std out is ready to be read from
        bool stdOutReady;
        //! \brief determines if std err is ready to be read from
        bool stdErrReady;
    };

} // namespace morph
