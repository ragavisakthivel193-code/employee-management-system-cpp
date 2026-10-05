/* ==========================================================
   EMPLOYEES MANAGEMENT CONTROLLER (CRUD, SEARCH, FILTER)
   ========================================================== */

let debounceTimer = null;
let currentDeleteId = null;
let currentDeleteName = null;

// Initialize Employees List View
async function initEmployeesList() {
    loadDepartmentDropdown();
    loadEmployees();

    // Setup dynamic search with 300ms debounce
    const searchInput = document.getElementById("search-input");
    if (searchInput) {
        searchInput.addEventListener("input", (e) => {
            clearTimeout(debounceTimer);
            debounceTimer = setTimeout(() => {
                loadEmployees();
            }, 300);
        });
    }

    // Setup filter listeners
    const filterDept = document.getElementById("filter-department");
    const filterGender = document.getElementById("filter-gender");
    const filterStatus = document.getElementById("filter-status");
    const filterMinSalary = document.getElementById("filter-min-salary");
    const filterMaxSalary = document.getElementById("filter-max-salary");

    [filterDept, filterGender, filterStatus, filterMinSalary, filterMaxSalary].forEach(el => {
        if (el) {
            el.addEventListener("change", () => loadEmployees());
        }
    });
}

// Fetch and populate Department dropdown options
async function loadDepartmentDropdown(selectId = "filter-department") {
    const select = document.getElementById(selectId);
    if (!select) return;

    try {
        const res = await api.get("/api/departments");
        if (res.success && res.data) {
            // Keep default option if present
            const firstOption = select.options[0];
            select.innerHTML = "";
            if (firstOption) select.appendChild(firstOption);

            res.data.forEach(dept => {
                const opt = document.createElement("option");
                opt.value = dept.department_name;
                opt.textContent = dept.department_name;
                select.appendChild(opt);
            });
        }
    } catch (err) {
        console.error("Failed to load departments:", err);
    }
}

