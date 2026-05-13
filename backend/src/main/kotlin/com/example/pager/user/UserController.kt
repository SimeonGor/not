package com.example.pager.user

import com.example.pager.message.dto.MessageResponse
import org.springframework.web.bind.annotation.GetMapping
import org.springframework.web.bind.annotation.PathVariable
import org.springframework.web.bind.annotation.RequestMapping
import org.springframework.web.bind.annotation.RequestParam
import org.springframework.web.bind.annotation.RestController

@RestController
@RequestMapping("/api/v1/users")
class UserController(
    private val userProfileService: UserProfileService,
) {
    @GetMapping("/{username}")
    fun getUser(
        @PathVariable username: String,
    ) = userProfileService.getProfileByUsername(username)

    @GetMapping("/{username}/messages")
    fun listMessages(
        @PathVariable username: String,
        @RequestParam(defaultValue = "20") limit: Int,
    ): List<MessageResponse> = userProfileService.listMessagesByUsername(username, limit)
}
