# Smart Retro Pager — Backend (Phase 3)

Kotlin Spring Boot REST API: PostgreSQL (Flyway) + MQTT publish (Eclipse Paho) to `pager/{deviceId}/rx` as **plain text** (`Sender: text`).

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

## 7. Build and tests

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

Codes: `VALIDATION_ERROR`, `PAYLOAD_TOO_LONG`, `MQTT_PUBLISH_ERROR`, `INTERNAL_ERROR`.

If MQTT publish fails after the row is stored, `deliveredToMqtt` stays `false` and the API returns **500** with `MQTT_PUBLISH_ERROR`.
