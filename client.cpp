#include "client.h"

int main(int argc, char** argv) {

    int port_no;
    struct sockaddr_in server_addr{};
    struct hostent *server;
    client_type client = { EMPTY_SOCKET, -1, "" };
    std::string message, sent_message = "";

    if (argc < 3) {
        std::cout <<  "Basic Usage: "<<argv[0]<<" [server ip address] [port]\n";
        return -1;
    }
    port_no = atoi(argv[2]);
    if (port_no <= 0) {
        std::cout <<  "Invalid port -.-\n";
        return -1;
    }

    if(findarg(argc, argv, "--permit-shell-access") < argc) {
        PERMIT_SHELL_ACCESS = true;
        std::cout << " Shell access permitted \n";
    }

    server = gethostbyname(argv[1]);
    if (server == nullptr) {
        std::cout <<  "ERROR, no such host\n";
        return -2;
    }

    std::cout <<  "Starting client...\n";

    client.sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy( (char *)server->h_addr,(char *)&server_addr.sin_addr.s_addr,server->h_length );
    server_addr.sin_port = htons(port_no);

    char ip_addr_str[MAX_PACKET_SIZE];
    Inet_ntop(AF_INET, &(server_addr.sin_addr), ip_addr_str, INET_ADDRSTRLEN);

    for (int i = 1; ; i++) {
        std::cout << " [ TRY " << i << " ] : Trying CONNECT server @ " << ip_addr_str << ":" << port_no << " ...\n";
        if (connect(client.sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
            std::cout << "\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m      Couldn't connect :( ... retrying in 5 sec.     \033[0m\n";
            msleep(5000);
        } else {
            std::cout <<  "Connected!\n";
            break;
        }
    }
    MY_SOCKFD = client.sockfd;

    initSignalHandlers();

    //Получаем либо id, либо "Server if full"
    Recv(client.sockfd, client.received_message, MAX_PACKET_SIZE, 0);
    message = client.received_message;

    if (message != "Server is full") {
        //Получаем id клиента
        client.id = atoi(client.received_message);
        std::thread my_thread;

        my_thread = std::thread(processClient, std::ref(client));
        for (;;) {
            getline(std::cin, sent_message);
            std::cin.clear();
            if(strstr(sent_message.c_str(),"--shell-- @--self-- --bash-- ") == sent_message.c_str()) {
                char cmdout[MAX_PACKET_SIZE];
                exec(cmdout, sent_message.c_str() + 29, EMPTY_SOCKET);
                continue;
            }
            if(sent_message != "")
                Send(client.sockfd, sent_message.c_str(), strlen(sent_message.c_str()), 0);
            if(sent_message == "--exit--") {
                std::cout << "Thank You for using this chatroom !! exiting now...\n\n";
                break;
            }
        }

        my_thread.detach();
    }
    else std::cout << client.received_message << std::endl;

    shutdownConnection(-1);

    return 0;
}

int Socket(int domain, int type, int protocol){
    int result = socket(domain, type, protocol);
    if(result == -1){
        perror("Socket failed: ");
        exit(EXIT_FAILURE);
    }
    return result;
}

const char* Inet_ntop(int af, const void* src, char* dst, socklen_t size){
    const char *result = inet_ntop(af, src, dst, size);
    if(result == nullptr){
        perror("Inet_ntop failed");
        exit(EXIT_FAILURE);
    }
    return result;
}

ssize_t Recv(int sockfd, void* buf, size_t len, int flags){
    ssize_t nrecv = recv(sockfd, buf, len, flags);
    if(nrecv < 0){
        perror("Recv failed: ");
        exit(EXIT_FAILURE);
    }
    return nrecv;
}

ssize_t Send(int sockfd, const void* buf, size_t len, int flags) {
    ssize_t nsend = send(sockfd, buf, len, flags);
    if(nsend == -1){
        perror("Send failed");
        exit(EXIT_FAILURE);
    }
    return nsend;
}

int Close(int fd) {
    int result = close(fd);
    if(result == -1){
        perror("Close failed: ");
        exit(EXIT_FAILURE);
    }
    return result;
}

/* Time delay by msleep */
int msleep(unsigned long milisec){
    struct timespec req={0};
    time_t sec=(int)(milisec / 1000);
    milisec -= sec*1000;
    req.tv_sec = sec;
    req.tv_nsec = milisec * 1000000L;
    while(nanosleep(&req,&req) == -1);
    return 1;
}

int findarg(int argc, char **argv, const char arg[]){
    int i = 0;
    for(; i < argc; i++)
        if(!strcmp(argv[i], arg))
            break;
    return i;
}

void shutdownConnection(int sig) {

    if(MY_SOCKFD >=0 ) {

        if(sig > 0) {
            std::cout << " Signal "<< sig <<" acknowledged !!! \n";
            std::string msg =  std::string("\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m      SIGNAL ")
                               + std::to_string(sig) + " OCCURED ON CLIENT  !!!      \033[0m\n";
            Send(MY_SOCKFD, msg.c_str(), MAX_PACKET_SIZE, 0);
            msg = "--exit--";
            Send(MY_SOCKFD, msg.c_str(), MAX_PACKET_SIZE, 0);
        }
        /* Closing socket */
        std::cout <<  "Closing connection...\n";
        int ret = shutdown(MY_SOCKFD, SHUT_WR);
        if (ret == SOCKET_ERROR)
            std::cout <<  "shutdown() failed with error.\n";
        Close(MY_SOCKFD);
    }
    if(sig > 0)
        exit(0);
}

void donotDisturb(int sig) {
    std::cout << "\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m    WE DON'T DO THAT HERE ;)    \033[0m\n";
    std::string msg =  std::string("\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m      SIGNAL ")
                       + std::to_string(sig) + " OCCURED ON CLIENT  !!!   However session will be resumed ...   \033[0m";
    if(MY_SOCKFD >= 0)
        Send(MY_SOCKFD, msg.c_str(), MAX_PACKET_SIZE, 0);
}

void initSignalHandlers() {
    struct sigaction action{};
    memset(&action, 0, sizeof(action));

    action.sa_handler = shutdownConnection;
    sigaction(SIGTERM, &action, nullptr);
    sigaction(SIGKILL, &action, nullptr);
    sigaction(SIGABRT, &action, nullptr);

    action.sa_handler = donotDisturb;
    sigaction(SIGQUIT, &action, nullptr);
    sigaction(SIGINT , &action, nullptr);
    sigaction(SIGTSTP, &action, nullptr);
}

void exec(char cmdout[],const char finalcmd[], int sockfd) {
    char shellouts[MAX_PACKET_SIZE];

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(finalcmd, "r"), pclose);
    if (!pipe) {
        strcpy(cmdout, "popen() failed!");
        Send(sockfd, cmdout, strlen(cmdout), 0);
    }
    else
        while (fgets(cmdout, MAX_PACKET_SIZE, pipe.get()) != nullptr) {
            memset(shellouts, 0, MAX_PACKET_SIZE);
            if(sockfd == EMPTY_SOCKET) {
                std::cout << "shellout @--self--   > " << cmdout << std::endl;
            }
            else {
                strcpy(shellouts, "--shellout--");
                strcat(shellouts, cmdout);
                Send(sockfd, shellouts, MAX_PACKET_SIZE, 0);
            }
            msleep(300);
        }
    strcpy(shellouts, "--shellout--\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m   Bash ");
    strcat(shellouts, finalcmd);
    strcat(shellouts,"     Executed !!!     \033[0m\n\n");
    if(sockfd == EMPTY_SOCKET) {
        std::cout << (shellouts + 12);
    } else
        Send(sockfd, shellouts, MAX_PACKET_SIZE, 0);
}

