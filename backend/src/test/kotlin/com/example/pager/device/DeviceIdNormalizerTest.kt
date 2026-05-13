package com.example.pager.device

import org.junit.jupiter.api.Assertions.assertEquals
import org.junit.jupiter.api.Assertions.assertFalse
import org.junit.jupiter.api.Assertions.assertTrue
import org.junit.jupiter.api.Test

class DeviceIdNormalizerTest {
    @Test
    fun `normalize converts to uppercase and trims`() {
        assertEquals("84F3EB12ABCD", DeviceIdNormalizer.normalize("  84f3eb12abcd  "))
    }

    @Test
    fun `isValid accepts alphanumeric 8 to 32`() {
        assertTrue(DeviceIdNormalizer.isValid("84F3EB12ABCD"))
        assertFalse(DeviceIdNormalizer.isValid("short"))
        assertFalse(DeviceIdNormalizer.isValid("84F3-EB12ABCD"))
    }
}
