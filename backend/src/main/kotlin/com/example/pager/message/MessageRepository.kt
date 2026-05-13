package com.example.pager.message

import com.example.pager.device.DeviceEntity
import org.springframework.data.jpa.repository.JpaRepository
import java.util.UUID

interface MessageRepository : JpaRepository<MessageEntity, UUID> {
    fun findByDeviceOrderByCreatedAtDesc(device: DeviceEntity): List<MessageEntity>
}
