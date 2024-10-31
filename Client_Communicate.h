#pragma once
#include <string>
#include <map>
#include "Logger.h"
#include "Errors.h"

class Client_Communicate
{
public:
    int connection(int port, const char* database_file, const char* log_file, Logger* l1);
    static std::string generate_salt();
    static std::string md5(const std::string& input_str);
};
