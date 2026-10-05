#include "EmployeeService.h"
#include "Database.h"
#include <sstream>
#include <iomanip>
#include <iostream>

namespace {
    Employee rowToEmployee(const std::map<std::string, std::string>& row) {
        Employee emp;
        if (row.count("id") && !row.at("id").empty()) emp.setId(std::stoi(row.at("id")));
        if (row.count("employee_id")) emp.setEmployeeId(row.at("employee_id"));
        if (row.count("name")) emp.setName(row.at("name"));
        if (row.count("age") && !row.at("age").empty()) emp.setAge(std::stoi(row.at("age")));
        if (row.count("gender")) emp.setGender(row.at("gender"));
        if (row.count("email")) emp.setEmail(row.at("email"));
        if (row.count("phone")) emp.setPhone(row.at("phone"));
        if (row.count("address")) emp.setAddress(row.at("address"));
        if (row.count("department")) emp.setDepartment(row.at("department"));
        if (row.count("designation")) emp.setDesignation(row.at("designation"));
        if (row.count("salary") && !row.at("salary").empty()) emp.setSalary(std::stod(row.at("salary")));
        if (row.count("joining_date")) emp.setJoiningDate(row.at("joining_date"));
        if (row.count("status")) emp.setStatus(row.at("status"));
        if (row.count("created_at")) emp.setCreatedAt(row.at("created_at"));
        if (row.count("updated_at")) emp.setUpdatedAt(row.at("updated_at"));
        return emp;
    }
}

std::vector<Employee> EmployeeService::getAllEmployees(const std::map<std::string, std::string>& filters) {
    Database& db = Database::getInstance();
    std::string sql = "SELECT * FROM employees WHERE 1=1";

    if (filters.count("department") && !filters.at("department").empty() && filters.at("department") != "All") {
        sql += " AND department = '" + db.escape(filters.at("department")) + "'";
    }
    if (filters.count("designation") && !filters.at("designation").empty() && filters.at("designation") != "All") {
        sql += " AND designation LIKE '%" + db.escape(filters.at("designation")) + "%'";
    }
    if (filters.count("gender") && !filters.at("gender").empty() && filters.at("gender") != "All") {
        sql += " AND gender = '" + db.escape(filters.at("gender")) + "'";
    }
    if (filters.count("status") && !filters.at("status").empty() && filters.at("status") != "All") {
        sql += " AND status = '" + db.escape(filters.at("status")) + "'";
    }
    if (filters.count("min_salary") && !filters.at("min_salary").empty()) {
        try {
            double minSal = std::stod(filters.at("min_salary"));
            sql += " AND salary >= " + std::to_string(minSal);
        } catch (...) {}
    }
    if (filters.count("max_salary") && !filters.at("max_salary").empty()) {
        try {
            double maxSal = std::stod(filters.at("max_salary"));
            sql += " AND salary <= " + std::to_string(maxSal);
        } catch (...) {}
    }
    if (filters.count("search") && !filters.at("search").empty()) {
        std::string q = db.escape(filters.at("search"));
        sql += " AND (employee_id LIKE '%" + q + "%' OR name LIKE '%" + q + "%' OR email LIKE '%" + q + "%' OR department LIKE '%" + q + "%' OR designation LIKE '%" + q + "%')";
    }

    sql += " ORDER BY id DESC";

    auto rows = db.query(sql);
    std::vector<Employee> list;
    list.reserve(rows.size());
    for (const auto& row : rows) {
        list.push_back(rowToEmployee(row));
    }
    return list;
}

std::optional<Employee> EmployeeService::getEmployeeById(int id) {
    Database& db = Database::getInstance();
    std::string sql = "SELECT * FROM employees WHERE id = " + std::to_string(id) + " LIMIT 1";
    auto rows = db.query(sql);
    if (rows.empty()) {
        return std::nullopt;
    }
    return rowToEmployee(rows[0]);
}

std::optional<Employee> EmployeeService::getEmployeeByEmployeeId(const std::string& employeeId) {
    Database& db = Database::getInstance();
    std::string sql = "SELECT * FROM employees WHERE employee_id = '" + db.escape(employeeId) + "' LIMIT 1";
    auto rows = db.query(sql);
    if (rows.empty()) {
        return std::nullopt;
    }
    return rowToEmployee(rows[0]);
}

