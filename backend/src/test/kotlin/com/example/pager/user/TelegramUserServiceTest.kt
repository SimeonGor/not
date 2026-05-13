package com.example.pager.user

import io.mockk.mockk
import org.junit.jupiter.api.Assertions.assertEquals
import org.junit.jupiter.api.Assertions.assertNull
import org.junit.jupiter.api.BeforeEach
import org.junit.jupiter.api.Test
import java.time.Clock
import java.time.Instant
import java.time.ZoneOffset
import java.util.UUID

class TelegramUserServiceTest {
    private lateinit var service: TelegramUserService

    @BeforeEach
    fun setup() {
        val clock = Clock.fixed(Instant.parse("2026-05-13T12:00:00Z"), ZoneOffset.UTC)
        service = TelegramUserService(mockk(relaxed = true), clock)
    }

    @Test
    fun `normalizeUsername lowercases strips at`() {
        assertEquals("alice", service.normalizeUsername("@Alice"))
        assertEquals("bob", service.normalizeUsername("bob"))
        assertNull(service.normalizeUsername("   "))
        assertNull(service.normalizeUsername(null))
    }

    @Test
    fun `displayName prefers username then firstName then synthetic`() {
        val u1 =
            UserEntity(
                UUID.randomUUID(),
                99L,
                "alice",
                null,
                null,
                Instant.now(),
                null,
            )
        assertEquals("alice", service.displayNameForTelegramUser(u1))

        val u2 =
            UserEntity(
                UUID.randomUUID(),
                42L,
                null,
                "Bob",
                null,
                Instant.now(),
                null,
            )
        assertEquals("Bob", service.displayNameForTelegramUser(u2))

        val u3 =
            UserEntity(
                UUID.randomUUID(),
                7L,
                null,
                null,
                null,
                Instant.now(),
                null,
            )
        assertEquals("user-7", service.displayNameForTelegramUser(u3))
    }
}
