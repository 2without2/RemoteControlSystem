#ifndef COURSEPROJECT_SERVER_H
#define COURSEPROJECT_SERVER_H

#include <cstdio>
#include <iostream>
#include <fstream> // for logfile
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
#include <ctime>
#include <vector>

//DEFINE
#define STYLE "\033[0m\033[1;94m\033[38;2;192;192;192m"
#define COLS 10
#define MAX_PACKET_SIZE 4096
//INFORMATION

const char banner[] = STYLE"\n\n|------------------------------------------------------------------------------|\033[0m\n"
                      STYLE"|                                                                              |\033[0m\n"
                      STYLE"|                                Курсовая работа                               |\033[0m\n"
                      STYLE"|                          Система удаленного управления                       |\033[0m\n"
                      STYLE"|                                                                              |\033[0m\n"
                      STYLE"|------------------------------------------------------------------------------|\033[0m\n"
                      STYLE"|  1)   Введите сообщение и нажмите enter, чтобы отправить его.                |\033[0m\n"
                      STYLE"|  2)   Для того, чтобы выполнить команду для пользователя, который разрешает  |\033[0m\n"
                      STYLE"|     доступ к своей оболочке, используйте:                                    |\033[0m\n"
                      STYLE"|  --shell-- @[имя пользователя или --all--] --bash-- [команда для выполнения] |\033[0m\n"
                      STYLE"|       Если вы в \"команде для выполнения\" введёте команду --getout--, то вы   |\033[0m\n"
                      STYLE"|     просто разорвете клиентское соединение. Если вам это не удастся, то вы   |\033[0m\n"
                      STYLE"|     увидите сообщение об ошибке.                                             |\033[0m\n"
                      STYLE"|  3)   Чтобы посмотреть список всех онлайн пользователей, введите команду     |\033[0m\n"
                      STYLE"|     --anyonehere--. Вы получите список всех доступных пользователей с        |\033[0m\n"
                      STYLE"|     указанием их ip-адреса.                                                  |\033[0m\n"
                      STYLE"|  4)   Чтобы выйти из комнаты, просто отправьте команду --exit-- на сервер,   |\033[0m\n"
                      STYLE"|     чтобы он разорвал ваше соединение.                                       |\033[0m\n"
                      STYLE"|------------------------------------------------------------------------------|\033[0m\n\n"
                           "Пожалуйста, введите свое уникальное имя пользователя, чтобы другие участники \n  комнаты могли идентифицировать вас: ";

const  std::string cols[COLS] = {
        "\033[38;2;0;255;0m",
        "\033[38;2;0;0;255m",
        "\033[38;2;255;0;0m",
        "\033[38;2;255;255;0m",
        "\033[38;2;0;255;255m",
        "\033[38;2;255;0;255m",
        "\033[38;2;255;128;0m",
        "\033[38;2;128;255;0m",
        "\033[38;2;0;128;255m",
        "\033[38;2;0;255;128m"
};


//GLOBAL VAR
std::fstream logfile;
const int EMPTY_SOCKET = -1;
const int SOCKET_ERROR = -1;
const int MAX_CLIENTS = 100;
bool FATAL_TERMINATE = false;

//STRUCT
struct client_type {
    int id = 0;
    int sockfd = 0;
    int master_sockfd = 0; //???????????????????????????????????????????????????????
    std::string uname;
    std::string ip_addr_str;
};

//CONTAINERS
std::vector<client_type> client_array(MAX_CLIENTS);

//FUNCTIONS
void showOnlineUsers(int sockfd);

bool isClientArrayEmpty();

void shutdownAllConnections(int signum);

void donotDisturb(int signum);

void initSignalHandlers();

int processClient(client_type &new_client, std::thread &self_thread);

bool checkSeparatorTags(char *temp_msg, int sockfd);

//WRAPPER FUNCTIONS

int Socket(int domain, int type, int protocol);

int Bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen);

int System(const char* command);

int Listen(int sockfd, int backlog);

const char* Inet_ntop(int af, const void* src, char* dst, socklen_t size);

int Accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen);

ssize_t Send(int sockfd, const void* buf, size_t len, int flags);

ssize_t Recv(int sockfd, void* buf, size_t len, int flags);

int Close(int fd);


#endif //COURSEPROJECT_SERVER_H