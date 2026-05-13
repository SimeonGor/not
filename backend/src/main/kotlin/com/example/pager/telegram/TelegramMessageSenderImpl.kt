package com.example.pager.telegram

import com.pengrad.telegrambot.TelegramBot
import com.pengrad.telegrambot.request.SendMessage
import org.slf4j.LoggerFactory
import org.springframework.context.annotation.Conditional
import org.springframework.stereotype.Component

@Component
@Conditional(TelegramEnabledCondition::class)
class TelegramMessageSenderImpl(
    private val telegramBot: TelegramBot,
) : TelegramMessageSender {
    private val log = LoggerFactory.getLogger(javaClass)

    override fun sendTelegramMessage(telegramId: Long, text: String) {
        log.info("[TELEGRAM] Outbound DM to telegramId={}", telegramId)
        telegramBot.execute(SendMessage(telegramId, text))
    }
}
