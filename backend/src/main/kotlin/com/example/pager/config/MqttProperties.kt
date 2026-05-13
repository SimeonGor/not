package com.example.pager.config

import org.springframework.boot.context.properties.ConfigurationProperties

@ConfigurationProperties(prefix = "mqtt")
data class MqttProperties(
    var brokerUrl: String = "",
    var clientId: String = "",
    var qos: Int = 1,
)
