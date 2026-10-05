#include "Employee.h"
#include <regex>
#include <algorithm>

// Default constructor
Employee::Employee()
    : id(0), age(0), salary(0.0), status("Active") {}

// Parameterized constructor
Employee::Employee(int id,
                   const std::string& employeeId,
                   const std::string& name,
                   int age,
                   const std::string& gender,
                   const std::string& email,
                   const std::string& phone,
                   const std::string& address,
                   const std::string& department,
                   const std::string& designation,
                   double salary,
                   const std::string& joiningDate,
                   const std::string& status,
                   const std::string& createdAt,
                   const std::string& updatedAt)
    : id(id),
      employeeId(employeeId),
      name(name),
      age(age),
      gender(gender),
      email(email),
      phone(phone),
      address(address),
      department(department),
      designation(designation),
      salary(salary),
      joiningDate(joiningDate),
      status(status),
      createdAt(createdAt),
      updatedAt(updatedAt) {}

// Getters
int Employee::getId() const { return id; }
std::string Employee::getEmployeeId() const { return employeeId; }
std::string Employee::getName() const { return name; }
int Employee::getAge() const { return age; }
std::string Employee::getGender() const { return gender; }
std::string Employee::getEmail() const { return email; }
std::string Employee::getPhone() const { return phone; }
std::string Employee::getAddress() const { return address; }
std::string Employee::getDepartment() const { return department; }
std::string Employee::getDesignation() const { return designation; }
double Employee::getSalary() const { return salary; }
std::string Employee::getJoiningDate() const { return joiningDate; }
std::string Employee::getStatus() const { return status; }
std::string Employee::getCreatedAt() const { return createdAt; }
std::string Employee::getUpdatedAt() const { return updatedAt; }

// Setters
void Employee::setId(int id) { this->id = id; }
void Employee::setEmployeeId(const std::string& employeeId) { this->employeeId = employeeId; }
void Employee::setName(const std::string& name) { this->name = name; }
void Employee::setAge(int age) { this->age = age; }
void Employee::setGender(const std::string& gender) { this->gender = gender; }
void Employee::setEmail(const std::string& email) { this->email = email; }
void Employee::setPhone(const std::string& phone) { this->phone = phone; }
void Employee::setAddress(const std::string& address) { this->address = address; }
void Employee::setDepartment(const std::string& department) { this->department = department; }
void Employee::setDesignation(const std::string& designation) { this->designation = designation; }
void Employee::setSalary(double salary) { this->salary = salary; }
void Employee::setJoiningDate(const std::string& joiningDate) { this->joiningDate = joiningDate; }
void Employee::setStatus(const std::string& status) { this->status = status; }
void Employee::setCreatedAt(const std::string& createdAt) { this->createdAt = createdAt; }
void Employee::setUpdatedAt(const std::string& updatedAt) { this->updatedAt = updatedAt; }

