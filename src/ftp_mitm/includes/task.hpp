#pragma once
#include <string>
#include <vector>
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

