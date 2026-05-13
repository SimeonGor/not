package com.example.pager.common

import com.example.pager.mqtt.MqttPublishException
import jakarta.validation.ConstraintViolationException
import org.slf4j.LoggerFactory
import org.springframework.http.HttpStatus
import org.springframework.http.ResponseEntity
import org.springframework.http.converter.HttpMessageNotReadableException
import org.springframework.web.bind.MethodArgumentNotValidException
import org.springframework.web.bind.annotation.ExceptionHandler
import org.springframework.web.bind.annotation.RestControllerAdvice

@RestControllerAdvice
class GlobalExceptionHandler {
    private val log = LoggerFactory.getLogger(javaClass)

    @ExceptionHandler(MethodArgumentNotValidException::class)
    fun handleValidation(ex: MethodArgumentNotValidException): ResponseEntity<ErrorResponse> {
        val msg =
            ex.bindingResult.fieldErrors.joinToString("; ") { fe ->
                "${fe.field}: ${fe.defaultMessage ?: "invalid"}"
            }
        return ResponseEntity.status(HttpStatus.BAD_REQUEST).body(
            ErrorResponse(error = "VALIDATION_ERROR", message = msg.ifBlank { "Validation failed" }),
        )
    }

    @ExceptionHandler(ConstraintViolationException::class)
    fun handleConstraintViolation(ex: ConstraintViolationException): ResponseEntity<ErrorResponse> {
        val msg = ex.constraintViolations.joinToString("; ") { "${it.propertyPath}: ${it.message}" }
        return ResponseEntity.status(HttpStatus.BAD_REQUEST).body(
            ErrorResponse(error = "VALIDATION_ERROR", message = msg),
        )
    }

    @ExceptionHandler(HttpMessageNotReadableException::class)
    fun handleNotReadable(ex: HttpMessageNotReadableException): ResponseEntity<ErrorResponse> =
        ResponseEntity.status(HttpStatus.BAD_REQUEST).body(
            ErrorResponse(
                error = "VALIDATION_ERROR",
                message = ex.message ?: "Invalid JSON body",
            ),
        )

    @ExceptionHandler(PayloadTooLongException::class)
    fun handlePayloadTooLong(ex: PayloadTooLongException): ResponseEntity<ErrorResponse> =
        ResponseEntity.status(HttpStatus.BAD_REQUEST).body(
            ErrorResponse(error = "PAYLOAD_TOO_LONG", message = ex.message ?: "Payload too long"),
        )

    @ExceptionHandler(MqttPublishException::class)
    fun handleMqtt(ex: MqttPublishException): ResponseEntity<ErrorResponse> {
        log.warn("[MQTT] Publish error: {}", ex.message)
        return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).body(
            ErrorResponse(error = "MQTT_PUBLISH_ERROR", message = ex.message ?: "MQTT publish failed"),
        )
    }

    @ExceptionHandler(AlreadyBoundException::class)
    fun handleAlreadyBound(ex: AlreadyBoundException): ResponseEntity<ErrorResponse> =
        ResponseEntity.status(HttpStatus.CONFLICT).body(
            ErrorResponse(error = "ALREADY_BOUND", message = ex.message ?: "Device is already bound"),
        )

    @ExceptionHandler(DeviceBoundToAnotherUserException::class)
    fun handleDeviceBoundToOther(ex: DeviceBoundToAnotherUserException): ResponseEntity<ErrorResponse> =
        ResponseEntity.status(HttpStatus.CONFLICT).body(
            ErrorResponse(error = "DEVICE_BOUND_TO_ANOTHER_USER", message = ex.message ?: "Conflict"),
        )

    @ExceptionHandler(InvalidDeviceIdException::class)
    fun handleInvalidDeviceId(ex: InvalidDeviceIdException): ResponseEntity<ErrorResponse> =
        ResponseEntity.status(HttpStatus.BAD_REQUEST).body(
            ErrorResponse(error = "VALIDATION_ERROR", message = ex.message ?: "Invalid deviceId"),
        )

    @ExceptionHandler(InvalidPairingPinFormatException::class)
    fun handleInvalidPinFormat(ex: InvalidPairingPinFormatException): ResponseEntity<ErrorResponse> =
        ResponseEntity.status(HttpStatus.BAD_REQUEST).body(
            ErrorResponse(error = "INVALID_PIN_FORMAT", message = ex.message ?: "Invalid PIN format"),
        )

    @ExceptionHandler(InvalidOrExpiredPairingPinException::class)
    fun handleInvalidOrExpiredPin(ex: InvalidOrExpiredPairingPinException): ResponseEntity<ErrorResponse> =
        ResponseEntity.status(HttpStatus.BAD_REQUEST).body(
            ErrorResponse(error = "INVALID_OR_EXPIRED_PIN", message = ex.message ?: "Invalid or expired PIN"),
        )

    @ExceptionHandler(Exception::class)
    fun handleGeneric(ex: Exception): ResponseEntity<ErrorResponse> {
        log.error("[INTERNAL] Unhandled error", ex)
        return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).body(
            ErrorResponse(error = "INTERNAL_ERROR", message = "An unexpected error occurred"),
        )
    }
}
