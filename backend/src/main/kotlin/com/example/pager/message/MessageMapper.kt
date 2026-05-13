package com.example.pager.message

import com.example.pager.message.dto.MessageResponse

fun MessageEntity.toMessageResponse(externalDeviceId: String): MessageResponse =
    MessageResponse(
        id = id,
        deviceId = externalDeviceId,
        senderName = senderName,
        text = textContent,
        mqttTopic = mqttTopic,
        deliveredToMqtt = deliveredToMqtt,
        createdAt = createdAt,
    )
