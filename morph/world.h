#pragma once

#include "sockserve.h"
#include <fstream>
#include <vector>

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
        std::vector<std::string> getCommand(std::vector<double*>);
        const char* timeStamp(void);
        void waitForConnected(void);

        const char* processName;    // process name
        int seed;
        int portID;                 // tcpip port ID
        Client supervisor;
        std::vector<Client> ports;
        std::ofstream logfile;
        double TIME;
        double dt;
    };

} // namespace morph
