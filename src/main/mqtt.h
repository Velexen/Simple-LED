#pragma once
#include "esp_err.h"
#include <stdbool.h>

void mqttStart(void);
void mqttPublishState(const char *topic, const char *payload, bool retain);