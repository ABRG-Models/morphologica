//
//  world.h
//

#ifndef ____world__
#define ____world__

#include "sockserve.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>

using namespace std;

class world
{
public:
    world(const char*,
          const char*,
          int,
          int,
          double);

    virtual ~world();
    vector<string> getCommand(vector<double*>);
    const char* timeStamp(void);
    void waitForConnected(void);

    const char* processName;    // process name
    int seed;
    int portID;                 // tcpip port ID
    Client master;
    std::vector<Client> ports;
    std::ofstream logfile;
    double TIME;
    double dt;
};

#endif /* defined(____world__) */
