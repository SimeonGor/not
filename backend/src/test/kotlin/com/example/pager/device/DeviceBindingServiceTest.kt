package com.example.pager.device

import com.example.pager.common.AlreadyBoundException
import com.example.pager.common.InvalidOrExpiredPairingPinException
import com.example.pager.common.InvalidPairingPinFormatException
import com.example.pager.config.PairingProperties
import com.example.pager.mqtt.MqttPublisherService
import com.example.pager.pairing.PairingCodeEntity
import com.example.pager.pairing.PairingCodeRepository
import com.example.pager.pairing.PinHasher
import com.example.pager.user.TelegramUserService
import com.example.pager.user.UserEntity
import com.example.pager.user.UserRepository
import io.mockk.Runs
import io.mockk.clearMocks
import io.mockk.every
import io.mockk.just
import io.mockk.mockk
import io.mockk.verify
import org.junit.jupiter.api.Assertions.assertEquals
import org.junit.jupiter.api.Assertions.assertFalse
import org.junit.jupiter.api.Assertions.assertNull
import org.junit.jupiter.api.Assertions.assertThrows
import org.junit.jupiter.api.Assertions.assertTrue
import org.junit.jupiter.api.BeforeEach
import org.junit.jupiter.api.Test
import java.time.Clock
import java.time.Instant
import java.time.ZoneOffset
import java.util.Optional
import java.util.UUID

class DeviceBindingServiceTest {
    private val deviceRepository = mockk<DeviceRepository>()
    private val userRepository = mockk<UserRepository>()
    private val telegramUserService = mockk<TelegramUserService>()
    private val pairingCodeRepository = mockk<PairingCodeRepository>()
    private val pairingProperties =
        PairingProperties(
            pinTtlSeconds = 300,
            pinLength = 6,
            pinSalt = "unit-test-salt",
        )
    private val pinHasher = PinHasher(pairingProperties)
    private val mqttPublisherService = mockk<MqttPublisherService>()
    private val fixedInstant = Instant.parse("2026-05-13T12:00:00Z")
    private val clock = Clock.fixed(fixedInstant, ZoneOffset.UTC)

    private lateinit var service: DeviceBindingService

    private val deviceId = "84F3EB12ABCD"
    private val deviceUuid = UUID.fromString("00000000-0000-0000-0000-000000000001")

    private val unboundDevice =
        DeviceEntity(
            id = deviceUuid,
            deviceId = deviceId,
            displayName = null,
            createdAt = fixedInstant,
            lastSeenAt = null,
            user = null,
            boundAt = null,
        )

    @BeforeEach
    fun setup() {
        clearMocks(deviceRepository, userRepository, telegramUserService, pairingCodeRepository, mqttPublisherService)
        service =
            DeviceBindingService(
                deviceRepository,
                userRepository,
                telegramUserService,
                pairingCodeRepository,
                pairingProperties,
                pinHasher,
                mqttPublisherService,
                clock,
            )
        every { mqttPublisherService.publishSysCommand(any(), any()) } just Runs
    }

    @Test
    fun `requestPairingPin throws when device already bound`() {
        val user = UserEntity(UUID.randomUUID(), 99L, "alice", null, null, fixedInstant, fixedInstant)
        val bound = deviceWithBinding(user)
        every { deviceRepository.findByDeviceId(deviceId) } returns Optional.of(bound)

        assertThrows(AlreadyBoundException::class.java) {
            service.requestPairingPin(deviceId)
        }
    }

    @Test
    fun `bindByPin binds device publishes BIND_SUCCESS and marks code used`() {
        val pin = "654321"
        val hash = pinHasher.hash(pin)
        val code =
            PairingCodeEntity(
                id = UUID.randomUUID(),
                device = unboundDevice,
                pinHash = hash,
                expiresAt = fixedInstant.plusSeconds(60),
                usedAt = null,
                createdAt = fixedInstant,
            )

        every { pairingCodeRepository.findActiveByPinHash(hash, fixedInstant) } returns listOf(code)
        val createdUser =
            UserEntity(UUID.randomUUID(), 42L, "bob", "Bob", null, fixedInstant, fixedInstant)
        every {
            telegramUserService.findOrCreateOrUpdateTelegramUser(42L, "bob", "Bob", null)
        } returns createdUser
        every { pairingCodeRepository.saveAndFlush(any()) } answers { firstArg() }
        every { pairingCodeRepository.markActiveCodesUsedForDevice(deviceUuid, fixedInstant) } returns 0
        every { deviceRepository.save(any()) } answers { firstArg() }

        val msg = service.bindByPin(42L, "bob", "Bob", null, pin)

        assertTrue(msg.contains(deviceId))
        verify { mqttPublisherService.publishSysCommand(deviceId, "BIND_SUCCESS") }
        assertEquals(fixedInstant, code.usedAt)
        assertEquals(42L, unboundDevice.user?.telegramId)
        assertEquals(fixedInstant, unboundDevice.boundAt)
    }

    @Test
    fun `bindByPin rejects wrong pin length`() {
        assertThrows(InvalidPairingPinFormatException::class.java) {
            service.bindByPin(1L, null, null, null, "12")
        }
    }

    @Test
    fun `bindByPin rejects unknown pin`() {
        every { pairingCodeRepository.findActiveByPinHash(any(), fixedInstant) } returns emptyList()

        assertThrows(InvalidOrExpiredPairingPinException::class.java) {
            service.bindByPin(1L, null, null, null, "000000")
        }
    }

    @Test
    fun `resetBindingByDeviceId clears user and publishes UNBOUND`() {
        val user = UserEntity(UUID.randomUUID(), 7L, "u", null, null, fixedInstant, fixedInstant)
        val bound = deviceWithBinding(user)
        every { deviceRepository.findByDeviceId(deviceId) } returns Optional.of(bound)
        every { pairingCodeRepository.markActiveCodesUsedForDevice(deviceUuid, fixedInstant) } returns 1
        every { deviceRepository.save(any()) } answers { firstArg() }

        val res = service.resetBindingByDeviceId(deviceId)

        assertFalse(res.bound)
        assertNull(bound.user)
        assertNull(bound.boundAt)
        verify { mqttPublisherService.publishSysCommand(deviceId, "UNBOUND") }
    }

    @Test
    fun `getBindingStatus reflects bound state`() {
        val user = UserEntity(UUID.randomUUID(), 1L, "alice", null, null, fixedInstant, fixedInstant)
        val bound = deviceWithBinding(user)
        every { deviceRepository.findByDeviceId(deviceId) } returns Optional.of(bound)

        val status = service.getBindingStatus(deviceId)

        assertTrue(status.bound)
        assertEquals("alice", status.ownerUsername)
        assertFalse(status.pairingRequired)
    }

    private fun deviceWithBinding(user: UserEntity): DeviceEntity =
        DeviceEntity(
            id = deviceUuid,
            deviceId = deviceId,
            displayName = null,
            createdAt = fixedInstant,
            lastSeenAt = null,
            user = user,
            boundAt = fixedInstant,
        )
}
