#pragma once

#include <string>
#include <utility>
#include "json.hpp"

struct User {
    int id = 0;
    std::string username;
    std::string email;
    std::string role; // "Admin" or "HR"
    std::string createdAt;

    nlohmann::json toJson() const {
        return {
            {"id", id},
            {"username", username},
            {"email", email},
            {"role", role},
            {"created_at", createdAt}
        };
    }
};

class Auth {
private:
    static const std::string SALT;
    static const std::string JWT_SECRET;

public:
    // Password Hashing & Verification
    static std::string hashPassword(const std::string& plainPassword);
    static bool verifyPassword(const std::string& plainPassword, const std::string& storedHash);

    // Token Generation and Validation (JWT-style signed token)
    static std::string generateToken(int userId, const std::string& username, const std::string& role);
    static bool verifyToken(const std::string& token, int& outUserId, std::string& outUsername, std::string& outRole);

    // Business Services
    static std::pair<bool, std::string> login(const std::string& usernameOrEmail,
                                              const std::string& password,
                                              nlohmann::json& outUserData);

    static std::pair<bool, std::string> changePassword(int userId,
                                                      const std::string& oldPassword,
                                                      const std::string& newPassword);

    static std::pair<bool, User> getUserById(int userId);
};
