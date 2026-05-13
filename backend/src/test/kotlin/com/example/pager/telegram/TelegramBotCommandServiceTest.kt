package com.example.pager.telegram

import com.example.pager.device.DeviceBindingService
import com.example.pager.device.DeviceEntity
import com.example.pager.device.DeviceRepository
import com.example.pager.message.MessageRepository
import com.example.pager.message.MessageService
import com.example.pager.user.TelegramUserService
import com.example.pager.user.UserEntity
import com.example.pager.user.UserRepository
import io.mockk.every
import io.mockk.mockk
import io.mockk.verify
import org.junit.jupiter.api.Assertions.assertEquals
import org.junit.jupiter.api.Assertions.assertTrue
import org.junit.jupiter.api.BeforeEach
import org.junit.jupiter.api.Test
import java.time.Clock
import java.time.Instant
import java.time.ZoneOffset
import java.util.Optional
import java.util.UUID

class TelegramBotCommandServiceTest {
    private val userRepository = mockk<UserRepository>()
    private val telegramUserService =
        TelegramUserService(userRepository, Clock.fixed(Instant.parse("2026-05-13T12:00:00Z"), ZoneOffset.UTC))
    private val deviceRepository = mockk<DeviceRepository>()
    private val messageRepository = mockk<MessageRepository>()
    private val messageService = mockk<MessageService>()
    private val deviceBindingService = mockk<DeviceBindingService>()
    private lateinit var service: TelegramBotCommandService

    private val sender =
        UserEntity(UUID.randomUUID(), 1L, "sender", "S", null, Instant.parse("2026-05-13T12:00:00Z"), null)
    private val receiver =
        UserEntity(UUID.randomUUID(), 2L, "bob", "Bob", null, Instant.parse("2026-05-13T12:00:00Z"), null)

    @BeforeEach
    fun setup() {
        service =
            TelegramBotCommandService(
                telegramUserService,
                userRepository,
                deviceRepository,
                messageRepository,
                messageService,
                deviceBindingService,
            )
    }

    @Test
    fun `handleSend returns not found when receiver missing`() {
        every { userRepository.findByUsername("bob") } returns Optional.empty()
        val r = service.handleSend(sender, "/send bob hello")
        assertEquals("Пользователь bob не найден.", r)
    }

    @Test
    fun `handleSend returns no device when receiver has none`() {
        every { userRepository.findByUsername("bob") } returns Optional.of(receiver)
        every { deviceRepository.findFirstByUser_IdOrderByBoundAtDesc(receiver.id) } returns Optional.empty()
        val r = service.handleSend(sender, "/send bob hello")
        assertEquals("Пользователь bob найден, но у него нет привязанного пейджера.", r)
    }

    @Test
    fun `handleSend calls MessageService when receiver has device`() {
        val device =
            DeviceEntity(
                UUID.randomUUID(),
                "84F3EB12ABCD",
                null,
                Instant.now(),
                null,
                receiver,
                Instant.now(),
            )
        every { userRepository.findByUsername("bob") } returns Optional.of(receiver)
        every { deviceRepository.findFirstByUser_IdOrderByBoundAtDesc(receiver.id) } returns Optional.of(device)
        every { messageService.sendMessage(any()) } returns mockk(relaxed = true)

        val r = service.handleSend(sender, "/send bob Hello there")
        assertTrue(r.contains("отправлено"))
        verify { messageService.sendMessage(match { it.deviceId == "84F3EB12ABCD" && it.text == "Hello there" }) }
    }

    @Test
    fun `handleHistory without devices returns hint`() {
        every { deviceRepository.findAllByUser_Id(sender.id) } returns emptyList()
        val r = service.handleHistory(sender, "/history")
        assertTrue(r.contains("нет привязанного пейджера"))
        assertTrue(r.contains("/send"))
    }
}
