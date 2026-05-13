package com.example.pager.mqtt

interface MqttPublisherService {
    fun publish(topic: String, payload: String)

    fun publishSysCommand(deviceId: String, command: String)
}
