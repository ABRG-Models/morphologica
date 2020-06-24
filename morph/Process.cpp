/*
 * Process is a class to fork and exec processes.
 *
 *  Author: Seb James <seb.james@sheffield.ac.uk>
 */

#include <iostream>
#include <sstream>
extern "C" {
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <signal.h>
}

#include "morph/MorphDbg.h"
#include "morph/Process.h"

using namespace std;

/*!
 * Implementation of the Process class
 */
//@{

// Used when dealing with pipes
#define READING_END 0
#define WRITING_END 1
#define STDIN  0
#define STDOUT 1
#define STDERR 2

// Constructor
morph::Process::Process () :
        progName("unknown"),
        pauseBeforeStart(0),
        error (PROCESSNOERROR),
        pid(0),
        signalledStart(false)
{
        // Set up the polling structs
        this->p = static_cast<struct pollfd*>(malloc (2*sizeof (struct pollfd)));

        // Initialise the callbacks to 0
        this->callbacks = (ProcessCallbacks*)0;

        // Initialise pipes to 0
        this->parentToChild[WRITING_END] = 0;
        this->parentToChild[READING_END] = 0;
        this->childToParent[READING_END] = 0;
        this->childToParent[WRITING_END] = 0;
        this->childErrToParent[READING_END] = 0;
        this->childErrToParent[WRITING_END] = 0;
}

// Destructor
morph::Process::~Process ()
{
        // Close pipes
        this->closeAllFileDescriptors();
        // Free polling struct
        free (this->p);
}

void
morph::Process::closeAllFileDescriptors (void)
{
        if (this->parentToChild[WRITING_END] > 0) {
                DBG ("close parentToChild[WRITING_END]...");
                if (close (this->parentToChild[WRITING_END])) {
                        // Normally succeeds
                        DBG ("Failed to close parentToChild[WRITING_END]");
                }
                this->parentToChild[WRITING_END] = 0;
        }

        if (this->parentToChild[READING_END] > 0) {
                DBG ("Unexpectedly closing parentToChild[READING_END]...");
                if (close (this->parentToChild[READING_END])) {
                        // Normally already closed in Process::start
                        DBG ("Failed to close parentToChild[READING_END]");
                }
                this->parentToChild[READING_END] = 0;
        }

        if (this->childToParent[READING_END] > 0) {
                DBG ("close childToParent[READING_END]...");
                if (close (this->childToParent[READING_END])) {
                        // Normally succeeds
                        DBG ("Failed to close childToParent[READING_END]");
                }
                this->childToParent[READING_END] = 0;
        }

        if (this->childToParent[WRITING_END] > 0) {
                DBG ("Unexpectedly closing childToParent[WRITING_END]...");
                if (close (this->childToParent[WRITING_END])) {
                        // Normally already closed in Process::start
                        DBG ("Failed to close childToParent[WRITING_END]");
                }
                this->childToParent[WRITING_END] = 0;
        }

        if (this->childErrToParent[READING_END] > 0) {
                DBG ("close childErrToParent[READING_END]...");
                if (close (this->childErrToParent[READING_END])) {
                        // Normally succeeds
                        DBG ("Failed to close childErrToParent[READING_END]");
                }
                this->childErrToParent[READING_END] = 0;
        }

        if (this->childErrToParent[WRITING_END] > 0) {
                DBG ("Unexpectedly closing childErrToParent[READING_END]...");
                if (!close (this->childErrToParent[WRITING_END])) {
                        // Normally already closed in Process::start
                        DBG ("Failed to close childErrToParent[READING_END]");
                }
                this->childErrToParent[WRITING_END] = 0;
        }
}

