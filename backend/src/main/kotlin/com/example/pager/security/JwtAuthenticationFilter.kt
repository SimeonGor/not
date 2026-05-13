package com.example.pager.security

import com.example.pager.common.ErrorResponse
import com.fasterxml.jackson.databind.ObjectMapper
import io.jsonwebtoken.JwtException
import jakarta.servlet.FilterChain
import jakarta.servlet.http.HttpServletRequest
import jakarta.servlet.http.HttpServletResponse
import org.springframework.http.HttpHeaders
import org.springframework.http.MediaType
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken
import org.springframework.security.core.authority.SimpleGrantedAuthority
import org.springframework.security.core.context.SecurityContextHolder
import org.springframework.security.web.authentication.WebAuthenticationDetailsSource
import org.springframework.stereotype.Component
import org.springframework.web.filter.OncePerRequestFilter

@Component
class JwtAuthenticationFilter(
    private val jwtService: JwtService,
    private val objectMapper: ObjectMapper,
) : OncePerRequestFilter() {
    override fun doFilterInternal(
        request: HttpServletRequest,
        response: HttpServletResponse,
        filterChain: FilterChain,
    ) {
        val path = request.servletPath.ifEmpty { request.requestURI }
        if (!path.startsWith("/api/v1/me")) {
            filterChain.doFilter(request, response)
            return
        }

        try {
            val header = request.getHeader(HttpHeaders.AUTHORIZATION)
            if (header.isNullOrBlank() || !header.startsWith("Bearer ")) {
                writeJson(
                    response,
                    HttpServletResponse.SC_UNAUTHORIZED,
                    "AUTHENTICATION_REQUIRED",
                    "Authentication required",
                )
                return
            }
            val token = header.removePrefix("Bearer ").trim()
            if (token.isEmpty()) {
                writeJson(
                    response,
                    HttpServletResponse.SC_UNAUTHORIZED,
                    "AUTHENTICATION_REQUIRED",
                    "Authentication required",
                )
                return
            }

            val principal = jwtService.parseAndValidate(token)
            val authentication =
                UsernamePasswordAuthenticationToken(
                    principal,
                    null,
                    listOf(SimpleGrantedAuthority("ROLE_USER")),
                )
            authentication.details = WebAuthenticationDetailsSource().buildDetails(request)
            SecurityContextHolder.getContext().authentication = authentication
            filterChain.doFilter(request, response)
        } catch (e: JwtException) {
            writeJson(
                response,
                HttpServletResponse.SC_UNAUTHORIZED,
                "AUTHENTICATION_REQUIRED",
                "Invalid or expired session",
            )
        } finally {
            SecurityContextHolder.clearContext()
        }
    }

    private fun writeJson(
        response: HttpServletResponse,
        status: Int,
        error: String,
        message: String,
    ) {
        response.status = status
        response.contentType = MediaType.APPLICATION_JSON_VALUE
        response.characterEncoding = Charsets.UTF_8.name()
        val body = ErrorResponse(error = error, message = message)
        response.writer.write(objectMapper.writeValueAsString(body))
    }
}
