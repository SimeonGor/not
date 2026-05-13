package com.example.pager.me

import com.example.pager.common.ReceiverHasNoPagerException
import com.example.pager.common.UserNotFoundException
import com.example.pager.device.DeviceRepository
import com.example.pager.message.MessageService
import com.example.pager.message.dto.SendMessageRequest
import com.example.pager.me.dto.MeSendResponse
import com.example.pager.user.TelegramUserService
import com.example.pager.user.UserRepository
import org.springframework.stereotype.Service
import org.springframework.transaction.annotation.Transactional
import java.util.UUID

@Service
class MeSendService(
    private val userRepository: UserRepository,
    private val deviceRepository: DeviceRepository,
    private val messageService: MessageService,
    private val telegramUserService: TelegramUserService,
) {
    @Transactional
    fun send(
        senderUserId: UUID,
        targetUsernameRaw: String,
        text: String,
    ): MeSendResponse {
        val sender = userRepository.findById(senderUserId).orElseThrow { UserNotFoundException("User not found") }
        val targetUsername =
            telegramUserService.normalizeUsername(targetUsernameRaw)
                ?: throw UserNotFoundException("User not found")

        val receiver =
            userRepository.findByUsername(targetUsername).orElseThrow {
                UserNotFoundException("User not found: $targetUsername")
            }

        val device =
            deviceRepository.findFirstByUser_IdOrderByBoundAtDesc(receiver.id).orElseThrow {
                ReceiverHasNoPagerException(
                    "Пользователь $targetUsername найден, но у него нет привязанного пейджера.",
                )
            }

        val senderName = telegramUserService.displayNameForSend(sender)
        val response =
            messageService.sendMessage(
                SendMessageRequest(
                    deviceId = device.deviceId,
                    senderName = senderName,
                    text = text,
                ),
            )

        return MeSendResponse(
            status = "sent",
            targetUsername = targetUsername,
            messageId = response.id,
        )
    }
}
