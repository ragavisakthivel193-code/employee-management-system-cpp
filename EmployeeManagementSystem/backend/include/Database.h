#pragma once

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <memory>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif
#include <mysql.h>

class Database {
private:
    MYSQL* conn;
    std::string host;
    std::string user;
    std::string password;
    std::string databaseName;
    int port;
    mutable std::mutex dbMutex;
    std::string lastError;

    // Private constructor for singleton / managed instance
    Database();

public:
    ~Database();

    // Singleton access
    static Database& getInstance();

    // Prevent copying
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    // Connection lifecycle
    bool connect(const std::string& host,
                 const std::string& user,
                 const std::string& password,
                 const std::string& databaseName,
                 int port = 3306);
    void disconnect();
    bool isConnected();
    bool ping();

    // SQL execution
    bool execute(const std::string& sql,
                 unsigned long long* outInsertId = nullptr,
                 unsigned long long* outAffectedRows = nullptr);

    std::vector<std::map<std::string, std::string>> query(const std::string& sql);

    // SQL safety & escaping
    std::string escape(const std::string& input);

    // Error handling
    std::string getLastError() const;
};
