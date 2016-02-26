//
// Created by max on 26.02.16.
//

#ifndef SIMPLEHTTPSERVER_HTTPSERVER_H
#define SIMPLEHTTPSERVER_HTTPSERVER_H

#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>

using namespace std;

struct Options
{
    struct in_addr host_ip;
    unsigned short port;
    char *dir;
};

class HttpServer
{
public:
    HttpServer(struct Options opt)
    {
        opt_ = opt;
    }

    void start();

    void newClientRecvThr(int socket);

    string parseRequest(char *buffer);

    string genResponse(string &page);

private:
    struct Options opt_;
    int list;
};


#endif //SIMPLEHTTPSERVER_HTTPSERVER_H
