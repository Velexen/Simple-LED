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
        ESP_LOGI(TAG, "Connected");

        esp_mqtt_client_subscribe(client, "home/esp32/rgb/set", 1);
        esp_mqtt_client_publish(client, "home/esp32/status", "online", 0, 1, true);
        esp_mqtt_client_publish(
                                client,
                                "home/esp32/rgb/state",
                                "{\"r\":0,\"g\":0,\"b\":0}",
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

            cJSON *json = cJSON_Parse(payload);
            if (!json) break;

            cJSON *r = cJSON_GetObjectItem(json, "r");
            cJSON *g = cJSON_GetObjectItem(json, "g");
            cJSON *b = cJSON_GetObjectItem(json, "b");
            
            if (!cJSON_IsNumber(r) ||
                !cJSON_IsNumber(g) ||
                !cJSON_IsNumber(b)) {
                cJSON_Delete(json);
                break;
            }

            uint8_t colors[3] = {
                r->valueint,
                g->valueint,
                b->valueint
            };

            setColorByRGB(colors, led_channels);
            cJSON_Delete(json);
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