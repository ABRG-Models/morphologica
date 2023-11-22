#include "morph/Process.h"
#include "morph/MorphDbg.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <list>
#include <string>

using namespace std;

using morph::Process;
using morph::ProcessCallbacks;
using morph::ProcessData;

/*
 * Callbacks class extends ProcessCallbacks
 */
class TestProcessCallbacks : public ProcessCallbacks
{
public:
    TestProcessCallbacks (ProcessData* p) { this->parent = p; }
    void startedSignal (std::string msg) { std::cout << "Started. msg: " << msg << std::endl;}
    void errorSignal (int err) { this->parent->setErrorNum (err); }
    void processFinishedSignal (std::string msg) { this->parent->setProcessFinishedMsg (msg); }
    void readyReadStandardOutputSignal() { this->parent->setStdOutReady (true); }
    void readyReadStandardErrorSignal() { this->parent->setStdErrReady (true); }
private:
    ProcessData* parent;
};

int main()
{
    int rtn = 0;

    // Set up command and args
    list<string> args;
    args.push_back ("git");
    args.push_back ("status");
    args.push_back ("2>/dev/null");
    string command ("/usr/bin/git");

    // Create a ProcessData object to capture output from the program
    ProcessData pD;
    // Create a callbacks class that'll manage capturing the data from the program
    TestProcessCallbacks cb(&pD);
    // Instantiate the Process object
    Process p;

    try {
        // set its callbacks
        p.setCallbacks (&cb);
        // and start it
        p.start (command, args);
        p.probeProcess ();
        if (!p.waitForStarted()) {
            throw runtime_error ("Process failed to start");
        } else {
            DBG ("Process started successfully");
        }
        while (p.running() == true) {
            p.probeProcess();
        }
    } catch (const exception& e) {
        cerr << "Exception: " << e.what() << endl;
        rtn = -1;
    }

    // Do something with the output (print to screen)
    stringstream theOutput;
    theOutput << p.readAllStandardOutput();
    string line = "";
    cout << "A call to `git status` gives the following output:\n"
         << "--------------------------------------------------\n";
    while (getline (theOutput, line, '\n')) {
        cout << line << endl;
    }

    return rtn;
}
