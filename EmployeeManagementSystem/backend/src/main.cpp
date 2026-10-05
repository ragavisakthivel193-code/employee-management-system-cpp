#define WIN32_LEAN_AND_MEAN
#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include "httplib.h"
#include "json.hpp"
#include "Database.h"
#include "Employee.h"
#include "EmployeeService.h"
#include "Auth.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <memory>
#include <filesystem>

namespace fs = std::filesystem;

// Configuration loader for .env / config
struct AppConfig {
    std::string dbHost = "127.0.0.1";
    int dbPort = 3306;
    std::string dbName = "employee_management";
    std::string dbUser = "root";
    std::string dbPassword = "root";
    int serverPort = 8080;
    std::string jwtSecret = "ems_jwt_super_secure_key_2026_ragavi";
};

AppConfig loadConfig(const std::string& envPath) {
    AppConfig config;
    std::ifstream file(envPath);
    if (!file.is_open()) {
        std::cout << "[INFO] Config file not found at " << envPath << ", checking environment variables or defaults." << std::endl;
    } else {
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            size_t eqPos = line.find('=');
            if (eqPos != std::string::npos) {
                std::string key = line.substr(0, eqPos);
                std::string val = line.substr(eqPos + 1);
                // Trim carriage returns/spaces
                while (!val.empty() && (val.back() == '\r' || val.back() == ' ' || val.back() == '\t')) val.pop_back();
                while (!val.empty() && (val.front() == ' ' || val.front() == '\t')) val.erase(val.begin());

                if (key == "DB_HOST" && !val.empty()) config.dbHost = val;
                else if (key == "DB_PORT" && !val.empty()) config.dbPort = std::stoi(val);
                else if (key == "DB_NAME" && !val.empty()) config.dbName = val;
                else if (key == "DB_USER" && !val.empty()) config.dbUser = val;
                else if (key == "DB_PASSWORD") config.dbPassword = val;
                else if (key == "PORT" && !val.empty()) config.serverPort = std::stoi(val);
                else if (key == "JWT_SECRET" && !val.empty()) config.jwtSecret = val;
            }
        }
    }

    // Check system environment overrides
    const char* envDbHost = std::getenv("DB_HOST");
    if (envDbHost) config.dbHost = envDbHost;
    const char* envDbPort = std::getenv("DB_PORT");
    if (envDbPort) config.dbPort = std::stoi(envDbPort);
    const char* envDbName = std::getenv("DB_NAME");
    if (envDbName) config.dbName = envDbName;
    const char* envDbUser = std::getenv("DB_USER");
    if (envDbUser) config.dbUser = envDbUser;
    const char* envDbPass = std::getenv("DB_PASSWORD");
    if (envDbPass) config.dbPassword = envDbPass;
    const char* envPort = std::getenv("PORT");
    if (envPort) config.serverPort = std::stoi(envPort);

    return config;
}

// JSON Response Utilities
void sendJsonSuccess(httplib::Response& res, const std::string& message, const nlohmann::json& data = nlohmann::json::object(), int statusCode = 200) {
    nlohmann::json body = {
        {"success", true},
        {"message", message},
        {"data", data}
    };
    res.status = statusCode;
    res.set_content(body.dump(), "application/json");
}

void sendJsonError(httplib::Response& res, const std::string& message, int statusCode = 400) {
    nlohmann::json body = {
        {"success", false},
        {"message", message}
    };
    res.status = statusCode;
    res.set_content(body.dump(), "application/json");
}

// Token Extraction and Authentication Helper
struct AuthContext {
    bool authenticated = false;
    int userId = 0;
    std::string username;
    std::string role;
};

AuthContext extractAuthContext(const httplib::Request& req) {
    AuthContext ctx;
    std::string token;

    if (req.has_header("Authorization")) {
        std::string authHeader = req.get_header_value("Authorization");
        if (authHeader.rfind("Bearer ", 0) == 0) {
            token = authHeader.substr(7);
        }
    } else if (req.has_header("X-Auth-Token")) {
        token = req.get_header_value("X-Auth-Token");
    }

    if (!token.empty()) {
        if (Auth::verifyToken(token, ctx.userId, ctx.username, ctx.role)) {
            ctx.authenticated = true;
        }
    }

    return ctx;
}

