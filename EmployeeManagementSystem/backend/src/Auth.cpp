#include "Auth.h"
#include "Database.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cstring>
#include <vector>

const std::string Auth::SALT = "ems_salt_2026_";
const std::string Auth::JWT_SECRET = "ems_jwt_super_secure_key_2026_ragavi";

// --- Self-contained SHA-256 Implementation (FIPS 180-2) ---
namespace {
    inline uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
    inline uint32_t choose(uint32_t e, uint32_t f, uint32_t g) { return (e & f) ^ (~e & g); }
    inline uint32_t majority(uint32_t a, uint32_t b, uint32_t c) { return (a & b) ^ (a & c) ^ (b & c); }
    inline uint32_t sig0(uint32_t x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); }
    inline uint32_t sig1(uint32_t x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); }
    inline uint32_t theta0(uint32_t x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); }
    inline uint32_t theta1(uint32_t x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }

    const uint32_t K[64] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
    };

    std::string sha256Raw(const std::string& input) {
        uint32_t H[8] = {
            0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
            0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
        };

        std::vector<uint8_t> data(input.begin(), input.end());
        uint64_t bitLength = data.size() * 8;

        data.push_back(0x80);
        while ((data.size() % 64) != 56) {
            data.push_back(0x00);
        }

        for (int i = 7; i >= 0; --i) {
            data.push_back(static_cast<uint8_t>((bitLength >> (i * 8)) & 0xff));
        }

        for (size_t chunk = 0; chunk < data.size(); chunk += 64) {
            uint32_t W[64];
            for (int i = 0; i < 16; ++i) {
                W[i] = (static_cast<uint32_t>(data[chunk + i * 4]) << 24) |
                       (static_cast<uint32_t>(data[chunk + i * 4 + 1]) << 16) |
                       (static_cast<uint32_t>(data[chunk + i * 4 + 2]) << 8) |
                       (static_cast<uint32_t>(data[chunk + i * 4 + 3]));
            }
            for (int i = 16; i < 64; ++i) {
                W[i] = theta1(W[i - 2]) + W[i - 7] + theta0(W[i - 15]) + W[i - 16];
            }

            uint32_t a = H[0], b = H[1], c = H[2], d = H[3];
            uint32_t e = H[4], f = H[5], g = H[6], h = H[7];

            for (int i = 0; i < 64; ++i) {
                uint32_t T1 = h + sig1(e) + choose(e, f, g) + K[i] + W[i];
                uint32_t T2 = sig0(a) + majority(a, b, c);
                h = g;
                g = f;
                f = e;
                e = d + T1;
                d = c;
                c = b;
                b = a;
                a = T1 + T2;
            }

            H[0] += a; H[1] += b; H[2] += c; H[3] += d;
            H[4] += e; H[5] += f; H[6] += g; H[7] += h;
        }

        std::ostringstream oss;
        for (int i = 0; i < 8; ++i) {
            oss << std::hex << std::setw(8) << std::setfill('0') << H[i];
        }
        return oss.str();
    }

    // Base64 helper
    const std::string b64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

    std::string base64UrlEncode(const std::string& in) {
        std::string out;
        int val = 0, valb = -6;
        for (uint8_t c : in) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                out.push_back(b64_chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) out.push_back(b64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
        return out;
    }

    std::string base64UrlDecode(const std::string& in) {
        std::string out;
        std::vector<int> T(256, -1);
        for (int i = 0; i < 64; i++) T[b64_chars[i]] = i;

        int val = 0, valb = -8;
        for (uint8_t c : in) {
            if (T[c] == -1) break;
            val = (val << 6) + T[c];
            valb += 6;
            if (valb >= 0) {
                out.push_back(char((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
        return out;
    }
} // namespace

std::string Auth::hashPassword(const std::string& plainPassword) {
    return sha256Raw(SALT + plainPassword);
}

bool Auth::verifyPassword(const std::string& plainPassword, const std::string& storedHash) {
    return hashPassword(plainPassword) == storedHash;
}

std::string Auth::generateToken(int userId, const std::string& username, const std::string& role) {
    auto now = std::chrono::system_clock::now();
    auto epochSeconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

    std::string payload = std::to_string(userId) + ":" + username + ":" + role + ":" + std::to_string(epochSeconds);
    std::string signature = sha256Raw(payload + ":" + JWT_SECRET);
    std::string fullToken = payload + ":" + signature;

    return base64UrlEncode(fullToken);
}

bool Auth::verifyToken(const std::string& token, int& outUserId, std::string& outUsername, std::string& outRole) {
    std::string decoded = base64UrlDecode(token);
    std::istringstream iss(decoded);
    std::string userIdStr, username, role, tsStr, sig;

    if (!std::getline(iss, userIdStr, ':') ||
        !std::getline(iss, username, ':') ||
        !std::getline(iss, role, ':') ||
        !std::getline(iss, tsStr, ':') ||
        !std::getline(iss, sig)) {
        return false;
    }

    std::string payload = userIdStr + ":" + username + ":" + role + ":" + tsStr;
    std::string expectedSig = sha256Raw(payload + ":" + JWT_SECRET);
    if (sig != expectedSig) {
        return false;
    }

    // Check expiration (e.g. valid for 7 days)
    try {
        long long tokenTime = std::stoll(tsStr);
        auto now = std::chrono::system_clock::now();
        auto nowSeconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        if (nowSeconds - tokenTime > 7 * 24 * 3600) {
            return false; // Expired
        }

        outUserId = std::stoi(userIdStr);
        outUsername = username;
        outRole = role;
        return true;
    } catch (...) {
        return false;
    }
}

std::pair<bool, std::string> Auth::login(const std::string& usernameOrEmail,
                                         const std::string& password,
                                         nlohmann::json& outUserData) {
    Database& db = Database::getInstance();
    std::string cleanInput = db.escape(usernameOrEmail);

    std::string sql = "SELECT id, username, email, password_hash, role, created_at FROM users WHERE username = '" +
                      cleanInput + "' OR email = '" + cleanInput + "' LIMIT 1";

    auto rows = db.query(sql);
    if (rows.empty()) {
        return {false, "Invalid username or password."};
    }

    const auto& userRow = rows[0];
    std::string storedHash = userRow.at("password_hash");

    if (!verifyPassword(password, storedHash)) {
        return {false, "Invalid username or password."};
    }

    int userId = std::stoi(userRow.at("id"));
    std::string username = userRow.at("username");
    std::string email = userRow.at("email");
    std::string role = userRow.at("role");
    std::string token = generateToken(userId, username, role);

    outUserData = {
        {"token", token},
        {"user", {
            {"id", userId},
            {"username", username},
            {"email", email},
            {"role", role},
            {"created_at", userRow.at("created_at")}
        }}
    };

    return {true, "Login successful."};
}

std::pair<bool, std::string> Auth::changePassword(int userId,
                                                 const std::string& oldPassword,
                                                 const std::string& newPassword) {
    if (newPassword.length() < 6) {
        return {false, "New password must be at least 6 characters long."};
    }

    Database& db = Database::getInstance();
    std::string sql = "SELECT password_hash FROM users WHERE id = " + std::to_string(userId) + " LIMIT 1";
    auto rows = db.query(sql);
    if (rows.empty()) {
        return {false, "User not found."};
    }

    std::string storedHash = rows[0].at("password_hash");
    if (!verifyPassword(oldPassword, storedHash)) {
        return {false, "Current password does not match."};
    }

    std::string newHash = hashPassword(newPassword);
    std::string updateSql = "UPDATE users SET password_hash = '" + db.escape(newHash) + "' WHERE id = " + std::to_string(userId);
    if (!db.execute(updateSql)) {
        return {false, "Failed to update password: " + db.getLastError()};
    }

    return {true, "Password updated successfully."};
}

std::pair<bool, User> Auth::getUserById(int userId) {
    Database& db = Database::getInstance();
    std::string sql = "SELECT id, username, email, role, created_at FROM users WHERE id = " + std::to_string(userId) + " LIMIT 1";
    auto rows = db.query(sql);
    if (rows.empty()) {
        return {false, User{}};
    }

    User u;
    u.id = std::stoi(rows[0].at("id"));
    u.username = rows[0].at("username");
    u.email = rows[0].at("email");
    u.role = rows[0].at("role");
    u.createdAt = rows[0].at("created_at");
    return {true, u};
}
