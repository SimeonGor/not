package com.example.pager.device

import com.example.pager.common.AlreadyBoundException
import com.example.pager.common.DeviceBoundToAnotherUserException
import com.example.pager.common.InvalidDeviceIdException
import com.example.pager.common.InvalidOrExpiredPairingPinException
import com.example.pager.common.InvalidPairingPinFormatException
import com.example.pager.config.PairingProperties
import com.example.pager.device.dto.BindingStatusResponse
import com.example.pager.device.dto.PairingPinResponse
import com.example.pager.device.dto.ResetBindingResponse
import com.example.pager.mqtt.MqttPublisherService
import com.example.pager.pairing.PairingCodeEntity
import com.example.pager.pairing.PairingCodeRepository
import com.example.pager.pairing.PinHasher
import com.example.pager.user.TelegramUserService
import com.example.pager.user.UserRepository
import org.slf4j.LoggerFactory
import org.springframework.stereotype.Service
import org.springframework.transaction.annotation.Transactional
import java.security.SecureRandom
import java.time.Clock
import java.time.Instant
import java.time.temporal.ChronoUnit
import java.util.UUID

@Service
class DeviceBindingService(
    private val deviceRepository: DeviceRepository,
    private val userRepository: UserRepository,
    private val telegramUserService: TelegramUserService,
    private val pairingCodeRepository: PairingCodeRepository,
    private val pairingProperties: PairingProperties,
    private val pinHasher: PinHasher,
    private val mqttPublisherService: MqttPublisherService,
    private val clock: Clock,
) {
    private val log = LoggerFactory.getLogger(javaClass)
    private val secureRandom = SecureRandom()

    @Transactional
    fun getBindingStatus(rawDeviceId: String): BindingStatusResponse {
        val deviceId = normalizeDeviceIdOrThrow(rawDeviceId)
        val device = findOrCreateDevice(deviceId)
        val bound = device.user != null
        val ownerUsername = device.user?.username?.lowercase()
        return BindingStatusResponse(
            deviceId = deviceId,
            bound = bound,
            ownerUsername = ownerUsername,
            pairingRequired = !bound,
        )
    }

    @Transactional
    fun requestPairingPin(rawDeviceId: String): PairingPinResponse {
        val deviceId = normalizeDeviceIdOrThrow(rawDeviceId)
        val device = findOrCreateDevice(deviceId)
        if (device.user != null) {
            throw AlreadyBoundException("Device is already bound to a Telegram user")
        }
        val now = clock.instant()
        val invalidated = pairingCodeRepository.markActiveCodesUsedForDevice(device.id, now)
        if (invalidated > 0) {
            log.info("[PAIRING] Invalidated {} active PIN(s) for deviceId={}", invalidated, deviceId)
        }

        val pin = generateNumericPin()
        // TODO(security): never log plaintext PIN in production
        log.info("[PAIRING] Created new PIN for deviceId={}", deviceId)

        val pinHash = pinHasher.hash(pin)
        val expiresAt = now.plus(pairingProperties.pinTtlSeconds, ChronoUnit.SECONDS)
        val entity =
            PairingCodeEntity(
                id = UUID.randomUUID(),
                device = device,
                pinHash = pinHash,
                expiresAt = expiresAt,
                usedAt = null,
                createdAt = now,
            )
        pairingCodeRepository.save(entity)

        return PairingPinResponse(
            deviceId = deviceId,
            pin = pin,
            ttlSeconds = pairingProperties.pinTtlSeconds,
            expiresAt = expiresAt,
        )
    }

    @Transactional
    fun bindByPin(
        telegramId: Long,
        username: String?,
        firstName: String?,
        lastName: String?,
        pin: String,
    ): String {
        validatePinFormat(pin)
        val now = clock.instant()
        val hash = pinHasher.hash(pin)
        val candidates = pairingCodeRepository.findActiveByPinHash(hash, now)
        if (candidates.isEmpty()) {
            throw InvalidOrExpiredPairingPinException(
                "PIN недействителен или истёк. Запросите новый PIN на устройстве.",
            )
        }
        val code = candidates.first()
        val device = code.device

        if (device.user != null) {
            val ownTid = device.user!!.telegramId
            if (ownTid != null && ownTid != telegramId) {
                throw DeviceBoundToAnotherUserException("Device is already bound to another Telegram user")
            }
            if (ownTid == telegramId) {
                code.usedAt = now
                pairingCodeRepository.saveAndFlush(code)
                pairingCodeRepository.markActiveCodesUsedForDevice(device.id, now)
                log.info(
                    "[PAIRING] Idempotent bind: deviceId={} already bound to telegramId={}",
                    device.deviceId,
                    telegramId,
                )
                return "Устройство ${device.deviceId} уже привязано к вашему аккаунту."
            }
        }

        val user =
            telegramUserService.findOrCreateOrUpdateTelegramUser(
                telegramId = telegramId,
                rawUsername = username,
                firstName = firstName,
                lastName = lastName,
            )

        code.usedAt = now
        pairingCodeRepository.saveAndFlush(code)
        pairingCodeRepository.markActiveCodesUsedForDevice(device.id, now)

        device.user = user
        device.boundAt = now
        deviceRepository.save(device)

        mqttPublisherService.publishSysCommand(device.deviceId, "BIND_SUCCESS")
        log.info("[PAIRING] Bound deviceId={} to telegramId={}", device.deviceId, telegramId)

        return "Устройство ${device.deviceId} успешно привязано к вашему аккаунту."
    }

    @Transactional
    fun unbindByTelegramUser(telegramId: Long): String {
        val user = userRepository.findByTelegramId(telegramId).orElse(null)
            ?: return "Устройство не было привязано."

        val devices = deviceRepository.findAllByUser(user)
        if (devices.isEmpty()) {
            return "Устройство не было привязано."
        }

        val now = clock.instant()
        for (device in devices) {
            pairingCodeRepository.markActiveCodesUsedForDevice(device.id, now)
            device.user = null
            device.boundAt = null
            deviceRepository.save(device)
            mqttPublisherService.publishSysCommand(device.deviceId, "UNBOUND")
            log.info("[PAIRING] Unbound deviceId={} from telegramId={}", device.deviceId, telegramId)
        }

        return "Устройство отвязано. На экране появится новый PIN для повторной привязки."
    }

    @Transactional
    fun resetBindingByDeviceId(rawDeviceId: String): ResetBindingResponse {
        val deviceId = normalizeDeviceIdOrThrow(rawDeviceId)
        val device = findOrCreateDevice(deviceId)
        val now = clock.instant()
        pairingCodeRepository.markActiveCodesUsedForDevice(device.id, now)
        val wasBound = device.user != null
        device.user = null
        device.boundAt = null
        deviceRepository.save(device)
        mqttPublisherService.publishSysCommand(device.deviceId, "UNBOUND")
        log.info("[PAIRING] Reset binding for deviceId={} (wasBound={})", deviceId, wasBound)

        return ResetBindingResponse(
            deviceId = deviceId,
            bound = false,
            message = "Binding reset",
        )
    }

    private fun normalizeDeviceIdOrThrow(raw: String): String {
        val normalized = DeviceIdNormalizer.normalize(raw)
        if (!DeviceIdNormalizer.isValid(normalized)) {
            throw InvalidDeviceIdException("deviceId must be 8-32 alphanumeric characters")
        }
        return normalized
    }

    private fun findOrCreateDevice(deviceId: String): DeviceEntity {
        val existing = deviceRepository.findByDeviceId(deviceId)
        if (existing.isPresent) {
            return existing.get()
        }
        val created =
            DeviceEntity(
                id = UUID.randomUUID(),
                deviceId = deviceId,
                displayName = null,
                createdAt = clock.instant(),
                lastSeenAt = null,
                user = null,
                boundAt = null,
            )
        return deviceRepository.save(created)
    }

    private fun validatePinFormat(pin: String) {
        val n = pairingProperties.pinLength
        val regex = Regex("^[0-9]{$n}$")
        if (!regex.matches(pin)) {
            throw InvalidPairingPinFormatException("PIN must be exactly $n digits")
        }
    }

    private fun generateNumericPin(): String {
        val n = pairingProperties.pinLength
        require(n in 4..12) { "pin-length must be between 4 and 12" }
        val minInclusive = pow10Long(n - 1)
        val maxInclusive = pow10Long(n) - 1
        val pinLong = secureRandom.nextLong(minInclusive, maxInclusive + 1)
        return pinLong.toString().padStart(n, '0')
    }

    private fun pow10Long(exp: Int): Long {
        var r = 1L
        repeat(exp) { r *= 10L }
        return r
    }
}
