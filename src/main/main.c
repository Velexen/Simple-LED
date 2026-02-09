#include <stdio.h>
#include <stdlib.h>
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "mqtt.h"
#include "rgb_controll.h"
#include "wifi.h"

// Analog refers to an actual potentiometer here
// While digital refers to Home Assistant or something like that which will come at a later stage

#define ADC_THRESHOLD 20        // minimum ADC change to be considered significant
#define ADC_IDLE_TIMEOUT 5000  //10 Seconds Timer for Potentiometer

typedef enum {
    MODE_MANUAL,
    MODE_REMOTE
}control_mode_t;

void app_main(void)
{
    setupChannels();
    wifiInit();
    mqttStart();

    uint8_t analog_selected_color = 0;
    uint8_t previous_button_state = 0;
    int analog_previous_adc = 0;

    uint8_t colors_strip1[3] = {0, 0, 0};
    uint8_t colors_strip2[3] = {0, 0, 0};
    uint8_t brightness = 255; 
    bool light_on = true;
    bool strip_selected = false;  // false = strip1, true = strip2

    control_mode_t mode = MODE_REMOTE;
    unsigned long last_change_time = esp_timer_get_time() / 1000;
    unsigned long last_adc_activity = esp_timer_get_time() / 1000;

    printf("Hello...\n");
    while(true)
    {
        unsigned long current_time = esp_timer_get_time() / 1000;

        //Sleepy Time after 60 seconds
        if(current_time - last_change_time >= 60000) {
            printf("\nSleeping...\n");
            esp_sleep_enable_ext1_wakeup(1ULL << COLOR_BUTTON, !ESP_EXT1_WAKEUP_ANY_HIGH);
            esp_light_sleep_start();
            last_change_time = current_time;
        }

        int color_adc_raw;
        esp_err_t r = adc_oneshot_read(adc1_handle, COLOR_POTENTIOMETER_ADC_CHANNEL, &color_adc_raw);
        if (r != ESP_OK) {
            color_adc_raw = 0;
            printf("failed to read ADC\n");
        }

        int main_switch = (gpio_get_level(MAIN_SWITCH) == 0);
        light_on = main_switch ? true : false;

        int led_switch = (gpio_get_level(SELECTOR_SWITCH) == 0);
        strip_selected = led_switch ? true : false;

        int pressed = (gpio_get_level(COLOR_BUTTON) == 0);
        if (pressed && !previous_button_state) 
        {
            last_change_time = current_time;

            analog_selected_color = (analog_selected_color + 1) % 3;  // Only RGB, not 4 colors
            printf("Selected color: %d (Strip %d)\n", analog_selected_color, strip_selected + 1);

            mode = MODE_MANUAL;
            last_adc_activity = current_time;

        }
        previous_button_state = pressed;

        if(mode == MODE_MANUAL)
        {
            if(abs(color_adc_raw - analog_previous_adc) >= ADC_THRESHOLD)
            {
                last_change_time = current_time;
                last_adc_activity = current_time;
                
                analog_previous_adc = color_adc_raw;
                uint32_t value = (color_adc_raw * 255) / 4095;
                
                // Update selected color for selected strip
                uint8_t *target_colors = strip_selected ? colors_strip2 : colors_strip1;
                ledc_channel_t *target_channels = strip_selected ? led_channels_2 : led_channels;
                
                target_colors[analog_selected_color] = value;

                printf("New adc: %d (Strip %d, Color %d)\n", color_adc_raw, strip_selected + 1, analog_selected_color);
                setColorByRGB(target_colors, target_channels, brightness, light_on);
            } 
            else if(current_time - last_adc_activity >= ADC_IDLE_TIMEOUT)
            {
                mode = MODE_REMOTE;
                printf("MANUAL input idle, switching to REMOTE mode\n");
            }
        }


        vTaskDelay(pdMS_TO_TICKS(10));
    }

}
