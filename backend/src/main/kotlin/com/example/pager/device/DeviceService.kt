package com.example.pager.device

import com.example.pager.device.dto.DeviceResponse
import com.example.pager.message.MessageRepository
import com.example.pager.message.dto.MessageResponse
import com.example.pager.message.toMessageResponse
import org.slf4j.LoggerFactory
import org.springframework.stereotype.Service
import org.springframework.transaction.annotation.Transactional
import java.time.Instant
import java.util.UUID

@Service
class DeviceService(
    private val deviceRepository: DeviceRepository,
    private val messageRepository: MessageRepository,
) {
    private val log = LoggerFactory.getLogger(javaClass)

    @Transactional(readOnly = true)
    fun listDevices(): List<DeviceResponse> =
        deviceRepository.findAllByOrderByCreatedAtDesc().map { it.toDeviceResponse() }

    @Transactional(readOnly = true)
    fun listMessagesForDevice(deviceIdPath: String): List<MessageResponse> {
        val normalized = DeviceIdNormalizer.normalize(deviceIdPath)
        if (!DeviceIdNormalizer.isValid(normalized)) {
            return emptyList()
        }
        val device = deviceRepository.findByDeviceId(normalized).orElse(null) ?: return emptyList()
        return messageRepository.findByDeviceOrderByCreatedAtDesc(device).map { it.toMessageResponse(device.deviceId) }
    }

    @Transactional
    fun findOrCreateByDeviceId(deviceId: String): DeviceEntity {
        val existing = deviceRepository.findByDeviceId(deviceId)
        if (existing.isPresent) {
            return existing.get()
        }
        val created =
            DeviceEntity(
                id = UUID.randomUUID(),
                deviceId = deviceId,
                displayName = null,
                createdAt = Instant.now(),
                lastSeenAt = null,
                user = null,
                boundAt = null,
            )
        val saved = deviceRepository.save(created)
        log.info("[DEVICE] Auto-created device deviceId={}", saved.deviceId)
        return saved
    }
}

private fun DeviceEntity.toDeviceResponse() =
    DeviceResponse(
        id = id,
        deviceId = deviceId,
        displayName = displayName,
        createdAt = createdAt,
        lastSeenAt = lastSeenAt,
    )
