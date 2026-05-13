package com.example.pager.message

object MessagePayloadFormatter {
    const val MAX_MQTT_PAYLOAD_CHARS: Int = 256

    fun format(senderName: String, text: String): String = "$senderName: $text"

    fun exceedsLimit(senderName: String, text: String): Boolean =
        format(senderName, text).length > MAX_MQTT_PAYLOAD_CHARS
}
