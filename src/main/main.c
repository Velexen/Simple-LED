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
#define ADC_IDLE_TIMEOUT 10000  //10 Seconds Timer for Potentiometer

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
    uint8_t current_colors[3] = {0,0,0};
    
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

        int adc_raw;
        esp_err_t r = adc_oneshot_read(adc1_handle, POTENTIOMETER_ADC_CHANNEL, &adc_raw);
        if (r != ESP_OK) {
            adc_raw = 0;
            printf("failed to read ADC\n");
        }

        if(mode == MODE_MANUAL)
        {
            if(abs(adc_raw - analog_previous_adc) >= ADC_THRESHOLD)
            {
                last_change_time = current_time;
                last_adc_activity = current_time;
                
                analog_previous_adc = adc_raw;
                uint32_t value = (adc_raw * 255) / 4095;
                current_colors[analog_selected_color] = value;

                printf("New adc: %d\n", adc_raw); //For Debugging
                setColorByRGB(current_colors, led_channels);
            } 
            else if(current_time - last_adc_activity >= ADC_IDLE_TIMEOUT)
            {
                mode = MODE_REMOTE;
                printf("Manual input idle, switching to REMOTE mode\n");
            }
        }

        int pressed = (gpio_get_level(COLOR_BUTTON) == 0);
        if (pressed && !previous_button_state) 
        {
            last_change_time = current_time;

            analog_selected_color = (analog_selected_color + 1) % 3;
            printf("Selected color: %d\n", analog_selected_color);

            mode = MODE_MANUAL;
            last_adc_activity = current_time;

        }
        previous_button_state = pressed;

        vTaskDelay(pdMS_TO_TICKS(10));
    }

}
