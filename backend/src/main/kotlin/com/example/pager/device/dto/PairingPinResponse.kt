package com.example.pager.device.dto

import com.fasterxml.jackson.annotation.JsonProperty
import java.time.Instant

data class PairingPinResponse(
    @JsonProperty("deviceId")
    val deviceId: String,
    val pin: String,
    @JsonProperty("ttlSeconds")
    val ttlSeconds: Long,
    @JsonProperty("expiresAt")
    val expiresAt: Instant,
)
