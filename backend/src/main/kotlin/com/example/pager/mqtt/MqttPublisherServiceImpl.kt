package com.example.pager.mqtt

import com.example.pager.config.MqttProperties
import org.eclipse.paho.client.mqttv3.MqttClient
import org.eclipse.paho.client.mqttv3.MqttException
import org.eclipse.paho.client.mqttv3.MqttMessage
import org.eclipse.paho.client.mqttv3.MqttToken
import org.slf4j.LoggerFactory
import org.springframework.stereotype.Service
import java.nio.charset.StandardCharsets

@Service
class MqttPublisherServiceImpl(
    private val mqttClient: MqttClient,
    private val mqttProperties: MqttProperties,
) : MqttPublisherService {
    private val log = LoggerFactory.getLogger(javaClass)

    override fun publish(topic: String, payload: String) {
        val preview =
            if (payload.length > 80) {
                payload.take(80) + "..."
            } else {
                payload
            }
        log.info("[MQTT] Publishing to {}: {}", topic, preview)
        val message =
            MqttMessage(payload.toByteArray(StandardCharsets.UTF_8)).apply {
                qos = mqttProperties.qos
                isRetained = false
            }
        try {
            if (!mqttClient.isConnected) {
                throw MqttPublishException("MQTT client is not connected")
            }
            val token = mqttClient.publish(topic, message)
            (token as MqttToken).waitForCompletion(30_000)
            log.info("[MQTT] Publish success")
        } catch (e: MqttException) {
            throw MqttPublishException("MQTT publish failed for topic $topic", e)
        } catch (e: Exception) {
            if (e is MqttPublishException) throw e
            throw MqttPublishException("MQTT publish failed for topic $topic", e)
        }
    }
}
