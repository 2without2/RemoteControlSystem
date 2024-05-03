#include "server.h"

int main(int argc, char** argv) {

    int sockfd, port_no, num_clients = 0, temp_id = -1;
    struct sockaddr_in server_addr{}, client_addr{};
    socklen_t client_len;
    std::string msg = "";
    std::thread client_threads[MAX_CLIENTS];

    initSignalHandlers();

    if (argc < 2) {
        std::cerr <<  "ERROR, no port provided\n";
        return -1;
    }
    port_no = atoi(argv[1]);
    if (port_no <= 0) {
        std::cerr <<  "Invalid port -.-\n";
        return -1;
    }

    std::cout <<  "Creating server socket...\n";
    sockfd =  Socket(AF_INET, SOCK_STREAM, 0);

    bzero((char *) &server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_no);

    Bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr));

    System(" if ! [ -d ./logs ]\n "
           "   then \n"
           "   mkdir logs\n "
           " fi\n"
    );

    {
        time_t now = time(nullptr);
        tm* ltm = localtime(&now);

        logfile.open(std::string("logs/") + (std::to_string(port_no) + "."
                        + std::to_string(ltm->tm_mday) + "." + std::to_string(ltm->tm_mon) + "."
                        + std::to_string(ltm->tm_sec) + ".log").c_str(),std::ios::out | std::ios::ate);
    }

    Listen(sockfd, 5);
    char local_host[INET_ADDRSTRLEN];
    Inet_ntop(AF_INET, &(server_addr.sin_addr), local_host, INET_ADDRSTRLEN);
    std::string s = std::string(" Listening on ") + local_host + ":" + std::to_string(port_no) + "\n";
    std::cout << s; logfile << s;
    std::cout << " Current ip addresses of server :\n";
    int ret = System("hostname -I");

    std::cout<<" \033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m  Press Ctrl + C to terminate the server ( Will work only if no users are connected... ) \033[0m\n"<<ret;

    for (int i = 0; i < MAX_CLIENTS; i++){
        client_array[i].id = -1;
        client_array[i].sockfd = EMPTY_SOCKET;
        client_array[i].master_sockfd = EMPTY_SOCKET;
        client_array[i].uname = "";
        client_array[i].ip_addr_str = "";
    }
    client_len = sizeof(client_addr);

    while(true) {

        int incoming = EMPTY_SOCKET;
        incoming = Accept(sockfd, (struct sockaddr *) &client_addr, &client_len);
        if (incoming == EMPTY_SOCKET) continue;

        num_clients = -1;

        temp_id = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_array[i].sockfd == EMPTY_SOCKET && temp_id == -1) {
                client_array[i].sockfd = incoming;
                client_array[i].id = i; temp_id = i;
            }
            if (client_array[i].sockfd != EMPTY_SOCKET)
                num_clients++;
        }
        if (temp_id != -1) {
            char remote_host[INET_ADDRSTRLEN];
            Inet_ntop(AF_INET, &(client_addr.sin_addr), remote_host, INET_ADDRSTRLEN);
            std::string temp_msg = std::string("Connection accepted from ")
                                   + remote_host + ":" + std::to_string(ntohs(client_addr.sin_port))
                                   + " (Client #" + std::to_string(client_array[temp_id].id + 1)
                                   + ") accepted\n";
            std::cout << temp_msg;
            logfile << temp_msg;
            client_array[temp_id].ip_addr_str = std::string(remote_host) + ":" + std::to_string(ntohs(client_addr.sin_port));

            msg =  std::to_string(client_array[temp_id].id);
            Send(client_array[temp_id].sockfd, msg.c_str(), strlen(msg.c_str()), 0);

            client_threads[temp_id] =  std::thread(processClient, std::ref(client_array[temp_id]), ref(client_threads[temp_id]));
        }
        else {

            msg = "Server is full";
            Send(incoming, msg.c_str(), strlen(msg.c_str()), 0);
            char remote_host[INET_ADDRSTRLEN];
            Inet_ntop(AF_INET, &(client_addr.sin_addr), remote_host, INET_ADDRSTRLEN);
            std::string temp_msg = std::string("Connection rejected to ")
                                   + remote_host + ":" + std::to_string(ntohs(client_addr.sin_port))
                                   + " (Client #" + std::to_string(client_array[temp_id].id + 1)
                                   + "). The server is full!\n";
            std::cout << temp_msg;
            logfile << temp_msg;

            Close(client_array[temp_id].sockfd);
        }
    }
}

int Socket(int domain, int type, int protocol){
    int result = socket(domain, type, protocol);
    if(result == -1){
        perror("Socket failed: ");
        exit(EXIT_FAILURE);
    }
    return result;
}

int Bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen){
    int result = bind(sockfd, addr, addrlen);
    if(result == -1){
        perror("Bind failed: ");
        exit(EXIT_FAILURE);
    }
    return result;
}

int System(const char* command){
    int result = system(command);
    if(result != 0){
        std::cout << "System failed\n";
        exit(EXIT_FAILURE);
    }
    return result;
}