// Load Employees with all active filters and search query
async function loadEmployees() {
    const tbody = document.getElementById("employees-table-body");
    const countBadge = document.getElementById("employee-count-badge");
    if (!tbody) return;

    tbody.innerHTML = `<tr><td colspan="9" class="table-loading-overlay"><div class="spinner spinner-dark"></div>Loading employees...</td></tr>`;

    const search = document.getElementById("search-input") ? document.getElementById("search-input").value.trim() : "";
    const department = document.getElementById("filter-department") ? document.getElementById("filter-department").value : "";
    const gender = document.getElementById("filter-gender") ? document.getElementById("filter-gender").value : "";
    const status = document.getElementById("filter-status") ? document.getElementById("filter-status").value : "";
    const minSalary = document.getElementById("filter-min-salary") ? document.getElementById("filter-min-salary").value.trim() : "";
    const maxSalary = document.getElementById("filter-max-salary") ? document.getElementById("filter-max-salary").value.trim() : "";

    const params = {};
    if (search) params.search = search;
    if (department && department !== "All") params.department = department;
    if (gender && gender !== "All") params.gender = gender;
    if (status && status !== "All") params.status = status;
    if (minSalary) params.min_salary = minSalary;
    if (maxSalary) params.max_salary = maxSalary;

    try {
        const res = await api.get("/api/employees", params);
        if (!res.success) {
            tbody.innerHTML = `<tr><td colspan="9" style="text-align:center; padding: 2rem; color: var(--danger);">${escapeHtml(res.message)}</td></tr>`;
            return;
        }

        const employees = res.data || [];
        if (countBadge) countBadge.textContent = `${employees.length} Employee${employees.length === 1 ? '' : 's'}`;

        if (employees.length === 0) {
            tbody.innerHTML = `
                <tr>
                    <td colspan="9" style="text-align:center; padding: 3rem; color: var(--text-muted);">
                        <svg style="width:48px; height:48px; margin-bottom:0.75rem; color:#cbd5e1;" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="1.5" d="M17 20h5v-2a3 3 0 00-5.356-1.857M17 20H7m10 0v-2c0-.656-.126-1.283-.356-1.857M7 20H2v-2a3 3 0 015.356-1.857M7 20v-2c0-.656.126-1.283.356-1.857m0 0a5.002 5.002 0 019.288 0M15 7a3 3 0 11-6 0 3 3 0 016 0zm6 3a2 2 0 11-4 0 2 2 0 014 0zM7 10a2 2 0 11-4 0 2 2 0 014 0z"></path></svg>
                        <p style="font-weight:600; color:var(--text-secondary);">No employees found</p>
                        <p style="font-size:0.85rem;">Try adjusting your search criteria or clearing active filters.</p>
                    </td>
                </tr>
            `;
            return;
        }

        tbody.innerHTML = employees.map(emp => {
            let statusBadge = "badge-active";
            if (emp.status === "Inactive") statusBadge = "badge-inactive";
            else if (emp.status === "On Leave") statusBadge = "badge-on-leave";

            return `
                <tr>
                    <td><strong>${escapeHtml(emp.employee_id)}</strong></td>
                    <td>
                        <div style="display:flex; align-items:center; gap: 0.65rem;">
                            <span class="avatar" style="width:32px; height:32px; font-size:0.8rem;">${getInitials(emp.name)}</span>
                            <div>
                                <div style="font-weight:600;">${escapeHtml(emp.name)}</div>
                                <div style="font-size:0.75rem; color:var(--text-muted);">${escapeHtml(emp.gender)}, ${emp.age} yrs</div>
                            </div>
                        </div>
                    </td>
                    <td><a href="mailto:${escapeHtml(emp.email)}" style="color:var(--text-secondary);">${escapeHtml(emp.email)}</a></td>
                    <td>${escapeHtml(emp.phone)}</td>
                    <td><span class="badge badge-dept">${escapeHtml(emp.department)}</span></td>
                    <td>${escapeHtml(emp.designation)}</td>
                    <td><strong>${formatCurrency(emp.salary)}</strong></td>
                    <td><span class="badge ${statusBadge}">${escapeHtml(emp.status)}</span></td>
                    <td>
                        <div class="action-buttons">
                            <button class="btn-icon view" title="View Profile" onclick="openViewModal(${emp.id})">
                                <svg width="15" height="15" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M15 12a3 3 0 11-6 0 3 3 0 016 0z"></path><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M2.458 12C3.732 7.943 7.523 5 12 5c4.478 0 8.268 2.943 9.542 7-1.274 4.057-5.064 7-9.542 7-4.477 0-8.268-2.943-9.542-7z"></path></svg>
                            </button>
                            <button class="btn-icon edit" title="Edit Employee" onclick="window.location.href='edit-employee.html?id=${emp.id}'">
                                <svg width="15" height="15" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M15.232 5.232l3.536 3.536m-2.036-5.036a2.5 2.5 0 113.536 3.536L6.5 21.036H3v-3.572L16.732 3.732z"></path></svg>
                            </button>
                            <button class="btn-icon delete" title="Delete Employee" onclick="openDeleteModal(${emp.id}, '${escapeHtml(emp.name)}')">
                                <svg width="15" height="15" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M19 7l-.867 12.142A2 2 0 0116.138 21H7.862a2 2 0 01-1.995-1.858L5 7m5 4v6m4-6v6m1-10V4a1 1 0 00-1-1h-4a1 1 0 00-1 1v3M4 7h16"></path></svg>
                            </button>
                        </div>
                    </td>
                </tr>
            `;
        }).join("");

    } catch (err) {
        tbody.innerHTML = `<tr><td colspan="9" style="text-align:center; padding: 2rem; color: var(--danger);">Failed to connect to backend server.</td></tr>`;
    }
}

// Clear all search and filter fields
function clearFilters() {
    const searchInput = document.getElementById("search-input");
    const filterDept = document.getElementById("filter-department");
    const filterGender = document.getElementById("filter-gender");
    const filterStatus = document.getElementById("filter-status");
    const filterMinSalary = document.getElementById("filter-min-salary");
    const filterMaxSalary = document.getElementById("filter-max-salary");

    if (searchInput) searchInput.value = "";
    if (filterDept) filterDept.value = "All";
    if (filterGender) filterGender.value = "All";
    if (filterStatus) filterStatus.value = "All";
    if (filterMinSalary) filterMinSalary.value = "";
    if (filterMaxSalary) filterMaxSalary.value = "";

    loadEmployees();
    showToast("Filters cleared.", "info");
}

