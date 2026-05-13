package com.example.pager.auth

import com.example.pager.config.WebAuthProperties
import org.springframework.stereotype.Component
import java.security.MessageDigest

@Component
class WebLoginCodeHasher(
    private val webAuthProperties: WebAuthProperties,
) {
    fun hash(code: String): String {
        val digest = MessageDigest.getInstance("SHA-256")
        val input = code + webAuthProperties.codeSalt
        val bytes = digest.digest(input.toByteArray(Charsets.UTF_8))
        return bytes.joinToString("") { b -> "%02x".format(b) }
    }
}
