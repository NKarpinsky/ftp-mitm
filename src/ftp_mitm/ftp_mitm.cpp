#include <ftp_mitm.hpp>
#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <iostream>

void FtpMitm::LoadConfig(const std::string& config_path) {
    try {
        YAML::Node config = YAML::LoadFile(config_path);
        this->_subs_directory = config["config"]["directory"].as<std::string>();
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

void FtpMitm::StartServer() {

}