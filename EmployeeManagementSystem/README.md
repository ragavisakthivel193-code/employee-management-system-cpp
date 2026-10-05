# Employee Management System (EMS)

An enterprise-grade, high-performance, full-stack **Employee Management System** built with **C++ (C++17)**, **MySQL 8.0**, and a modern, responsive web frontend. The system demonstrates robust Object-Oriented Programming (OOP) paradigms, RESTful API architecture, multi-tiered security, role-based access control (RBAC), and interactive business analytics.

---

## Table of Contents
1. [Project Overview](#1-project-overview)
2. [Key Features](#2-key-features)
3. [Technology Stack](#3-technology-stack)
4. [System Architecture](#4-system-architecture)
5. [Folder Structure](#5-folder-structure)
6. [Database Schema](#6-database-schema)
7. [REST API Documentation](#7-rest-api-documentation)
8. [C++ Concepts Demonstrated](#8-c-concepts-demonstrated)
9. [OOP Principles & Design Patterns](#9-oop-principles--design-patterns)
10. [Installation & Prerequisites](#10-installation--prerequisites)
11. [MySQL Database Setup](#11-mysql-database-setup)
12. [Backend Compilation & Setup](#12-backend-compilation--setup)
13. [Frontend Setup](#13-frontend-setup)
14. [Environment Variables](#14-environment-variables)
15. [How to Run the Application](#15-how-to-run-the-application)
16. [Test Login Credentials](#16-test-login-credentials)
17. [UI Preview & Pages](#17-ui-preview--pages)
18. [Future Enhancements](#18-future-enhancements)

---

## 1. Project Overview

The **Employee Management System** is built to streamline human resource workflows, departmental headcount oversight, compensation auditing, and employee record governance. Unlike typical basic web applications built solely on scripting languages, this project features a high-throughput, memory-efficient native **C++ backend** communicating directly with **MySQL 8.0** through prepared statements, and providing JSON REST APIs consumed by a modern dashboard UI.

---

## 2. Key Features

- **Executive Analytics Dashboard:**
  - Real-time stat cards: Total Employees, Active Employees, Departments Count, and Average Organization Salary.
  - Interactive Chart.js visualizations:
    - *Department Headcounts* (Doughnut chart)
    - *Gender Distribution* (Pie chart)
    - *Salary Distribution by Department* (Bar chart)
    - *Monthly Hiring Trends* (Area line chart)
  - Recent hires feed with quick profile drawer.

- **Employee Lifecycle (CRUD):**
  - Searchable, sortable, responsive employee directory.
  - Granular details: Employee ID, Full Name, Age, Gender, Email, Phone, Residential Address, Department, Designation, Salary, Joining Date, and Employment Status (`Active`, `Inactive`, `On Leave`).
  - View Employee Profile modal.
  - Full edit flow with immutable employee ID validation.
  - Safe delete confirmation modal.

- **Dynamic Search & Multi-Filters:**
  - Real-time search with client-side debounce (300ms) across ID, Name, Email, Department, and Designation.
  - Composite multi-filter criteria: Department, Designation, Gender, Status, and Min/Max Salary ranges.
  - Dedicated `Clear Filters` button.

- **Department Governance & Deletion Safeguard:**
  - Department catalog with live assigned employee headcounts.
  - Add and update departments.
  - **Relational Safeguard:** Prevents deleting any department that currently has employees assigned to it, returning descriptive feedback.

- **Reports & Data Export:**
  - Filterable organizational summaries (salary boundaries, total payroll cost, headcount breakdowns).
  - One-click **Export Report (CSV)** streaming directory data directly to a downloadable file.

- **Role-Based Access Control (RBAC):**
  - **Admin:** Full read, write, update, and delete authority across employees and departments.
  - **HR User:** Read, add, and update authority; deletion is strictly restricted to prevent accidental data loss.
  - Salted SHA-256 password hashing.
  - Signed session tokens with expiration checking.

- **Form & Backend Dual Validation:**
  - Client-side validation for instant user feedback.
  - Server-side C++ class `validate()` method ensuring data integrity, valid email formats, phone length, age restrictions (18–75), positive salary bounds, and unique constraints.

- **User Feedback & States:**
  - Elegant toast notification system (`Success`, `Error`, `Warning`, `Info`).
  - Loading indicators and button spinners during API operations.

---

## 3. Technology Stack

### Backend:
- **Language:** C++17
- **Compiler:** Microsoft Visual C++ (MSVC x64) / Clang / GCC
- **Web Framework:** `cpp-httplib` (High-performance lightweight REST API server)
- **JSON Engine:** `nlohmann/json` (Modern JSON for C++)
- **Database Connector:** MySQL C Client Library (`libmysql.lib` / `libmysql.dll`)
- **Crypto & Networking:** Native Windows Sockets (`ws2_32.lib`), Windows Crypto (`crypt32.lib`), and standard self-contained SHA-256 implementation.

### Frontend:
- **Structure:** HTML5 Semantic Elements
- **Styling:** Vanilla CSS3 (Custom Design System, Glassmorphic Modals, Responsive Flexbox/Grid, Dark/Light accents)
- **Typography:** Google Fonts (*Plus Jakarta Sans*)
- **Logic:** Vanilla JavaScript (ES6+ Modules, Fetch API, Async/Await)
- **Data Visualization:** Chart.js 4.x

### Database:
- **RDBMS:** MySQL 8.0+
- **Engine:** InnoDB (ACID compliant, UTF8MB4 charset, foreign keys, indexes)

---

## 4. System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                 Modern Web Browser (Client)                 │
│   (HTML5 / CSS3 / ES6 / Chart.js / Responsive Drawer)      │
└──────────────────────────────┬──────────────────────────────┘
                               │ HTTP / REST API (JSON)
                               ▼
┌─────────────────────────────────────────────────────────────┐
│                 C++ REST API Server (Port 8088)             │
│                                                             │
│   ┌─────────────────────────────────────────────────────┐   │
│   │ HTTP Routing & CORS Handler (cpp-httplib)           │   │
│   └──────────────────────────┬──────────────────────────┘   │
│                              │                              │
│   ┌──────────────────────────▼──────────────────────────┐   │
│   │ Authentication & RBAC Layer (Auth.h / Auth.cpp)     │   │
│   │ (Salted SHA-256, Signed Tokens, Role Verification)  │   │
│   └──────────────────────────┬──────────────────────────┘   │
│                              │                              │
│   ┌──────────────────────────▼──────────────────────────┐   │
│   │ Business Logic Layer (EmployeeService.h / .cpp)     │   │
│   │ (Validation, Search, Filters, CSV Export, Stats)   │   │
│   └──────────────────────────┬──────────────────────────┘   │
│                              │                              │
│   ┌──────────────────────────▼──────────────────────────┐   │
│   │ Database Gateway Layer (Database.h / Database.cpp)  │   │
│   │ (Thread-Safe Mutex, SQL Escaping, Query Engine)     │   │
│   └──────────────────────────┬──────────────────────────┘   │
└──────────────────────────────┼──────────────────────────────┘
                               │ Native MySQL Socket Protocol
                               ▼
┌─────────────────────────────────────────────────────────────┐
│                 MySQL 8.0 Server Database                   │
│   Tables: employees | users | departments                   │
└─────────────────────────────────────────────────────────────┘
```

---

## 5. Folder Structure

```
EmployeeManagementSystem/
│
├── backend/
│   ├── include/
│   │   ├── Employee.h             # Employee model declaration with validation
│   │   ├── Database.h             # Singleton MySQL wrapper & query manager
│   │   ├── Auth.h                 # Authentication, hashing & token helpers
│   │   ├── EmployeeService.h      # Business logic & reporting service
│   │   ├── httplib.h              # C++ lightweight REST server framework
│   │   └── json.hpp               # nlohmann::json modern library
│   │
│   ├── src/
│   │   ├── main.cpp               # REST API routing, CORS, and static file mount
│   │   ├── Employee.cpp           # Employee model implementation & JSON mapping
│   │   ├── Database.cpp           # MySQL connection and query execution
│   │   ├── Auth.cpp               # SHA-256 password hashing & token validation
│   │   └── EmployeeService.cpp    # Full business logic & department safeguards
│   │
│   ├── config/
│   │   ├── .env                   # Local environment configuration
│   │   └── .env.example           # Configuration template
│   │
│   ├── CMakeLists.txt             # CMake cross-platform build definition
│   ├── build.bat                  # One-click MSVC x64 build script
│   ├── run.bat                    # One-click server launcher
│   └── test_suite.ps1             # 36-point automated integration test suite
│
├── frontend/
│   ├── index.html                 # Landing portal & automatic auth router
│   ├── login.html                 # Administrator & HR login page
│   ├── dashboard.html             # Executive statistics & 4 Chart.js charts
│   ├── employees.html             # Employee roster, live search, multi-filters
│   ├── add-employee.html          # Registration form with dynamic departments
│   ├── edit-employee.html         # Profile edit page with immutable ID
│   ├── departments.html           # Department CRUD & employee count tracker
│   ├── reports.html               # Headcount analytics & CSV export
│   ├── settings.html              # Profile overview & password change
│   │
│   ├── css/
│   │   └── style.css              # Custom Enterprise Design System
│   │
│   └── js/
│       ├── api.js                 # Central fetch wrapper & toast notifications
│       ├── auth.js                # Auth guard, user profile widget & logout
│       ├── dashboard.js           # Chart rendering & metrics fetcher
│       ├── employees.js           # Directory search, filter & modal handlers
│       ├── departments.js         # Department management controller
│       ├── reports.js             # Reports calculator & CSV trigger
│       └── settings.js            # Change password form controller
│
├── database/
│   └── schema.sql                 # MySQL schema, tables, indexes & sample data
│
├── .gitignore                     # Git ignore rules for binaries and secrets
└── README.md                      # Comprehensive documentation
```

---

## 6. Database Schema

The database `employee_management` contains 3 primary relational tables:

```sql
-- 1. Departments Table
CREATE TABLE departments (
    id INT AUTO_INCREMENT PRIMARY KEY,
    department_name VARCHAR(50) NOT NULL UNIQUE,
    description TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 2. Users Table (Authentication & RBAC)
CREATE TABLE users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    email VARCHAR(100) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    role VARCHAR(20) NOT NULL DEFAULT 'HR',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_user_username (username),
    INDEX idx_user_email (email)
);

-- 3. Employees Table
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
);
```

---

## 7. REST API Documentation

All API responses follow a uniform JSON schema:

**Success Response:**
```json
{
  "success": true,
  "message": "Operation completed successfully",
  "data": {}
}
```

**Error Response:**
```json
{
  "success": false,
  "message": "Descriptive error message"
}
```

### Endpoints:

| Category | Method | Endpoint | Auth Required | Description |
|---|---|---|---|---|
| **Auth** | `POST` | `/api/auth/login` | No | Authenticate user; returns token and role |
| **Auth** | `POST` | `/api/auth/logout` | No | Clear active session |
| **Auth** | `GET` | `/api/auth/me` | Bearer | Get current user's profile |
| **Auth** | `POST` | `/api/auth/change-password` | Bearer | Change user password |
| **Dashboard**| `GET` | `/api/dashboard/stats` | No | Get aggregated metrics, charts & recent hires |
| **Employees**| `GET` | `/api/employees` | No | List employees with optional query filters |
| **Employees**| `GET` | `/api/employees/search?query=...` | No | Live search across ID, Name, Email, Dept, Role |
| **Employees**| `GET` | `/api/employees/{id}` | No | Get single employee details |
| **Employees**| `POST`| `/api/employees` | Admin / HR | Create new employee (validates unique ID/email) |
| **Employees**| `PUT` | `/api/employees/{id}` | Admin / HR | Update employee details (immutable employee ID) |
| **Employees**| `DELETE`| `/api/employees/{id}`| Admin Only | Permanently delete employee record |
| **Departments**| `GET`| `/api/departments` | No | List departments with employee count |
| **Departments**| `POST`| `/api/departments` | Admin | Create department |
| **Departments**| `PUT`| `/api/departments/{id}` | Admin | Update department details |
| **Departments**| `DELETE`| `/api/departments/{id}` | Admin | Delete department (guarded against active employees) |
| **Reports** | `GET` | `/api/reports/stats` | No | Filtered headcount and salary metrics |
| **Reports** | `GET` | `/api/reports/export` | No | Download CSV report |

---

## 8. C++ Concepts Demonstrated

- **Standard Template Library (STL):** Extensive use of `std::vector`, `std::map`, `std::string`, `std::optional`, `std::pair`, and `<algorithm>`.
- **Object Lifetime & Resource Management (RAII):** Memory and MySQL resources are freed automatically using destructor semantics and `std::lock_guard` for mutex synchronization.
- **Smart Data Transfer:** Pass-by-reference-to-const (`const std::string&`) avoiding unnecessary heap reallocations.
- **Type Safety & C++17 Features:** `std::optional`, structured bindings (`auto [success, message]`), `<filesystem>` for asset resolution.
- **Exception Handling:** Robust `try ... catch` blocks intercepting JSON parse exceptions, type conversion issues, and database errors.
- **Streams & Formatting:** `std::ostringstream` with `std::fixed` and `std::setprecision` for financial calculation accuracy.

---

## 9. OOP Principles & Design Patterns

1. **Encapsulation:** The `Employee` class hides private member variables (`employeeId`, `salary`, `email`, etc.) behind public accessor and mutator methods with constraint validation.
2. **Singleton Pattern:** The `Database` class is structured as a thread-safe singleton (`Database::getInstance()`), guaranteeing a single centralized point of connection to the MySQL server with thread-synchronized query locks.
3. **Service Layer Architecture:** Decoupling business logic (`EmployeeService`) from the web presentation/routing layer (`main.cpp`), making code reusable, modular, and maintainable.
4. **Data Transfer Object (DTO) & Serialization:** Clean mapping between C++ object instances and JSON structures via `toJson()` and `fromJson()`.

---

## 10. Installation & Prerequisites

- **Operating System:** Windows 10/11 (or Linux/macOS with standard GCC/Clang)
- **C++ Compiler:** Visual Studio 2022 Community (with Desktop C++ workload) or MinGW-w64 (GCC 9+)
- **Database:** MySQL 8.0 Server
- **Browser:** Any modern browser (Chrome, Edge, Firefox, Safari)

---

## 11. MySQL Database Setup

1. Open MySQL command line or MySQL Workbench:
```powershell
mysql -u root -p
```
2. Run the provided database initialization script:
```powershell
mysql -u root -p < database/schema.sql
```
*(On Windows PowerShell, use: `Get-Content database/schema.sql -Raw | mysql -u root -p`)*

This creates:
- The `employee_management` database.
- `departments`, `users`, and `employees` tables with indexes.
- Default Admin and HR user accounts.
- 12 realistic sample employee records.

---

## 12. Backend Compilation & Setup

### Option A: One-Click Build Script (Recommended)
Open a terminal in `backend/` and run:
```cmd
build.bat
```
This automatically initializes the Visual Studio 2022 x64 build environment, compiles all source files with `/std:c++17 /O2`, links MySQL client and Windows networking libraries, and copies `libmysql.dll`.

### Option B: Using CMake
```bash
cd backend
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

---

## 13. Frontend Setup

The frontend consists of pure, dependency-free HTML5, CSS3, and JavaScript:
- No `npm install` or node bundlers are required!
- The C++ backend automatically serves all frontend files directly from `http://localhost:8088`.
- You can also open `frontend/index.html` or `frontend/login.html` directly in your browser.

---

## 14. Environment Variables

Configure connection settings in `backend/config/.env`:

```env
# Database Configuration
DB_HOST=127.0.0.1
DB_PORT=3306
DB_NAME=employee_management
DB_USER=root
DB_PASSWORD=root

# REST API Server Port
PORT=8088
JWT_SECRET=ems_jwt_super_secure_key_2026_ragavi
```

*(A template is preserved in `backend/config/.env.example`)*

---

## 15. How to Run the Application

1. **Start the C++ Backend Server:**
```powershell
cd backend
.\run.bat
```
*Output:*
```
==========================================================
  EMPLOYEE MANAGEMENT SYSTEM - C++ REST API SERVER
==========================================================
[DB] Connecting to MySQL at 127.0.0.1:3306 | Database: employee_management | User: root
[DB] Connected to MySQL successfully!
[STATIC] Serving frontend assets from: ...\EmployeeManagementSystem\frontend
[SERVER] Starting REST API Server at http://localhost:8088
[INFO] Ready to accept frontend requests.
==========================================================
```

2. **Open in Browser:**
Navigate to:
```
http://localhost:8088
```
or open `http://localhost:8088/login.html` directly!

---

## 16. Test Login Credentials

For quick and effortless testing, the login screen includes **one-click demo credential fill buttons**:

| Role | Username | Password | Permissions |
|---|---|---|---|
| **Admin** | `admin` | `Admin@123` | Full access (CRUD employees, manage departments, system settings) |
| **HR User** | `hr_priya` | `Hr@123` | Operational access (View, add, and edit employees; deletion protected) |

---

## 17. UI Preview & Pages

- **`login.html`:** Clean enterprise authentication with show/hide password toggle, input validation, and demo credential quick-fill.
- **`dashboard.html`:** Metric stat cards, 4 interactive Chart.js graphs, and recent onboardings table.
- **`employees.html`:** Complete employee directory with debounced search, composite dropdown filters, and action buttons.
- **`add-employee.html`:** Full registration form with real-time field validation and dynamic department options.
- **`edit-employee.html`:** Form for updating details with an immutable employee ID badge.
- **`departments.html`:** Department oversight displaying headcounts and deletion prevention safeguards.
- **`reports.html`:** Audit reports with salary boundaries, total payroll burden, and instant CSV export.
- **`settings.html`:** User profile inspection, salted SHA-256 change password flow, and system diagnostics.

---

## 18. Future Enhancements

1. **Document & Resume Attachments:** Support uploading PDF resumes or profile avatars stored on disk or S3.
2. **Attendance & Leave Management:** Clock-in/clock-out tracking and leave request approval workflows.
3. **Audit Trail Logging:** Maintain a history of who created, edited, or deleted records with exact timestamps.
4. **Email Notifications:** Automated welcome emails to new employees and password reset tokens via SMTP.
5. **Pagination:** Offset/limit pagination support for enterprise catalogs containing tens of thousands of records.

---

*Designed and Developed for Portfolio & Academic Demonstration.*
