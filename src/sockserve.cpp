#include "sockserve.h"
#include <string.h>
#include <stdio.h>

morph::Server::Server (int portno)
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error ("Server::Server: Error opening socket");
    }
    bzero ((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons (portno);
    if (bind (sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error ("Server::Server: Error on binding");
    }
    listen (sockfd,5);
    clilen = sizeof(cli_addr);
    newsockfd = accept (sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
        error ("Server::Server: Error on accept");
    }
}

void
morph::Server::closeSocket (void)
{
    close (newsockfd);
    close (sockfd);
}

char*
morph::Server::exchange (void)
{
    bzero (buffer, strlen(buffer));
    n = read (newsockfd, buffer, strlen(buffer));
    if (n < 0) {
        error ("Server::exchange: Error reading from socket");
    }
    n = write (newsockfd, "Received: ", strlen(buffer));
    if (n < 0) {
        error ("Server::exchange: Error writing to socket");
    }
    return buffer;
}

void
morph::Server::error (const char *msg)
{
    perror (msg);
    exit (1);
}

morph::Client::Client (int portno)
{
    this->init (portno);
}

morph::Client::Client (void)
{
}

void
morph::Client::init (int portno)
{
    sockfd = socket (AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error ("Client::init: Error opening socket");
    }
    server = gethostbyname ("localhost");
    if (server == NULL) {
        fprintf (stderr, "Client::init: Error, no such host\n");
        exit(0);
    }
    bzero ((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy ((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons (portno);
    if (connect (sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error ("Client::init: Error connecting");
    }
}

void
morph::Client::closeSocket (void)
{
    close (sockfd);
}

morph::Client::~Client (void)
{
    close (sockfd);
}

char*
morph::Client::exchange (const char* message)
{
    bzero (buffer, morph::BUFFERSIZE);
    memcpy (&buffer[0], message, strlen(message));
    n = write (sockfd, buffer, strlen(buffer));
    if (n < 0) {
        error ("Client::exchange: Error writing to socket");
    }
    bzero (buffer, morph::BUFFERSIZE);
    n = read (sockfd, buffer, morph::BUFFERSIZE);
    if (n < 0) {
        error ("Client::exchange: Error reading from socket");
    }
    return buffer;
}

void
morph::Client::error (const char *msg)
{
    perror (msg);
    exit (1);
}
