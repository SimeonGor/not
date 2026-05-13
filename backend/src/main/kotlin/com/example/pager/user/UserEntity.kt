package com.example.pager.user

import jakarta.persistence.Column
import jakarta.persistence.Entity
import jakarta.persistence.Id
import jakarta.persistence.Table
import java.time.Instant
import java.util.UUID

@Entity
@Table(name = "users")
class UserEntity(
    @Id
    @Column(name = "id", nullable = false)
    var id: UUID,
    @Column(name = "telegram_id")
    var telegramId: Long? = null,
    @Column(name = "username", length = 64)
    var username: String? = null,
    @Column(name = "created_at", nullable = false)
    var createdAt: Instant,
)