std::vector<Employee> EmployeeService::searchEmployees(const std::string& query) {
    std::map<std::string, std::string> filters;
    filters["search"] = query;
    return getAllEmployees(filters);
}

std::pair<bool, std::string> EmployeeService::addEmployee(const Employee& emp) {
    std::string errorMsg;
    if (!emp.validate(errorMsg)) {
        return {false, errorMsg};
    }

    Database& db = Database::getInstance();

    // Check unique employee_id
    std::string checkIdSql = "SELECT id FROM employees WHERE employee_id = '" + db.escape(emp.getEmployeeId()) + "' LIMIT 1";
    if (!db.query(checkIdSql).empty()) {
        return {false, "Employee ID '" + emp.getEmployeeId() + "' already exists."};
    }

    // Check unique email
    std::string checkEmailSql = "SELECT id FROM employees WHERE email = '" + db.escape(emp.getEmail()) + "' LIMIT 1";
    if (!db.query(checkEmailSql).empty()) {
        return {false, "Email '" + emp.getEmail() + "' is already registered to another employee."};
    }

    std::ostringstream oss;
    oss << "INSERT INTO employees (employee_id, name, age, gender, email, phone, address, department, designation, salary, joining_date, status) VALUES ("
        << "'" << db.escape(emp.getEmployeeId()) << "', "
        << "'" << db.escape(emp.getName()) << "', "
        << emp.getAge() << ", "
        << "'" << db.escape(emp.getGender()) << "', "
        << "'" << db.escape(emp.getEmail()) << "', "
        << "'" << db.escape(emp.getPhone()) << "', "
        << "'" << db.escape(emp.getAddress()) << "', "
        << "'" << db.escape(emp.getDepartment()) << "', "
        << "'" << db.escape(emp.getDesignation()) << "', "
        << std::fixed << std::setprecision(2) << emp.getSalary() << ", "
        << "'" << db.escape(emp.getJoiningDate()) << "', "
        << "'" << db.escape(emp.getStatus()) << "')";

    unsigned long long newId = 0;
    if (!db.execute(oss.str(), &newId)) {
        return {false, "Database error: " + db.getLastError()};
    }

    return {true, "Employee added successfully."};
}

std::pair<bool, std::string> EmployeeService::updateEmployee(int id, const Employee& emp) {
    Database& db = Database::getInstance();

    // Check existence
    auto existingOpt = getEmployeeById(id);
    if (!existingOpt.has_value()) {
        return {false, "Employee not found."};
    }

    Employee toUpdate = emp;
    // Employee ID is immutable and preserved from existing record (adhering to Section 12)
    toUpdate.setEmployeeId(existingOpt->getEmployeeId());

    std::string errorMsg;
    if (!toUpdate.validate(errorMsg)) {
        return {false, errorMsg};
    }

    // Ensure email is not duplicated for any OTHER employee
    std::string checkEmailSql = "SELECT id FROM employees WHERE email = '" + db.escape(toUpdate.getEmail()) + "' AND id != " + std::to_string(id) + " LIMIT 1";
    if (!db.query(checkEmailSql).empty()) {
        return {false, "Email '" + toUpdate.getEmail() + "' is already in use by another employee."};
    }

    std::ostringstream oss;
    oss << "UPDATE employees SET "
        << "name = '" << db.escape(toUpdate.getName()) << "', "
        << "age = " << toUpdate.getAge() << ", "
        << "gender = '" << db.escape(toUpdate.getGender()) << "', "
        << "email = '" << db.escape(toUpdate.getEmail()) << "', "
        << "phone = '" << db.escape(toUpdate.getPhone()) << "', "
        << "address = '" << db.escape(toUpdate.getAddress()) << "', "
        << "department = '" << db.escape(toUpdate.getDepartment()) << "', "
        << "designation = '" << db.escape(toUpdate.getDesignation()) << "', "
        << "salary = " << std::fixed << std::setprecision(2) << toUpdate.getSalary() << ", "
        << "joining_date = '" << db.escape(toUpdate.getJoiningDate()) << "', "
        << "status = '" << db.escape(toUpdate.getStatus()) << "' "
        << "WHERE id = " << id;

    if (!db.execute(oss.str())) {
        return {false, "Database update error: " + db.getLastError()};
    }

    return {true, "Employee updated successfully."};
}

