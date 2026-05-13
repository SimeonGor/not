package com.example.pager.telegram

import com.example.pager.config.TelegramBotProperties
import org.slf4j.LoggerFactory
import org.springframework.boot.ApplicationArguments
import org.springframework.boot.ApplicationRunner
import org.springframework.core.annotation.Order
import org.springframework.stereotype.Component

/**
 * Logs once at startup when Telegram long polling is not enabled (no bot token).
 */
@Component
@Order(0)
class TelegramDisabledWarningRunner(
    private val telegramBotProperties: TelegramBotProperties,
) : ApplicationRunner {
    private val log = LoggerFactory.getLogger(javaClass)

    override fun run(args: ApplicationArguments) {
        val token = telegramBotProperties.botToken.trim()
        if (token.isEmpty()) {
            log.warn("[TELEGRAM] Bot disabled: set telegram.bot-token (e.g. env TG_BOT_TOKEN) to enable /bind and /unbind")
        }
    }
}
