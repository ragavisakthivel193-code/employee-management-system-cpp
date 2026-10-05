/* ==========================================================
   DASHBOARD CONTROLLER & DATA VISUALIZATIONS
   ========================================================== */

let deptChart = null;
let genderChart = null;
let salaryChart = null;
let trendChart = null;

async function loadDashboard() {
    try {
        const res = await api.get("/api/dashboard/stats");
        if (!res.success) {
            showToast(res.message || "Failed to load dashboard statistics", "error");
            return;
        }

        const data = res.data;

        // 1. Update Stat Cards
        const totalEmpEl = document.getElementById("stat-total-employees");
        const activeEmpEl = document.getElementById("stat-active-employees");
        const totalDeptsEl = document.getElementById("stat-total-departments");
        const avgSalaryEl = document.getElementById("stat-average-salary");

        if (totalEmpEl) totalEmpEl.textContent = data.total_employees;
        if (activeEmpEl) activeEmpEl.textContent = data.active_employees;
        if (totalDeptsEl) totalDeptsEl.textContent = data.total_departments;
        if (avgSalaryEl) avgSalaryEl.textContent = formatCurrency(data.average_salary);

        // 2. Render Charts
        renderDepartmentChart(data.by_department || []);
        renderGenderChart(data.by_gender || []);
        renderSalaryChart(data.salary_by_department || []);
        renderTrendChart(data.joining_trend || []);

        // 3. Render Recent Employees
        renderRecentEmployees(data.recent_employees || []);

    } catch (err) {
        console.error("Dashboard error:", err);
        showToast("Error connecting to server. Please check backend status.", "error");
    }
}

