#pragma once

#include <string>
#include "json.hpp"

class Employee {
private:
    int id;
    std::string employeeId;
    std::string name;
    int age;
    std::string gender;
    std::string email;
    std::string phone;
    std::string address;
    std::string department;
    std::string designation;
    double salary;
    std::string joiningDate;
    std::string status;
    std::string createdAt;
    std::string updatedAt;

public:
    // Constructors
    Employee();
    Employee(int id,
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
             const std::string& createdAt = "",
             const std::string& updatedAt = "");

    // Getters
    int getId() const;
    std::string getEmployeeId() const;
    std::string getName() const;
    int getAge() const;
    std::string getGender() const;
    std::string getEmail() const;
    std::string getPhone() const;
    std::string getAddress() const;
    std::string getDepartment() const;
    std::string getDesignation() const;
    double getSalary() const;
    std::string getJoiningDate() const;
    std::string getStatus() const;
    std::string getCreatedAt() const;
    std::string getUpdatedAt() const;

    // Setters
    void setId(int id);
    void setEmployeeId(const std::string& employeeId);
    void setName(const std::string& name);
    void setAge(int age);
    void setGender(const std::string& gender);
    void setEmail(const std::string& email);
    void setPhone(const std::string& phone);
    void setAddress(const std::string& address);
    void setDepartment(const std::string& department);
    void setDesignation(const std::string& designation);
    void setSalary(double salary);
    void setJoiningDate(const std::string& joiningDate);
    void setStatus(const std::string& status);
    void setCreatedAt(const std::string& createdAt);
    void setUpdatedAt(const std::string& updatedAt);

    // Business Logic & Validation
    bool validate(std::string& outErrorMessage) const;

    // Serialization
    nlohmann::json toJson() const;
    static Employee fromJson(const nlohmann::json& j);
};
