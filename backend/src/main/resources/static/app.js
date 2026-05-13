const TOKEN_KEY = "smartPagerAccessToken";
const USERNAME_KEY = "smartPagerUsername";

const $ = (id) => document.getElementById(id);

function setStatus(el, text, isError) {
  el.textContent = text || "";
  el.classList.toggle("error", Boolean(isError));
}

function normalizeUsername(raw) {
  const s = (raw || "").trim().replace(/^@/, "").toLowerCase();
  return s || null;
}

function getToken() {
  return localStorage.getItem(TOKEN_KEY);
}

function setSession(token, username) {
  if (token) localStorage.setItem(TOKEN_KEY, token);
  else localStorage.removeItem(TOKEN_KEY);
  if (username) localStorage.setItem(USERNAME_KEY, username);
  else localStorage.removeItem(USERNAME_KEY);
}

function clearSession() {
  localStorage.removeItem(TOKEN_KEY);
  localStorage.removeItem(USERNAME_KEY);
}

function showLogin(message) {
  $("view-app").classList.add("hidden");
  $("view-login").classList.remove("hidden");
  if (message) setStatus($("login-status"), message, true);
  $("btn-request-code").disabled = false;
  $("btn-verify").disabled = false;
}

function showApp() {
  $("view-login").classList.add("hidden");
  $("view-app").classList.remove("hidden");
  const u = localStorage.getItem(USERNAME_KEY) || "";
  $("session-line").textContent = u ? `Вы вошли как: @${u}` : "Вы вошли.";
}

async function parseError(res) {
  try {
    const j = await res.json();
    return j.message || j.error || `HTTP ${res.status}`;
  } catch {
    return `HTTP ${res.status}`;
  }
}

function authHeaders() {
  const t = getToken();
  const h = { "Content-Type": "application/json" };
  if (t) h.Authorization = `Bearer ${t}`;
  return h;
}

async function fetchMe() {
  setStatus($("me-status"), "Загрузка…", false);
  const res = await fetch("/api/v1/me", { headers: authHeaders() });
  if (res.status === 401 || res.status === 403) {
    clearSession();
    showLogin("Сессия истекла. Войдите снова.");
    return;
  }
  if (!res.ok) {
    setStatus($("me-status"), await parseError(res), true);
    $("me-block").classList.add("hidden");
    return;
  }
  const data = await res.json();
  $("me-block").textContent = JSON.stringify(data, null, 2);
  $("me-block").classList.remove("hidden");
  setStatus($("me-status"), "OK", false);
}

async function fetchMeMessages() {
  setStatus($("me-messages-status"), "Загрузка…", false);
  $("me-messages-list").innerHTML = "";
  const res = await fetch("/api/v1/me/messages?limit=50", { headers: authHeaders() });
  if (res.status === 401 || res.status === 403) {
    clearSession();
    showLogin("Сессия истекла. Войдите снова.");
    return;
  }
  if (!res.ok) {
    setStatus($("me-messages-status"), await parseError(res), true);
    return;
  }
  const arr = await res.json();
  if (!arr.length) {
    setStatus($("me-messages-status"), "Нет сообщений.", false);
    return;
  }
  setStatus($("me-messages-status"), `${arr.length} сообщ.`, false);
  for (const m of arr) {
    const div = document.createElement("div");
    div.className = "msg";
    const meta = document.createElement("div");
    meta.className = "meta";
    meta.textContent = `${m.createdAt} · ${m.deviceId}`;
    const body = document.createElement("div");
    body.textContent = `${m.senderName}: ${m.text}`;
    div.appendChild(meta);
    div.appendChild(body);
    $("me-messages-list").appendChild(div);
  }
}

async function refreshAppData() {
  await fetchMe();
  await fetchMeMessages();
}

function updateLoginButtons() {
  const u = normalizeUsername($("login-username").value);
  const c = $("login-code").value.trim();
  $("btn-request-code").disabled = !u;
  $("btn-verify").disabled = !u || c.length !== 6;
}

