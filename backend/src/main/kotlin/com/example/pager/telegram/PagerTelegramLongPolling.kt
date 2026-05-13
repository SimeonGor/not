package com.example.pager.telegram

import com.example.pager.config.TelegramBotProperties
import com.example.pager.device.DeviceBindingService
import com.example.pager.user.TelegramUserService
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
    private val telegramUserService: TelegramUserService,
    private val telegramBotCommandService: TelegramBotCommandService,
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

    private fun stripCommandAtSuffix(text: String): String =
        text.trim().replaceFirst(Regex("^(/[a-zA-Z0-9_]+)@[A-Za-z0-9_]+"), "$1")

    private fun handleUpdateSafe(update: Update) {
        val message = update.message() ?: return
        val chatId = message.chat().id()
        val from = message.from() ?: return
        val textRaw = message.text()?.trim() ?: return
        val text = stripCommandAtSuffix(textRaw)

        val reply =
            try {
                val user = telegramUserService.findOrCreateOrUpdateTelegramUser(from)
                routeCommand(from, user, text)
            } catch (e: Exception) {
                log.warn("[TELEGRAM] Command failed: {}", e.message, e)
                e.message ?: "Произошла ошибка. Попробуйте позже."
            }

        bot.execute(SendMessage(chatId, reply))
    }

    private fun routeCommand(
        from: com.pengrad.telegrambot.model.User,
        user: com.example.pager.user.UserEntity,
        text: String,
    ): String {
        val head = text.split(Regex("\\s+")).firstOrNull()?.lowercase() ?: return "Введите /help"
        return when {
            head == "/start" -> telegramBotCommandService.buildStartMessage(user)
            head == "/help" -> telegramBotCommandService.buildHelpMessage()
            head == "/me" -> telegramBotCommandService.buildMeMessage(user)
            head.startsWith("/send_device") -> telegramBotCommandService.handleSendDevice(user, text)
            head.startsWith("/send") -> telegramBotCommandService.handleSend(user, text)
            head.startsWith("/history") -> telegramBotCommandService.handleHistory(user, text)
            head == "/devices" -> telegramBotCommandService.handleDevices(user)
            head.startsWith("/bind") -> telegramBotCommandService.handleBind(from, text)
            head == "/unbind" -> deviceBindingService.unbindByTelegramUser(from.id())
            else -> "Неизвестная команда. Введите /help."
        }
    }
}
