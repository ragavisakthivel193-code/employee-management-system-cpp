$baseUrl = "http://localhost:8088"
$passed = 0
$failed = 0

function Assert-Test($name, $condition, $msg = "") {
    if ($condition) {
        Write-Host "  [PASS] $name" -ForegroundColor Green
        $global:passed++
    } else {
        Write-Host "  [FAIL] $name - $msg" -ForegroundColor Red
        $global:failed++
    }
}

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host "  RUNNING FULL INTEGRATION TEST SUITE FOR EMS (C++ API)" -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan

# 1. Dashboard Stats
try {
    $stats = Invoke-RestMethod "$baseUrl/api/dashboard/stats" -Method Get
    Assert-Test "GET /api/dashboard/stats returns success" ($stats.success -eq $true)
    Assert-Test "Total employees > 0" ($stats.data.total_employees -ge 12)
    Assert-Test "Active employees present" ($stats.data.active_employees -gt 0)
    Assert-Test "Departments count matches" ($stats.data.total_departments -ge 6)
    Assert-Test "Average salary computed" ($stats.data.average_salary -gt 0)
} catch {
    Assert-Test "GET /api/dashboard/stats" $false $_.Exception.Message
}

# 2. Authentication - Admin Login
$adminToken = ""
try {
    $body = @{ username = "admin"; password = "Admin@123" } | ConvertTo-Json
    $res = Invoke-RestMethod "$baseUrl/api/auth/login" -Method Post -Body $body -ContentType "application/json"
    Assert-Test "Admin Login Success" ($res.success -eq $true)
    Assert-Test "Admin Role returned" ($res.data.user.role -eq "Admin")
    $adminToken = $res.data.token
} catch {
    Assert-Test "Admin Login" $false $_.Exception.Message
}

# 3. Authentication - HR Login
$hrToken = ""
try {
    $body = @{ username = "hr_priya"; password = "Hr@123" } | ConvertTo-Json
    $res = Invoke-RestMethod "$baseUrl/api/auth/login" -Method Post -Body $body -ContentType "application/json"
    Assert-Test "HR Login Success" ($res.success -eq $true)
    Assert-Test "HR Role returned" ($res.data.user.role -eq "HR")
    $hrToken = $res.data.token
} catch {
    Assert-Test "HR Login" $false $_.Exception.Message
}

# 4. Authentication - Invalid Password Rejection
try {
    $body = @{ username = "admin"; password = "WrongPassword999" } | ConvertTo-Json
    $res = Invoke-RestMethod "$baseUrl/api/auth/login" -Method Post -Body $body -ContentType "application/json"
    Assert-Test "Invalid password rejected" $false "Should have failed"
} catch {
    Assert-Test "Invalid password rejected" $true
}

# 5. Auth Me Verification
try {
    $res = Invoke-RestMethod "$baseUrl/api/auth/me" -Method Get -Headers @{ "Authorization" = "Bearer $adminToken" }
    Assert-Test "GET /api/auth/me with Bearer token" ($res.data.username -eq "admin")
} catch {
    Assert-Test "GET /api/auth/me" $false $_.Exception.Message
}

# 6. Unauthenticated Request Rejection
try {
    $body = @{ employee_id = "TEST"; name = "Test" } | ConvertTo-Json
    $res = Invoke-RestMethod "$baseUrl/api/employees" -Method Post -Body $body -ContentType "application/json"
    Assert-Test "Unauthenticated POST /api/employees rejected" $false "Should fail with 401"
} catch {
    Assert-Test "Unauthenticated POST /api/employees rejected" $true
}

# 7. Add Employee (Valid)
$newEmpId = "EMP" + (Get-Random -Minimum 1000 -Maximum 9999)
$createdId = 0
try {
    $empData = @{
        employee_id = $newEmpId
        name = "Kavitha Sundaram"
        age = 29
        gender = "Female"
        email = "kavitha.$newEmpId@company.com"
        phone = "9876543300"
        address = "14 Green Park, Chennai"
        department = "Engineering"
        designation = "Lead Cloud Specialist"
        salary = 85000
        joining_date = "2024-01-10"
        status = "Active"
    } | ConvertTo-Json

    $res = Invoke-RestMethod "$baseUrl/api/employees" -Method Post -Body $empData -Headers @{ "Authorization" = "Bearer $adminToken" } -ContentType "application/json"
    Assert-Test "POST /api/employees created employee" ($res.success -eq $true)

    # Fetch created employee
    $searchRes = Invoke-RestMethod "$baseUrl/api/employees/search?query=$newEmpId" -Method Get
    Assert-Test "Search finds created employee" ($searchRes.data.Count -ge 1)
    $createdId = $searchRes.data[0].id
} catch {
    Assert-Test "POST /api/employees" $false $_.Exception.Message
}

# 8. Duplicate Employee ID Validation
try {
    $dupData = @{
        employee_id = $newEmpId
        name = "Duplicate Person"
        age = 30
        gender = "Male"
        email = "diff.$newEmpId@company.com"
        phone = "9876543301"
        address = "Test address"
        department = "IT"
        designation = "Engineer"
        salary = 50000
        joining_date = "2024-01-10"
        status = "Active"
    } | ConvertTo-Json

    $res = Invoke-RestMethod "$baseUrl/api/employees" -Method Post -Body $dupData -Headers @{ "Authorization" = "Bearer $adminToken" } -ContentType "application/json"
    Assert-Test "Duplicate Employee ID rejected" $false "Should reject duplicate"
} catch {
    Assert-Test "Duplicate Employee ID rejected" $true
}

