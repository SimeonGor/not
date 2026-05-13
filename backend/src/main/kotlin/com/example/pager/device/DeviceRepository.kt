package com.example.pager.device

import org.springframework.data.jpa.repository.JpaRepository
import java.util.Optional
import java.util.UUID

interface DeviceRepository : JpaRepository<DeviceEntity, UUID> {
    fun findByDeviceId(deviceId: String): Optional<DeviceEntity>

    fun findAllByOrderByCreatedAtDesc(): List<DeviceEntity>
}
