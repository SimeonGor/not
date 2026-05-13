const $ = (id) => document.getElementById(id);

function setStatus(el, text, isError) {
  el.textContent = text || "";
  el.classList.toggle("error", Boolean(isError));
}

function normalizeUsername(raw) {
  const s = (raw || "").trim().replace(/^@/, "").toLowerCase();
  return s || null;
}

async function loadProfile(username) {
  const u = normalizeUsername(username);
  if (!u) {
    setStatus($("profile-status"), "Enter a username.", true);
    $("profile-block").classList.add("hidden");
    return null;
  }
  setStatus($("profile-status"), "Loading profile…", false);
  const res = await fetch(`/api/v1/users/${encodeURIComponent(u)}`);
  if (!res.ok) {
    setStatus($("profile-status"), res.status === 404 ? "User not found." : `HTTP ${res.status}`, true);
    $("profile-block").classList.add("hidden");
    return null;
  }
  const data = await res.json();
  $("profile-block").textContent = JSON.stringify(data, null, 2);
  $("profile-block").classList.remove("hidden");
  setStatus($("profile-status"), "OK", false);
  return u;
}

async function loadMessages(username) {
  const u = normalizeUsername(username);
  if (!u) {
    setStatus($("messages-status"), "", false);
    $("messages-list").innerHTML = "";
    return;
  }
  setStatus($("messages-status"), "Loading messages…", false);
  $("messages-list").innerHTML = "";
  const res = await fetch(`/api/v1/users/${encodeURIComponent(u)}/messages?limit=50`);
  if (!res.ok) {
    setStatus($("messages-status"), res.status === 404 ? "User not found." : `HTTP ${res.status}`, true);
    return;
  }
  const arr = await res.json();
  if (!arr.length) {
    setStatus($("messages-status"), "No messages.", false);
    return;
  }
  setStatus($("messages-status"), `${arr.length} message(s)`, false);
  for (const m of arr) {
    const div = document.createElement("div");
    div.className = "msg";
    const meta = document.createElement("div");
    meta.className = "meta";
    meta.textContent = `${m.createdAt} · ${m.deviceId} · mqtt: ${m.deliveredToMqtt}`;
    const body = document.createElement("div");
    body.textContent = `${m.senderName}: ${m.text}`;
    div.appendChild(meta);
    div.appendChild(body);
    $("messages-list").appendChild(div);
  }
}

$("btn-load").addEventListener("click", async () => {
  const raw = $("username").value;
  const u = await loadProfile(raw);
  if (u) await loadMessages(raw);
});

$("btn-send").addEventListener("click", async () => {
  const deviceId = $("send-device").value.trim();
  const senderName = $("send-sender").value.trim() || "web";
  const text = $("send-text").value.trim();
  if (!deviceId || !text) {
    setStatus($("send-status"), "deviceId and text required.", true);
    return;
  }
  setStatus($("send-status"), "Sending…", false);
  const res = await fetch("/api/v1/messages", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ deviceId, senderName, text }),
  });
  const body = await res.json().catch(() => ({}));
  if (!res.ok) {
    setStatus($("send-status"), body.message || `HTTP ${res.status}`, true);
    return;
  }
  setStatus($("send-status"), "Sent OK.", false);
  const u = normalizeUsername($("username").value);
  if (u) await loadMessages($("username").value);
});
