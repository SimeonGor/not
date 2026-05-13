package com.example.pager.me.dto

import com.fasterxml.jackson.annotation.JsonProperty
import jakarta.validation.constraints.NotBlank
import jakarta.validation.constraints.Size
import java.util.UUID

data class MeSendRequest(
    @JsonProperty("targetUsername")
    @field:NotBlank
    val targetUsername: String,
    @field:NotBlank
    @field:Size(max = 220)
    val text: String,
)

data class MeSendResponse(
    val status: String = "sent",
    @JsonProperty("targetUsername")
    val targetUsername: String,
    @JsonProperty("messageId")
    val messageId: UUID,
)
