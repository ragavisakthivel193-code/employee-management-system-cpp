-- ==========================================================
-- Employee Management System - Database Schema
-- Compatible with MySQL 8.0+ / MariaDB
-- ==========================================================

CREATE DATABASE IF NOT EXISTS employee_management
    CHARACTER SET utf8mb4
    COLLATE utf8mb4_unicode_ci;

USE employee_management;

-- ----------------------------------------------------------
-- 1. Departments Table
-- ----------------------------------------------------------
DROP TABLE IF EXISTS employees;
DROP TABLE IF EXISTS users;
DROP TABLE IF EXISTS departments;

CREATE TABLE departments (
    id INT AUTO_INCREMENT PRIMARY KEY,
    department_name VARCHAR(50) NOT NULL UNIQUE,
    description TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ----------------------------------------------------------
-- 2. Users Table (Authentication & RBAC)
-- ----------------------------------------------------------
CREATE TABLE users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    email VARCHAR(100) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    role VARCHAR(20) NOT NULL DEFAULT 'HR',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_user_username (username),
    INDEX idx_user_email (email)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ----------------------------------------------------------
-- 3. Employees Table
-- ----------------------------------------------------------
CREATE TABLE employees (
    id INT AUTO_INCREMENT PRIMARY KEY,
    employee_id VARCHAR(50) NOT NULL UNIQUE,
    name VARCHAR(100) NOT NULL,
    age INT NOT NULL,
    gender VARCHAR(20) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    phone VARCHAR(30) NOT NULL,
    address TEXT,
    department VARCHAR(50) NOT NULL,
    designation VARCHAR(100) NOT NULL,
    salary DECIMAL(12, 2) NOT NULL,
    joining_date DATE NOT NULL,
    status ENUM('Active', 'Inactive', 'On Leave') NOT NULL DEFAULT 'Active',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_employee_id (employee_id),
    INDEX idx_emp_email (email),
    INDEX idx_emp_department (department),
    INDEX idx_emp_status (status),
    INDEX idx_emp_name (name)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ----------------------------------------------------------
-- Sample Data Insertion
-- ----------------------------------------------------------

-- Departments
INSERT INTO departments (department_name, description) VALUES
('IT', 'Information Technology, Infrastructure & Software Architecture'),
('Engineering', 'Product Engineering, Cloud Platform & Systems Development'),
('HR', 'Human Resources, Talent Acquisition & People Operations'),
('Finance', 'Accounting, Payroll, Budgeting & Financial Auditing'),
('Marketing', 'Digital Marketing, Brand Management & Growth Strategy'),
('Operations', 'Logistics, Facility Management & Business Workflows');

-- Default Accounts:
-- Admin: username 'admin', password 'Admin@123', role 'Admin'
-- HR:    username 'hr_priya', password 'Hr@123', role 'HR'
-- Hashes generated using SHA-256 with salt 'ems_salt_2026_'
INSERT INTO users (username, email, password_hash, role) VALUES
('admin', 'admin@company.com', '0f6ee78da8f0731a1d71b128042dcb97865d76aac77d7902ae6cdeb2ee85901f', 'Admin'),
('hr_priya', 'priya.hr@company.com', '33e9ad50185e36be3c61d1b5edba13a40297d962b8c71ada9659efcca0666d64', 'HR');

-- 12 Realistic Sample Employees
INSERT INTO employees (employee_id, name, age, gender, email, phone, address, department, designation, salary, joining_date, status) VALUES
('EMP101', 'Arun Kumar', 28, 'Male', 'arun.kumar@company.com', '9876543210', '42 Tech Park Avenue, Bangalore', 'IT', 'Senior Full Stack Engineer', 75000.00, '2023-01-15', 'Active'),
('EMP102', 'Priya Sharma', 32, 'Female', 'priya.sharma@company.com', '9876543211', '15 Green Valley, Pune', 'HR', 'HR Manager', 68000.00, '2022-03-10', 'Active'),
('EMP103', 'Rajesh Patel', 35, 'Male', 'rajesh.patel@company.com', '9876543212', '78 Sunset Boulevard, Mumbai', 'Finance', 'Financial Controller', 82000.00, '2021-08-20', 'Active'),
('EMP104', 'Ananya Roy', 26, 'Female', 'ananya.roy@company.com', '9876543213', '104 Lakeview Enclave, Kolkata', 'Marketing', 'Digital Marketing Lead', 58000.00, '2023-06-01', 'Active'),
('EMP105', 'Vikram Singh', 40, 'Male', 'vikram.singh@company.com', '9876543214', '22 Heritage Enclave, Delhi', 'Operations', 'Operations Director', 95000.00, '2020-04-12', 'Active'),
('EMP106', 'Sneha Reddy', 29, 'Female', 'sneha.reddy@company.com', '9876543215', '89 Cyber Hills, Hyderabad', 'Engineering', 'Cloud Architect', 90000.00, '2022-11-05', 'Active'),
('EMP107', 'Karthik Raman', 25, 'Male', 'karthik.raman@company.com', '9876543216', '12 Temple Street, Chennai', 'IT', 'DevOps Specialist', 62000.00, '2023-09-18', 'Active'),
('EMP108', 'Meera Nair', 31, 'Female', 'meera.nair@company.com', '9876543217', '55 Coastal Road, Kochi', 'HR', 'Talent Acquisition Lead', 54000.00, '2022-07-25', 'On Leave'),
('EMP109', 'Aditya Joshi', 38, 'Male', 'aditya.joshi@company.com', '9876543218', '63 Shivaji Nagar, Pune', 'Finance', 'Senior Accountant', 60000.00, '2021-12-01', 'Active'),
('EMP110', 'Kavita Verma', 27, 'Female', 'kavita.verma@company.com', '9876543219', '34 Civil Lines, Jaipur', 'Marketing', 'Content Strategist', 48000.00, '2024-02-14', 'Inactive'),
('EMP111', 'Rohan Deshmukh', 30, 'Male', 'rohan.deshmukh@company.com', '9876543220', '91 MG Road, Bangalore', 'Engineering', 'Backend C++ Developer', 72000.00, '2023-04-10', 'Active'),
('EMP112', 'Divya Menon', 34, 'Female', 'divya.menon@company.com', '9876543221', '17 Palm Meadows, Coimbatore', 'Operations', 'Supply Chain Analyst', 52000.00, '2022-09-15', 'Active');
