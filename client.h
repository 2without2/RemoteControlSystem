#ifndef COURSEPROJECT_CLIENT_H
#define COURSEPROJECT_CLIENT_H

#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <csignal>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <netdb.h>

#define MAX_PACKET_SIZE 2048

struct client_type {
    int id;
    int sockfd;
    char received_message[MAX_PACKET_SIZE];
};

const int EMPTY_SOCKET = -1,
        SOCKET_ERROR = -1;
int MY_SOCKFD = -1;
bool PERMIT_SHELL_ACCESS = false;

//FUNCTION
int msleep(unsigned long milisec);

int findarg(int argc, char **argv, const char arg[]);

void shutdownConnection(int signum);

void donotDisturb(int signum);

void initSignalHandlers();

void exec(char cmdout[],const char finalcmd[], int sockfd);

int processClient(client_type &new_client);

int Socket(int domain, int type, int protocol);

const char* Inet_ntop(int af, const void* src, char* dst, socklen_t size);

ssize_t Recv(int sockfd, void* buf, size_t len, int flags);

ssize_t Send(int sockfd, const void* buf, size_t len, int flags);

int Close(int fd);

#endif