// Export CSV handler
function exportCsv() {
    const search = document.getElementById("search-input") ? document.getElementById("search-input").value.trim() : "";
    const department = document.getElementById("filter-department") ? document.getElementById("filter-department").value : "";
    const status = document.getElementById("filter-status") ? document.getElementById("filter-status").value : "";

    let url = `${API_CONFIG.baseUrl}/api/reports/export?`;
    if (department && department !== "All") url += `department=${encodeURIComponent(department)}&`;
    if (status && status !== "All") url += `status=${encodeURIComponent(status)}&`;

    window.open(url, '_blank');
    showToast("Exporting employee report...", "info");
}

// Delete Confirmation Modal
function openDeleteModal(id, name) {
    currentDeleteId = id;
    currentDeleteName = name;

    const modal = document.getElementById("delete-confirm-modal");
    const targetNameEl = document.getElementById("delete-employee-target-name");
    if (targetNameEl) targetNameEl.textContent = name;
    if (modal) modal.classList.add("active");
}

function closeDeleteModal() {
    currentDeleteId = null;
    currentDeleteName = null;
    const modal = document.getElementById("delete-confirm-modal");
    if (modal) modal.classList.remove("active");
}

async function confirmDeleteEmployee() {
    if (!currentDeleteId) return;

    const deleteBtn = document.getElementById("btn-confirm-delete-action");
    if (deleteBtn) {
        deleteBtn.disabled = true;
        deleteBtn.innerHTML = `<span class="spinner"></span> Deleting...`;
    }

    try {
        const res = await api.delete(`/api/employees/${currentDeleteId}`);
        if (!res.success) {
            showToast(res.message || "Failed to delete employee", "error");
            return;
        }

        showToast("Employee deleted successfully.", "success");
        closeDeleteModal();
        loadEmployees();
    } catch (err) {
        showToast("Error processing deletion request", "error");
    } finally {
        if (deleteBtn) {
            deleteBtn.disabled = false;
            deleteBtn.innerHTML = "Delete";
        }
    }
}

/* ==========================================================
   ADD EMPLOYEE CONTROLLER
   ========================================================== */
async function initAddEmployee() {
    await loadDepartmentDropdown("add-emp-department");

    // Suggest an Employee ID based on timestamp
    const empIdInput = document.getElementById("add-emp-id");
    if (empIdInput && !empIdInput.value) {
        empIdInput.value = "EMP" + Math.floor(100 + Math.random() * 900);
    }

    // Set default joining date to today
    const dateInput = document.getElementById("add-emp-joining-date");
    if (dateInput && !dateInput.value) {
        dateInput.value = new Date().toISOString().split('T')[0];
    }

    const form = document.getElementById("add-employee-form");
    if (!form) return;

    form.addEventListener("submit", async (e) => {
        e.preventDefault();

        // Validation
        const employeeId = document.getElementById("add-emp-id").value.trim();
        const name = document.getElementById("add-emp-name").value.trim();
        const age = parseInt(document.getElementById("add-emp-age").value);
        const gender = document.getElementById("add-emp-gender").value;
        const email = document.getElementById("add-emp-email").value.trim();
        const phone = document.getElementById("add-emp-phone").value.trim();
        const address = document.getElementById("add-emp-address").value.trim();
        const department = document.getElementById("add-emp-department").value;
        const designation = document.getElementById("add-emp-designation").value.trim();
        const salary = parseFloat(document.getElementById("add-emp-salary").value);
        const joiningDate = document.getElementById("add-emp-joining-date").value;
        const status = document.getElementById("add-emp-status").value;

        if (!employeeId || !name || isNaN(age) || !email || !phone || !department || !designation || isNaN(salary) || !joiningDate) {
            showToast("Please fill in all required fields accurately.", "warning");
            return;
        }

        if (age < 18 || age > 75) {
            showToast("Employee age must be between 18 and 75.", "warning");
            return;
        }

        if (salary <= 0) {
            showToast("Salary must be a positive number greater than 0.", "warning");
            return;
        }

        const submitBtn = document.getElementById("btn-submit-add-emp");
        if (submitBtn) {
            submitBtn.disabled = true;
            submitBtn.innerHTML = `<span class="spinner"></span> Adding Employee...`;
        }

        try {
            const res = await api.post("/api/employees", {
                employee_id: employeeId,
                name,
                age,
                gender,
                email,
                phone,
                address,
                department,
                designation,
                salary,
                joining_date: joiningDate,
                status
            });

            if (!res.success) {
                showToast(res.message || "Failed to add employee", "error");
                return;
            }

            showToast("Employee added successfully.", "success");
            setTimeout(() => {
                window.location.href = "employees.html";
            }, 700);

        } catch (err) {
            showToast("Server connection error.", "error");
        } finally {
            if (submitBtn) {
                submitBtn.disabled = false;
                submitBtn.innerHTML = "Add Employee";
            }
        }
    });
}

