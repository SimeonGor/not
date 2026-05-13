package com.example.pager.me

import com.example.pager.me.dto.MeSendRequest
import com.example.pager.me.dto.MeSendResponse
import com.example.pager.message.dto.MessageResponse
import com.example.pager.security.AuthenticatedUser
import com.example.pager.user.UserProfileService
import com.example.pager.user.dto.UserProfileResponse
import jakarta.validation.Valid
import org.springframework.security.core.annotation.AuthenticationPrincipal
import org.springframework.web.bind.annotation.GetMapping
import org.springframework.web.bind.annotation.PostMapping
import org.springframework.web.bind.annotation.RequestBody
import org.springframework.web.bind.annotation.RequestMapping
import org.springframework.web.bind.annotation.RequestParam
import org.springframework.web.bind.annotation.RestController

@RestController
@RequestMapping("/api/v1/me")
class MeController(
    private val userProfileService: UserProfileService,
    private val meSendService: MeSendService,
) {
    @GetMapping
    fun me(
        @AuthenticationPrincipal principal: AuthenticatedUser,
    ): UserProfileResponse = userProfileService.getProfileByUserId(principal.userId)

    @GetMapping("/messages")
    fun messages(
        @AuthenticationPrincipal principal: AuthenticatedUser,
        @RequestParam(defaultValue = "20") limit: Int,
    ): List<MessageResponse> = userProfileService.listMessagesForUserId(principal.userId, limit)

    @PostMapping("/send")
    fun send(
        @AuthenticationPrincipal principal: AuthenticatedUser,
        @RequestBody @Valid body: MeSendRequest,
    ): MeSendResponse =
        meSendService.send(
            senderUserId = principal.userId,
            targetUsernameRaw = body.targetUsername,
            text = body.text,
        )
}
