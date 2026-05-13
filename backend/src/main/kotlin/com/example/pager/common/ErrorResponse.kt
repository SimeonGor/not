package com.example.pager.common

import java.time.Instant

data class ErrorResponse(
    val error: String,
    val message: String,
    val timestamp: Instant = Instant.now(),
)