/* ==========================================================
   EDIT EMPLOYEE CONTROLLER
   ========================================================== */
async function initEditEmployee() {
    await loadDepartmentDropdown("edit-emp-department");

    const urlParams = new URLSearchParams(window.location.search);
    const empId = urlParams.get("id");
    if (!empId) {
        showToast("No employee ID specified for editing.", "error");
        setTimeout(() => window.location.href = "employees.html", 1000);
        return;
    }

    // Load existing employee data
    try {
        const res = await api.get(`/api/employees/${empId}`);
        if (!res.success || !res.data) {
            showToast(res.message || "Employee not found", "error");
            return;
        }

        const emp = res.data;
        document.getElementById("edit-emp-id-display").value = emp.employee_id;
        document.getElementById("edit-emp-name").value = emp.name;
        document.getElementById("edit-emp-age").value = emp.age;
        document.getElementById("edit-emp-gender").value = emp.gender;
        document.getElementById("edit-emp-email").value = emp.email;
        document.getElementById("edit-emp-phone").value = emp.phone;
        document.getElementById("edit-emp-address").value = emp.address || "";
        document.getElementById("edit-emp-department").value = emp.department;
        document.getElementById("edit-emp-designation").value = emp.designation;
        document.getElementById("edit-emp-salary").value = emp.salary;
        document.getElementById("edit-emp-joining-date").value = emp.joining_date;
        document.getElementById("edit-emp-status").value = emp.status;

    } catch (err) {
        showToast("Error loading employee profile", "error");
    }

    const form = document.getElementById("edit-employee-form");
    if (!form) return;

    form.addEventListener("submit", async (e) => {
        e.preventDefault();

        const name = document.getElementById("edit-emp-name").value.trim();
        const age = parseInt(document.getElementById("edit-emp-age").value);
        const gender = document.getElementById("edit-emp-gender").value;
        const email = document.getElementById("edit-emp-email").value.trim();
        const phone = document.getElementById("edit-emp-phone").value.trim();
        const address = document.getElementById("edit-emp-address").value.trim();
        const department = document.getElementById("edit-emp-department").value;
        const designation = document.getElementById("edit-emp-designation").value.trim();
        const salary = parseFloat(document.getElementById("edit-emp-salary").value);
        const joiningDate = document.getElementById("edit-emp-joining-date").value;
        const status = document.getElementById("edit-emp-status").value;

        if (!name || isNaN(age) || !email || !phone || !department || !designation || isNaN(salary) || !joiningDate) {
            showToast("Please fill in all required fields accurately.", "warning");
            return;
        }

        const submitBtn = document.getElementById("btn-submit-edit-emp");
        if (submitBtn) {
            submitBtn.disabled = true;
            submitBtn.innerHTML = `<span class="spinner"></span> Updating...`;
        }

        try {
            const res = await api.put(`/api/employees/${empId}`, {
                name,
                age,
                gender,
                email,
                phone,
                address,
                department,
                designation,
                salary,
                joining_date: joiningDate,
                status
            });

            if (!res.success) {
                showToast(res.message || "Failed to update employee", "error");
                return;
            }

            showToast("Employee updated successfully.", "success");
            setTimeout(() => {
                window.location.href = "employees.html";
            }, 800);

        } catch (err) {
            showToast("Error communicating with backend server", "error");
        } finally {
            if (submitBtn) {
                submitBtn.disabled = false;
                submitBtn.innerHTML = "Save Changes";
            }
        }
    });
}
