# Smart Retro Pager — Backend (Phase 3–4B)

Kotlin Spring Boot REST API: PostgreSQL (Flyway) + MQTT publish (Eclipse Paho) to `pager/{deviceId}/rx` as **plain text** (`Sender: text`). **Phase 4A** adds device–Telegram pairing (`/bind`, `/unbind`, PIN, MQTT `pager/{deviceId}/sys`). **Phase 4B** adds **Telegram messaging without a sender pager**: `/send`, `/send_device`, `/history`, `/devices`, user profile REST, and a **static Web UI** at `http://localhost:8080/` (no auth).

## Prerequisites

- **JDK 17+** (recommended **17–24** for running Gradle; JDK 25 is not supported by the Kotlin compiler daemon yet).
- Docker with Compose v2.

### macOS: use JDK 17 for Gradle

```bash
export JAVA_HOME=$(/usr/libexec/java_home -v 17)
```

## 1. Start stack (Postgres, Mosquitto, Backend)

From the **repository root** (directory that contains `docker-compose.yml`):

```bash
docker compose up -d --build
docker ps
```

Expected containers: `pager-postgres`, `pager-mosquitto`, `pager-backend`.

API when backend runs in Docker: `http://localhost:8080` (порт проброшен на хост).

Чтобы поднять **только** Postgres и Mosquitto (backend локально через `./gradlew bootRun`):

```bash
docker compose up -d postgres mosquitto
```

## 2. Run the backend locally (without Docker image)

```bash
export JAVA_HOME=$(/usr/libexec/java_home -v 17)   # macOS, if needed
cd backend
./gradlew bootRun
```

API: `http://localhost:8080`

- Datasource: `jdbc:postgresql://localhost:5432/pager_db` (`pager_user` / `pager_password`).
- MQTT: `tcp://localhost:1883`, client id `pager-backend`, QoS **1**.

## 3. Send a test message

Use the same `deviceId` as on the NodeMCU (MAC without colons, uppercase), e.g. `84F3EB12ABCD`:

```bash
curl -s -X POST http://localhost:8080/api/v1/messages \
  -H "Content-Type: application/json" \
  -d '{
    "deviceId": "84F3EB12ABCD",
    "senderName": "Alice",
    "text": "Hello from Kotlin backend!"
  }'
```

Expected: JSON with `mqttTopic` `pager/84F3EB12ABCD/rx`, `deliveredToMqtt: true`, and timestamps.

The pager must use the **LAN IP** of this machine as MQTT host (not `localhost`); the backend uses `localhost:1883` when running on the same host as Docker.

## 4. Message history

```bash
curl -s http://localhost:8080/api/v1/devices/84F3EB12ABCD/messages
```

Unknown / invalid `deviceId` returns `[]`.

## 5. List devices

```bash
curl -s http://localhost:8080/api/v1/devices
```

## 6. Verify on the OLED pager

With Wi‑Fi + MQTT connected to the same broker:

1. Send the `curl` POST above with your real `deviceId`.
2. The device should show `Alice: Hello from Kotlin backend!` (or your text) and beep.

## 7. Phase 4A — Pairing (REST, MQTT sys, Telegram)

### Environment variables

| Variable | Purpose |
|----------|---------|
| `PAIRING_PIN_SALT` | Salt appended to PIN before SHA-256 (default in `application.yml` is for dev only). |
| `TG_BOT_TOKEN` | Telegram Bot API token; if empty, the bot bean is **not** registered and a startup **WARN** explains that Telegram is disabled. |
| `TG_BOT_USERNAME` | Optional display / docs; not required for API calls. |

`docker compose` passes these through from the host when set (see repository `docker-compose.yml`).

### REST: binding status, PIN, reset

Use your real `deviceId` (MAC without colons, uppercase), e.g. `84F3EB12ABCD`:

