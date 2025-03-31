#pragma once
#include <task.hpp>

class Session {
public:
    int client;
    int server;
    int semaphore;
    unsigned int buffer_size = 1024;
    Task task;
    bool TranslateMessages();
    ~Session();
private:
    std::string receiveMsg(int socket);
    void sendMsg(int socket, const std::string &msg);
    bool openClientDataSocket(const std::string& ip, int port);
    bool openServerDataSocket(int& port);
    bool _hold_session = true;
    int _client_data = -1;   // socket for client when receiving data
    int _server_data = -1;   // socket for server when receiving data
};