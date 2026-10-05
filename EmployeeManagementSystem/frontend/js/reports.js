/* ==========================================================
   REPORTS & ANALYTICS CONTROLLER
   ========================================================== */

async function loadReports() {
    await loadDepartmentDropdown("report-filter-department");
    fetchReportData();

    const deptFilter = document.getElementById("report-filter-department");
    const statusFilter = document.getElementById("report-filter-status");

    if (deptFilter) deptFilter.addEventListener("change", fetchReportData);
    if (statusFilter) statusFilter.addEventListener("change", fetchReportData);
}

async function fetchReportData() {
    const dept = document.getElementById("report-filter-department") ? document.getElementById("report-filter-department").value : "";
    const status = document.getElementById("report-filter-status") ? document.getElementById("report-filter-status").value : "";

    const params = {};
    if (dept && dept !== "All") params.department = dept;
    if (status && status !== "All") params.status = status;

    try {
        const res = await api.get("/api/reports/stats", params);
        if (!res.success) {
            showToast(res.message || "Failed to load report", "error");
            return;
        }

        const data = res.data;

        // Metric values
        document.getElementById("rep-total-emp").textContent = data.total_employees;
        document.getElementById("rep-active-emp").textContent = data.active_employees;
        document.getElementById("rep-avg-salary").textContent = formatCurrency(data.average_salary);
        document.getElementById("rep-max-salary").textContent = formatCurrency(data.max_salary);
        document.getElementById("rep-min-salary").textContent = formatCurrency(data.min_salary);
        document.getElementById("rep-total-payroll").textContent = formatCurrency(data.total_payroll);

        // Breakdown Tables
        const deptTbody = document.getElementById("rep-dept-table-body");
        if (deptTbody) {
            const depts = data.by_department || {};
            const keys = Object.keys(depts);
            if (keys.length === 0) {
                deptTbody.innerHTML = `<tr><td colspan="2" style="text-align:center; color:var(--text-muted);">No records found.</td></tr>`;
            } else {
                deptTbody.innerHTML = keys.map(k => `
                    <tr>
                        <td><strong>${escapeHtml(k)}</strong></td>
                        <td><span class="badge badge-dept">${depts[k]} Employees</span></td>
                    </tr>
                `).join("");
            }
        }

        const statusTbody = document.getElementById("rep-status-table-body");
        if (statusTbody) {
            const statuses = data.by_status || {};
            const keys = Object.keys(statuses);
            if (keys.length === 0) {
                statusTbody.innerHTML = `<tr><td colspan="2" style="text-align:center; color:var(--text-muted);">No records found.</td></tr>`;
            } else {
                statusTbody.innerHTML = keys.map(k => {
                    let badgeClass = "badge-active";
                    if (k === "Inactive") badgeClass = "badge-inactive";
                    else if (k === "On Leave") badgeClass = "badge-on-leave";

                    return `
                        <tr>
                            <td><span class="badge ${badgeClass}">${escapeHtml(k)}</span></td>
                            <td><strong>${statuses[k]} Employees</strong></td>
                        </tr>
                    `;
                }).join("");
            }
        }

    } catch (err) {
        showToast("Error retrieving report data", "error");
    }
}

function exportReportCsv() {
    const dept = document.getElementById("report-filter-department") ? document.getElementById("report-filter-department").value : "";
    const status = document.getElementById("report-filter-status") ? document.getElementById("report-filter-status").value : "";

    let url = `${API_CONFIG.baseUrl}/api/reports/export?`;
    if (dept && dept !== "All") url += `department=${encodeURIComponent(dept)}&`;
    if (status && status !== "All") url += `status=${encodeURIComponent(status)}&`;

    window.open(url, '_blank');
    showToast("Generating and downloading CSV report...", "info");
}
