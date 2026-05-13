package com.example.pager.device

object DeviceIdNormalizer {
    private val PATTERN = Regex("^[A-Za-z0-9]{8,32}$")

    fun normalize(raw: String): String = raw.trim().uppercase()

    fun isValid(deviceId: String): Boolean = PATTERN.matches(deviceId.trim())
}
