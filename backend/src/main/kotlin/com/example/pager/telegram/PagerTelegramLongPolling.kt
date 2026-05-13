package com.example.pager.telegram

import com.example.pager.config.TelegramBotProperties
import com.example.pager.device.DeviceBindingService
import com.pengrad.telegrambot.TelegramBot
import com.pengrad.telegrambot.UpdatesListener
import com.pengrad.telegrambot.model.Update
import com.pengrad.telegrambot.request.SendMessage
import jakarta.annotation.PostConstruct
import jakarta.annotation.PreDestroy
import org.slf4j.LoggerFactory
import org.springframework.context.annotation.Conditional
import org.springframework.stereotype.Component

@Component
@Conditional(TelegramEnabledCondition::class)
class PagerTelegramLongPolling(
    private val telegramBotProperties: TelegramBotProperties,
    private val deviceBindingService: DeviceBindingService,
) {
    private val log = LoggerFactory.getLogger(javaClass)
    private lateinit var bot: TelegramBot

    @PostConstruct
    fun start() {
        val token = telegramBotProperties.botToken.trim()
        log.info("[TELEGRAM] Starting long polling (botUsername={})", telegramBotProperties.botUsername)
        bot = TelegramBot(token)
        bot.setUpdatesListener { updates ->
            for (update in updates) {
                handleUpdateSafe(update)
            }
            UpdatesListener.CONFIRMED_UPDATES_ALL
        }
    }

    @PreDestroy
    fun stop() {
        if (::bot.isInitialized) {
            bot.removeGetUpdatesListener()
            log.info("[TELEGRAM] Long polling stopped")
        }
    }

    private fun handleUpdateSafe(update: Update) {
        val message = update.message() ?: return
        val chatId = message.chat().id()
        val from = message.from() ?: return
        val telegramId = from.id()
        val username = from.username()
        val firstName = from.firstName()
        val text = message.text()?.trim() ?: return

        val reply =
            try {
                when {
                    text.equals("/start", ignoreCase = true) || text.equals("/help", ignoreCase = true) ->
                        helpText()
                    text.startsWith("/bind", ignoreCase = true) ->
                        handleBind(text, telegramId, username, firstName)
                    text.equals("/unbind", ignoreCase = true) ->
                        deviceBindingService.unbindByTelegramUser(telegramId)
                    text.equals("/me", ignoreCase = true) ->
                        deviceBindingService.describeTelegramUser(telegramId)
                    else -> "Неизвестная команда. Введите /help."
                }
            } catch (e: Exception) {
                log.warn("[TELEGRAM] Command failed: {}", e.message, e)
                e.message ?: "Произошла ошибка. Попробуйте позже."
            }

        bot.execute(SendMessage(chatId, reply))
    }

    private fun handleBind(
        text: String,
        telegramId: Long,
        username: String?,
        firstName: String?,
    ): String {
        val parts = text.split(Regex("\\s+"), limit = 3)
        if (parts.size < 2) {
            return "Использование: /bind 123456"
        }
        val pin = parts[1].trim()
        return deviceBindingService.bindByPin(telegramId, username, firstName, pin)
    }

    private fun helpText(): String =
        """
        Команды:
        /bind PIN — привязать устройство по PIN с экрана пейджера
        /unbind — отвязать устройство
        /me — ваш Telegram ID и привязанное устройство
        /help — это сообщение
        """.trimIndent()
}