// Chart 1: Employees by Department (Doughnut)
function renderDepartmentChart(items) {
    const ctx = document.getElementById("chart-department");
    if (!ctx) return;

    const labels = items.map(i => i.department);
    const counts = items.map(i => i.count);

    if (deptChart) deptChart.destroy();

    deptChart = new Chart(ctx, {
        type: 'doughnut',
        data: {
            labels: labels,
            datasets: [{
                data: counts,
                backgroundColor: [
                    '#4f46e5', '#06b6d4', '#10b981', '#f59e0b', '#ec4899', '#8b5cf6', '#64748b'
                ],
                borderWidth: 2,
                borderColor: '#ffffff'
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: {
                    position: 'bottom',
                    labels: { boxWidth: 12, padding: 12, font: { family: 'inherit', size: 12 } }
                }
            },
            cutout: '68%'
        }
    });
}

// Chart 2: Employees by Gender (Pie)
function renderGenderChart(items) {
    const ctx = document.getElementById("chart-gender");
    if (!ctx) return;

    const labels = items.map(i => i.gender);
    const counts = items.map(i => i.count);

    if (genderChart) genderChart.destroy();

    genderChart = new Chart(ctx, {
        type: 'pie',
        data: {
            labels: labels,
            datasets: [{
                data: counts,
                backgroundColor: ['#3b82f6', '#ec4899', '#a855f7'],
                borderWidth: 2,
                borderColor: '#ffffff'
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: {
                    position: 'bottom',
                    labels: { boxWidth: 12, padding: 12, font: { family: 'inherit', size: 12 } }
                }
            }
        }
    });
}

// Chart 3: Salary Distribution by Department (Bar)
function renderSalaryChart(items) {
    const ctx = document.getElementById("chart-salary");
    if (!ctx) return;

    const labels = items.map(i => i.department);
    const salaries = items.map(i => i.avg_salary);

    if (salaryChart) salaryChart.destroy();

    salaryChart = new Chart(ctx, {
        type: 'bar',
        data: {
            labels: labels,
            datasets: [{
                label: 'Avg Salary (₹)',
                data: salaries,
                backgroundColor: 'rgba(79, 70, 229, 0.85)',
                borderRadius: 6,
                borderSkipped: false
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            scales: {
                y: {
                    beginAtZero: true,
                    ticks: {
                        callback: val => '₹' + (val / 1000) + 'k',
                        font: { family: 'inherit' }
                    },
                    grid: { color: '#f1f5f9' }
                },
                x: {
                    grid: { display: false },
                    ticks: { font: { family: 'inherit' } }
                }
            },
            plugins: {
                legend: { display: false },
                tooltip: {
                    callbacks: {
                        label: ctx => ' Avg Salary: ' + formatCurrency(ctx.raw)
                    }
                }
            }
        }
    });
}

// Chart 4: Employee Joining Trend (Line)
function renderTrendChart(items) {
    const ctx = document.getElementById("chart-trend");
    if (!ctx) return;

    const labels = items.map(i => i.month);
    const counts = items.map(i => i.count);

    if (trendChart) trendChart.destroy();

    trendChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: labels,
            datasets: [{
                label: 'New Joinings',
                data: counts,
                fill: true,
                backgroundColor: 'rgba(14, 165, 233, 0.12)',
                borderColor: '#0284c7',
                borderWidth: 2.5,
                tension: 0.35,
                pointBackgroundColor: '#0284c7',
                pointRadius: 4
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            scales: {
                y: {
                    beginAtZero: true,
                    ticks: { stepSize: 1, font: { family: 'inherit' } },
                    grid: { color: '#f1f5f9' }
                },
                x: {
                    grid: { display: false },
                    ticks: { font: { family: 'inherit' } }
                }
            },
            plugins: {
                legend: { display: false }
            }
        }
    });
}

// Render Recent Employees in Table
function renderRecentEmployees(list) {
    const tbody = document.getElementById("recent-employees-table-body");
    if (!tbody) return;

    if (list.length === 0) {
        tbody.innerHTML = `<tr><td colspan="7" style="text-align:center; padding: 2rem; color: var(--text-muted);">No employees found.</td></tr>`;
        return;
    }

    tbody.innerHTML = list.map(emp => {
        let statusBadge = "badge-active";
        if (emp.status === "Inactive") statusBadge = "badge-inactive";
        else if (emp.status === "On Leave") statusBadge = "badge-on-leave";

        return `
            <tr>
                <td><strong>${escapeHtml(emp.employee_id)}</strong></td>
                <td>
                    <div style="display:flex; align-items:center; gap: 0.6rem;">
                        <span class="avatar" style="width:28px; height:28px; font-size:0.75rem;">${getInitials(emp.name)}</span>
                        <span>${escapeHtml(emp.name)}</span>
                    </div>
                </td>
                <td><span class="badge badge-dept">${escapeHtml(emp.department)}</span></td>
                <td>${escapeHtml(emp.designation)}</td>
                <td><strong>${formatCurrency(emp.salary)}</strong></td>
                <td><span class="badge ${statusBadge}">${escapeHtml(emp.status)}</span></td>
                <td>
                    <button class="btn-icon view" title="View Profile" onclick="openViewModal(${emp.id})">
                        <svg width="16" height="16" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M15 12a3 3 0 11-6 0 3 3 0 016 0z"></path><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M2.458 12C3.732 7.943 7.523 5 12 5c4.478 0 8.268 2.943 9.542 7-1.274 4.057-5.064 7-9.542 7-4.477 0-8.268-2.943-9.542-7z"></path></svg>
                    </button>
                </td>
            </tr>
        `;
    }).join("");
}

// Global modal view handler
async function openViewModal(id) {
    try {
        const res = await api.get(`/api/employees/${id}`);
        if (!res.success) {
            showToast(res.message || "Could not retrieve employee details", "error");
            return;
        }
        const emp = res.data;
        const modal = document.getElementById("view-employee-modal");
        if (!modal) return;

        let statusClass = "badge-active";
        if (emp.status === "Inactive") statusClass = "badge-inactive";
        else if (emp.status === "On Leave") statusClass = "badge-on-leave";

        document.getElementById("view-avatar").textContent = getInitials(emp.name);
        document.getElementById("view-name").textContent = emp.name;
        document.getElementById("view-designation").textContent = emp.designation;
        document.getElementById("view-empid-badge").textContent = emp.employee_id;
        document.getElementById("view-status-badge").className = `badge ${statusClass}`;
        document.getElementById("view-status-badge").textContent = emp.status;

        document.getElementById("view-field-email").textContent = emp.email;
        document.getElementById("view-field-phone").textContent = emp.phone;
        document.getElementById("view-field-department").textContent = emp.department;
        document.getElementById("view-field-designation").textContent = emp.designation;
        document.getElementById("view-field-salary").textContent = formatCurrency(emp.salary);
        document.getElementById("view-field-joining").textContent = formatDate(emp.joining_date);
        document.getElementById("view-field-age").textContent = emp.age + " years";
        document.getElementById("view-field-gender").textContent = emp.gender;
        document.getElementById("view-field-address").textContent = emp.address || "Not specified";

        // Setup edit button inside modal
        const editBtn = document.getElementById("view-modal-edit-btn");
        if (editBtn) {
            editBtn.onclick = () => {
                window.location.href = `edit-employee.html?id=${emp.id}`;
            };
        }

        modal.classList.add("active");
    } catch (err) {
        showToast("Error loading employee details", "error");
    }
}

function closeViewModal() {
    const modal = document.getElementById("view-employee-modal");
    if (modal) modal.classList.remove("active");
}
