package com.example.pager.device.dto

import com.fasterxml.jackson.annotation.JsonProperty

data class ResetBindingResponse(
    @JsonProperty("deviceId")
    val deviceId: String,
    val bound: Boolean,
    val message: String,
)
