#include "Interface.h"
#include "Connector_to_base.h"
#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <vector>
#include "Client_Communicate.h"
#include "Errors.h"
#include "Logger.h"

namespace po = boost::program_options;

int Interface::comm_proc(int argc, const char** argv)
{
    bool flag_b = false;
    bool flag_l = false;
    bool flag_p = false;
    int PORT;
    std::string logfile;
    std::string basefile;

    try {
        po::options_description opts("Allowed options");
        opts.add_options()
        ("help,h", "Show help")
        ("basefile,b",
         po::value<std::string>()->default_value("/home/stud/курсовая/base/base.txt"),
         "option is string(path to file with database)")
        ("logfile,l",
         po::value<std::string>()->default_value("/home/stud/курсовая/base/log.txt"),
         "option is string(path to file with logs)")
        ("PORT,p",
         po::value<int>(&PORT)->default_value(33333),
         "option is int (PORT for server)");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, opts), vm);
        po::notify(vm);
        if (vm.count("help")) {
            std::cout << opts << "\n";
            exit(0);
        }

        if (vm.count("basefile")) {
            if (vm["basefile"].as<std::string>() == "/home/stud/курсовая/base/base.txt") {
                flag_b = true;
            }
            basefile = vm["basefile"].as<std::string>();
        }
        if (vm.count("logfile")) {
            if (vm["logfile"].as<std::string>() == "/home/stud/курсовая/base/log.txt") {
                flag_l = true;
            }
            logfile = vm["logfile"].as<std::string>();
        }

        if (vm.count("PORT")) {
            if (vm["PORT"].as<int>() == 33333) {
                flag_p = true;
            }
            PORT = vm["PORT"].as<int>();
        }

        if (PORT < 1024 or PORT > 65535) {
            throw crit_err("Incorrect port");
        }
        if (flag_b and flag_l and flag_p) {
            std::cout << "Server started with default parameters.Use -h for help" << std::endl;
        }
        Logger l1(logfile);
        if (logfile != "/home/stud/курсовая/base/log.txt") {
            l1.writelog("Path to logfile set value: " + logfile);
        } else {
            l1.writelog("Path to logfile set default value");
        }
        if (basefile != "/home/stud/курсовая/base/base.txt") {
            l1.writelog("Path to basefile set value: " + basefile);
        } else {
            l1.writelog("Path to basefile set default value");
        }
        if (PORT != 33333) {
            l1.writelog("Port set not default value");
        } else {
            l1.writelog("Port set default value");
        }
        Connector_to_base c1;
        c1.connect_to_base(basefile);
        l1.writelog("Connect to database success!");
        l1.writelog("Server started");
        Client_Communicate con;
        con.connection(PORT, basefile.c_str(), logfile.c_str(), &l1);
    } catch (crit_err& e) {
        std::cerr << "error: " << e.what() << "\n";
        std::cerr << "Критическая ошибка" << std::endl;
    } catch (po::error& e) {
        std::cerr << "error: " << e.what() << "\n";
        std::cerr << "Use -h for help\n";
    } catch (std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        std::cerr << "Use -h for help\n";
    } catch (...) {
        std::cerr << "Exception of unknown type!\n";
        std::cerr << "Use -h for help\n";
        std::terminate();
    }
    return 0;
}
