package com.example.pager.pairing

import com.example.pager.config.PairingProperties
import org.junit.jupiter.api.Assertions.assertEquals
import org.junit.jupiter.api.Assertions.assertNotEquals
import org.junit.jupiter.api.Test

class PinHasherTest {
    private val props = PairingProperties(pinSalt = "test-salt-fixed")
    private val hasher = PinHasher(props)

    @Test
    fun `hash is deterministic for same pin and salt`() {
        val a = hasher.hash("123456")
        val b = hasher.hash("123456")
        assertEquals(a, b)
        assertEquals(64, a.length)
    }

    @Test
    fun `different pins produce different hashes`() {
        assertNotEquals(hasher.hash("111111"), hasher.hash("222222"))
    }
}