int Listen(int sockfd, int backlog){
    int result = listen(sockfd, backlog);
    if(result == -1){
        perror("Listen failed\n");
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

int Accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen){
    int result  = accept(sockfd, addr, addrlen);

    return result;
}

ssize_t Send(int sockfd, const void* buf, size_t len, int flags) {
    ssize_t nsend = send(sockfd, buf, len, flags);
    if(nsend == -1){
        perror("Send failed");
        exit(EXIT_FAILURE);
    }
    return nsend;
}

ssize_t Recv(int sockfd, void* buf, size_t len, int flags){
    ssize_t nrecv = recv(sockfd, buf, len, flags);
    if(nrecv < 0){
        perror("Recv failed: ");
        exit(EXIT_FAILURE);
    }
    return nrecv;
}

int Close(int fd) {
    int result = close(fd);
    if(result == -1){
        perror("Close failed: ");
        exit(EXIT_FAILURE);
    }
    return result;
}

void showOnlineUsers(int sockfd){
    std::string allUsers = "\n\n\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m ONLINE USERS: \033[0m\n\n";
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(!client_array[i].uname.empty())
            allUsers += std::to_string(i + 1) + ") " + client_array[i].ip_addr_str + " > @" + client_array[i].uname + "\n";
    }
    send(sockfd, allUsers.c_str(), MAX_PACKET_SIZE, 0);
}

bool isClientArrayEmpty() {
    for (int i = 0; i < MAX_CLIENTS; i++)
        if(client_array[i].sockfd != EMPTY_SOCKET)
            return false;
    return true;
}

void shutdownAllConnections(int signum) {
    std::cout << "\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m  Signal "
              << signum << " Caught ... Terminating all active connections !!  \033[0m\n";
    logfile  << "\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m  Signal "
             << signum << " Caught ... Terminating all active connections !!  \033[0m\n";
    char request[MAX_PACKET_SIZE] = "--shell-- --getout--";
    for(int i = 0; i < MAX_CLIENTS; i++)
        if(client_array[i].sockfd != EMPTY_SOCKET)
            send(client_array[i].sockfd, request, MAX_PACKET_SIZE, 0);
    FATAL_TERMINATE = true;
}

void donotDisturb(int signum) {
    if(isClientArrayEmpty()) {
        std::cout << "\n\n TERMINATING SERVER...\n\n";
        exit(0);
    } else {
        std::cout << "\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m  Signal "
                  << signum << " Caught ... Cant terminate server, Some users are online !!  \033[0m\n";
    }
}

bool checkSeparatorTags(char *temp_msg, int sockfd) {
    char error_msg[MAX_PACKET_SIZE];
    if(strstr(temp_msg, "@") == nullptr){
        strcpy(error_msg, "no target!!!\n");
        Send(sockfd, error_msg, MAX_PACKET_SIZE, 0);
        return false;
    }
    if(strstr(temp_msg, "--shell--") != nullptr
       && strstr(temp_msg, "--bash--") == nullptr) {
        strcpy(error_msg, " '--bash--' separated needed!!\n");
        Send(sockfd, error_msg, MAX_PACKET_SIZE, 0);
        return false;
    }
    return true;
}

void initSignalHandlers() {
    struct sigaction sa = {nullptr};
    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = shutdownAllConnections;
    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGKILL, &sa, nullptr);
    sigaction(SIGABRT, &sa, nullptr);

    sa.sa_handler = donotDisturb;
    sigaction(SIGQUIT, &sa, nullptr);
    sigaction(SIGINT , &sa, nullptr);
    sigaction(SIGTSTP, &sa, nullptr);
}

