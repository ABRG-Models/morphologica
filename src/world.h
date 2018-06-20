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
        vector<Client> ports;
        ofstream logfile;
        double TIME;
        double dt;
    };

} // namespace morph

#endif /* defined(____world__) */
