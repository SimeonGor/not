package com.example.pager.message.dto

import com.fasterxml.jackson.annotation.JsonProperty
import jakarta.validation.constraints.NotBlank
import jakarta.validation.constraints.Pattern
import jakarta.validation.constraints.Size

data class SendMessageRequest(
    @field:NotBlank(message = "deviceId must not be blank")
    @field:Pattern(
        regexp = "^[A-Za-z0-9]{8,32}$",
        message = "deviceId must be 8-32 alphanumeric characters",
    )
    @JsonProperty("deviceId")
    val deviceId: String,
    @field:NotBlank(message = "senderName must not be blank")
    @field:Size(min = 1, max = 64, message = "senderName must be 1-64 characters")
    @JsonProperty("senderName")
    val senderName: String,
    @field:NotBlank(message = "text must not be blank")
    @field:Size(min = 1, max = 220, message = "text must be 1-220 characters")
    @JsonProperty("text")
    val text: String,
)
