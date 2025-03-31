#include <ftp_session.hpp>
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

std::string Session::receiveMsg(int socket) {
    char* buffer = new char[this->buffer_size]{};
    recv(socket, buffer, this->buffer_size, 0);
    std::string result(buffer);
    delete[] buffer;
    return result;
}

void Session::sendMsg(int socket, const std::string& msg) {
    send(socket, msg.c_str(), msg.size(), 0);
}

bool parse_port(const std::string& port_cmd, std::string& ip, int& port) {
    if (port_cmd.find("PORT") == port_cmd.npos) {
        return false;
    }

    size_t space_pos = port_cmd.find(' ');
    if (space_pos == std::string::npos) {
        return false;
    }

    std::string params = port_cmd.substr(space_pos + 1);
    
    std::vector<int> numbers;
    std::stringstream ss(params);
    std::string item;
    
    while (std::getline(ss, item, ',')) {
        try {
            numbers.push_back(std::stoi(item));
        } catch (...) {
            return false;
        }
    }

    if (numbers.size() != 6) {
        return false;
    }
    
    ip = std::to_string(numbers[0]) + "." + 
         std::to_string(numbers[1]) + "." + 
         std::to_string(numbers[2]) + "." + 
         std::to_string(numbers[3]);

    port = numbers[4] * 256 + numbers[5];
    return true;
}

bool Session::openClientDataSocket(const std::string& ip, int port) {
    if (this->_client_data > 0)
        close(this->_client_data);   // closing old socket
    std::cout << "Client waiting data on " << ip << ":" << port << std::endl;
    this->_client_data = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_client_data < 0) {
        std::cout << "Could not open socket for data transfer to client!" << std::endl;
        return false;
    }
    sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &clientAddr.sin_addr) <= 0) {
        std::cout << "Could not find client address for data transfer to client!" << std::endl;
        return false;
    }
    if (connect(this->_client_data, (sockaddr*)&clientAddr, sizeof clientAddr)) {
        std::cout << "Could not connect to client for data transfer to client!" << std::endl;
        return false;
    }
    std::cout << "Connected to client for data transfering!" << std::endl;
    return true;
}

bool Session::openServerDataSocket(int& port) {
    if (this->_server_data > 0)
        close(this->_server_data);  // closing old socket
    std::cout << "Opening socket for data transfer from server!" << std::endl;
    this->_server_data = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_server_data < 0) {
        std::cout << "Could not open socket for data transfer from server!" << std::endl;
        return false;
    }
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(2122); // test FTP port change later

    if (bind(this->_server_data, (sockaddr*)&serverAddr, sizeof serverAddr)) {
        std::cout << "Could not bind socket for data transfer from server!" << std::endl;
        return false;
    }
    if (listen(this->_server_data, 1) < 0) {
        std::cout << "Could not listen socket for data tranfser from server!" << std::endl;
        return false;
    } 
    std::cout << "Opened socket for data transfer from server..." << std::endl;
    return true;
}

bool Session::TranslateMessages() {
    if (this->semaphore) {
        std::string msg = this->receiveMsg(this->server);
        std::cout << "[*] Server: " << msg << std::endl;
        this->sendMsg(this->client, msg);
        this->semaphore = 0;
    } else {
        std::string msg = this->receiveMsg(this->client);
        std::cout << "[*] Client: " << msg << std::endl;
        
        std::string cmd = msg.substr(0, 4);
        
        if (cmd == "QUIT") {
            this->sendMsg(this->server, msg);
            msg = this->receiveMsg(this->server);
            this->sendMsg(this->client, msg);
            this->_hold_session = false;
        }
        if (cmd == "RETR") {
            // replace file task
        }
        if (cmd == "LIST") {
            // relay data from directories
        }
        if (cmd == "PORT") {
            std::string ip;
            int port, listen_port;
            
            if (!parse_port(msg, ip, port)) {
                std::cout << "Could not parse PORT command! Transfering command to server..." << std::endl;
                this->sendMsg(this->server, msg);
                this->semaphore = 1;
                return this->_hold_session;
            }
            this->_hold_session = this->openClientDataSocket(ip, port) & this->openServerDataSocket(listen_port);
            // send listen_ip and listen_port for server
            return this->_hold_session;
        }
        this->sendMsg(this->server, msg);
        this->semaphore = 1;
    }
    return this->_hold_session;
}

Session::~Session() {
    if (this->_client_data > 0)
        close(this->_client_data);
    if (this->_server_data > 0)
        close(this->_server_data);
}