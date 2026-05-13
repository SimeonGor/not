package com.example.pager.me

import com.example.pager.device.DeviceEntity
import com.example.pager.device.DeviceRepository
import com.example.pager.message.MessageService
import com.example.pager.message.dto.MessageResponse
import com.example.pager.user.TelegramUserService
import com.example.pager.user.UserEntity
import com.example.pager.user.UserRepository
import io.mockk.every
import io.mockk.mockk
import io.mockk.verify
import org.junit.jupiter.api.Assertions.assertEquals
import org.junit.jupiter.api.Test
import java.time.Instant
import java.util.Optional
import java.util.UUID

class MeSendServiceTest {
    private val userRepository = mockk<UserRepository>()
    private val deviceRepository = mockk<DeviceRepository>()
    private val messageService = mockk<MessageService>()
    private val telegramUserService = TelegramUserService(userRepository, java.time.Clock.systemUTC())
    private val service = MeSendService(userRepository, deviceRepository, messageService, telegramUserService)

    @Test
    fun `sender without bound device can send to receiver with device`() {
        val senderId = UUID.fromString("00000000-0000-0000-0000-0000000000aa")
        val receiverId = UUID.fromString("00000000-0000-0000-0000-0000000000bb")
        val sender =
            UserEntity(senderId, 1L, "alice", "A", null, Instant.parse("2026-05-13T12:00:00Z"), null)
        val receiver =
            UserEntity(receiverId, 2L, "bob", "B", null, Instant.parse("2026-05-13T12:00:00Z"), null)
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
        val msgId = UUID.randomUUID()
        every { userRepository.findById(senderId) } returns Optional.of(sender)
        every { userRepository.findByUsername("bob") } returns Optional.of(receiver)
        every { deviceRepository.findFirstByUser_IdOrderByBoundAtDesc(receiverId) } returns Optional.of(device)
        every { messageService.sendMessage(any()) } returns
            MessageResponse(
                id = msgId,
                deviceId = "84F3EB12ABCD",
                senderName = "alice",
                text = "hi",
                mqttTopic = "pager/84F3EB12ABCD/rx",
                deliveredToMqtt = true,
                createdAt = Instant.now(),
            )

        val res = service.send(senderId, "bob", "hi")

        assertEquals("sent", res.status)
        assertEquals("bob", res.targetUsername)
        assertEquals(msgId, res.messageId)
        verify { messageService.sendMessage(match { it.deviceId == "84F3EB12ABCD" && it.text == "hi" }) }
    }
}
