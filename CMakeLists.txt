cmake_minimum_required(VERSION 3.31)

project(FTP_MITM CXX)

include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
)
FetchContent_MakeAvailable(yaml-cpp)

add_library(ftp_mitm_lib src/ftp_mitm/ftp_mitm.cpp src/ftp_mitm/includes/ftp_mitm.hpp)
target_include_directories(ftp_mitm_lib PRIVATE src/ftp_mitm/includes)
target_link_libraries(ftp_mitm_lib PUBLIC yaml-cpp::yaml-cpp)

add_executable(client src/client/main.cpp)
target_include_directories(client PRIVATE src/ftp_mitm/includes)

add_dependencies(client ftp_mitm_lib)