std::pair<bool, std::string> EmployeeService::deleteEmployee(int id) {
    Database& db = Database::getInstance();

    auto existingOpt = getEmployeeById(id);
    if (!existingOpt.has_value()) {
        return {false, "Employee not found."};
    }

    std::string sql = "DELETE FROM employees WHERE id = " + std::to_string(id);
    if (!db.execute(sql)) {
        return {false, "Database delete error: " + db.getLastError()};
    }

    return {true, "Employee deleted successfully."};
}

nlohmann::json EmployeeService::getDashboardStats() {
    Database& db = Database::getInstance();
    nlohmann::json stats;

    // 1. Total Employees
    auto totalRows = db.query("SELECT COUNT(*) AS total FROM employees");
    int totalEmployees = totalRows.empty() ? 0 : std::stoi(totalRows[0].at("total"));

    // 2. Active Employees
    auto activeRows = db.query("SELECT COUNT(*) AS active FROM employees WHERE status = 'Active'");
    int activeEmployees = activeRows.empty() ? 0 : std::stoi(activeRows[0].at("active"));

    // 3. Inactive Employees & On Leave
    auto inactiveRows = db.query("SELECT COUNT(*) AS cnt FROM employees WHERE status = 'Inactive'");
    int inactiveEmployees = inactiveRows.empty() ? 0 : std::stoi(inactiveRows[0].at("cnt"));

    auto onLeaveRows = db.query("SELECT COUNT(*) AS cnt FROM employees WHERE status = 'On Leave'");
    int onLeaveEmployees = onLeaveRows.empty() ? 0 : std::stoi(onLeaveRows[0].at("cnt"));

    // 4. Total Departments
    auto deptCountRows = db.query("SELECT COUNT(*) AS total_depts FROM departments");
    int totalDepartments = deptCountRows.empty() ? 0 : std::stoi(deptCountRows[0].at("total_depts"));

    // 5. Salary aggregations
    auto salRows = db.query("SELECT IFNULL(AVG(salary), 0) AS avg_sal, IFNULL(MAX(salary), 0) AS max_sal, IFNULL(MIN(salary), 0) AS min_sal, IFNULL(SUM(salary), 0) AS total_payroll FROM employees");
    double avgSalary = 0.0, maxSalary = 0.0, minSalary = 0.0, totalPayroll = 0.0;
    if (!salRows.empty()) {
        try {
            avgSalary = std::stod(salRows[0].at("avg_sal"));
            maxSalary = std::stod(salRows[0].at("max_sal"));
            minSalary = std::stod(salRows[0].at("min_sal"));
            totalPayroll = std::stod(salRows[0].at("total_payroll"));
        } catch (...) {}
    }

    // 6. Employees by Department
    auto deptRows = db.query("SELECT department, COUNT(*) AS count FROM employees GROUP BY department ORDER BY count DESC");
    nlohmann::json deptBreakdown = nlohmann::json::array();
    for (const auto& r : deptRows) {
        deptBreakdown.push_back({
            {"department", r.at("department")},
            {"count", std::stoi(r.at("count"))}
        });
    }

    // 7. Employees by Gender
    auto genderRows = db.query("SELECT gender, COUNT(*) AS count FROM employees GROUP BY gender");
    nlohmann::json genderBreakdown = nlohmann::json::array();
    for (const auto& r : genderRows) {
        genderBreakdown.push_back({
            {"gender", r.at("gender")},
            {"count", std::stoi(r.at("count"))}
        });
    }

    // 8. Salary Distribution by Department
    auto salDistRows = db.query("SELECT department, ROUND(AVG(salary), 2) AS avg_sal FROM employees GROUP BY department ORDER BY avg_sal DESC");
    nlohmann::json salaryByDept = nlohmann::json::array();
    for (const auto& r : salDistRows) {
        salaryByDept.push_back({
            {"department", r.at("department")},
            {"avg_salary", std::stod(r.at("avg_sal"))}
        });
    }

    // 9. Employee Joining Trend
    auto trendRows = db.query("SELECT DATE_FORMAT(joining_date, '%Y-%m') AS join_month, COUNT(*) AS count FROM employees GROUP BY join_month ORDER BY join_month ASC");
    nlohmann::json joiningTrend = nlohmann::json::array();
    for (const auto& r : trendRows) {
        joiningTrend.push_back({
            {"month", r.at("join_month")},
            {"count", std::stoi(r.at("count"))}
        });
    }

    // 10. Recent Employees
    auto recentRows = db.query("SELECT * FROM employees ORDER BY id DESC LIMIT 5");
    nlohmann::json recentList = nlohmann::json::array();
    for (const auto& r : recentRows) {
        recentList.push_back(rowToEmployee(r).toJson());
    }

    stats["total_employees"] = totalEmployees;
    stats["active_employees"] = activeEmployees;
    stats["inactive_employees"] = inactiveEmployees;
    stats["on_leave_employees"] = onLeaveEmployees;
    stats["total_departments"] = totalDepartments;
    stats["average_salary"] = avgSalary;
    stats["max_salary"] = maxSalary;
    stats["min_salary"] = minSalary;
    stats["total_payroll"] = totalPayroll;
    stats["by_department"] = deptBreakdown;
    stats["by_gender"] = genderBreakdown;
    stats["salary_by_department"] = salaryByDept;
    stats["joining_trend"] = joiningTrend;
    stats["recent_employees"] = recentList;

    return stats;
}

