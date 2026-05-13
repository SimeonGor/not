package com.example.pager.device

import com.example.pager.device.dto.BindingStatusResponse
import com.example.pager.device.dto.PairingPinResponse
import com.example.pager.device.dto.ResetBindingResponse
import org.springframework.web.bind.annotation.GetMapping
import org.springframework.web.bind.annotation.PathVariable
import org.springframework.web.bind.annotation.PostMapping
import org.springframework.web.bind.annotation.RequestBody
import org.springframework.web.bind.annotation.RequestMapping
import org.springframework.web.bind.annotation.RestController

@RestController
@RequestMapping("/api/v1/devices")
class DeviceBindingController(
    private val deviceBindingService: DeviceBindingService,
) {
    @GetMapping("/{deviceId}/binding-status")
    fun getBindingStatus(
        @PathVariable deviceId: String,
    ): BindingStatusResponse = deviceBindingService.getBindingStatus(deviceId)

    @PostMapping("/{deviceId}/pairing-pin")
    fun requestPairingPin(
        @PathVariable deviceId: String,
        @RequestBody(required = false) body: Map<String, Any>?,
    ): PairingPinResponse = deviceBindingService.requestPairingPin(deviceId)

    @PostMapping("/{deviceId}/reset-binding")
    fun resetBinding(
        @PathVariable deviceId: String,
    ): ResetBindingResponse = deviceBindingService.resetBindingByDeviceId(deviceId)
}
