#include "Database.h"
#include <iostream>

Database::Database() : conn(nullptr), port(3306) {}

Database::~Database() {
    disconnect();
}

Database& Database::getInstance() {
    static Database instance;
    return instance;
}

bool Database::connect(const std::string& host,
                       const std::string& user,
                       const std::string& password,
                       const std::string& databaseName,
                       int port) {
    std::lock_guard<std::mutex> lock(dbMutex);

    this->host = host;
    this->user = user;
    this->password = password;
    this->databaseName = databaseName;
    this->port = port;

    if (conn != nullptr) {
        mysql_close(conn);
        conn = nullptr;
    }

    conn = mysql_init(nullptr);
    if (!conn) {
        lastError = "Failed to initialize MySQL client library.";
        return false;
    }

    bool reconnect = true;
    mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);
    mysql_options(conn, MYSQL_SET_CHARSET_NAME, "utf8mb4");

    if (!mysql_real_connect(conn,
                            host.c_str(),
                            user.c_str(),
                            password.c_str(),
                            databaseName.c_str(),
                            port,
                            nullptr,
                            0)) {
        lastError = mysql_error(conn);
        mysql_close(conn);
        conn = nullptr;
        return false;
    }

    mysql_set_character_set(conn, "utf8mb4");
    lastError.clear();
    return true;
}

void Database::disconnect() {
    std::lock_guard<std::mutex> lock(dbMutex);
    if (conn != nullptr) {
        mysql_close(conn);
        conn = nullptr;
    }
}

bool Database::isConnected() {
    std::lock_guard<std::mutex> lock(dbMutex);
    return conn != nullptr;
}

bool Database::ping() {
    if (!conn) return false;
    return mysql_ping(conn) == 0;
}

std::string Database::escape(const std::string& input) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if (!conn || input.empty()) {
        return input;
    }
    std::vector<char> buffer(input.length() * 2 + 1);
    unsigned long length = mysql_real_escape_string(conn, buffer.data(), input.c_str(), static_cast<unsigned long>(input.length()));
    return std::string(buffer.data(), length);
}

bool Database::execute(const std::string& sql,
                      unsigned long long* outInsertId,
                      unsigned long long* outAffectedRows) {
    std::lock_guard<std::mutex> lock(dbMutex);

    if (!conn) {
        lastError = "Database not connected.";
        return false;
    }

    if (mysql_query(conn, sql.c_str()) != 0) {
        lastError = mysql_error(conn);
        return false;
    }

    if (outInsertId) {
        *outInsertId = mysql_insert_id(conn);
    }
    if (outAffectedRows) {
        *outAffectedRows = mysql_affected_rows(conn);
    }

    lastError.clear();
    return true;
}

std::vector<std::map<std::string, std::string>> Database::query(const std::string& sql) {
    std::lock_guard<std::mutex> lock(dbMutex);
    std::vector<std::map<std::string, std::string>> rows;

    if (!conn) {
        lastError = "Database not connected.";
        return rows;
    }

    if (mysql_query(conn, sql.c_str()) != 0) {
        lastError = mysql_error(conn);
        return rows;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) {
        // Could be a query that returns no result set (e.g. non-SELECT)
        if (mysql_field_count(conn) > 0) {
            lastError = mysql_error(conn);
        }
        return rows;
    }

    int numFields = mysql_num_fields(result);
    MYSQL_FIELD* fields = mysql_fetch_fields(result);

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        std::map<std::string, std::string> rowMap;
        for (int i = 0; i < numFields; ++i) {
            std::string fieldName = fields[i].name;
            std::string fieldValue = row[i] ? row[i] : "";
            rowMap[fieldName] = fieldValue;
        }
        rows.push_back(rowMap);
    }

    mysql_free_result(result);
    lastError.clear();
    return rows;
}

std::string Database::getLastError() const {
    return lastError;
}
