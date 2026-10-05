/* ==========================================================
   SETTINGS & USER PREFERENCES CONTROLLER
   ========================================================== */

function initSettings() {
    const user = api.getCurrentUser();
    if (user) {
        const usernameEl = document.getElementById("settings-profile-username");
        const emailEl = document.getElementById("settings-profile-email");
        const roleEl = document.getElementById("settings-profile-role");
        const memberSinceEl = document.getElementById("settings-profile-joined");

        if (usernameEl) usernameEl.textContent = user.username;
        if (emailEl) emailEl.textContent = user.email;
        if (roleEl) roleEl.textContent = user.role;
        if (memberSinceEl) memberSinceEl.textContent = formatDate(user.created_at);
    }

    const form = document.getElementById("change-password-form");
    if (!form) return;

    form.addEventListener("submit", async (e) => {
        e.preventDefault();

        const oldPass = document.getElementById("current-password").value;
        const newPass = document.getElementById("new-password").value;
        const confirmPass = document.getElementById("confirm-new-password").value;

        if (!oldPass || !newPass || !confirmPass) {
            showToast("All password fields are required.", "warning");
            return;
        }

        if (newPass.length < 6) {
            showToast("New password must be at least 6 characters long.", "warning");
            return;
        }

        if (newPass !== confirmPass) {
            showToast("New passwords do not match.", "warning");
            return;
        }

        const btn = document.getElementById("btn-change-password");
        if (btn) {
            btn.disabled = true;
            btn.innerHTML = `<span class="spinner"></span> Updating Password...`;
        }

        try {
            const res = await api.post("/api/auth/change-password", {
                old_password: oldPass,
                new_password: newPass
            });

            if (!res.success) {
                showToast(res.message || "Failed to update password", "error");
                return;
            }

            showToast("Password updated successfully.", "success");
            form.reset();

        } catch (err) {
            showToast("Error updating password.", "error");
        } finally {
            if (btn) {
                btn.disabled = false;
                btn.innerHTML = "Update Password";
            }
        }
    });
}
