package com.example.pager.security

import com.example.pager.config.WebAuthProperties
import com.example.pager.user.UserEntity
import io.jsonwebtoken.Claims
import io.jsonwebtoken.JwtException
import io.jsonwebtoken.Jwts
import io.jsonwebtoken.MalformedJwtException
import io.jsonwebtoken.security.Keys
import org.springframework.stereotype.Service
import java.nio.charset.StandardCharsets
import java.time.Clock
import java.time.Instant
import java.util.Date
import java.util.UUID
import javax.crypto.SecretKey

@Service
class JwtService(
    private val webAuthProperties: WebAuthProperties,
    private val clock: Clock,
) {
    private val signingKey: SecretKey by lazy {
        val raw = webAuthProperties.jwtSecret.toByteArray(StandardCharsets.UTF_8)
        require(raw.size >= 32) {
            "web-auth.jwt-secret must be at least 32 bytes (256 bits) for HS256"
        }
        Keys.hmacShaKeyFor(raw)
    }

    fun generateToken(user: UserEntity): String {
        val now = clock.instant()
        val exp = now.plusSeconds(webAuthProperties.jwtTtlSeconds)
        return Jwts
            .builder()
            .subject(user.id.toString())
            .claim("username", user.username)
            .claim("telegramId", user.telegramId)
            .issuedAt(Date.from(now))
            .expiration(Date.from(exp))
            .signWith(signingKey)
            .compact()
    }

    fun parseAndValidate(token: String): AuthenticatedUser {
        try {
            val claims: Claims =
                Jwts
                    .parser()
                    .verifyWith(signingKey)
                    .clock { Date.from(clock.instant()) }
                    .build()
                    .parseSignedClaims(token)
                    .payload
            val userId = UUID.fromString(claims.subject)
            val username = claims["username"] as String?
            val tidClaim = claims["telegramId"]
            val telegramId =
                when (tidClaim) {
                    null -> null
                    is Number -> tidClaim.toLong()
                    else -> (tidClaim as? String)?.toLongOrNull()
                }
            return AuthenticatedUser(userId = userId, username = username, telegramId = telegramId)
        } catch (e: JwtException) {
            throw e
        } catch (e: IllegalArgumentException) {
            throw MalformedJwtException("Invalid JWT", e)
        }
    }
}
