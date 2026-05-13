package com.example.pager.security

import com.example.pager.config.WebAuthProperties
import com.example.pager.user.UserEntity
import org.junit.jupiter.api.Assertions.assertEquals
import org.junit.jupiter.api.Assertions.assertThrows
import org.junit.jupiter.api.Test
import java.time.Clock
import java.time.Instant
import java.time.ZoneOffset
import java.util.UUID

class JwtServiceTest {
    private val fixed = Instant.parse("2026-05-13T12:00:00Z")
    private val clock = Clock.fixed(fixed, ZoneOffset.UTC)
    private val props =
        WebAuthProperties(
            jwtSecret = "0123456789abcdef0123456789abcdef", // 32 chars
            jwtTtlSeconds = 3600,
        )
    private val jwtService = JwtService(props, clock)

    @Test
    fun `generate and parse round trip`() {
        val user =
            UserEntity(
                UUID.fromString("00000000-0000-0000-0000-000000000099"),
                42L,
                "alice",
                "A",
                null,
                fixed,
                fixed,
            )
        val token = jwtService.generateToken(user)
        val auth = jwtService.parseAndValidate(token)
        assertEquals(user.id, auth.userId)
        assertEquals("alice", auth.username)
        assertEquals(42L, auth.telegramId)
    }

    @Test
    fun `invalid token throws`() {
        assertThrows(Exception::class.java) {
            jwtService.parseAndValidate("not-a-jwt")
        }
    }
}
