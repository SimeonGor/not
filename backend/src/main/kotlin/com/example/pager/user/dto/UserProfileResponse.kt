package com.example.pager.user.dto

import com.fasterxml.jackson.annotation.JsonProperty
import java.time.Instant

data class UserDevicePublicDto(
    @JsonProperty("deviceId")
    val deviceId: String,
    @JsonProperty("boundAt")
    val boundAt: Instant?,
)

data class UserProfileResponse(
    @JsonProperty("telegramId")
    val telegramId: Long?,
    val username: String,
    @JsonProperty("firstName")
    val firstName: String?,
    @JsonProperty("lastName")
    val lastName: String?,
    val devices: List<UserDevicePublicDto>,
)
