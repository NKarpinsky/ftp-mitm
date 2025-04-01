#include <ftp_session.hpp>
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <random>

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

uint16_t generate_random_port() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(49152, 65535);
    return static_cast<uint16_t>(dis(gen));
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
    port = generate_random_port();
    serverAddr.sin_port = htons(port); 

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

std::string get_local_ip() {
    struct ifaddrs *ifaddr, *ifa;
    std::string ip;

    if (getifaddrs(&ifaddr)) {
        perror("getifaddrs");
        return "";
    }

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr || ifa->ifa_addr->sa_family != AF_INET) {
            continue;
        }

        void* tmpAddrPtr = &((sockaddr_in*)ifa->ifa_addr)->sin_addr;
        char addressBuffer[INET_ADDRSTRLEN] = {};
        inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
        //if (std::string(addressBuffer) != "127.0.0.1") {
        //    ip = addressBuffer;
        //    break;
        //}
        ip = addressBuffer;
        break;
    }

    freeifaddrs(ifaddr);
    return ip;
}

std::string create_port_command(const std::string& ip, int port) {
    std::vector<std::string> ip_parts;
    std::stringstream ss(ip);
    std::string part;
    
    while (std::getline(ss, part, '.')) {
        ip_parts.push_back(part);
    }

    if (ip_parts.size() != 4) {
        return "";
    }

    uint8_t p1 = port >> 8;
    uint8_t p2 = port & 0xFF;

    std::ostringstream port_cmd;
    port_cmd << "PORT " << ip_parts[0] << "," << ip_parts[1] << ","
             << ip_parts[2] << "," << ip_parts[3] << ","
             << static_cast<int>(p1) << "," << static_cast<int>(p2) << "\r\n";

    return port_cmd.str();
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
            this->sendMsg(this->server, msg);
            char* buffer = new char[this->buffer_size]{};
            int size = 1;
            sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof clientAddr;
            int _sock = accept(this->_server_data, (sockaddr*)&clientAddr, &clientAddrLen);
            if (_sock < 0) {
                std::cout << "Could not accept server connection for data transfer!" << std::endl;
                return this->_hold_session;
            }
            while (size > 0) {
                size = recv(_sock, buffer, this->buffer_size, 0);
                send(this->_client_data, buffer, size, 0);
            }
            delete[] buffer;
            close(_sock);
            close(this->_server_data);
            close(this->_client_data);
            msg = this->receiveMsg(this->server);
            this->sendMsg(this->client, msg);
            return this->_hold_session;
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
            std::string listen_ip = get_local_ip();
            std::string port_cmd = create_port_command(listen_ip, listen_port);
            this->sendMsg(this->server, port_cmd);
            msg = this->receiveMsg(this->server);
            this->sendMsg(this->client, msg);
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