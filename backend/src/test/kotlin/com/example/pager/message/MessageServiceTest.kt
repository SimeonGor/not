package com.example.pager.message

import com.example.pager.device.DeviceEntity
import com.example.pager.device.DeviceService
import com.example.pager.message.dto.SendMessageRequest
import com.example.pager.mqtt.MqttPublisherService
import io.mockk.Runs
import io.mockk.clearMocks
import io.mockk.every
import io.mockk.just
import io.mockk.mockk
import io.mockk.verify
import org.junit.jupiter.api.Assertions.assertEquals
import org.junit.jupiter.api.Assertions.assertTrue
import org.junit.jupiter.api.BeforeEach
import org.junit.jupiter.api.Test
import org.springframework.transaction.TransactionStatus
import org.springframework.transaction.support.TransactionTemplate
import java.time.Instant
import java.util.Optional
import java.util.UUID
import java.util.function.Consumer

class MessageServiceTest {
    private val deviceService = mockk<DeviceService>()
    private val messageRepository = mockk<MessageRepository>()
    private val mqttPublisherService = mockk<MqttPublisherService>()
    private val transactionTemplate = mockk<TransactionTemplate>()

    private lateinit var messageService: MessageService

    private val device =
        DeviceEntity(
            id = UUID.randomUUID(),
            deviceId = "84F3EB12ABCD",
            displayName = null,
            createdAt = Instant.parse("2026-05-13T12:00:00Z"),
            lastSeenAt = null,
        )

    @BeforeEach
    fun setup() {
        clearMocks(deviceService, messageRepository, mqttPublisherService, transactionTemplate)
        messageService = MessageService(deviceService, messageRepository, mqttPublisherService, transactionTemplate)
        every { transactionTemplate.executeWithoutResult(any()) } answers
            {
                @Suppress("UNCHECKED_CAST")
                val action = firstArg<Consumer<TransactionStatus>>()
                action.accept(mockk(relaxed = true))
            }
    }

    @Test
    fun `sendMessage creates device via service and publishes mqtt`() {
        every { deviceService.findOrCreateByDeviceId("84F3EB12ABCD") } returns device
        var lastSaved: MessageEntity? = null
        every { messageRepository.save(any()) } answers
            {
                lastSaved = firstArg()
                lastSaved!!
            }
        every { messageRepository.findById(any()) } answers { Optional.of(lastSaved!!) }
        every { mqttPublisherService.publish(any(), any()) } just Runs

        val response =
            messageService.sendMessage(
                SendMessageRequest(
                    deviceId = "84f3eb12abcd",
                    senderName = "Alice",
                    text = "Hello",
                ),
            )

        verify { deviceService.findOrCreateByDeviceId("84F3EB12ABCD") }
        verify { mqttPublisherService.publish("pager/84F3EB12ABCD/rx", "Alice: Hello") }
        assertEquals("84F3EB12ABCD", response.deviceId)
        assertTrue(response.deliveredToMqtt)
        assertEquals("pager/84F3EB12ABCD/rx", response.mqttTopic)
    }
}
