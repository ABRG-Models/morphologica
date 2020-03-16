//
//  world.h
//

#ifndef ____world__
#define ____world__

#include "sockserve.h"
#include <fstream>
#include <vector>

using std::vector;
using std::ofstream;
using std::string;

namespace morph
{
    class World
    {
    public:
        World(const char* processName,
              const char* logfileLocation,
              int seed,
              int portID,
              double dt);

        /*!
         * Construct a World that won't do any network comms
         */
        World(const char* processName,
              const char* logfileLocation,
              int seed,
              double dt);

        virtual ~World();
        vector<string> getCommand(vector<double*>);
        const char* timeStamp(void);
        void waitForConnected(void);

        const char* processName;    // process name
        int seed;
        int portID;                 // tcpip port ID
        Client master;
        vector<Client> ports;
        ofstream logfile;
        double TIME;
        double dt;
    };

} // namespace morph

#endif /* defined(____world__) */
