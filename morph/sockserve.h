#pragma once

#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string>

namespace morph
{
    // Replaces #define BUFFERSIZE:
    const unsigned int BUFFERSIZE = 512;

    class Server
    {
    private:
        int sockfd, newsockfd, n;
        socklen_t clilen;
        char buffer[BUFFERSIZE];
        struct sockaddr_in serv_addr, cli_addr;

    public:
        Server(int);
        void closeSocket(void);
        char* exchange(void);
        void error(const char *);
    };

    class Client
    {
    private:
        int sockfd, n;
        struct sockaddr_in serv_addr;
        struct hostent *server;
        char buffer[BUFFERSIZE];

    public:
        Client(void);
        ~Client(void);
        Client(int);
        void init(int);
        void closeSocket(void);
        char* exchange(const char*);
        void error(const char *);
    };

} // namespace morph
