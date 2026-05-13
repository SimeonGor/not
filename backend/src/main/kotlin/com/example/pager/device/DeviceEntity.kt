package com.example.pager.device

import jakarta.persistence.Column
import jakarta.persistence.Entity
import jakarta.persistence.Id
import jakarta.persistence.Table
import java.time.Instant
import java.util.UUID

@Entity
@Table(name = "devices")
class DeviceEntity(
    @Id
    @Column(name = "id", nullable = false)
    var id: UUID,
    @Column(name = "device_id", nullable = false, length = 32)
    var deviceId: String,
    @Column(name = "display_name", length = 128)
    var displayName: String? = null,
    @Column(name = "created_at", nullable = false)
    var createdAt: Instant,
    @Column(name = "last_seen_at")
    var lastSeenAt: Instant? = null,
)
