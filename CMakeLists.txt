cmake_minimum_required(VERSION 3.31)

project(FTP_MITM CXX)

include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
)
FetchContent_MakeAvailable(yaml-cpp)

add_library(ftp_mitm_lib src/ftp-mitm/ftp-mitm.cpp src/ftp-mitm/includes/ftp-mitm.hpp src/ftp-mitm/ftp-session.cpp)
target_include_directories(ftp_mitm_lib PRIVATE src/ftp-mitm/includes)
target_link_libraries(ftp_mitm_lib PUBLIC yaml-cpp::yaml-cpp)

add_executable(client src/client/main.cpp)
target_include_directories(client PRIVATE src/ftp-mitm/includes)
target_link_libraries(client PRIVATE ftp_mitm_lib)

add_dependencies(client ftp_mitm_lib)