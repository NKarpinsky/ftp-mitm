#pragma once
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <ftp_session.hpp>
#include <task.hpp>


class FtpMitm {
public:
    FtpMitm(){};
    ~FtpMitm();
    void LoadConfig(const std::string& config_path);
    int StartServer();
    void Attack();
private:
    std::vector<Task> _tasks;
    int _port;
    int _socket = -1;
    unsigned int _buffer_size = 1024;

    void holdClient(int clientSocket, sockaddr_in clientAddr);
    void translateSession(int clientSocket, Task task);
};
