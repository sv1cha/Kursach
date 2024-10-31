#include "Logger.h"
#include <chrono>
#include <string>
#include <time.h>
#include <fstream>
#include "Errors.h"

using namespace std;

int Logger::set_path(std::string path_file)
{
    if (path_file.find('.') == std::string::npos) throw std::invalid_argument("file without extension");
    std::ofstream filelog;
    filelog.open(path_file, std::ios_base::out | std::ios_base::app);
    if (filelog.is_open()) {
        return 0;
    } else {
        throw crit_err("path to log not exist");
    }
}

string Logger::getCurrentDateTime(string s)
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    if (s == "now")
        strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    else if (s == "date")
        strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
    return string(buf);
}

int Logger::writelog(std::string s)
{
    std::ofstream filelog;
    filelog.open(path_to_logfile, std::ios_base::out | std::ios_base::app);
    if (filelog.is_open()) {
        std::string time = getCurrentDateTime("now");
        filelog << time << ' ' << s << std::endl;
    } else {
        throw crit_err("path to log not exist");
    }
    return 0;
}