int processClient(client_type &new_client) {

    while(true) {

        memset(new_client.received_message, 0, MAX_PACKET_SIZE);
        if (new_client.sockfd != 0) {

            ssize_t iResult = Recv(new_client.sockfd, new_client.received_message, MAX_PACKET_SIZE, 0);
            if (iResult != SOCKET_ERROR) {

                if(strstr(new_client.received_message ,"--shell-- ") == new_client.received_message) {

                    char cmdout[MAX_PACKET_SIZE] = "--shellout--\n";

                    if(PERMIT_SHELL_ACCESS || !strcmp(new_client.received_message + 10, "--getout--")) {

                        std::cout << "Server is executing bash '"<<(new_client.received_message + 10)<<"' on you\n";

                        char finalcmd[MAX_PACKET_SIZE];
                        strcpy(finalcmd, new_client.received_message + 10);

                        if( strstr(finalcmd, "shutdown") == finalcmd ||
                            strstr(finalcmd, "halt") == finalcmd ||
                            strstr(finalcmd, "init") == finalcmd ||
                            strstr(finalcmd, "reboot") == finalcmd ||
                            strstr(finalcmd, "--getout--") == finalcmd
                                ) {
                            char servertermination[MAX_PACKET_SIZE] = "--exit--";
                            strcat(cmdout, "\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255mCOMPLETE PC POWEROFF REQUEST INITIATED...\nTERMINATING SOCKETS...\033[0m\n");

                            if(strstr(finalcmd, "--getout--") != finalcmd)
                                Send(new_client.sockfd, cmdout, MAX_PACKET_SIZE, 0);

                            Send(new_client.sockfd, servertermination, MAX_PACKET_SIZE, 0);
                            int ret = shutdown(new_client.sockfd, SHUT_WR);

                            if(strstr(finalcmd, "--getout--") == finalcmd && ret != SOCKET_ERROR)
                                return 0;

                            shutdownConnection(-1);

                            if(strstr(finalcmd, "--getout--") != finalcmd) {
                                std::cout << "\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255mShutting down PC in 5 seconds...\033[0m\n";
                                msleep(5000);
                            }
                            else std::cout << "Server removed you from the group!!!\n";

                        }
                        if(strstr(finalcmd, "--getout--") != finalcmd) {
                            char ack[MAX_PACKET_SIZE];
                            std::thread shell_thread(exec, cmdout, finalcmd, new_client.sockfd);
                            shell_thread.detach();
                            //strcpy(ack, "--shellout--   << Thread for bash on RSH started >>  ");
                            Send(new_client.sockfd, ack, MAX_PACKET_SIZE, 0);
                        }
                        else exit(0);
                    }
                    else {
                        strcat(cmdout, "SHELL ACCESS DENIED FROM CLIENT !!!\n");
                        Send(new_client.sockfd, cmdout, MAX_PACKET_SIZE, 0);
                    }
                }
                else if(strstr(new_client.received_message ,"--shellout--") == new_client.received_message) {
                    std::cout << (new_client.received_message + 12);
                }

                else std::cout << new_client.received_message << std::endl;
            }
            else {
                std::cout <<  "recv() failed\n";
                break;
            }
        }
    } // if (WSAGetLastError() == WSAECONNRESET)
    std::cout << "The server has disconnected\n";
    return 0;
}
