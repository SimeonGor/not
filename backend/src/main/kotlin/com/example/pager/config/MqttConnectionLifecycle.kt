package com.example.pager.config

import jakarta.annotation.PostConstruct
import jakarta.annotation.PreDestroy
import org.eclipse.paho.client.mqttv3.MqttClient
import org.eclipse.paho.client.mqttv3.MqttConnectOptions
import org.slf4j.LoggerFactory
import org.springframework.stereotype.Component

@Component
class MqttConnectionLifecycle(
    private val mqttClient: MqttClient,
    private val mqttProperties: MqttProperties,
) {
    private val log = LoggerFactory.getLogger(javaClass)

    @PostConstruct
    fun connect() {
        log.info("[MQTT] Connecting to broker {}", mqttProperties.brokerUrl)
        val options =
            MqttConnectOptions().apply {
                isCleanSession = true
                isAutomaticReconnect = true
            }
        mqttClient.connect(options)
        log.info("[MQTT] Connected as {}", mqttProperties.clientId)
    }

    @PreDestroy
    fun disconnect() {
        if (mqttClient.isConnected) {
            mqttClient.disconnect()
            log.info("[MQTT] Disconnected")
        }
    }
}
