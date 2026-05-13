package com.example.pager.message

import com.example.pager.device.DeviceEntity
import jakarta.persistence.Column
import jakarta.persistence.Entity
import jakarta.persistence.FetchType
import jakarta.persistence.Id
import jakarta.persistence.JoinColumn
import jakarta.persistence.ManyToOne
import jakarta.persistence.Table
import java.time.Instant
import java.util.UUID

@Entity
@Table(name = "messages")
class MessageEntity(
    @Id
    @Column(name = "id", nullable = false)
    var id: UUID,
    @ManyToOne(fetch = FetchType.LAZY, optional = false)
    @JoinColumn(name = "device_id", nullable = false)
    var device: DeviceEntity,
    @Column(name = "sender_name", nullable = false, length = 64)
    var senderName: String,
    @Column(name = "text_content", nullable = false, columnDefinition = "TEXT")
    var textContent: String,
    @Column(name = "mqtt_topic", nullable = false, length = 256)
    var mqttTopic: String,
    @Column(name = "delivered_to_mqtt", nullable = false)
    var deliveredToMqtt: Boolean,
    @Column(name = "created_at", nullable = false)
    var createdAt: Instant,
)
