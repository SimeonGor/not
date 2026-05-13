package com.example.pager.user

import com.example.pager.common.UserNotFoundException
import com.example.pager.device.DeviceRepository
import com.example.pager.message.MessageRepository
import com.example.pager.message.dto.MessageResponse
import com.example.pager.message.toMessageResponse
import com.example.pager.user.dto.UserDevicePublicDto
import com.example.pager.user.dto.UserProfileResponse
import org.springframework.data.domain.PageRequest
import org.springframework.stereotype.Service
import org.springframework.transaction.annotation.Transactional

@Service
class UserProfileService(
    private val userRepository: UserRepository,
    private val deviceRepository: DeviceRepository,
    private val messageRepository: MessageRepository,
    private val telegramUserService: TelegramUserService,
) {
    @Transactional(readOnly = true)
    fun getProfileByUsername(rawUsername: String): UserProfileResponse {
        val username =
            telegramUserService.normalizeUsername(rawUsername)
                ?: throw UserNotFoundException("User not found")
        val user = userRepository.findByUsername(username).orElseThrow { UserNotFoundException("User not found: $username") }
        val devices = deviceRepository.findAllByUser_Id(user.id)
        return UserProfileResponse(
            telegramId = user.telegramId,
            username = username,
            firstName = user.firstName,
            lastName = user.lastName,
            devices =
                devices.map { d ->
                    UserDevicePublicDto(deviceId = d.deviceId, boundAt = d.boundAt)
                },
        )
    }

    @Transactional(readOnly = true)
    fun listMessagesByUsername(
        rawUsername: String,
        limit: Int,
    ): List<MessageResponse> {
        val username =
            telegramUserService.normalizeUsername(rawUsername)
                ?: throw UserNotFoundException("User not found")
        val user = userRepository.findByUsername(username).orElseThrow { UserNotFoundException("User not found: $username") }
        val devices = deviceRepository.findAllByUser_Id(user.id)
        if (devices.isEmpty()) {
            return emptyList()
        }
        val page = PageRequest.of(0, limit.coerceIn(1, 100))
        return messageRepository.findByDeviceInOrderByCreatedAtDesc(devices, page).map { entity ->
            entity.toMessageResponse(entity.device.deviceId)
        }
    }
}
