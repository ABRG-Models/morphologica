//
//  world.cpp
//

#include "world.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>

using namespace std;

morph::World::World (const char* processName,
                     const char* logfileLocation,
                     int seed,
                     int portID,
                     double dt)
{
    this->processName=processName; // process name
    stringstream ss;          // logfile location
    ss<<logfileLocation;           // logfile location
    srand(seed);                   // random seed
    this->portID=portID;           // tcpip port ID
    this->master.init(portID);
    vector<Client> ports;          // Remember this can be used for multi inputs

    TIME = 0;
    this->dt = dt;

    logfile.open(ss.str().c_str(),ios::out|ios::app);
    ss.clear();
    time_t timer = time(NULL);
    logfile<<"*********"<<endl;
    logfile<<"   HI!"<<endl;
    logfile<<"*********"<<endl;
    logfile<<"Time now: "<<timer<<endl;
    logfile<<"Sim name: "<<processName<<endl;
    logfile<<"**********"<<endl<<flush;
};

vector<string>
morph::World::getCommand (vector<double*> msgOut)
{
    stringstream out;
    out.clear();
    out.setf(ios::fixed,ios::floatfield);

    for(unsigned int i=0;i<msgOut.size();i++){
        out<<*msgOut[i]<<",";
    }

    vector <string> command;
    string messageI=master.exchange(out.str().c_str());
    stringstream ss(messageI);
    while (ss.good()){
        string substr;
        getline(ss,substr,',');
        command.push_back(substr);
    } ss.clear();

    return command;
};

const char*
morph::World::timeStamp (void)
{
    const char* TIMEcs;
    stringstream TIMEss;
    TIMEss<<setw(10)<<setfill('0')<<TIME;
    TIMEcs = TIMEss.str().c_str();
    return TIMEcs;
}

morph::World::~World()
{
    logfile<<"*********"<<endl;
    logfile<<"   FIN"<<endl;
    logfile<<"*********"<<endl<<flush;

    logfile.close();
    // master.closeSocket();
    master.~Client();

    for(unsigned int i=0;i<ports.size();i++){
        // ports[i].closeSocket();
        ports[i].~Client();
    }
};