bool
morph::Process::reset (bool keepCallbacks)
{
        if (this->running()) {
                return false;
        }
        if (!keepCallbacks && this->callbacks != (ProcessCallbacks*)0) {
                DBG ("Resetting callbacks pointer");
                this->callbacks = (ProcessCallbacks*)0;
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

void
morph::Process::writeIn (const string& input) const
{
        write (this->parentToChild[WRITING_END], input.c_str(), input.size());
}

void
morph::Process::setPauseBeforeStart (const unsigned int useconds)
{
        this->pauseBeforeStart = useconds;
}

// fork and exec a new process using execv, which takes stdin via a
// fifo and returns output also via a fifo.
int
morph::Process::start (const string& program, const list<string>& args)
{
        char** argarray;
        list<string> myargs = args;
        list<string>::iterator i;
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
                DBG ("Failed to set up pipes, return PROCESS_FAILURE" << flush);
                this->error = PROCESSNOMOREPIPES;
                return PROCESS_FAILURE;
        }

        this->pid = fork();
        switch (this->pid) {
        case -1:
                DBG ("fork() returned -1, return PROCESS_FAILURE" << flush);
                this->error = PROCESSFORKFAILED;
                return PROCESS_FAILURE;
        case 0:
                // This is the CHILD process

                // Close unwanted ends of the pipes
                close (this->parentToChild[WRITING_END]);
                this->parentToChild[WRITING_END] = 0;
                close (this->childToParent[READING_END]);
                this->childToParent[READING_END] = 0;
                close (this->childErrToParent[READING_END]);
                this->childErrToParent[READING_END] = 0;

                // Now all we have to do is make the writing file
                // descriptors 0,1 or 2 and they will be used instead
                // of stdout, stderr and stdin.
                if ((dup2 (this->parentToChild[READING_END], STDIN)) == -1  ||
                    (dup2 (this->childToParent[WRITING_END], STDOUT)) == -1 ||
                    (dup2 (this->childErrToParent[WRITING_END], STDERR)) == -1) {
                        theError = errno;
                        cout << "ERROR! Couldn't get access to stdin/out/err! errno was " << theError << endl;
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
                cout << "Process error: " << this->pid << " crashed. errno:" << theError << endl;

                // This won't get picked up by the parent process.
                this->error = PROCESSCRASHED;

                // In this case, we close the pipes to signal to the parent that we crashed
                close (this->parentToChild[READING_END]);
                this->parentToChild[READING_END] = 0;
                close (this->childToParent[WRITING_END]);
                this->childToParent[WRITING_END] = 0;
                close (this->childErrToParent[WRITING_END]);
                this->childErrToParent[WRITING_END] = 0;

                // Child should exit now.
                _exit(-1);

        default:
                // This is the PARENT process

                // Close unwanted ends of the pipes
                close (this->parentToChild[READING_END]);
                this->parentToChild[READING_END] = 0;
                close (this->childToParent[WRITING_END]);
                this->childToParent[WRITING_END] = 0;
                close (this->childErrToParent[WRITING_END]);
                this->childErrToParent[WRITING_END] = 0;

                // Write to this->parentToChild[WRITING_END] to write to stdin of the child
                // Read from this->childToParent[READING_END] to read from stdout of child
                // Read from this->childErrToParent[READING_END] to read from stderr of child

                break;
        }
        return PROCESS_MAIN_APP;
}

#define START_SLEEP_PERIOD 10 // useconds
#define START_SLEEP_TOTAL 100000000 // useconds
// If no pid after a while, return false.
bool
morph::Process::waitForStarted (void)
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
                if (this->callbacks != (ProcessCallbacks*)0) {
                        this->callbacks->startedSignal (this->progName);
                }
                this->signalledStart = true;
                return true;
        } else {
                this->error = PROCESSFAILEDTOSTART;
                if (this->callbacks != (ProcessCallbacks*)0) {
                        this->callbacks->errorSignal (this->error);
                }
        }
        return false;
}

// Send a TERM signal to the process.
void
morph::Process::terminate (void)
{
        kill (this->pid, 15); // 15 is TERM
        // Now check if the process has gone and kill it with signal 9 (KILL)
        this->pid = 0;
        this->error = PROCESSNOERROR;
        this->signalledStart = false;
        return;
}

