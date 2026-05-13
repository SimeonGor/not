package com.example.pager.message

import com.example.pager.message.dto.MessageResponse
import com.example.pager.message.dto.SendMessageRequest
import jakarta.validation.Valid
import org.springframework.web.bind.annotation.PostMapping
import org.springframework.web.bind.annotation.RequestBody
import org.springframework.web.bind.annotation.RequestMapping
import org.springframework.web.bind.annotation.RestController

@RestController
@RequestMapping("/api/v1/messages")
class MessageController(
    private val messageService: MessageService,
) {
    @PostMapping
    fun sendMessage(@Valid @RequestBody body: SendMessageRequest): MessageResponse =
        messageService.sendMessage(body)
}