# 9. Invalid Input Validation (Negative Salary, Underage)
try {
    $invalidData = @{
        employee_id = "INV001"
        name = "Young Boy"
        age = 15 # Under 18
        gender = "Male"
        email = "young@company.com"
        phone = "9876543302"
        department = "IT"
        designation = "Intern"
        salary = -5000 # Negative
        joining_date = "2024-01-10"
        status = "Active"
    } | ConvertTo-Json

    $res = Invoke-RestMethod "$baseUrl/api/employees" -Method Post -Body $invalidData -Headers @{ "Authorization" = "Bearer $adminToken" } -ContentType "application/json"
    Assert-Test "Underage & Negative Salary rejected" $false "Should fail validation"
} catch {
    Assert-Test "Underage & Negative Salary rejected" $true
}

# 10. Update Employee (PUT /api/employees/{id})
try {
    $updateData = @{
        name = "Kavitha Sundaram Raman"
        age = 30
        gender = "Female"
        email = "kavitha.updated.$newEmpId@company.com"
        phone = "9876543300"
        address = "14 Green Park, Chennai (Updated)"
        department = "Engineering"
        designation = "Principal Cloud Specialist"
        salary = 95000
        joining_date = "2024-01-10"
        status = "Active"
    } | ConvertTo-Json

    $res = Invoke-RestMethod "$baseUrl/api/employees/$createdId" -Method Put -Body $updateData -Headers @{ "Authorization" = "Bearer $adminToken" } -ContentType "application/json"
    Assert-Test "PUT /api/employees/{id} update success" ($res.success -eq $true)

    # Verify updated values
    $getRes = Invoke-RestMethod "$baseUrl/api/employees/$createdId" -Method Get
    Assert-Test "Updated designation verified" ($getRes.data.designation -eq "Principal Cloud Specialist")
    Assert-Test "Updated salary verified" ($getRes.data.salary -eq 95000)
} catch {
    Assert-Test "PUT /api/employees/{id}" $false $_.Exception.Message
}

# 11. Role-Based Access Control: HR Deletion Rejection
try {
    $res = Invoke-RestMethod "$baseUrl/api/employees/$createdId" -Method Delete -Headers @{ "Authorization" = "Bearer $hrToken" }
    Assert-Test "HR Deletion Forbidden (403)" $false "HR should not be allowed to delete"
} catch {
    Assert-Test "HR Deletion Forbidden (403)" $true
}

# 12. Admin Deletion Success
try {
    $res = Invoke-RestMethod "$baseUrl/api/employees/$createdId" -Method Delete -Headers @{ "Authorization" = "Bearer $adminToken" }
    Assert-Test "Admin Delete Success" ($res.success -eq $true)

    # Verify deleted
    try {
        Invoke-RestMethod "$baseUrl/api/employees/$createdId" -Method Get
        Assert-Test "Deleted employee not found (404)" $false "Should be deleted"
    } catch {
        Assert-Test "Deleted employee not found (404)" $true
    }
} catch {
    Assert-Test "Admin Delete Employee" $false $_.Exception.Message
}

# 13. Combined Filtering (Department + Gender + Status)
try {
    $res = Invoke-RestMethod "$baseUrl/api/employees?department=IT&gender=Male&status=Active" -Method Get
    Assert-Test "Combined Filtering returns matching records" ($res.data.Count -gt 0)
    $allMatch = $true
    foreach ($emp in $res.data) {
        if ($emp.department -ne "IT" -or $emp.gender -ne "Male" -or $emp.status -ne "Active") {
            $allMatch = $false
        }
    }
    Assert-Test "All filtered records match criteria" $allMatch
} catch {
    Assert-Test "Combined Filtering" $false $_.Exception.Message
}

# 14. Department Deletion Guard (Cannot delete assigned department)
try {
    # IT department has employees
    $depts = Invoke-RestMethod "$baseUrl/api/departments" -Method Get
    $itDept = $depts.data | Where-Object { $_.department_name -eq "IT" }
    
    $res = Invoke-RestMethod "$baseUrl/api/departments/$($itDept.id)" -Method Delete -Headers @{ "Authorization" = "Bearer $adminToken" }
    Assert-Test "Delete assigned department rejected" $false "Should protect assigned department"
} catch {
    Assert-Test "Delete assigned department rejected" $true
}

# 15. Reports & CSV Export
try {
    $rep = Invoke-RestMethod "$baseUrl/api/reports/stats" -Method Get
    Assert-Test "Reports stats generated" ($rep.data.total_employees -gt 0)

    $csv = Invoke-RestMethod "$baseUrl/api/reports/export" -Method Get
    Assert-Test "CSV export contains headers" ($csv.Contains("Employee ID,Full Name,Age,Gender"))
} catch {
    Assert-Test "Reports and CSV Export" $false $_.Exception.Message
}

# 16. Static Frontend Files Served
try {
    $pages = @("index.html", "login.html", "dashboard.html", "employees.html", "add-employee.html", "edit-employee.html", "departments.html", "reports.html", "settings.html")
    foreach ($p in $pages) {
        $html = Invoke-WebRequest "$baseUrl/$p" -UseBasicParsing
        Assert-Test "Static page served: $p" ($html.StatusCode -eq 200)
    }
} catch {
    Assert-Test "Static Pages" $false $_.Exception.Message
}

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host "TEST RESULTS: Passed: $passed, Failed: $failed" -ForegroundColor $(if ($failed -eq 0) { "Green" } else { "Red" })
Write-Host "==========================================================" -ForegroundColor Cyan