$("login-username").addEventListener("input", updateLoginButtons);
$("login-code").addEventListener("input", updateLoginButtons);

$("login-username").addEventListener("keydown", (e) => {
  if (e.key === "Enter" && !$("btn-request-code").disabled) $("btn-request-code").click();
});

$("login-code").addEventListener("keydown", (e) => {
  if (e.key === "Enter" && !$("btn-verify").disabled) $("btn-verify").click();
});

$("btn-request-code").addEventListener("click", async () => {
  const u = normalizeUsername($("login-username").value);
  if (!u) return;
  setStatus($("login-status"), "Отправка кода…", false);
  $("btn-request-code").disabled = true;
  const res = await fetch("/api/v1/auth/request-code", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ username: u }),
  });
  $("btn-request-code").disabled = false;
  updateLoginButtons();
  if (!res.ok) {
    setStatus($("login-status"), await parseError(res), true);
    return;
  }
  const data = await res.json();
  setStatus($("login-status"), data.message || "Код отправлен.", false);
});

$("btn-verify").addEventListener("click", async () => {
  const u = normalizeUsername($("login-username").value);
  const code = $("login-code").value.trim();
  if (!u || code.length !== 6) return;
  setStatus($("login-status"), "Проверка кода…", false);
  $("btn-verify").disabled = true;
  const res = await fetch("/api/v1/auth/verify-code", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ username: u, code }),
  });
  $("btn-verify").disabled = false;
  updateLoginButtons();
  if (!res.ok) {
    setStatus($("login-status"), await parseError(res), true);
    return;
  }
  const data = await res.json();
  setSession(data.accessToken, data.username || u);
  setStatus($("login-status"), "", false);
  showApp();
  await refreshAppData();
});

$("btn-logout").addEventListener("click", () => {
  clearSession();
  $("login-code").value = "";
  showLogin("");
  updateLoginButtons();
});

$("btn-me-send").addEventListener("click", async () => {
  const target = normalizeUsername($("send-target-user").value);
  const text = $("send-target-text").value.trim();
  if (!target || !text) {
    setStatus($("me-send-status"), "Укажите пользователя и текст.", true);
    return;
  }
  setStatus($("me-send-status"), "Отправка…", false);
  const res = await fetch("/api/v1/me/send", {
    method: "POST",
    headers: authHeaders(),
    body: JSON.stringify({ targetUsername: target, text }),
  });
  if (res.status === 401 || res.status === 403) {
    clearSession();
    showLogin("Сессия истекла. Войдите снова.");
    return;
  }
  if (!res.ok) {
    setStatus($("me-send-status"), await parseError(res), true);
    return;
  }
  const data = await res.json();
  setStatus($("me-send-status"), `Отправлено → @${data.targetUsername}`, false);
  await fetchMeMessages();
});

$("btn-demo-send").addEventListener("click", async () => {
  const deviceId = $("demo-device").value.trim();
  const senderName = $("demo-sender").value.trim() || "web";
  const text = $("demo-text").value.trim();
  if (!deviceId || !text) {
    setStatus($("demo-send-status"), "deviceId и text обязательны.", true);
    return;
  }
  setStatus($("demo-send-status"), "Отправка…", false);
  const res = await fetch("/api/v1/messages", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ deviceId, senderName, text }),
  });
  const body = await res.json().catch(() => ({}));
  if (!res.ok) {
    setStatus($("demo-send-status"), body.message || `HTTP ${res.status}`, true);
    return;
  }
  setStatus($("demo-send-status"), "OK (публичный API)", false);
  if (getToken()) await fetchMeMessages();
});

function boot() {
  if (getToken()) {
    showApp();
    refreshAppData();
  } else {
    showLogin("");
    $("view-login").classList.remove("hidden");
    updateLoginButtons();
  }
}

boot();
