package com.example.pager.config

import org.springframework.boot.context.properties.ConfigurationProperties

@ConfigurationProperties(prefix = "web-auth")
data class WebAuthProperties(
    var codeTtlSeconds: Long = 300,
    var codeLength: Int = 6,
    var codeSalt: String = "dev-web-auth-salt-change-me",
    var jwtSecret: String = "dev-jwt-secret-change-me-min-32-chars!!",
    var jwtTtlSeconds: Long = 3600,
)
