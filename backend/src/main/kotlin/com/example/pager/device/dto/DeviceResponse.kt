package com.example.pager.device.dto

import com.fasterxml.jackson.annotation.JsonProperty
import java.time.Instant
import java.util.UUID

data class DeviceResponse(
    val id: UUID,
    @JsonProperty("deviceId")
    val deviceId: String,
    @JsonProperty("displayName")
    val displayName: String?,
    @JsonProperty("createdAt")
    val createdAt: Instant,
    @JsonProperty("lastSeenAt")
    val lastSeenAt: Instant?,
)
