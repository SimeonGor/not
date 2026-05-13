package com.example.pager.common

class TelegramUserNotAvailableException(message: String) : RuntimeException(message)

class InvalidLoginCodeException(message: String) : RuntimeException(message)

class ReceiverHasNoPagerException(message: String) : RuntimeException(message)
