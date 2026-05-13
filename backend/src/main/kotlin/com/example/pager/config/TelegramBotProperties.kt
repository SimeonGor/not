package com.example.pager.config

import org.springframework.boot.context.properties.ConfigurationProperties

@ConfigurationProperties(prefix = "telegram")
data class TelegramBotProperties(
    var botToken: String = "",
    var botUsername: String = "",
)
