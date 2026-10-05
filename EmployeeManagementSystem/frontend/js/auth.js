/* ==========================================================
   AUTHENTICATION & SESSION MANAGEMENT
   ========================================================== */

function requireAuth() {
    const token = api.getToken();
    const user = api.getCurrentUser();
    if (!token || !user) {
        window.location.href = "login.html";
    }
}

function requireGuest() {
    const token = api.getToken();
    const user = api.getCurrentUser();
    if (token && user) {
        window.location.href = "dashboard.html";
    }
}

function initUserWidget() {
    const user = api.getCurrentUser();
    if (!user) return;

    // Sidebar user details
    const nameEl = document.getElementById("sidebar-username");
    const roleEl = document.getElementById("sidebar-role");
    const avatarEl = document.getElementById("sidebar-avatar");

    if (nameEl) nameEl.textContent = user.username || "User";
    if (roleEl) roleEl.textContent = user.role || "HR";
    if (avatarEl) avatarEl.textContent = getInitials(user.username || "User");

    // Header user details if present
    const headerNameEl = document.getElementById("header-user-name");
    const headerRoleEl = document.getElementById("header-user-role");
    if (headerNameEl) headerNameEl.textContent = user.username || "User";
    if (headerRoleEl) headerRoleEl.textContent = user.role || "HR";
}

function logout() {
    api.post("/api/auth/logout").finally(() => {
        api.clearSession();
        window.location.href = "login.html";
    });
}

// Toggle password visibility helper
function setupPasswordToggle(inputSelector, btnSelector) {
    const input = document.querySelector(inputSelector);
    const btn = document.querySelector(btnSelector);
    if (!input || !btn) return;

    btn.addEventListener("click", () => {
        const isPassword = input.getAttribute("type") === "password";
        input.setAttribute("type", isPassword ? "text" : "password");
        btn.innerHTML = isPassword 
            ? `<svg width="18" height="18" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M13.875 18.825A10.05 10.05 0 0112 19c-4.478 0-8.268-2.943-9.543-7a9.97 9.97 0 011.563-3.029m5.858.908a3 3 0 114.243 4.243M9.878 9.878l4.242 4.242M9.88 9.88l-3.29-3.29m7.532 7.532l3.29 3.29M3 3l18 18"></path></svg>`
            : `<svg width="18" height="18" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M15 12a3 3 0 11-6 0 3 3 0 016 0z"></path><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M2.458 12C3.732 7.943 7.523 5 12 5c4.478 0 8.268 2.943 9.542 7-1.274 4.057-5.064 7-9.542 7-4.477 0-8.268-2.943-9.542-7z"></path></svg>`;
    });
}

// Mobile sidebar drawer toggle helper
function setupMobileMenu() {
    const toggleBtn = document.querySelector(".mobile-nav-toggle");
    const sidebar = document.querySelector(".sidebar");
    if (!toggleBtn || !sidebar) return;

    toggleBtn.addEventListener("click", () => {
        sidebar.classList.toggle("mobile-open");
    });

    // Close on click outside on mobile
    document.addEventListener("click", (e) => {
        if (sidebar.classList.contains("mobile-open") && !sidebar.contains(e.target) && !toggleBtn.contains(e.target)) {
            sidebar.classList.remove("mobile-open");
        }
    });
}

document.addEventListener("DOMContentLoaded", () => {
    initUserWidget();
    setupMobileMenu();
});
