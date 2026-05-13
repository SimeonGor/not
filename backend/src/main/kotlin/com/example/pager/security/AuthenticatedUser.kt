package com.example.pager.security

import java.util.UUID

/**
 * Principal stored in [org.springframework.security.core.context.SecurityContext] after JWT validation.
 */
data class AuthenticatedUser(
    val userId: UUID,
    val username: String?,
    val telegramId: Long?,
)
