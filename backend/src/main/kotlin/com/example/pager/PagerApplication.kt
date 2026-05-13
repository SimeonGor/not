package com.example.pager

import org.springframework.boot.autoconfigure.SpringBootApplication
import org.springframework.boot.context.properties.ConfigurationPropertiesScan
import org.springframework.boot.runApplication

@SpringBootApplication
@ConfigurationPropertiesScan
class PagerApplication

fun main(args: Array<String>) {
    runApplication<PagerApplication>(*args)
}