// Check on this process
void
morph::Process::probeProcess (void)
{
        // Has the process started?
        if (!this->signalledStart) {
                if (this->pid > 0) {
                        if (this->callbacks != (ProcessCallbacks*)0) {
                                this->callbacks->startedSignal (this->progName);
                        }
                        this->signalledStart = true;
                        DBG ("Process::probeProcess set signalledStart and signalled the start...");
                }
        }

        // Check for error condition
        if (this->error>0) {
                if (this->callbacks != (ProcessCallbacks*)0) {
                        this->callbacks->errorSignal (this->error);
                }
                DBG ("have error in probeProcess, returning");
                return;
        }

        if (this->pid == 0) {
                // Not yet started.
                return;
        }

        // Why can't these 4 lines go in contructor? Because
        // this->childToParent etc aren't set up until Process::start().
        this->p[0].fd = this->childToParent[READING_END];
        this->p[0].events = POLLIN | POLLPRI;
        this->p[1].fd = this->childErrToParent[READING_END];
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
                if (this->callbacks != (ProcessCallbacks*)0) {
                        this->callbacks->errorSignal (this->error);
                }
                return;
        }

        if (this->p[0].revents & POLLIN || this->p[0].revents & POLLPRI) {
                if (this->callbacks != (ProcessCallbacks*)0) {
                        this->callbacks->readyReadStandardOutputSignal();
                }
        }
        if (this->p[1].revents & POLLIN || this->p[1].revents & POLLPRI) {
                if (this->callbacks != (ProcessCallbacks*)0) {
                        this->callbacks->readyReadStandardErrorSignal();
                }
        }

        // Is the process running? We check last, so that we get any
        // messages on stdout/stderr that we may wish to process, such
        // as error messages from nxssh key authentication.
        int theError;
        if (this->signalledStart == true) {
                int rtn = 0;
                if ((rtn = waitpid (this->pid, (int *)0, WNOHANG)) == this->pid) {
                        if (this->callbacks != (ProcessCallbacks*)0) {
                                this->callbacks->processFinishedSignal (this->progName);
                        }
                        this->pid = 0;
                        DBG ("Process finished, returning");
                        return;
                } else if (rtn == -1) {
                        theError = errno;
                        if (theError != 10) { // We ignore errno 10 "no child" as this commonly occurs
                                cerr << "waitpid returned errno: " << theError;
                        }
                } // else rtn == 0
        }

        return;
}

// Read stdout pipe, without blocking.
string
morph::Process::readAllStandardOutput (void) const
{
        DBG ("Called");
        string s;
        int bytes = 0;
        char c;
        struct pollfd p;

        p.fd = this->childToParent[READING_END];
        p.events = POLLIN | POLLPRI;
        // We know we have at least one character to read, so seed revents
        p.revents = POLLIN;
        while (p.revents & POLLIN || p.revents & POLLPRI) {
                // This read of 1 byte should never block
                if ((bytes = read (this->childToParent[READING_END], &c, 1)) == 1) {
                        s.append (1, c);
                }
                p.revents = 0;
                poll (&p, 1, 0);
        }
        return s;
}

// Read stderr pipe without blocking
string
morph::Process::readAllStandardError (void) const
{
        string s;
        int bytes = 0;
        char c;
        struct pollfd p;

        p.fd = this->childErrToParent[READING_END];
        p.events = POLLIN | POLLPRI;
        // We know we have at least one character to read, so seed revents
        p.revents = POLLIN;
        while (p.revents & POLLIN || p.revents & POLLPRI) {
                // This read of 1 byte should never block because a poll() call tells us there is data
                if ((bytes = read (this->childErrToParent[READING_END], &c, 1)) == 1) {
                        s.append (1, c);
                }
                p.revents = 0;
                poll (&p, 1, 0);
        }
        return s;
}

pid_t
morph::Process::getPid (void) const
{
        return this->pid;
}

bool
morph::Process::running (void) const
{
        if (this->pid > 0) {
                return true;
        }
        return false;
}

int
morph::Process::getError (void) const
{
        return this->error;
}

void
morph::Process::setError (const int e)
{
        this->error = e;
}

void
morph::Process::setCallbacks (ProcessCallbacks * cb)
{
        this->callbacks = cb;
}

//@}

/*!
 * Implementation of ProcessData class
 */
//@{
morph::ProcessData::ProcessData (void)
{
        this->errorNum = -1;
        this->stdOutReady = false;
        this->stdErrReady = false;
}

morph::ProcessData::~ProcessData (void)
{
}

void
morph::ProcessData::setProcessFinishedMsg (const string& message)
{
        this->processFinishedMessage = message;
}

void
morph::ProcessData::setErrorNum (const int err)
{
        this->errorNum = err;
}

void
morph::ProcessData::setStdOutReady (const bool ready)
{
        this->stdOutReady = ready;
}

void
morph::ProcessData::setStdErrReady (const bool ready)
{
        this->stdErrReady = ready;
}

string
morph::ProcessData::getProcessFinishedMsg (void) const
{
        return this->processFinishedMessage;
}

int
morph::ProcessData::getErrorNum (void) const
{
        return this->errorNum;
}

bool
morph::ProcessData::getStdOutReady (void) const
{
        return this->stdOutReady;
}

bool
morph::ProcessData::getStdErrReady (void) const
{
        return this->stdErrReady;
}

//@}
