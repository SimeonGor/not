package com.example.pager.telegram

import com.example.pager.config.TelegramBotProperties
import com.pengrad.telegrambot.TelegramBot
import org.springframework.context.annotation.Bean
import org.springframework.context.annotation.Configuration
import org.springframework.context.annotation.Conditional

@Configuration
@Conditional(TelegramEnabledCondition::class)
class TelegramClientConfiguration(
    private val telegramBotProperties: TelegramBotProperties,
) {
    @Bean
    fun telegramBot(): TelegramBot = TelegramBot(telegramBotProperties.botToken.trim())
}