```bash
# Whether the device is bound and whether pairing is required
curl -s http://localhost:8080/api/v1/devices/84F3EB12ABCD/binding-status

# Issue a new numeric PIN (409 if already bound)
curl -s -X POST http://localhost:8080/api/v1/devices/84F3EB12ABCD/pairing-pin \
  -H "Content-Type: application/json" -d '{}'

# Clear binding from the device side (also publishes UNBOUND on sys)
curl -s -X POST http://localhost:8080/api/v1/devices/84F3EB12ABCD/reset-binding \
  -H "Content-Type: application/json" -d '{}'
```

### Telegram flow (`/bind`)

1. On the pager, call `pairing-pin` (or your firmware equivalent) and read the **6-digit PIN** from the UI.
2. In Telegram, send: `/bind 123456` (replace with the real PIN).
3. On success the backend publishes **`BIND_SUCCESS`** to `pager/{deviceId}/sys` (plain text, not JSON). The firmware can subscribe to `pager/{yourDeviceId}/sys` and react (e.g. clear PIN UI, show “bound”).
4. `/unbind` clears binding and publishes **`UNBOUND`** on the same topic.
5. `/me` shows your Telegram id, username, bound devices, and reminds you that you can **send to others without your own pager** (Phase 4B).

### Verify MQTT `sys` on the device

Subscribe (e.g. `mosquitto_sub -h <broker> -t 'pager/84F3EB12ABCD/sys' -v`) while completing `/bind` or `reset-binding`. You should see payload `BIND_SUCCESS` or `UNBOUND` as plain UTF-8 text.

## 8. Phase 4B — Telegram messaging + Web UI

### Rules

- **Sender** does **not** need a bound device; every command registers/updates the Telegram user in `users` (username stored **lowercase**, without `@`).
- **Receiver** (for `/send username …`) must exist in the DB **with a username** and have **at least one bound** device; the first bound device (by `bound_at` desc) receives the MQTT message.
- All sends go through **`MessageService`** → topic `pager/{deviceId}/rx`, payload `senderName: text`.

### Telegram commands (in addition to 4A)

| Command | Purpose |
|---------|---------|
| `/send username текст` | Message to another user’s pager (your pager not required). |
| `/send_device deviceId текст` | Direct send to a device id (lab / debug). |
| `/history` or `/history 10` | Last messages **to your** bound device(s); default 5, max 20. |
| `/devices` | List your bound devices and `bound_at`. |
| `/start`, `/help`, `/me` | Welcome / help / profile (see bot texts for username warnings). |

Examples:

```text
/send bob Привет с телеграма!
/send_device 84F3EB12ABCD Тест напрямую
/history 10
```

### REST — user profile and history (Web UI uses this)

```bash
curl -s http://localhost:8080/api/v1/users/bob
curl -s 'http://localhost:8080/api/v1/users/bob/messages?limit=20'
```

- `username` in the path is normalized like Telegram (`@Bob` → `bob`). Unknown user → **404** `USER_NOT_FOUND`.
- Messages: all devices bound to that user, newest first; `limit` default **20**, max **100**. Empty list if the user has no devices.

### Web UI (static)

With the backend running, open **`http://localhost:8080/`** in a browser. Load profile + message history by **username**, and use the form to **`POST /api/v1/messages`** (deviceId + senderName + text) for a quick test send.

## 9. Build and tests

```bash
export JAVA_HOME=$(/usr/libexec/java_home -v 17)   # if needed
cd backend
./gradlew build
```

## Error responses

Structured JSON, e.g.:

```json
{
  "error": "VALIDATION_ERROR",
  "message": "...",
  "timestamp": "2026-05-13T12:00:00Z"
}
```

Codes: `VALIDATION_ERROR`, `PAYLOAD_TOO_LONG`, `MQTT_PUBLISH_ERROR`, `INTERNAL_ERROR`, `USER_NOT_FOUND`, `ALREADY_BOUND`, `DEVICE_BOUND_TO_ANOTHER_USER`, `INVALID_PIN_FORMAT`, `INVALID_OR_EXPIRED_PIN`.

If MQTT publish fails after the row is stored, `deliveredToMqtt` stays `false` and the API returns **500** with `MQTT_PUBLISH_ERROR`.
