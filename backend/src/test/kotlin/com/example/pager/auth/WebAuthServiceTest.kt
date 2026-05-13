package com.example.pager.auth

import com.example.pager.common.InvalidLoginCodeException
import com.example.pager.common.TelegramUserNotAvailableException
import com.example.pager.config.WebAuthProperties
import com.example.pager.security.JwtService
import com.example.pager.telegram.TelegramMessageSender
import com.example.pager.user.TelegramUserService
import com.example.pager.user.UserEntity
import com.example.pager.user.UserRepository
import io.mockk.Runs
import io.mockk.every
import io.mockk.just
import io.mockk.mockk
import io.mockk.slot
import io.mockk.verify
import org.junit.jupiter.api.Assertions.assertEquals
import org.junit.jupiter.api.Assertions.assertThrows
import org.junit.jupiter.api.Assertions.assertTrue
import org.junit.jupiter.api.BeforeEach
import org.junit.jupiter.api.Test
import org.springframework.beans.factory.ObjectProvider
import java.time.Clock
import java.time.Instant
import java.time.ZoneOffset
import java.util.Optional
import java.util.UUID

class WebAuthServiceTest {
    private val userRepository = mockk<UserRepository>()
    private val telegramUserService = TelegramUserService(userRepository, Clock.fixed(Instant.parse("2026-05-13T12:00:00Z"), ZoneOffset.UTC))
    private val webLoginCodeRepository = mockk<WebLoginCodeRepository>()
    private val webLoginCodeHasher = WebLoginCodeHasher(WebAuthProperties(codeSalt = "test-salt"))
    private val webAuthProperties = WebAuthProperties(codeTtlSeconds = 300, codeLength = 6, codeSalt = "test-salt", jwtSecret = "0123456789abcdef0123456789abcdef", jwtTtlSeconds = 3600)
    private val jwtService = JwtService(webAuthProperties, Clock.fixed(Instant.parse("2026-05-13T12:00:00Z"), ZoneOffset.UTC))
    private val telegramSender = mockk<TelegramMessageSender>()
    private val telegramProvider = mockk<ObjectProvider<TelegramMessageSender>>()
    private val clock = Clock.fixed(Instant.parse("2026-05-13T12:00:00Z"), ZoneOffset.UTC)

    private lateinit var service: WebAuthService

    private val userId = UUID.fromString("00000000-0000-0000-0000-000000000001")
    private val user =
        UserEntity(userId, 99L, "alice", "A", null, Instant.parse("2026-05-13T12:00:00Z"), null)

    @BeforeEach
    fun setup() {
        every { telegramProvider.ifAvailable } returns telegramSender
        every { telegramSender.sendTelegramMessage(any(), any()) } just Runs
        service =
            WebAuthService(
                userRepository,
                telegramUserService,
                webLoginCodeRepository,
                webLoginCodeHasher,
                webAuthProperties,
                jwtService,
                telegramProvider,
                clock,
            )
    }

    @Test
    fun `requestCode invalidates previous and sends telegram`() {
        every { userRepository.findByUsername("alice") } returns Optional.of(user)
        every { webLoginCodeRepository.markActiveCodesUsedForUser(userId, clock.instant()) } returns 2
        val slotEntity = slot<WebLoginCodeEntity>()
        every { webLoginCodeRepository.save(capture(slotEntity)) } answers { firstArg() }

        val res = service.requestCode("Alice")

        assertEquals("Код отправлен в Telegram", res.message)
        assertEquals(300L, res.ttlSeconds)
        verify { webLoginCodeRepository.markActiveCodesUsedForUser(userId, clock.instant()) }
        verify { telegramSender.sendTelegramMessage(99L, match { it.contains("Web UI") }) }
        assertTrue(slotEntity.captured.codeHash.isNotEmpty())
    }

    @Test
    fun `requestCode throws when no telegram id`() {
        val u = UserEntity(userId, null, "bob", null, null, Instant.now(), null)
        every { userRepository.findByUsername("bob") } returns Optional.of(u)
        assertThrows(TelegramUserNotAvailableException::class.java) {
            service.requestCode("bob")
        }
    }

    @Test
    fun `verifyCode rejects wrong code`() {
        every { userRepository.findByUsername("alice") } returns Optional.of(user)
        every { webLoginCodeRepository.findActiveByCodeHash(any(), clock.instant()) } returns emptyList()
        assertThrows(InvalidLoginCodeException::class.java) {
            service.verifyCode("alice", "000000")
        }
    }
}