// Validation
bool Employee::validate(std::string& outErrorMessage) const {
    if (employeeId.empty()) {
        outErrorMessage = "Employee ID is required.";
        return false;
    }

    if (name.empty() || name.length() < 2) {
        outErrorMessage = "Full Name is required and must be at least 2 characters long.";
        return false;
    }

    // Name should only contain letters, spaces, hyphens, and dots
    const std::regex nameRegex("^[A-Za-z\\s\\-\\.]+$");
    if (!std::regex_match(name, nameRegex)) {
        outErrorMessage = "Full Name contains invalid characters. Only letters, spaces, hyphens, and dots are allowed.";
        return false;
    }

    if (age < 18 || age > 75) {
        outErrorMessage = "Age must be between 18 and 75.";
        return false;
    }

    if (gender != "Male" && gender != "Female" && gender != "Other") {
        outErrorMessage = "Gender must be 'Male', 'Female', or 'Other'.";
        return false;
    }

    if (email.empty()) {
        outErrorMessage = "Email address is required.";
        return false;
    }

    // Standard email validation regex
    const std::regex emailRegex("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
    if (!std::regex_match(email, emailRegex)) {
        outErrorMessage = "Invalid email format.";
        return false;
    }

    if (phone.empty()) {
        outErrorMessage = "Phone number is required.";
        return false;
    }

    // Phone: only digits, plus, hyphens, brackets, spaces; at least 7 digits
    int digitCount = 0;
    for (char c : phone) {
        if (std::isdigit(static_cast<unsigned char>(c))) digitCount++;
        else if (c != '+' && c != '-' && c != ' ' && c != '(' && c != ')') {
            outErrorMessage = "Phone number contains invalid characters.";
            return false;
        }
    }
    if (digitCount < 7 || digitCount > 15) {
        outErrorMessage = "Phone number must have between 7 and 15 digits.";
        return false;
    }

    if (department.empty()) {
        outErrorMessage = "Department is required.";
        return false;
    }

    if (designation.empty()) {
        outErrorMessage = "Designation is required.";
        return false;
    }

    if (salary <= 0) {
        outErrorMessage = "Salary must be a positive number greater than 0.";
        return false;
    }

    if (joiningDate.empty()) {
        outErrorMessage = "Joining date is required.";
        return false;
    }

    if (status != "Active" && status != "Inactive" && status != "On Leave") {
        outErrorMessage = "Status must be 'Active', 'Inactive', or 'On Leave'.";
        return false;
    }

    return true;
}

// Convert to JSON
nlohmann::json Employee::toJson() const {
    nlohmann::json j;
    j["id"] = id;
    j["employee_id"] = employeeId;
    j["name"] = name;
    j["age"] = age;
    j["gender"] = gender;
    j["email"] = email;
    j["phone"] = phone;
    j["address"] = address;
    j["department"] = department;
    j["designation"] = designation;
    j["salary"] = salary;
    j["joining_date"] = joiningDate;
    j["status"] = status;
    j["created_at"] = createdAt;
    j["updated_at"] = updatedAt;
    return j;
}

// Construct from JSON
Employee Employee::fromJson(const nlohmann::json& j) {
    Employee emp;
    if (j.contains("id") && !j["id"].is_null()) {
        emp.setId(j["id"].is_string() ? std::stoi(j["id"].get<std::string>()) : j["id"].get<int>());
    }
    if (j.contains("employee_id") && !j["employee_id"].is_null()) {
        emp.setEmployeeId(j["employee_id"].get<std::string>());
    }
    if (j.contains("name") && !j["name"].is_null()) {
        emp.setName(j["name"].get<std::string>());
    }
    if (j.contains("age") && !j["age"].is_null()) {
        emp.setAge(j["age"].is_string() ? std::stoi(j["age"].get<std::string>()) : j["age"].get<int>());
    }
    if (j.contains("gender") && !j["gender"].is_null()) {
        emp.setGender(j["gender"].get<std::string>());
    }
    if (j.contains("email") && !j["email"].is_null()) {
        emp.setEmail(j["email"].get<std::string>());
    }
    if (j.contains("phone") && !j["phone"].is_null()) {
        emp.setPhone(j["phone"].get<std::string>());
    }
    if (j.contains("address") && !j["address"].is_null()) {
        emp.setAddress(j["address"].get<std::string>());
    }
    if (j.contains("department") && !j["department"].is_null()) {
        emp.setDepartment(j["department"].get<std::string>());
    }
    if (j.contains("designation") && !j["designation"].is_null()) {
        emp.setDesignation(j["designation"].get<std::string>());
    }
    if (j.contains("salary") && !j["salary"].is_null()) {
        emp.setSalary(j["salary"].is_string() ? std::stod(j["salary"].get<std::string>()) : j["salary"].get<double>());
    }
    if (j.contains("joining_date") && !j["joining_date"].is_null()) {
        emp.setJoiningDate(j["joining_date"].get<std::string>());
    }
    if (j.contains("status") && !j["status"].is_null()) {
        emp.setStatus(j["status"].get<std::string>());
    }
    if (j.contains("created_at") && !j["created_at"].is_null()) {
        emp.setCreatedAt(j["created_at"].get<std::string>());
    }
    if (j.contains("updated_at") && !j["updated_at"].is_null()) {
        emp.setUpdatedAt(j["updated_at"].get<std::string>());
    }
    return emp;
}