int processClient(client_type &new_client, std::thread &self_thread) {

    bool is_uname_set = false;

    while(true) {
        //Обновляем серверное время
        time_t timer;
        char time_string[MAX_PACKET_SIZE], temp_msg[MAX_PACKET_SIZE] = "";
        std::string msg = "";
        struct tm *tm_info;
        time(&timer);
        tm_info = localtime(&timer);
        strftime(time_string, MAX_PACKET_SIZE, "[ %d/%m/%Y  %H:%M:%S ] >     ", tm_info);

        if (new_client.sockfd != 0) {

            if (!is_uname_set) {
                msg = std::string(banner);
                Send(new_client.sockfd, msg.c_str(), std::strlen(msg.c_str()), 0);
            }

            uname_exists:
            memset(temp_msg, 0, MAX_PACKET_SIZE);
            ssize_t iResult = Recv(new_client.sockfd, temp_msg, MAX_PACKET_SIZE, 0);

            //Обработка выполняющих команд
            if (strstr(temp_msg, "--shell--") == temp_msg) {

                int i = 0;
                char *cmd = strstr(temp_msg, "--bash-- ") + 9;
                char err_msg[MAX_PACKET_SIZE];
                if (!checkSeparatorTags(temp_msg, new_client.sockfd)) continue;


                for (; i < MAX_CLIENTS; i++)
                    if ((strstr(temp_msg + 11, client_array[i].uname.c_str()) == temp_msg + 11
                         || strstr(temp_msg + 11, "--all--") == temp_msg + 11)
                        && client_array[i].uname != "" && client_array[i].id != new_client.id) {
                        char final_cmd[MAX_PACKET_SIZE] = "--shell-- ";
                        strcat(final_cmd, cmd);
                        Send(client_array[i].sockfd, final_cmd, MAX_PACKET_SIZE, 0);
                        client_array[i].master_sockfd = new_client.sockfd;
                        std::cout << "Shell '" << (final_cmd + 10) << "' sent to user @" << client_array[i].uname << "\n";
                        logfile << "Shell '" << (final_cmd + 10) << "' sent to user @" << client_array[i].uname << "\n";
                        if (strstr(temp_msg + 11, "--all--") != temp_msg + 11)
                            break;
                    }

                if (strstr(temp_msg + 11, "--all--") != temp_msg + 11 && i == MAX_CLIENTS) {
                    std::strcpy(err_msg, "The USER doesnt exist!!!\n");
                    Send(new_client.sockfd, err_msg, MAX_PACKET_SIZE, 0);
                }
            }

            if (strstr(temp_msg, "--anyonehere--") == temp_msg)
                showOnlineUsers(new_client.sockfd);

            if (strstr(temp_msg, "--shellout--") == temp_msg) {

                char finalout[MAX_PACKET_SIZE] = "shellout @ ";
                strcat(finalout, new_client.uname.c_str());
                strcat(finalout, "@");
                strcat(finalout, new_client.ip_addr_str.c_str());
                strcat(finalout, "   > ");
                strcat(finalout, temp_msg + 12);
                std::cout << finalout;
                logfile << finalout;
                finalout[strlen(finalout) - 1] = 0;
                Send(new_client.master_sockfd, finalout, MAX_PACKET_SIZE, 0);
                if (strstr(temp_msg, "\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m   Bash ") == temp_msg + 12) {
                    //std::cout << "MASTER HAS SURRENDERED!!\n";
                    new_client.master_sockfd = EMPTY_SOCKET;
                }
            }

            if (iResult != SOCKET_ERROR && strcmp(temp_msg, "--exit--")) {

                if (!is_uname_set) {
                    char err_msg[MAX_PACKET_SIZE];

                    if (!strcmp(temp_msg, "--all--")) {
                        strcpy(err_msg,"--all-- represents everyone online ... this cant be your username!!\n Please enter a different username: ");
                        Send(new_client.sockfd, err_msg, strlen(err_msg),0);
                        goto uname_exists;
                    }
                    for (int i = 0; i < MAX_CLIENTS; i++)
                        if (client_array[i].uname == std::string(temp_msg)) {
                            strcpy(err_msg,"SORRY this username already exists!!\n Please enter a different username: ");
                            Send(new_client.sockfd, err_msg, strlen(err_msg), 0);
                            goto uname_exists;
                        }

                    new_client.uname = std::string(temp_msg);
                    strcpy(temp_msg, "\n\n\n          Welcome to group \033[1;94m");
                    strcat(temp_msg, cols[new_client.id % COLS].c_str());
                    strcat(temp_msg, new_client.uname.c_str());
                    strcat(temp_msg, "\033[0m !! You can now send messages...\n");
                    Send(new_client.sockfd, temp_msg, strlen(temp_msg), 0);
                    showOnlineUsers(new_client.sockfd);

                }
                if (strcmp("", temp_msg) != 0) {
                    if (!is_uname_set) {
                        msg = std::string(time_string) + new_client.uname + " has joined the group!\n";
                        is_uname_set = true;
                    } else
                        msg = "\033[1;94m" + cols[new_client.id % COLS]
                              + time_string + "@"
                              + new_client.uname + ": "
                              + temp_msg + " \033[0m";
                }

                if (strstr(temp_msg, "--shellout--") != temp_msg) {
                    std::cout << msg << std::endl;
                    logfile << msg << std::endl;
                }

                if (strstr(temp_msg, "--shell--") != temp_msg &&
                    strstr(temp_msg, "--shellout--") != temp_msg &&
                    strstr(temp_msg, "--anyonehere--") != temp_msg && msg != "") {
                    for (int i = 0; i < MAX_CLIENTS; i++)
                        if (client_array[i].sockfd != EMPTY_SOCKET && new_client.id != i)
                            iResult = Send(client_array[i].sockfd, msg.c_str(), strlen(msg.c_str()), 0);
                }
            }
            else {

                msg = std::string(time_string) + new_client.uname + " has left the group";
                std::cout << msg << std::endl;
                logfile << msg << std::endl;

                for (int i = 0; i < MAX_CLIENTS; i++)
                    if (client_array[i].sockfd != EMPTY_SOCKET)
                        iResult = Send(client_array[i].sockfd, msg.c_str(), strlen(msg.c_str()), 0);
                close(new_client.sockfd);
                close(client_array[new_client.id].sockfd);

                client_array[new_client.id].uname = "";
                client_array[new_client.id].sockfd = EMPTY_SOCKET;

                if (FATAL_TERMINATE)
                    if (isClientArrayEmpty()) {
                        std::cout << "\n\n TERMINATING SERVER ON FATAL REQUEST ...\n\n";
                        exit(0);
                    }

                self_thread.detach();
                return 0;
            }
        }
    }
}