#include <iostream>
#include <fstream>
#include "Errors.h"
#include "Logger.h"
#include "Connector_to_base.h"

using namespace std;

int Connector_to_base::connect_to_base(string base_file)
{
    if (base_file.find('.') == std::string::npos) {
        throw crit_err("invalid_base_path: File name does not contain an extension");
    }
    ifstream file_read;
    file_read.open(base_file);
    if (!file_read.is_open()) {
        throw crit_err("invalid_base_path: Unable to open file " + base_file);
    }
    string line;
    string temp_pass;
    string temp_login;
    while (getline(file_read, line)) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            temp_login = line.substr(0, pos);
            temp_pass = line.substr(pos + 1);
            data_base[temp_login] = temp_pass;
        }
    }
    if (data_base.empty()) {
        throw crit_err("invalid_base_path: File is empty or no valid data found");
    }
    return 0;
}
