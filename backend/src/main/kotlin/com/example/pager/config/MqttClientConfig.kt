package com.example.pager.config

import org.eclipse.paho.client.mqttv3.MqttClient
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence
import org.springframework.context.annotation.Bean
import org.springframework.context.annotation.Configuration

@Configuration
class MqttClientConfig(
    private val mqttProperties: MqttProperties,
) {
    @Bean
    fun mqttClient(): MqttClient =
        MqttClient(mqttProperties.brokerUrl, mqttProperties.clientId, MemoryPersistence())
}
