/* ==========================================================
   DEPARTMENTS MANAGEMENT CONTROLLER
   ========================================================== */

let currentEditDeptId = null;

async function loadDepartments() {
    const tbody = document.getElementById("departments-table-body");
    if (!tbody) return;

    tbody.innerHTML = `<tr><td colspan="5" class="table-loading-overlay"><div class="spinner spinner-dark"></div>Loading departments...</td></tr>`;

    try {
        const res = await api.get("/api/departments");
        if (!res.success) {
            tbody.innerHTML = `<tr><td colspan="5" style="text-align:center; padding: 2rem; color: var(--danger);">${escapeHtml(res.message)}</td></tr>`;
            return;
        }

        const depts = res.data || [];
        if (depts.length === 0) {
            tbody.innerHTML = `<tr><td colspan="5" style="text-align:center; padding: 2rem; color: var(--text-muted);">No departments configured.</td></tr>`;
            return;
        }

        tbody.innerHTML = depts.map(d => `
            <tr>
                <td><strong>#${d.id}</strong></td>
                <td><span class="badge badge-dept" style="font-size:0.85rem;">${escapeHtml(d.department_name)}</span></td>
                <td>${escapeHtml(d.description || "No description provided.")}</td>
                <td>
                    <span class="badge ${d.employee_count > 0 ? 'badge-active' : 'badge-inactive'}">
                        ${d.employee_count} Employee${d.employee_count === 1 ? '' : 's'}
                    </span>
                </td>
                <td>
                    <div class="action-buttons">
                        <button class="btn-icon edit" title="Edit Department" onclick="openEditDeptModal(${d.id}, '${escapeHtml(d.department_name)}', '${escapeHtml(d.description || '')}')">
                            <svg width="15" height="15" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M15.232 5.232l3.536 3.536m-2.036-5.036a2.5 2.5 0 113.536 3.536L6.5 21.036H3v-3.572L16.732 3.732z"></path></svg>
                        </button>
                        <button class="btn-icon delete" title="Delete Department" onclick="deleteDepartment(${d.id}, '${escapeHtml(d.department_name)}', ${d.employee_count})">
                            <svg width="15" height="15" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M19 7l-.867 12.142A2 2 0 0116.138 21H7.862a2 2 0 01-1.995-1.858L5 7m5 4v6m4-6v6m1-10V4a1 1 0 00-1-1h-4a1 1 0 00-1 1v3M4 7h16"></path></svg>
                        </button>
                    </div>
                </td>
            </tr>
        `).join("");

    } catch (err) {
        tbody.innerHTML = `<tr><td colspan="5" style="text-align:center; padding: 2rem; color: var(--danger);">Failed to load departments.</td></tr>`;
    }
}

// Add Department Modal Handlers
function openAddDeptModal() {
    const modal = document.getElementById("add-dept-modal");
    if (modal) {
        document.getElementById("add-dept-name").value = "";
        document.getElementById("add-dept-desc").value = "";
        modal.classList.add("active");
    }
}

function closeAddDeptModal() {
    const modal = document.getElementById("add-dept-modal");
    if (modal) modal.classList.remove("active");
}

async function submitAddDepartment(e) {
    if (e) e.preventDefault();
    const name = document.getElementById("add-dept-name").value.trim();
    const desc = document.getElementById("add-dept-desc").value.trim();

    if (!name) {
        showToast("Department name is required.", "warning");
        return;
    }

    try {
        const res = await api.post("/api/departments", { department_name: name, description: desc });
        if (!res.success) {
            showToast(res.message || "Failed to add department", "error");
            return;
        }

        showToast("Department added successfully.", "success");
        closeAddDeptModal();
        loadDepartments();
    } catch (err) {
        showToast("Error adding department.", "error");
    }
}

// Edit Department Modal Handlers
function openEditDeptModal(id, name, desc) {
    currentEditDeptId = id;
    const modal = document.getElementById("edit-dept-modal");
    if (modal) {
        document.getElementById("edit-dept-name").value = name;
        document.getElementById("edit-dept-desc").value = desc;
        modal.classList.add("active");
    }
}

function closeEditDeptModal() {
    currentEditDeptId = null;
    const modal = document.getElementById("edit-dept-modal");
    if (modal) modal.classList.remove("active");
}

async function submitEditDepartment(e) {
    if (e) e.preventDefault();
    if (!currentEditDeptId) return;

    const name = document.getElementById("edit-dept-name").value.trim();
    const desc = document.getElementById("edit-dept-desc").value.trim();

    if (!name) {
        showToast("Department name is required.", "warning");
        return;
    }

    try {
        const res = await api.put(`/api/departments/${currentEditDeptId}`, { department_name: name, description: desc });
        if (!res.success) {
            showToast(res.message || "Failed to update department", "error");
            return;
        }

        showToast("Department updated successfully.", "success");
        closeEditDeptModal();
        loadDepartments();
    } catch (err) {
        showToast("Error updating department.", "error");
    }
}

// Delete Department with Guard
async function deleteDepartment(id, name, employeeCount) {
    if (employeeCount > 0) {
        showToast(`Cannot delete department '${name}' because ${employeeCount} employee(s) are currently assigned to it. Please reassign them first.`, "warning", 5000);
        return;
    }

    if (!confirm(`Are you sure you want to delete department '${name}'?`)) {
        return;
    }

    try {
        const res = await api.delete(`/api/departments/${id}`);
        if (!res.success) {
            showToast(res.message || "Failed to delete department", "error");
            return;
        }

        showToast("Department deleted successfully.", "success");
        loadDepartments();
    } catch (err) {
        showToast("Error deleting department.", "error");
    }
}
