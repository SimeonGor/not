package com.example.pager.auth.dto

import com.fasterxml.jackson.annotation.JsonProperty
import jakarta.validation.constraints.NotBlank
import jakarta.validation.constraints.Pattern

data class RequestCodeRequest(
    @field:NotBlank
    val username: String,
)

data class RequestCodeResponse(
    val message: String,
    @JsonProperty("ttlSeconds")
    val ttlSeconds: Long,
)

data class VerifyCodeRequest(
    @field:NotBlank
    val username: String,
    @field:NotBlank
    @field:Pattern(regexp = "^[0-9]{6}$", message = "code must be 6 digits")
    val code: String,
)

data class AuthTokenResponse(
    @JsonProperty("accessToken")
    val accessToken: String,
    @JsonProperty("tokenType")
    val tokenType: String = "Bearer",
    @JsonProperty("expiresIn")
    val expiresIn: Long,
    val username: String,
)
