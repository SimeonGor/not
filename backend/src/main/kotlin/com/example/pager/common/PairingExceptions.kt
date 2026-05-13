package com.example.pager.common

/** Device already has an owner; cannot request a new pairing PIN. */
class AlreadyBoundException(message: String) : RuntimeException(message)

/** PIN must be exactly 6 digits (Telegram or API validation). */
class InvalidPairingPinFormatException(message: String) : RuntimeException(message)

/** No active pairing code for this hash (wrong, expired, or already used). */
class InvalidOrExpiredPairingPinException(message: String) : RuntimeException(message)

/** Device is linked to another Telegram account. */
class DeviceBoundToAnotherUserException(message: String) : RuntimeException(message)