std::vector<nlohmann::json> EmployeeService::getAllDepartments() {
    Database& db = Database::getInstance();
    std::string sql = "SELECT d.id, d.department_name, d.description, d.created_at, "
                      "COUNT(e.id) AS employee_count "
                      "FROM departments d "
                      "LEFT JOIN employees e ON d.department_name = e.department "
                      "GROUP BY d.id, d.department_name, d.description, d.created_at "
                      "ORDER BY d.department_name ASC";
    auto rows = db.query(sql);
    std::vector<nlohmann::json> list;
    for (const auto& r : rows) {
        list.push_back({
            {"id", std::stoi(r.at("id"))},
            {"department_name", r.at("department_name")},
            {"description", r.at("description")},
            {"created_at", r.at("created_at")},
            {"employee_count", std::stoi(r.at("employee_count"))}
        });
    }
    return list;
}

std::pair<bool, std::string> EmployeeService::addDepartment(const std::string& name, const std::string& desc) {
    if (name.empty()) {
        return {false, "Department name is required."};
    }
    Database& db = Database::getInstance();
    std::string checkSql = "SELECT id FROM departments WHERE department_name = '" + db.escape(name) + "' LIMIT 1";
    if (!db.query(checkSql).empty()) {
        return {false, "Department '" + name + "' already exists."};
    }

    std::string sql = "INSERT INTO departments (department_name, description) VALUES ('" + db.escape(name) + "', '" + db.escape(desc) + "')";
    if (!db.execute(sql)) {
        return {false, "Failed to create department: " + db.getLastError()};
    }
    return {true, "Department added successfully."};
}

std::pair<bool, std::string> EmployeeService::updateDepartment(int id, const std::string& name, const std::string& desc) {
    if (name.empty()) {
        return {false, "Department name is required."};
    }
    Database& db = Database::getInstance();

    auto checkDept = db.query("SELECT department_name FROM departments WHERE id = " + std::to_string(id) + " LIMIT 1");
    if (checkDept.empty()) {
        return {false, "Department not found."};
    }
    std::string oldName = checkDept[0].at("department_name");

    std::string checkDup = "SELECT id FROM departments WHERE department_name = '" + db.escape(name) + "' AND id != " + std::to_string(id) + " LIMIT 1";
    if (!db.query(checkDup).empty()) {
        return {false, "Department name '" + name + "' already exists."};
    }

    std::string updateSql = "UPDATE departments SET department_name = '" + db.escape(name) + "', description = '" + db.escape(desc) + "' WHERE id = " + std::to_string(id);
    if (!db.execute(updateSql)) {
        return {false, "Failed to update department: " + db.getLastError()};
    }

    // Also cascade rename department in employees table if name changed
    if (oldName != name) {
        db.execute("UPDATE employees SET department = '" + db.escape(name) + "' WHERE department = '" + db.escape(oldName) + "'");
    }

    return {true, "Department updated successfully."};
}

