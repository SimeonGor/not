package com.example.pager.message

import com.example.pager.common.PayloadTooLongException
import com.example.pager.device.DeviceIdNormalizer
import com.example.pager.device.DeviceService
import com.example.pager.message.dto.MessageResponse
import com.example.pager.message.dto.SendMessageRequest
import com.example.pager.message.toMessageResponse
import com.example.pager.mqtt.MqttPublisherService
import org.slf4j.LoggerFactory
import org.springframework.stereotype.Service
import org.springframework.transaction.support.TransactionTemplate
import java.time.Instant
import java.util.UUID

@Service
class MessageService(
    private val deviceService: DeviceService,
    private val messageRepository: MessageRepository,
    private val mqttPublisherService: MqttPublisherService,
    private val transactionTemplate: TransactionTemplate,
) {
    private val log = LoggerFactory.getLogger(javaClass)

    fun sendMessage(request: SendMessageRequest): MessageResponse {
        val deviceId = DeviceIdNormalizer.normalize(request.deviceId)
        val topic = "pager/$deviceId/rx"
        val payload = MessagePayloadFormatter.format(request.senderName, request.text)
        if (payload.length > MessagePayloadFormatter.MAX_MQTT_PAYLOAD_CHARS) {
            throw PayloadTooLongException(
                "MQTT payload must not exceed ${MessagePayloadFormatter.MAX_MQTT_PAYLOAD_CHARS} characters",
            )
        }

        val device = deviceService.findOrCreateByDeviceId(deviceId)
        val now = Instant.now()
        val messageId = UUID.randomUUID()
        val pending =
            MessageEntity(
                id = messageId,
                device = device,
                senderName = request.senderName,
                textContent = request.text,
                mqttTopic = topic,
                deliveredToMqtt = false,
                createdAt = now,
            )
        transactionTemplate.executeWithoutResult {
            messageRepository.save(pending)
        }
        log.info("[MESSAGE] Saved message id={} deliveredToMqtt=false", messageId)

        mqttPublisherService.publish(topic, payload)

        transactionTemplate.executeWithoutResult {
            val msg = messageRepository.findById(messageId).orElseThrow()
            msg.deliveredToMqtt = true
            messageRepository.save(msg)
        }
        log.info("[MESSAGE] Saved message id={} deliveredToMqtt=true", messageId)

        val saved = messageRepository.findById(messageId).orElseThrow()
        return saved.toMessageResponse(deviceId)
    }
}