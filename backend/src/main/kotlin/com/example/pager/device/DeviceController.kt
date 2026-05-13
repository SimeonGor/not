package com.example.pager.device

import com.example.pager.device.dto.DeviceResponse
import com.example.pager.message.dto.MessageResponse
import org.springframework.web.bind.annotation.GetMapping
import org.springframework.web.bind.annotation.PathVariable
import org.springframework.web.bind.annotation.RequestMapping
import org.springframework.web.bind.annotation.RestController

@RestController
@RequestMapping("/api/v1/devices")
class DeviceController(
    private val deviceService: DeviceService,
) {
    @GetMapping
    fun listDevices(): List<DeviceResponse> = deviceService.listDevices()

    @GetMapping("/{deviceId}/messages")
    fun listMessages(
        @PathVariable deviceId: String,
    ): List<MessageResponse> = deviceService.listMessagesForDevice(deviceId)
}
