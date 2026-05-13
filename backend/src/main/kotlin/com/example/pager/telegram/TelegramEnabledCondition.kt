package com.example.pager.telegram

import org.springframework.context.annotation.Condition
import org.springframework.context.annotation.ConditionContext
import org.springframework.core.type.AnnotatedTypeMetadata

class TelegramEnabledCondition : Condition {
    override fun matches(context: ConditionContext, metadata: AnnotatedTypeMetadata): Boolean {
        val token = context.environment.getProperty("telegram.bot-token")?.trim()
        return !token.isNullOrEmpty()
    }
}
