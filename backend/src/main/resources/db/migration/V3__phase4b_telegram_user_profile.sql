-- Phase 4B: last name, updated_at; normalize existing usernames (lowercase, no leading @)

ALTER TABLE users
    ADD COLUMN IF NOT EXISTS last_name VARCHAR(128);

ALTER TABLE users
    ADD COLUMN IF NOT EXISTS updated_at TIMESTAMPTZ;

UPDATE users
SET updated_at = created_at
WHERE updated_at IS NULL;

UPDATE users
SET username = LOWER(REGEXP_REPLACE(TRIM(username), '^@', ''))
WHERE username IS NOT NULL AND TRIM(username) <> '';

UPDATE users
SET username = NULLIF(TRIM(username), '')
WHERE username IS NOT NULL;
