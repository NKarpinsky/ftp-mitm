#pragma once
#include <string>
#include <vector>
#include <map>
#include <regex>

class Substitution {
private:
    std::string _target;
    std::string _sub;
public:
    Substitution(const std::string& target, const std::string& sub) : _target(target), _sub(sub) {}
    const bool match(const std::string& filename) {
        std::regex pattern(_target);
        return std::regex_match(filename, pattern);
    }

    const std::string get_substitution() { return this->_sub; }
};

class Task {
private:
    std::string client_addr;
    std::string server_addr;
    std::vector<Substitution> _subs;
public:
    Task() = default;
    Task(const std::string& client, const std::string& server) {
        this->client_addr = client;
        this->server_addr = server;
    }
    void AddSubstitution(const Substitution& sub) {
        this->_subs.push_back(sub);
    }

    const std::string GetClient() {return client_addr;}
    const std::string GetServer() {return server_addr;} 
};

class FtpMitm {
public:
    FtpMitm(){};
    ~FtpMitm();
    void LoadConfig(const std::string& config_path);
    int StartServer();
    void Attack();
private:
    std::vector<Task> _tasks;
    std::string _subs_directory;
    int _port;
    int _socket = -1;
    unsigned int _buffer_size = 1024;

    void holdClient(int clientSocket, sockaddr_in clientAddr);
    std::string recvCommand(int clientSocket);
    void translateSession(int clientSocket, Task task);
};