int main(int argc, char* argv[]) {
    // UTF-8 console output setup on Windows
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::cout << "==========================================================" << std::endl;
    std::cout << "  EMPLOYEE MANAGEMENT SYSTEM - C++ REST API SERVER" << std::endl;
    std::cout << "==========================================================" << std::endl;

    // Determine config path
    std::string envPath = "config/.env";
    if (!fs::exists(envPath)) {
        if (fs::exists("backend/config/.env")) envPath = "backend/config/.env";
        else if (fs::exists("../backend/config/.env")) envPath = "../backend/config/.env";
        else if (fs::exists("../../backend/config/.env")) envPath = "../../backend/config/.env";
    }

    AppConfig config = loadConfig(envPath);

    std::cout << "[DB] Connecting to MySQL at " << config.dbHost << ":" << config.dbPort
              << " | Database: " << config.dbName
              << " | User: " << config.dbUser << std::endl;

    Database& db = Database::getInstance();
    if (!db.connect(config.dbHost, config.dbUser, config.dbPassword, config.dbName, config.dbPort)) {
        std::cerr << "[FATAL] Failed to connect to MySQL: " << db.getLastError() << std::endl;
        std::cerr << "[HINT] Make sure MySQL is running and password matches config in .env" << std::endl;
        return 1;
    }
    std::cout << "[DB] Connected to MySQL successfully!" << std::endl;

    httplib::Server svr;

    // CORS & Common Headers Middleware
    svr.set_post_routing_handler([](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Auth-Token, X-Requested-With");
    });

    // Handle OPTIONS Preflight Requests
    svr.Options(R"(/.*)", [](const httplib::Request&, httplib::Response& res) {
        res.status = 204;
    });

    // -------------------------------------------------------------
    // AUTHENTICATION ROUTES
    // -------------------------------------------------------------

    // POST /api/auth/login
    svr.Post("/api/auth/login", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = nlohmann::json::parse(req.body);
            std::string usernameOrEmail = body.value("username", "");
            std::string password = body.value("password", "");

            if (usernameOrEmail.empty() || password.empty()) {
                sendJsonError(res, "Username/email and password are required.", 400);
                return;
            }

            nlohmann::json userData;
            auto [success, message] = Auth::login(usernameOrEmail, password, userData);
            if (!success) {
                sendJsonError(res, message, 401);
                return;
            }

            sendJsonSuccess(res, message, userData);
        } catch (const std::exception& e) {
            sendJsonError(res, std::string("Invalid request format: ") + e.what(), 400);
        }
    });

    // POST /api/auth/logout
    svr.Post("/api/auth/logout", [](const httplib::Request&, httplib::Response& res) {
        sendJsonSuccess(res, "Logged out successfully.");
    });

    // GET /api/auth/me
    svr.Get("/api/auth/me", [](const httplib::Request& req, httplib::Response& res) {
        AuthContext auth = extractAuthContext(req);
        if (!auth.authenticated) {
            sendJsonError(res, "Unauthorized. Please log in.", 401);
            return;
        }

        auto [found, user] = Auth::getUserById(auth.userId);
        if (!found) {
            sendJsonError(res, "User not found.", 404);
            return;
        }

        sendJsonSuccess(res, "User profile retrieved.", user.toJson());
    });

    // POST /api/auth/change-password
    svr.Post("/api/auth/change-password", [](const httplib::Request& req, httplib::Response& res) {
        AuthContext auth = extractAuthContext(req);
        if (!auth.authenticated) {
            sendJsonError(res, "Unauthorized.", 401);
            return;
        }

        try {
            auto body = nlohmann::json::parse(req.body);
            std::string oldPassword = body.value("old_password", "");
            std::string newPassword = body.value("new_password", "");

            auto [success, message] = Auth::changePassword(auth.userId, oldPassword, newPassword);
            if (!success) {
                sendJsonError(res, message, 400);
                return;
            }

            sendJsonSuccess(res, message);
        } catch (const std::exception& e) {
            sendJsonError(res, std::string("Error: ") + e.what(), 400);
        }
    });

    // -------------------------------------------------------------
    // DASHBOARD & STATS ROUTES
    // -------------------------------------------------------------

    // GET /api/dashboard/stats
    svr.Get("/api/dashboard/stats", [](const httplib::Request&, httplib::Response& res) {
        try {
            nlohmann::json stats = EmployeeService::getDashboardStats();
            sendJsonSuccess(res, "Dashboard statistics retrieved.", stats);
        } catch (const std::exception& e) {
            sendJsonError(res, std::string("Error retrieving statistics: ") + e.what(), 500);
        }
    });

    // -------------------------------------------------------------
    // EMPLOYEE MANAGEMENT ROUTES
    // -------------------------------------------------------------

    // GET /api/employees/search?query=val
    svr.Get("/api/employees/search", [](const httplib::Request& req, httplib::Response& res) {
        std::string query = req.get_param_value("query");
        try {
            auto list = EmployeeService::searchEmployees(query);
            nlohmann::json jsonList = nlohmann::json::array();
            for (const auto& emp : list) {
                jsonList.push_back(emp.toJson());
            }
            sendJsonSuccess(res, "Employees search completed.", jsonList);
        } catch (const std::exception& e) {
            sendJsonError(res, std::string("Search error: ") + e.what(), 500);
        }
    });

    // GET /api/employees (with optional filters)
    svr.Get("/api/employees", [](const httplib::Request& req, httplib::Response& res) {
        std::map<std::string, std::string> filters;
        if (req.has_param("department")) filters["department"] = req.get_param_value("department");
        if (req.has_param("designation")) filters["designation"] = req.get_param_value("designation");
        if (req.has_param("gender")) filters["gender"] = req.get_param_value("gender");
        if (req.has_param("status")) filters["status"] = req.get_param_value("status");
        if (req.has_param("min_salary")) filters["min_salary"] = req.get_param_value("min_salary");
        if (req.has_param("max_salary")) filters["max_salary"] = req.get_param_value("max_salary");
        if (req.has_param("search")) filters["search"] = req.get_param_value("search");

        try {
            auto list = EmployeeService::getAllEmployees(filters);
            nlohmann::json jsonList = nlohmann::json::array();
            for (const auto& emp : list) {
                jsonList.push_back(emp.toJson());
            }
            sendJsonSuccess(res, "Employees retrieved successfully.", jsonList);
        } catch (const std::exception& e) {
            sendJsonError(res, std::string("Error fetching employees: ") + e.what(), 500);
        }
    });

    // GET /api/employees/:id
    svr.Get(R"(/api/employees/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        int id = std::stoi(req.matches[1]);
        auto empOpt = EmployeeService::getEmployeeById(id);
        if (!empOpt.has_value()) {
            sendJsonError(res, "Employee not found.", 404);
            return;
        }

        sendJsonSuccess(res, "Employee found.", empOpt->toJson());
    });

    // POST /api/employees (Create Employee)
    svr.Post("/api/employees", [](const httplib::Request& req, httplib::Response& res) {
        AuthContext auth = extractAuthContext(req);
        if (!auth.authenticated) {
            sendJsonError(res, "Unauthorized. Authentication required to add employees.", 401);
            return;
        }

        try {
            auto body = nlohmann::json::parse(req.body);
            Employee emp = Employee::fromJson(body);

            auto [success, message] = EmployeeService::addEmployee(emp);
            if (!success) {
                sendJsonError(res, message, 400);
                return;
            }

            sendJsonSuccess(res, message, emp.toJson(), 201);
        } catch (const std::exception& e) {
            sendJsonError(res, std::string("Invalid employee data: ") + e.what(), 400);
        }
    });

    // PUT /api/employees/:id (Update Employee)
    svr.Put(R"(/api/employees/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        AuthContext auth = extractAuthContext(req);
        if (!auth.authenticated) {
            sendJsonError(res, "Unauthorized. Authentication required to edit employees.", 401);
            return;
        }

        int id = std::stoi(req.matches[1]);
        try {
            auto body = nlohmann::json::parse(req.body);
            Employee emp = Employee::fromJson(body);

            auto [success, message] = EmployeeService::updateEmployee(id, emp);
            if (!success) {
                sendJsonError(res, message, 400);
                return;
            }

            sendJsonSuccess(res, message);
        } catch (const std::exception& e) {
            sendJsonError(res, std::string("Invalid update data: ") + e.what(), 400);
        }
    });

    // DELETE /api/employees/:id (Delete Employee)
    svr.Delete(R"(/api/employees/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        AuthContext auth = extractAuthContext(req);
        if (!auth.authenticated) {
            sendJsonError(res, "Unauthorized. Authentication required.", 401);
            return;
        }

        // Role-based access control (RBAC): Admin has full deletion rights; HR has restricted deletion
        if (auth.role != "Admin") {
            sendJsonError(res, "Forbidden: Only administrators have permission to delete employee records.", 403);
            return;
        }

        int id = std::stoi(req.matches[1]);
        auto [success, message] = EmployeeService::deleteEmployee(id);
        if (!success) {
            sendJsonError(res, message, 400);
            return;
        }

        sendJsonSuccess(res, message);
    });

    // -------------------------------------------------------------
    // DEPARTMENT MANAGEMENT ROUTES
    // -------------------------------------------------------------

    // GET /api/departments
    svr.Get("/api/departments", [](const httplib::Request&, httplib::Response& res) {
        try {
            auto depts = EmployeeService::getAllDepartments();
            sendJsonSuccess(res, "Departments retrieved successfully.", depts);
        } catch (const std::exception& e) {
            sendJsonError(res, std::string("Error fetching departments: ") + e.what(), 500);
        }
    });

    // POST /api/departments
    svr.Post("/api/departments", [](const httplib::Request& req, httplib::Response& res) {
        AuthContext auth = extractAuthContext(req);
        if (!auth.authenticated || auth.role != "Admin") {
            sendJsonError(res, "Forbidden: Only administrators can create departments.", 403);
            return;
        }

        try {
            auto body = nlohmann::json::parse(req.body);
            std::string name = body.value("department_name", "");
            std::string desc = body.value("description", "");

            auto [success, message] = EmployeeService::addDepartment(name, desc);
            if (!success) {
                sendJsonError(res, message, 400);
                return;
            }

            sendJsonSuccess(res, message, nlohmann::json::object(), 201);
        } catch (const std::exception& e) {
            sendJsonError(res, std::string("Error: ") + e.what(), 400);
        }
    });

    // PUT /api/departments/:id
    svr.Put(R"(/api/departments/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        AuthContext auth = extractAuthContext(req);
        if (!auth.authenticated || auth.role != "Admin") {
            sendJsonError(res, "Forbidden: Only administrators can edit departments.", 403);
            return;
        }

        int id = std::stoi(req.matches[1]);
        try {
            auto body = nlohmann::json::parse(req.body);
            std::string name = body.value("department_name", "");
            std::string desc = body.value("description", "");

            auto [success, message] = EmployeeService::updateDepartment(id, name, desc);
            if (!success) {
                sendJsonError(res, message, 400);
                return;
            }

            sendJsonSuccess(res, message);
        } catch (const std::exception& e) {
            sendJsonError(res, std::string("Error: ") + e.what(), 400);
        }
    });

    // DELETE /api/departments/:id
    svr.Delete(R"(/api/departments/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        AuthContext auth = extractAuthContext(req);
        if (!auth.authenticated || auth.role != "Admin") {
            sendJsonError(res, "Forbidden: Only administrators can delete departments.", 403);
            return;
        }

        int id = std::stoi(req.matches[1]);
        auto [success, message] = EmployeeService::deleteDepartment(id);
        if (!success) {
            sendJsonError(res, message, 400);
            return;
        }

        sendJsonSuccess(res, message);
    });

    // -------------------------------------------------------------
    // REPORTS & EXPORT ROUTES
    // -------------------------------------------------------------

    // GET /api/reports/stats
    svr.Get("/api/reports/stats", [](const httplib::Request& req, httplib::Response& res) {
        std::map<std::string, std::string> filters;
        if (req.has_param("department")) filters["department"] = req.get_param_value("department");
        if (req.has_param("status")) filters["status"] = req.get_param_value("status");

        try {
            nlohmann::json stats = EmployeeService::getReportStats(filters);
            sendJsonSuccess(res, "Report statistics generated.", stats);
        } catch (const std::exception& e) {
            sendJsonError(res, std::string("Report error: ") + e.what(), 500);
        }
    });

    // GET /api/reports/export (CSV export)
    svr.Get("/api/reports/export", [](const httplib::Request& req, httplib::Response& res) {
        std::map<std::string, std::string> filters;
        if (req.has_param("department")) filters["department"] = req.get_param_value("department");
        if (req.has_param("status")) filters["status"] = req.get_param_value("status");

        try {
            std::string csv = EmployeeService::exportEmployeesCsv(filters);
            res.set_header("Content-Disposition", "attachment; filename=\"employees_report.csv\"");
            res.set_content(csv, "text/csv; charset=utf-8");
        } catch (const std::exception& e) {
            sendJsonError(res, std::string("CSV export error: ") + e.what(), 500);
        }
    });

    // -------------------------------------------------------------
    // FRONTEND STATIC FILES SERVING & REDIRECTS
    // -------------------------------------------------------------
    std::string frontendDir = "";
    if (fs::exists("../frontend")) frontendDir = "../frontend";
    else if (fs::exists("../../frontend")) frontendDir = "../../frontend";
    else if (fs::exists("frontend")) frontendDir = "frontend";
    else if (fs::exists("EmployeeManagementSystem/frontend")) frontendDir = "EmployeeManagementSystem/frontend";

    if (!frontendDir.empty()) {
        std::cout << "[STATIC] Serving frontend assets from: " << fs::canonical(frontendDir).string() << std::endl;
        svr.set_mount_point("/", frontendDir);
    }

    // Default redirect to /index.html
    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_redirect("/index.html");
    });

    // Dashboard clean route
    svr.Get("/dashboard", [](const httplib::Request&, httplib::Response& res) {
        res.set_redirect("/dashboard.html");
    });

    std::cout << "[SERVER] Starting REST API Server at http://localhost:" << config.serverPort << std::endl;
    std::cout << "[INFO] Ready to accept frontend requests." << std::endl;
    std::cout << "==========================================================" << std::endl;

    svr.listen("0.0.0.0", config.serverPort);

    return 0;
}