std::pair<bool, std::string> EmployeeService::deleteDepartment(int id) {
    Database& db = Database::getInstance();
    auto deptRows = db.query("SELECT department_name FROM departments WHERE id = " + std::to_string(id) + " LIMIT 1");
    if (deptRows.empty()) {
        return {false, "Department not found."};
    }

    std::string deptName = deptRows[0].at("department_name");

    // Guard against deleting department with assigned employees (Section 28)
    auto empCheck = db.query("SELECT COUNT(*) AS count FROM employees WHERE department = '" + db.escape(deptName) + "'");
    int count = empCheck.empty() ? 0 : std::stoi(empCheck[0].at("count"));
    if (count > 0) {
        return {false, "Cannot delete department '" + deptName + "' because " + std::to_string(count) + " employee(s) are currently assigned to it. Please reassign them first."};
    }

    std::string delSql = "DELETE FROM departments WHERE id = " + std::to_string(id);
    if (!db.execute(delSql)) {
        return {false, "Failed to delete department: " + db.getLastError()};
    }

    return {true, "Department deleted successfully."};
}

nlohmann::json EmployeeService::getReportStats(const std::map<std::string, std::string>& filters) {
    auto employees = getAllEmployees(filters);

    int total = static_cast<int>(employees.size());
    int active = 0, inactive = 0, onLeave = 0;
    double totalSalary = 0.0;
    double maxSalary = 0.0;
    double minSalary = total > 0 ? 1e12 : 0.0;
    std::map<std::string, int> deptCounts;
    std::map<std::string, int> statusCounts;

    for (const auto& emp : employees) {
        if (emp.getStatus() == "Active") active++;
        else if (emp.getStatus() == "Inactive") inactive++;
        else if (emp.getStatus() == "On Leave") onLeave++;

        deptCounts[emp.getDepartment()]++;
        statusCounts[emp.getStatus()]++;

        double sal = emp.getSalary();
        totalSalary += sal;
        if (sal > maxSalary) maxSalary = sal;
        if (sal < minSalary) minSalary = sal;
    }

    if (total == 0) minSalary = 0.0;
    double avgSalary = total > 0 ? (totalSalary / total) : 0.0;

    nlohmann::json byDept = nlohmann::json::object();
    for (const auto& pair : deptCounts) {
        byDept[pair.first] = pair.second;
    }

    nlohmann::json byStatus = nlohmann::json::object();
    for (const auto& pair : statusCounts) {
        byStatus[pair.first] = pair.second;
    }

    return {
        {"total_employees", total},
        {"active_employees", active},
        {"inactive_employees", inactive},
        {"on_leave_employees", onLeave},
        {"average_salary", avgSalary},
        {"max_salary", maxSalary},
        {"min_salary", minSalary},
        {"total_payroll", totalSalary},
        {"by_department", byDept},
        {"by_status", byStatus}
    };
}

std::string EmployeeService::exportEmployeesCsv(const std::map<std::string, std::string>& filters) {
    auto employees = getAllEmployees(filters);
    std::ostringstream csv;

    csv << "ID,Employee ID,Full Name,Age,Gender,Email,Phone,Address,Department,Designation,Salary,Joining Date,Status\n";

    for (const auto& emp : employees) {
        csv << emp.getId() << ","
            << "\"" << emp.getEmployeeId() << "\","
            << "\"" << emp.getName() << "\","
            << emp.getAge() << ","
            << "\"" << emp.getGender() << "\","
            << "\"" << emp.getEmail() << "\","
            << "\"" << emp.getPhone() << "\","
            << "\"" << emp.getAddress() << "\","
            << "\"" << emp.getDepartment() << "\","
            << "\"" << emp.getDesignation() << "\","
            << std::fixed << std::setprecision(2) << emp.getSalary() << ","
            << "\"" << emp.getJoiningDate() << "\","
            << "\"" << emp.getStatus() << "\"\n";
    }

    return csv.str();
}
