#pragma once
#include <task.hpp>

struct Session {
    int client;
    int server;
    int semaphore;
    Task task;
};