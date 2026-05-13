-- Phase 4C: one-time web login codes (hash only)

CREATE TABLE web_login_codes (
    id UUID PRIMARY KEY,
    user_id UUID NOT NULL REFERENCES users (id),
    code_hash VARCHAR(128) NOT NULL,
    expires_at TIMESTAMPTZ NOT NULL,
    used_at TIMESTAMPTZ,
    created_at TIMESTAMPTZ NOT NULL
);

CREATE INDEX idx_web_login_codes_user_id ON web_login_codes (user_id);
CREATE INDEX idx_web_login_codes_code_hash ON web_login_codes (code_hash);
CREATE INDEX idx_web_login_codes_expires_at ON web_login_codes (expires_at);
