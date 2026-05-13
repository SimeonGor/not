package com.example.pager.pairing

import com.example.pager.config.PairingProperties
import org.springframework.stereotype.Component
import java.security.MessageDigest

@Component
class PinHasher(
    private val pairingProperties: PairingProperties,
) {
    fun hash(pin: String): String {
        val digest = MessageDigest.getInstance("SHA-256")
        val input = pin + pairingProperties.pinSalt
        val bytes = digest.digest(input.toByteArray(Charsets.UTF_8))
        return bytes.joinToString("") { b -> "%02x".format(b) }
    }
}
