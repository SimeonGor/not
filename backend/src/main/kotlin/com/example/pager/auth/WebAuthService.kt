package com.example.pager.auth

import com.example.pager.auth.dto.AuthTokenResponse
import com.example.pager.auth.dto.RequestCodeResponse
import com.example.pager.common.InvalidLoginCodeException
import com.example.pager.common.TelegramUserNotAvailableException
import com.example.pager.common.UserNotFoundException
import com.example.pager.config.WebAuthProperties
import com.example.pager.security.JwtService
import com.example.pager.telegram.TelegramMessageSender
import com.example.pager.user.TelegramUserService
import com.example.pager.user.UserRepository
import org.slf4j.LoggerFactory
import org.springframework.beans.factory.ObjectProvider
import org.springframework.stereotype.Service
import org.springframework.transaction.annotation.Transactional
import java.security.SecureRandom
import java.time.Clock
import java.time.temporal.ChronoUnit
import java.util.UUID

@Service
class WebAuthService(
    private val userRepository: UserRepository,
    private val telegramUserService: TelegramUserService,
    private val webLoginCodeRepository: WebLoginCodeRepository,
    private val webLoginCodeHasher: WebLoginCodeHasher,
    private val webAuthProperties: WebAuthProperties,
    private val jwtService: JwtService,
    private val telegramMessageSender: ObjectProvider<TelegramMessageSender>,
    private val clock: Clock,
) {
    private val log = LoggerFactory.getLogger(javaClass)
    private val secureRandom = SecureRandom()

    @Transactional
    fun requestCode(rawUsername: String): RequestCodeResponse {
        val username =
            telegramUserService.normalizeUsername(rawUsername)
                ?: throw UserNotFoundException("User not found")

        val user = userRepository.findByUsername(username).orElseThrow { UserNotFoundException("User not found: $username") }
        val telegramId = user.telegramId ?: throw TelegramUserNotAvailableException("User has no Telegram id linked")

        val sender =
            telegramMessageSender.ifAvailable
                ?: throw TelegramUserNotAvailableException("Cannot send login code: Telegram bot is not configured")

        val now = clock.instant()
        val invalidated = webLoginCodeRepository.markActiveCodesUsedForUser(user.id, now)
        if (invalidated > 0) {
            log.info("[WEB-AUTH] Invalidated {} active login code(s) for userId={}", invalidated, user.id)
        }

        val code = generateNumericCode()
        val hash = webLoginCodeHasher.hash(code)
        val expiresAt = now.plus(webAuthProperties.codeTtlSeconds, ChronoUnit.SECONDS)
        webLoginCodeRepository.save(
            WebLoginCodeEntity(
                id = UUID.randomUUID(),
                user = user,
                codeHash = hash,
                expiresAt = expiresAt,
                usedAt = null,
                createdAt = now,
            ),
        )

        val ttlMin = (webAuthProperties.codeTtlSeconds + 59) / 60
        val text =
            """
            Ваш код для входа в Smart Retro Pager Web UI: $code

            Код действует $ttlMin мин.
            Если вы не запрашивали вход, просто проигнорируйте это сообщение.
            """.trimIndent()

        sender.sendTelegramMessage(telegramId, text)
        log.info("[WEB-AUTH] Login code sent to telegramId={} for username={}", telegramId, username)

        return RequestCodeResponse(
            message = "Код отправлен в Telegram",
            ttlSeconds = webAuthProperties.codeTtlSeconds,
        )
    }

    @Transactional
    fun verifyCode(
        rawUsername: String,
        code: String,
    ): AuthTokenResponse {
        if (!CODE_PATTERN.matches(code.trim())) {
            throw InvalidLoginCodeException("Code must be exactly 6 digits")
        }
        val normalizedCode = code.trim()
        val username =
            telegramUserService.normalizeUsername(rawUsername)
                ?: throw UserNotFoundException("User not found")

        val user = userRepository.findByUsername(username).orElseThrow { UserNotFoundException("User not found: $username") }
        val now = clock.instant()
        val hash = webLoginCodeHasher.hash(normalizedCode)
        val rows = webLoginCodeRepository.findActiveByCodeHash(hash, now)
        val row =
            rows.firstOrNull { it.user.id == user.id }
                ?: throw InvalidLoginCodeException("Invalid or expired login code")

        row.usedAt = now
        webLoginCodeRepository.save(row)

        val token = jwtService.generateToken(user)
        return AuthTokenResponse(
            accessToken = token,
            tokenType = "Bearer",
            expiresIn = webAuthProperties.jwtTtlSeconds,
            username = user.username ?: username,
        )
    }

    private fun generateNumericCode(): String {
        val n = webAuthProperties.codeLength
        require(n == 6) { "web-auth.code-length must be 6 for this MVP" }
        val v = secureRandom.nextInt(900_000) + 100_000
        return v.toString()
    }

    companion object {
        private val CODE_PATTERN = Regex("^[0-9]{6}$")
    }
}
