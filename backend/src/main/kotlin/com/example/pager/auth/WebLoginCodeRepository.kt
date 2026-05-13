package com.example.pager.auth

import org.springframework.data.jpa.repository.JpaRepository
import org.springframework.data.jpa.repository.Modifying
import org.springframework.data.jpa.repository.Query
import org.springframework.data.repository.query.Param
import java.time.Instant
import java.util.UUID

interface WebLoginCodeRepository : JpaRepository<WebLoginCodeEntity, UUID> {
    @Query(
        """
        SELECT w FROM WebLoginCodeEntity w
        JOIN FETCH w.user u
        WHERE w.codeHash = :hash AND w.usedAt IS NULL AND w.expiresAt > :now
        """,
    )
    fun findActiveByCodeHash(
        @Param("hash") hash: String,
        @Param("now") now: Instant,
    ): List<WebLoginCodeEntity>

    @Modifying(clearAutomatically = true, flushAutomatically = true)
    @Query(
        """
        UPDATE WebLoginCodeEntity w SET w.usedAt = :now
        WHERE w.user.id = :userId AND w.usedAt IS NULL AND w.expiresAt > :now
        """,
    )
    fun markActiveCodesUsedForUser(
        @Param("userId") userId: UUID,
        @Param("now") now: Instant,
    ): Int
}
