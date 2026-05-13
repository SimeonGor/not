package com.example.pager.user

import org.springframework.data.jpa.repository.JpaRepository
import java.util.Optional
import java.util.UUID

interface UserRepository : JpaRepository<UserEntity, UUID> {
    fun findByTelegramId(telegramId: Long): Optional<UserEntity>
}
