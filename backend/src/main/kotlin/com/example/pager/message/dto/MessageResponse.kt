package com.example.pager.message.dto

import com.fasterxml.jackson.annotation.JsonProperty
import java.time.Instant
import java.util.UUID

data class MessageResponse(
    val id: UUID,
    @JsonProperty("deviceId")
    val deviceId: String,
    @JsonProperty("senderName")
    val senderName: String,
    val text: String,
    @JsonProperty("mqttTopic")
    val mqttTopic: String,
    @JsonProperty("deliveredToMqtt")
    val deliveredToMqtt: Boolean,
    @JsonProperty("createdAt")
    val createdAt: Instant,
)
