-- Phase 4A: device pairing lifecycle (binding + expiring PIN)

ALTER TABLE users
    ADD COLUMN IF NOT EXISTS first_name VARCHAR(128);

ALTER TABLE devices
    ADD COLUMN IF NOT EXISTS user_id UUID REFERENCES users (id);

ALTER TABLE devices
    ADD COLUMN IF NOT EXISTS bound_at TIMESTAMPTZ;

CREATE INDEX IF NOT EXISTS idx_devices_user_id ON devices (user_id);

CREATE TABLE IF NOT EXISTS pairing_codes (
    id UUID PRIMARY KEY,
    device_id UUID NOT NULL REFERENCES devices (id),
    pin_hash VARCHAR(128) NOT NULL,
    expires_at TIMESTAMPTZ NOT NULL,
    used_at TIMESTAMPTZ,
    created_at TIMESTAMPTZ NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_pairing_codes_device_id ON pairing_codes (device_id);
CREATE INDEX IF NOT EXISTS idx_pairing_codes_pin_hash ON pairing_codes (pin_hash);
CREATE INDEX IF NOT EXISTS idx_pairing_codes_expires_at ON pairing_codes (expires_at);
