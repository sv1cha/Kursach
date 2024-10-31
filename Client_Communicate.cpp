#include "Client_Communicate.h"
#include "Logger.h"
#include "Errors.h"
#include "Calculator.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <time.h>

#include <random>
#include <iomanip>
#include <sstream>

#include <cryptopp/hex.h>
#include <cryptopp/base64.h>
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/md5.h>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/variate_generator.hpp>

#define buff_size 1024

std::unique_ptr<char[]> buff(new char[buff_size]);

std::map<std::string, std::string> client_database;

std::string Client_Communicate::generate_salt()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;

    uint64_t salt = dis(gen);
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << salt;
    return ss.str();
}

std::string Client_Communicate::md5(const std::string& input_str)
{
    using namespace CryptoPP;

    Weak::MD5 hash;
    std::string digest;

    StringSource ss(input_str, true,
        new HashFilter(hash,
            new HexEncoder(
                new StringSink(digest)
            )
        )
    );

    return digest;
}

void load_client_database(const char* filename)
{
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string login = line.substr(0, pos);
                std::string password = line.substr(pos + 1);
                client_database[login] = password;
            }
        }
        file.close();
    } else {
        throw crit_err("Failed to open client database file");
    }
}

int Client_Communicate::connection(int port, const char* database_file, const char* log_file, Logger* l1)
{
    try {
        load_client_database(database_file);
        l1->set_path(log_file);

        int queue_len = 100;
        sockaddr_in *addr = new sockaddr_in;
        addr->sin_family = AF_INET;
        addr->sin_port = htons(port);
        inet_aton("127.0.0.1", &addr->sin_addr);
        int s = socket(AF_INET, SOCK_STREAM, 0);
        if (s <= 0) {
            throw crit_err("Socket created err");
        } else {
            l1->writelog("listen socket created");
        }
        auto rc = bind(s, (const sockaddr*)addr, sizeof(sockaddr_in));
        if (rc == -1) {
            throw crit_err("Socket bind err");
        } else {
            l1->writelog("bind success");
        }
        rc = listen(s, queue_len);
        if (rc == -1) {
            throw crit_err("Socket listen err");
        }

        for (;;) {
            try {
                sockaddr_in *client_addr = new sockaddr_in;
                socklen_t len = sizeof(sockaddr_in);
                int work_sock = accept(s, (sockaddr*)(client_addr), &len);
                if (work_sock <= 0) {
                    throw no_crit_err("[Uncritical]Client socket error");
                }
                l1->writelog("Client socket created");

                // Получаем идентификатор клиента
                char id_buffer[64] = {0};
                rc = recv(work_sock, id_buffer, sizeof(id_buffer) - 1, 0);
                if (rc <= 0) {
                    close(work_sock);
                    throw no_crit_err("[Uncritical]ID receive error");
                }
                std::string client_id(id_buffer);
                l1->writelog("Received client ID: " + client_id);

                // Проверяем идентификатор клиента в базе
                if (client_database.find(client_id) == client_database.end()) {
                    send(work_sock, "ERR", 3, 0);
                    close(work_sock);
                    throw no_crit_err("[Uncritical]Unknown client ID");
                }

                // Генерируем и отправляем соль
                std::string salt_s = generate_salt();
                rc = send(work_sock, salt_s.c_str(), salt_s.length(), 0);
                if (rc <= 0) {
                    close(work_sock);
                    throw no_crit_err("[Uncritical]send SALT error");
                }
                l1->writelog("SALT sent: " + salt_s);

                // Получаем хэш от клиента
                char hash_buffer[64] = {0};
                rc = recv(work_sock, hash_buffer, sizeof(hash_buffer) - 1, 0);
                if (rc <= 0) {
                    close(work_sock);
                    throw no_crit_err("[Uncritical]HASH received error");
                }
                std::string client_hash(hash_buffer);
                l1->writelog("Received hash from client: " + client_hash);

                // Проверяем хэш
                std::string password = client_database[client_id];
                std::string correct_hash = md5(salt_s + password);
                l1->writelog("Salt: " + salt_s);
                l1->writelog("Password from database: " + password);
                l1->writelog("Salt + Password: " + salt_s + password);
                l1->writelog("Correct hash: " + correct_hash);

                char response[17] = {0};  // 16 байт + нулевой символ
                if (client_hash == correct_hash) {
                    std::strncpy(response, "OK____________", 16);
                    l1->writelog("Authentication successful");
                } else {
                    std::strncpy(response, "ERR___________", 16);
                    l1->writelog("Authentication failed");
                }

                rc = send(work_sock, response, 16, 0);
                if (rc <= 0) {
                    close(work_sock);
                    throw no_crit_err("[Uncritical]Send response error");
                }
                l1->writelog("Sent response: " + std::string(response));

                if (client_hash != correct_hash) {
                    close(work_sock);
                    continue;
                }

                // Обработка векторов
                uint32_t vector_count;
                rc = recv(work_sock, &vector_count, sizeof(vector_count), 0);
                if (rc <= 0) {
                    close(work_sock);
                    throw no_crit_err("[Uncritical error]count of vectors not received");
                }
                l1->writelog("Vector count received: " + std::to_string(vector_count));

                for (uint32_t i = 0; i < vector_count; i++) {
                    uint32_t vector_len;
                    rc = recv(work_sock, &vector_len, sizeof(vector_len), 0);
                    if (rc <= 0) {
                        close(work_sock);
                        throw no_crit_err("[Uncritical error]len of vector not received");
                    }
                    std::vector<int64_t> v(vector_len);
                    rc = recv(work_sock, v.data(), vector_len * sizeof(int64_t), 0);
                    if (rc <= 0) {
                        close(work_sock);
                        throw no_crit_err("[Uncritical error]vector not received");
                    }
                    Calculator calc(v);
                    int64_t result = calc.send_res();

                    rc = send(work_sock, &result, sizeof(result), 0);
                    if (rc <= 0) {
                        close(work_sock);
                        throw no_crit_err("[Uncritical error]result of calculating vector not send");
                    }
                    l1->writelog("Result of calculating vector " + std::to_string(i+1) + " sent");
                }

                close(work_sock);
                l1->writelog("Connection closed");

            } catch (no_crit_err& e) {
                l1->writelog(e.what());
            }
        }
    } catch (crit_err& e) {
        l1->writelog(e.what());
    }
    return 0;
}
