package com.example.pager.user

import com.example.pager.common.UserNotFoundException
import com.pengrad.telegrambot.model.User
import org.springframework.stereotype.Service
import org.springframework.transaction.annotation.Transactional
import java.time.Clock
import java.time.Instant
import java.util.UUID

@Service
class TelegramUserService(
    private val userRepository: UserRepository,
    private val clock: Clock,
) {
    fun normalizeUsername(username: String?): String? =
        username
            ?.removePrefix("@")
            ?.trim()
            ?.lowercase()
            ?.takeIf { it.isNotBlank() }

    fun displayNameForTelegramUser(user: UserEntity): String {
        val fromUsername = user.username?.takeIf { it.isNotBlank() }
        if (fromUsername != null) return fromUsername
        val fromFirst = user.firstName?.trim()?.takeIf { it.isNotBlank() }
        if (fromFirst != null) return fromFirst
        val tid = user.telegramId ?: return "user-unknown"
        return "user-$tid"
    }

    /** Truncate for MQTT / API sender_name limits. */
    fun displayNameForSend(user: UserEntity, maxLen: Int = 64): String =
        displayNameForTelegramUser(user).take(maxLen)

    @Transactional
    fun findOrCreateOrUpdateTelegramUser(from: User): UserEntity =
        findOrCreateOrUpdateTelegramUser(
            telegramId = from.id(),
            rawUsername = from.username(),
            firstName = from.firstName(),
            lastName = from.lastName(),
        )

    @Transactional
    fun findOrCreateOrUpdateTelegramUser(
        telegramId: Long,
        rawUsername: String?,
        firstName: String?,
        lastName: String?,
    ): UserEntity {
        val username = normalizeUsername(rawUsername)
        val now = clock.instant()
        val existing = userRepository.findByTelegramId(telegramId)
        if (existing.isPresent) {
            val u = existing.get()
            u.username = username
            u.firstName = firstName?.trim()?.takeIf { it.isNotEmpty() }
            u.lastName = lastName?.trim()?.takeIf { it.isNotEmpty() }
            u.updatedAt = now
            return userRepository.save(u)
        }
        return userRepository.save(
            UserEntity(
                id = UUID.randomUUID(),
                telegramId = telegramId,
                username = username,
                firstName = firstName?.trim()?.takeIf { it.isNotEmpty() },
                lastName = lastName?.trim()?.takeIf { it.isNotEmpty() },
                createdAt = now,
                updatedAt = now,
            ),
        )
    }

    fun requireByNormalizedUsername(username: String): UserEntity =
        userRepository.findByUsername(username).orElseThrow {
            UserNotFoundException("User not found: $username")
        }
}
