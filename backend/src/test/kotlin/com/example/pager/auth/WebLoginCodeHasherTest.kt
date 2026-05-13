package com.example.pager.auth

import com.example.pager.config.WebAuthProperties
import org.junit.jupiter.api.Assertions.assertEquals
import org.junit.jupiter.api.Assertions.assertNotEquals
import org.junit.jupiter.api.Test

class WebLoginCodeHasherTest {
    private val props = WebAuthProperties(codeSalt = "fixed-salt-for-test")
    private val hasher = WebLoginCodeHasher(props)

    @Test
    fun `hash is deterministic`() {
        assertEquals(hasher.hash("123456"), hasher.hash("123456"))
    }

    @Test
    fun `different codes differ`() {
        assertNotEquals(hasher.hash("111111"), hasher.hash("222222"))
    }
}
