package com.example.pager.device

import com.example.pager.user.UserEntity
import jakarta.persistence.Column
import jakarta.persistence.Entity
import jakarta.persistence.FetchType
import jakarta.persistence.Id
import jakarta.persistence.JoinColumn
import jakarta.persistence.ManyToOne
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
    @ManyToOne(fetch = FetchType.LAZY)
    @JoinColumn(name = "user_id")
    var user: UserEntity? = null,
    @Column(name = "bound_at")
    var boundAt: Instant? = null,
)
