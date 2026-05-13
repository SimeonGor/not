package com.example.pager.security

import com.example.pager.common.ErrorResponse
import com.fasterxml.jackson.databind.ObjectMapper
import org.springframework.context.annotation.Bean
import org.springframework.context.annotation.Configuration
import org.springframework.http.HttpMethod
import org.springframework.http.MediaType
import org.springframework.security.config.annotation.web.builders.HttpSecurity
import org.springframework.security.config.annotation.web.configuration.EnableWebSecurity
import org.springframework.security.config.http.SessionCreationPolicy
import org.springframework.security.web.SecurityFilterChain
import org.springframework.security.web.authentication.UsernamePasswordAuthenticationFilter
import org.springframework.security.web.util.matcher.AntPathRequestMatcher

@Configuration
@EnableWebSecurity
class SecurityConfig(
    private val jwtAuthenticationFilter: JwtAuthenticationFilter,
    private val objectMapper: ObjectMapper,
) {
    @Bean
    fun securityFilterChain(http: HttpSecurity): SecurityFilterChain {
        http
            .csrf { it.disable() }
            .formLogin { it.disable() }
            .httpBasic { it.disable() }
            .sessionManagement { it.sessionCreationPolicy(SessionCreationPolicy.STATELESS) }
            .authorizeHttpRequests { auth ->
                auth
                    .requestMatchers(
                        AntPathRequestMatcher("/"),
                        AntPathRequestMatcher("/index.html"),
                        AntPathRequestMatcher("/styles.css"),
                        AntPathRequestMatcher("/app.js"),
                        AntPathRequestMatcher("/favicon.ico"),
                        AntPathRequestMatcher("/error"),
                    ).permitAll()
                    .requestMatchers(AntPathRequestMatcher("/api/v1/auth/**")).permitAll()
                    .requestMatchers(AntPathRequestMatcher("/api/v1/users/**")).permitAll()
                    .requestMatchers(AntPathRequestMatcher("/api/v1/devices/**")).permitAll()
                    .requestMatchers(HttpMethod.POST, "/api/v1/messages").permitAll()
                    .requestMatchers(AntPathRequestMatcher("/api/v1/me/**")).authenticated()
                    .anyRequest()
                    .permitAll()
            }.exceptionHandling { ex ->
                ex.authenticationEntryPoint { _, response, authEx ->
                    if (!response.isCommitted) {
                        response.status = 401
                        response.contentType = MediaType.APPLICATION_JSON_VALUE
                        val body =
                            ErrorResponse(
                                error = "AUTHENTICATION_REQUIRED",
                                message = authEx?.message ?: "Authentication required",
                            )
                        response.writer.write(objectMapper.writeValueAsString(body))
                    }
                }
                ex.accessDeniedHandler { _, response, deniedEx ->
                    if (!response.isCommitted) {
                        response.status = 403
                        response.contentType = MediaType.APPLICATION_JSON_VALUE
                        val body =
                            ErrorResponse(
                                error = "ACCESS_DENIED",
                                message = deniedEx?.message ?: "Access denied",
                            )
                        response.writer.write(objectMapper.writeValueAsString(body))
                    }
                }
            }.addFilterBefore(jwtAuthenticationFilter, UsernamePasswordAuthenticationFilter::class.java)

        return http.build()
    }
}
