package com.example.pager.pairing

import org.springframework.data.jpa.repository.JpaRepository
import org.springframework.data.jpa.repository.Modifying
import org.springframework.data.jpa.repository.Query
import org.springframework.data.repository.query.Param
import java.time.Instant
import java.util.Optional
import java.util.UUID

interface PairingCodeRepository : JpaRepository<PairingCodeEntity, UUID> {
    @Query(
        """
        SELECT p FROM PairingCodeEntity p
        JOIN FETCH p.device d
        WHERE p.pinHash = :hash AND p.usedAt IS NULL AND p.expiresAt > :now
        """,
    )
    fun findActiveByPinHash(
        @Param("hash") hash: String,
        @Param("now") now: Instant,
    ): List<PairingCodeEntity>

    @Modifying(clearAutomatically = true, flushAutomatically = true)
    @Query(
        """
        UPDATE PairingCodeEntity p SET p.usedAt = :now
        WHERE p.device.id = :deviceId AND p.usedAt IS NULL AND p.expiresAt > :now
        """,
    )
    fun markActiveCodesUsedForDevice(
        @Param("deviceId") deviceId: UUID,
        @Param("now") now: Instant,
    ): Int
}
