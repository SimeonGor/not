package com.example.pager.message

import org.junit.jupiter.api.Assertions.assertEquals
import org.junit.jupiter.api.Assertions.assertFalse
import org.junit.jupiter.api.Assertions.assertTrue
import org.junit.jupiter.api.Test

class MessagePayloadFormatterTest {
    @Test
    fun format() {
        assertEquals("Alice: Hello", MessagePayloadFormatter.format("Alice", "Hello"))
    }

    @Test
    fun `exceedsLimit when combined payload over 256`() {
        val sender = "A".repeat(64)
        val text = "B".repeat(220)
        assertTrue(MessagePayloadFormatter.exceedsLimit(sender, text))
    }

    @Test
    fun `within limit for typical message`() {
        assertFalse(MessagePayloadFormatter.exceedsLimit("Alice", "Hello from Kotlin backend!"))
    }
}
