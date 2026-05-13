package com.example.pager.config

import org.springframework.context.annotation.Bean
import org.springframework.context.annotation.Configuration
import org.springframework.transaction.PlatformTransactionManager
import org.springframework.transaction.support.TransactionTemplate

@Configuration
class TransactionConfig {
    @Bean
    fun transactionTemplate(platformTransactionManager: PlatformTransactionManager): TransactionTemplate =
        TransactionTemplate(platformTransactionManager)
}
