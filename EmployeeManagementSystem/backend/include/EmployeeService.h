#pragma once

#include <vector>
#include <string>
#include <map>
#include <optional>
#include <utility>
#include "Employee.h"
#include "json.hpp"

class EmployeeService {
public:
    // Core Employee CRUD
    static std::vector<Employee> getAllEmployees(const std::map<std::string, std::string>& filters = {});
    static std::optional<Employee> getEmployeeById(int id);
    static std::optional<Employee> getEmployeeByEmployeeId(const std::string& employeeId);
    static std::vector<Employee> searchEmployees(const std::string& query);
    static std::pair<bool, std::string> addEmployee(const Employee& emp);
    static std::pair<bool, std::string> updateEmployee(int id, const Employee& emp);
    static std::pair<bool, std::string> deleteEmployee(int id);

    // Dashboard Statistics & Data Visualization
    static nlohmann::json getDashboardStats();

    // Department Management
    static std::vector<nlohmann::json> getAllDepartments();
    static std::pair<bool, std::string> addDepartment(const std::string& name, const std::string& desc);
    static std::pair<bool, std::string> updateDepartment(int id, const std::string& name, const std::string& desc);
    static std::pair<bool, std::string> deleteDepartment(int id);

    // Reports & CSV Export
    static nlohmann::json getReportStats(const std::map<std::string, std::string>& filters = {});
    static std::string exportEmployeesCsv(const std::map<std::string, std::string>& filters = {});
};
