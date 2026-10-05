/* ==========================================================
   API Client & Core Utilities
   ========================================================== */

const API_CONFIG = {
    // Automatically use the host if served by the backend, or fallback to port 8088
    baseUrl: (window.location.port === "8088") 
        ? "" 
        : (window.location.protocol === "file:" || !window.location.port || window.location.port !== "8088")
            ? "http://localhost:8088"
            : ""
};

const api = {
    getToken() {
        return localStorage.getItem("ems_token") || "";
    },

    setSession(token, user) {
        localStorage.setItem("ems_token", token);
        localStorage.setItem("ems_user", JSON.stringify(user));
    },

    clearSession() {
        localStorage.removeItem("ems_token");
        localStorage.removeItem("ems_user");
    },

    getCurrentUser() {
        try {
            const raw = localStorage.getItem("ems_user");
            return raw ? JSON.parse(raw) : null;
        } catch (e) {
            return null;
        }
    },

    async request(endpoint, options = {}) {
        const url = `${API_CONFIG.baseUrl}${endpoint}`;
        const headers = {
            "Content-Type": "application/json",
            ...(options.headers || {})
        };

        const token = this.getToken();
        if (token) {
            headers["Authorization"] = `Bearer ${token}`;
            headers["X-Auth-Token"] = token;
        }

        try {
            const res = await fetch(url, { ...options, headers });
            
            // Check for unauthorized
            if (res.status === 401 && !endpoint.includes("/login")) {
                this.clearSession();
                window.location.href = "login.html?expired=1";
                throw new Error("Session expired. Please log in again.");
            }

            const data = await res.json();
            return data;
        } catch (err) {
            console.error(`[API ERROR] ${options.method || 'GET'} ${endpoint}:`, err);
            throw err;
        }
    },

    get(endpoint, params = {}) {
        const query = new URLSearchParams();
        for (const [k, v] of Object.entries(params)) {
            if (v !== undefined && v !== null && v !== "" && v !== "All") {
                query.append(k, v);
            }
        }
        const qs = query.toString();
        return this.request(qs ? `${endpoint}?${qs}` : endpoint, { method: "GET" });
    },

    post(endpoint, body = {}) {
        return this.request(endpoint, {
            method: "POST",
            body: JSON.stringify(body)
        });
    },

    put(endpoint, body = {}) {
        return this.request(endpoint, {
            method: "PUT",
            body: JSON.stringify(body)
        });
    },

    delete(endpoint) {
        return this.request(endpoint, { method: "DELETE" });
    }
};

/* ==========================================================
   TOAST NOTIFICATION COMPONENT
   ========================================================== */
function showToast(message, type = "success", duration = 3500) {
    let container = document.getElementById("toast-container");
    if (!container) {
        container = document.createElement("div");
        container.id = "toast-container";
        container.className = "toast-container";
        document.body.appendChild(container);
    }

    const toast = document.createElement("div");
    toast.className = `toast toast-${type}`;

    let iconSvg = "";
    if (type === "success") {
        iconSvg = `<svg class="toast-icon" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M5 13l4 4L19 7"></path></svg>`;
    } else if (type === "error") {
        iconSvg = `<svg class="toast-icon" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M6 18L18 6M6 6l12 12"></path></svg>`;
    } else if (type === "warning") {
        iconSvg = `<svg class="toast-icon" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 9v2m0 4h.01m-6.938 4h13.856c1.54 0 2.502-1.667 1.732-3L13.732 4c-.77-1.333-2.694-1.333-3.464 0L3.34 16c-.77 1.333.192 3 1.732 3z"></path></svg>`;
    } else {
        iconSvg = `<svg class="toast-icon" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M13 16h-1v-4h-1m1-4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z"></path></svg>`;
    }

    toast.innerHTML = `
        ${iconSvg}
        <span class="toast-message">${escapeHtml(message)}</span>
    `;

    container.appendChild(toast);

    setTimeout(() => {
        toast.classList.add("toast-hiding");
        setTimeout(() => toast.remove(), 300);
    }, duration);
}

/* ==========================================================
   FORMATTERS & HELPERS
   ========================================================== */
function formatCurrency(val) {
    const num = parseFloat(val) || 0;
    return new Intl.NumberFormat('en-IN', {
        style: 'currency',
        currency: 'INR',
        maximumFractionDigits: 0
    }).format(num);
}

function formatDate(dateStr) {
    if (!dateStr) return "-";
    try {
        const d = new Date(dateStr);
        if (isNaN(d.getTime())) return dateStr;
        return d.toLocaleDateString('en-GB', { day: '2-digit', month: 'short', year: 'numeric' });
    } catch {
        return dateStr;
    }
}

function escapeHtml(str) {
    if (!str) return "";
    return String(str)
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;")
        .replace(/'/g, "&#039;");
}

function getInitials(name) {
    if (!name) return "EM";
    const parts = name.trim().split(" ");
    if (parts.length === 1) return parts[0].substring(0, 2).toUpperCase();
    return (parts[0][0] + parts[parts.length - 1][0]).toUpperCase();
}
