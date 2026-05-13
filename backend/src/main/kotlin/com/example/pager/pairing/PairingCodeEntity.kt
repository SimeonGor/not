package com.example.pager.pairing

import com.example.pager.device.DeviceEntity
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
@Table(name = "pairing_codes")
class PairingCodeEntity(
    @Id
    @Column(name = "id", nullable = false)
    var id: UUID,
    @ManyToOne(fetch = FetchType.LAZY, optional = false)
    @JoinColumn(name = "device_id", nullable = false)
    var device: DeviceEntity,
    @Column(name = "pin_hash", nullable = false, length = 128)
    var pinHash: String,
    @Column(name = "expires_at", nullable = false)
    var expiresAt: Instant,
    @Column(name = "used_at")
    var usedAt: Instant? = null,
    @Column(name = "created_at", nullable = false)
    var createdAt: Instant,
)
