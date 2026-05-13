package com.example.pager.mqtt

import com.example.pager.config.MqttProperties
import com.example.pager.device.DeviceIdNormalizer
import org.eclipse.paho.client.mqttv3.IMqttToken
import org.eclipse.paho.client.mqttv3.MqttClient
import org.eclipse.paho.client.mqttv3.MqttException
import org.eclipse.paho.client.mqttv3.MqttMessage
import org.slf4j.LoggerFactory
import org.springframework.stereotype.Service
import java.nio.charset.StandardCharsets

@Service
class MqttPublisherServiceImpl(
    private val mqttClient: MqttClient,
    private val mqttProperties: MqttProperties,
) : MqttPublisherService {
    private val log = LoggerFactory.getLogger(javaClass)

    /**
     * Paho's synchronous client is not safe for concurrent [publish] + [IMqttToken.waitForCompletion]
     * when automatic reconnect is enabled: parallel HTTP + Telegram can throw [MqttException] even
     * though the broker already accepted the message. Serialize the critical section.
     */
    private val publishLock = Any()

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
        synchronized(publishLock) {
            try {
                if (!mqttClient.isConnected) {
                    throw MqttPublishException("MQTT client is not connected")
                }
                val token = mqttClient.publish(topic, message) as IMqttToken
                token.waitForCompletion(30_000)
                log.info("[MQTT] Publish success")
            } catch (e: MqttException) {
                log.warn(
                    "[MQTT] Publish failed topic={} reasonCode={}: {}",
                    topic,
                    e.reasonCode,
                    e.message,
                    e,
                )
                throw MqttPublishException("MQTT publish failed for topic $topic", e)
            } catch (e: Exception) {
                if (e is MqttPublishException) throw e
                log.warn("[MQTT] Publish failed topic={}: {}", topic, e.message, e)
                throw MqttPublishException("MQTT publish failed for topic $topic", e)
            }
        }
    }

    override fun publishSysCommand(deviceId: String, command: String) {
        val normalized = DeviceIdNormalizer.normalize(deviceId)
        val topic = "pager/$normalized/sys"
        log.info("[MQTT] Publishing sys command to {} payload={}", topic, command)
        publish(topic, command)
    }
}
