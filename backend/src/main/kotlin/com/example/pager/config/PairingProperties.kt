package com.example.pager.config

import org.springframework.boot.context.properties.ConfigurationProperties

@ConfigurationProperties(prefix = "pairing")
data class PairingProperties(
    var pinTtlSeconds: Long = 300,
    var pinLength: Int = 6,
    var pinSalt: String = "dev-salt-change-me",
)
