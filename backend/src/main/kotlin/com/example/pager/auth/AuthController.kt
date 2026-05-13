package com.example.pager.auth

import com.example.pager.auth.dto.AuthTokenResponse
import com.example.pager.auth.dto.RequestCodeRequest
import com.example.pager.auth.dto.RequestCodeResponse
import com.example.pager.auth.dto.VerifyCodeRequest
import jakarta.validation.Valid
import org.springframework.web.bind.annotation.PostMapping
import org.springframework.web.bind.annotation.RequestBody
import org.springframework.web.bind.annotation.RequestMapping
import org.springframework.web.bind.annotation.RestController

@RestController
@RequestMapping("/api/v1/auth")
class AuthController(
    private val webAuthService: WebAuthService,
) {
    @PostMapping("/request-code")
    fun requestCode(
        @RequestBody @Valid body: RequestCodeRequest,
    ): RequestCodeResponse = webAuthService.requestCode(body.username)

    @PostMapping("/verify-code")
    fun verifyCode(
        @RequestBody @Valid body: VerifyCodeRequest,
    ): AuthTokenResponse = webAuthService.verifyCode(body.username, body.code)
}
