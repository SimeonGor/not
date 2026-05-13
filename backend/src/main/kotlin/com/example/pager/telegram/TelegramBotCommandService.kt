package com.example.pager.telegram

import com.example.pager.common.PayloadTooLongException
import com.example.pager.device.DeviceBindingService
import com.example.pager.device.DeviceIdNormalizer
import com.example.pager.message.MessagePayloadFormatter
import com.example.pager.message.MessageRepository
import com.example.pager.message.MessageService
import com.example.pager.message.dto.SendMessageRequest
import com.example.pager.mqtt.MqttPublishException
import com.example.pager.user.TelegramUserService
import com.example.pager.user.UserEntity
import com.example.pager.user.UserRepository
import com.example.pager.device.DeviceRepository
import com.pengrad.telegrambot.model.User
import org.springframework.data.domain.PageRequest
import org.springframework.stereotype.Service
import org.springframework.transaction.annotation.Transactional
import java.time.ZoneId
import java.time.format.DateTimeFormatter

@Service
class TelegramBotCommandService(
    private val telegramUserService: TelegramUserService,
    private val userRepository: UserRepository,
    private val deviceRepository: DeviceRepository,
    private val messageRepository: MessageRepository,
    private val messageService: MessageService,
    private val deviceBindingService: DeviceBindingService,
) {
    private val timeFmt = DateTimeFormatter.ofPattern("HH:mm").withZone(ZoneId.systemDefault())
    private val sendRegex = Regex("^/send\\s+(\\S+)\\s+(.+)$", setOf(RegexOption.DOT_MATCHES_ALL, RegexOption.IGNORE_CASE))
    private val sendDeviceRegex = Regex("^/send_device\\s+(\\S+)\\s+(.+)$", setOf(RegexOption.DOT_MATCHES_ALL, RegexOption.IGNORE_CASE))

    @Transactional(readOnly = true)
    fun buildStartMessage(user: UserEntity): String {
        val lines = mutableListOf<String>()
        lines += "Привет! Это Smart Retro Pager."
        lines += ""
        if (user.username != null && user.username!!.isNotBlank()) {
            lines += "Ваш username: @${user.username}"
            lines += ""
            lines += "Вы можете:"
            lines += "— отправлять сообщения: /send username текст"
            lines += "— напрямую на устройство: /send_device deviceId текст"
            lines += "— привязать пейджер: /bind PIN"
            lines += "— профиль: /me, устройства: /devices, история: /history"
            lines += "— справка: /help"
        } else {
            lines += "У вас не задан Telegram username."
            lines += "Вы можете отправлять сообщения другим пользователям,"
            lines += "но другие пользователи не смогут найти вас по /send username,"
            lines += "пока вы не зададите username в настройках Telegram."
            lines += ""
            lines += "Команды:"
            lines += "/send username текст"
            lines += "/send_device deviceId текст"
            lines += "/bind PIN"
            lines += "/me, /devices, /history, /help"
        }
        return lines.joinToString("\n")
    }

    fun buildHelpMessage(): String =
        """
        Команды Smart Retro Pager:
        /start — приветствие
        /send username текст — сообщение владельцу пейджера (ваш пейджер не нужен)
        /send_device deviceId текст — отправка напрямую на устройство (режим отладки)
        /bind PIN — привязать пейджер
        /unbind — отвязать пейджер
        /me — профиль и пейджеры
        /devices — список привязанных устройств
        /history [N] — последние сообщения на ваш пейджер (N по умолчанию 5, макс. 20)
        /help — эта справка
        """.trimIndent()

    @Transactional(readOnly = true)
    fun buildMeMessage(user: UserEntity): String {
        val devices = deviceRepository.findAllByUser_Id(user.id)
        val sb = StringBuilder()
        sb.appendLine("Ваш профиль:")
        sb.appendLine("Telegram ID: ${user.telegramId}")
        sb.appendLine(
            "Username: " + if (user.username.isNullOrBlank()) "не задан" else "@${user.username}",
        )
        sb.appendLine("Имя: ${user.firstName ?: "—"}")
        if (!user.lastName.isNullOrBlank()) {
            sb.appendLine("Фамилия: ${user.lastName}")
        }
        sb.appendLine("")
        if (devices.isEmpty()) {
            sb.appendLine("Пейджер не привязан, но вы всё равно можете отправлять сообщения другим пользователям через /send username текст")
        } else {
            sb.appendLine("Привязанные пейджеры:")
            devices.forEachIndexed { i, d ->
                sb.appendLine("${i + 1}. ${d.deviceId}")
            }
        }
        return sb.toString().trimEnd()
    }

    @Transactional
    fun handleSend(sender: UserEntity, commandLine: String): String {
        val m = sendRegex.matchEntire(commandLine.trim()) ?: return "Использование: /send username текст"
        val rawTarget = m.groupValues[1]
        val text = m.groupValues[2].trim()
        if (text.isEmpty()) return "Использование: /send username текст"

        val targetUsername = telegramUserService.normalizeUsername(rawTarget)
            ?: return "Использование: /send username текст"

        val receiver = userRepository.findByUsername(targetUsername).orElse(null)
            ?: return "Пользователь $targetUsername не найден."

        val device =
            deviceRepository.findFirstByUser_IdOrderByBoundAtDesc(receiver.id).orElse(null)
                ?: return "Пользователь $targetUsername найден, но у него нет привязанного пейджера."

        val senderName = telegramUserService.displayNameForSend(sender)
        return try {
            messageService.sendMessage(
                SendMessageRequest(
                    deviceId = device.deviceId,
                    senderName = senderName,
                    text = text,
                ),
            )
            "Сообщение отправлено пользователю $targetUsername."
        } catch (e: PayloadTooLongException) {
            "Сообщение слишком длинное. Сократите текст или имя отправителя (макс. ${MessagePayloadFormatter.MAX_MQTT_PAYLOAD_CHARS} символов в строке «имя: текст»)."
        } catch (e: MqttPublishException) {
            "Не удалось доставить сообщение по MQTT. Попробуйте позже."
        }
    }

    @Transactional
    fun handleSendDevice(sender: UserEntity, commandLine: String): String {
        val m = sendDeviceRegex.matchEntire(commandLine.trim()) ?: return "Использование: /send_device deviceId текст"
        val rawDevice = m.groupValues[1]
        val text = m.groupValues[2].trim()
        if (text.isEmpty()) return "Использование: /send_device deviceId текст"

        val deviceId = DeviceIdNormalizer.normalize(rawDevice)
        if (!DeviceIdNormalizer.isValid(deviceId)) {
            return "Неверный формат deviceId."
        }

        val senderName = telegramUserService.displayNameForSend(sender)
        return try {
            messageService.sendMessage(
                SendMessageRequest(
                    deviceId = deviceId,
                    senderName = senderName,
                    text = text,
                ),
            )
            "Сообщение отправлено напрямую на устройство $deviceId."
        } catch (e: PayloadTooLongException) {
            "Сообщение слишком длинное. Сократите текст (лимит API и MQTT)."
        } catch (e: MqttPublishException) {
            "Не удалось доставить сообщение по MQTT. Попробуйте позже."
        }
    }

    @Transactional(readOnly = true)
    fun handleHistory(user: UserEntity, commandLine: String): String {
        val parts = commandLine.trim().split(Regex("\\s+"))
        val limit =
            when (parts.size) {
                1 -> DEFAULT_HISTORY_LIMIT
                2 ->
                    parts[1].toIntOrNull()?.coerceIn(1, MAX_HISTORY_LIMIT)
                        ?: return "Использование: /history [количество]"
                else -> return "Использование: /history [количество]"
            }

        val devices = deviceRepository.findAllByUser_Id(user.id)
        if (devices.isEmpty()) {
            return "У вас нет привязанного пейджера.\nНо вы можете отправлять сообщения другим пользователям через /send username текст."
        }

        val page = PageRequest.of(0, limit)
        val msgs = messageRepository.findByDeviceInOrderByCreatedAtDesc(devices, page)
        if (msgs.isEmpty()) {
            return "История сообщений пуста."
        }
        val lines = mutableListOf<String>()
        lines += "Последние сообщения:"
        msgs.forEachIndexed { i, m ->
            val t = timeFmt.format(m.createdAt)
            lines += "${i + 1}. [$t] ${m.senderName}: ${m.textContent}"
        }
        return lines.joinToString("\n")
    }

    @Transactional(readOnly = true)
    fun handleDevices(user: UserEntity): String {
        val devices = deviceRepository.findAllByUser_Id(user.id)
        if (devices.isEmpty()) {
            return "У вас нет привязанных устройств. Чтобы привязать пейджер, включите его и отправьте /bind PIN."
        }
        val sb = StringBuilder()
        sb.appendLine("Ваши устройства:")
        devices.forEachIndexed { i, d ->
            sb.appendLine("${i + 1}. ${d.deviceId}")
            sb.appendLine("   привязан: ${d.boundAt ?: "—"}")
        }
        return sb.toString().trimEnd()
    }

    fun handleBind(from: User, text: String): String {
        val parts = text.split(Regex("\\s+"), limit = 3)
        if (parts.size < 2) {
            return "Использование: /bind 123456"
        }
        val pin = parts[1].trim()
        return deviceBindingService.bindByPin(
            from.id(),
            from.username(),
            from.firstName(),
            from.lastName(),
            pin,
        )
    }

    companion object {
        private const val DEFAULT_HISTORY_LIMIT = 5
        private const val MAX_HISTORY_LIMIT = 20
    }
}
