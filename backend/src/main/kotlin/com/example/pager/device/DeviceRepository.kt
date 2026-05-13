package com.example.pager.device

import com.example.pager.user.UserEntity
import org.springframework.data.jpa.repository.JpaRepository
import java.util.Optional
import java.util.UUID

interface DeviceRepository : JpaRepository<DeviceEntity, UUID> {
    fun findByDeviceId(deviceId: String): Optional<DeviceEntity>

    fun findAllByOrderByCreatedAtDesc(): List<DeviceEntity>

    fun findAllByUser(user: UserEntity): List<DeviceEntity>

    fun findAllByUser_Id(userId: UUID): List<DeviceEntity>

    fun findFirstByUser_IdOrderByBoundAtDesc(userId: UUID): Optional<DeviceEntity>
}
