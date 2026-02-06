#include <stdio.h>
#include "mqtt.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "cJSON.h"
#include "rgb_controll.h"

static const char *TAG = "MQTT";
static esp_mqtt_client_handle_t client = NULL;

static void mqttEventHandler(void *arg,
                            esp_event_base_t event_base,
                            int32_t event_id,
                            void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch (event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT connected");

        const char *discovery_topic = "homeassistant/light/esp32_rgb/config";
        const char *payload = "{\"name\":\"ESP32 RGB\","
                            "\"schema\":\"json\","
                            "\"command_topic\":\"home/esp32/rgb/set\","
                            "\"state_topic\":\"home/esp32/rgb/state\","
                            "\"availability_topic\":\"home/esp32/availability\","
                            "\"payload_available\":\"online\","
                            "\"payload_not_available\":\"offline\","
                            "\"supported_color_modes\":[\"rgb\"]}";

        esp_mqtt_client_publish(client, discovery_topic, payload, 0, 1, true);

        esp_mqtt_client_publish(
            client,
            "home/esp32/availability",
            "online",
            0, 1, true
        );

        esp_mqtt_client_subscribe(
            client, "home/esp32/rgb/set", 1
        );

        esp_mqtt_client_publish(
            client,
            "home/esp32/rgb/state",
            "{\"state\":\"ON\",\"brightness\":255,"
            "\"color\":{\"r\":0,\"g\":0,\"b\":0}}", 
            0, 1, true
        );
        break;
    
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "Topic: %.*s", event->topic_len, event->topic);

        if(strncmp(event->topic, "home/esp32/rgb/set", event->topic_len) == 0)
        {
            char payload[32];
            memcpy(payload, event->data, event->data_len);
            payload[event->data_len] = '\0';

            cJSON *root = cJSON_Parse(payload);
            if (!root) break;

            bool light_on = true;
            cJSON *state = cJSON_GetObjectItem(root, "state");            
            if (cJSON_IsString(state) && state->valuestring) 
            {
                if (strcmp(state->valuestring, "OFF") == 0) 
                {
                    light_on = false;
                }
            }

            uint8_t brightness = 255;
            cJSON *j_brightness = cJSON_GetObjectItem(root, "brightness");
            if(cJSON_IsNumber(j_brightness))
            {
                brightness = j_brightness->valueint;
            }

            uint8_t rgb[3] = {0, 0, 0};
            cJSON *color = cJSON_GetObjectItem(root, "color");
            if (cJSON_IsObject(color)) {
                cJSON *r = cJSON_GetObjectItem(color, "r");
                cJSON *g = cJSON_GetObjectItem(color, "g");
                cJSON *b = cJSON_GetObjectItem(color, "b");

                if (cJSON_IsNumber(r)) rgb[0] = r->valueint;
                if (cJSON_IsNumber(g)) rgb[1] = g->valueint;
                if (cJSON_IsNumber(b)) rgb[2] = b->valueint;
            }

            setColorByRGB(rgb, led_channels, brightness, light_on);
            cJSON_Delete(root);
        }
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "Disconnected");
        break;
    }
}

void mqttStart(void)
{
    esp_mqtt_client_config_t cfg = {
        .broker.address.uri = "mqtt://192.168.68.80", //If someone ever uses this code adjust this for your needs
        .credentials.client_id = "esp_rgb_01",
        .session.last_will.topic = "home/esp32/status",
        .session.last_will.msg = "offline",
        .session.last_will.retain = true,
        .session.last_will.qos = 1,
    };

    client = esp_mqtt_client_init(&cfg);

    esp_mqtt_client_register_event(
        client,
        ESP_EVENT_ANY_ID,
        mqttEventHandler,
        NULL
    );

    esp_mqtt_client_start(client);

    printf("\nMQTT Setup...\n");
}

void mqttPublishState(const char *topic, const char *payload, bool retain)
{
    if (!client) return;

    esp_mqtt_client_publish(
        client,
        topic,
        payload,
        0,
        1,
        retain
    );
}