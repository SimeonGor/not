package com.example.pager.device.dto

import com.fasterxml.jackson.annotation.JsonProperty

data class BindingStatusResponse(
    @JsonProperty("deviceId")
    val deviceId: String,
    val bound: Boolean,
    @JsonProperty("ownerUsername")
    val ownerUsername: String?,
    @JsonProperty("pairingRequired")
    val pairingRequired: Boolean,
)
