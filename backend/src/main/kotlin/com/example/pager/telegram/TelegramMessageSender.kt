package com.example.pager.telegram

fun interface TelegramMessageSender {
    fun sendTelegramMessage(telegramId: Long, text: String)
}
