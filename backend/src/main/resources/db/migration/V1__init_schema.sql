CREATE TABLE users (
    id UUID PRIMARY KEY,
    telegram_id BIGINT,
    username VARCHAR(64),
    created_at TIMESTAMPTZ NOT NULL
);

CREATE UNIQUE INDEX idx_users_telegram_id ON users (telegram_id);
CREATE UNIQUE INDEX idx_users_username ON users (username);

CREATE TABLE devices (
    id UUID PRIMARY KEY,
    device_id VARCHAR(32) NOT NULL,
    display_name VARCHAR(128),
    created_at TIMESTAMPTZ NOT NULL,
    last_seen_at TIMESTAMPTZ
);

CREATE UNIQUE INDEX idx_devices_device_id ON devices (device_id);

CREATE TABLE messages (
    id UUID PRIMARY KEY,
    device_id UUID NOT NULL REFERENCES devices (id),
    sender_name VARCHAR(64) NOT NULL,
    text_content TEXT NOT NULL,
    mqtt_topic VARCHAR(256) NOT NULL,
    delivered_to_mqtt BOOLEAN NOT NULL,
    created_at TIMESTAMPTZ NOT NULL
);

CREATE INDEX idx_messages_device_id_created_at ON messages (device_id, created_at DESC);
