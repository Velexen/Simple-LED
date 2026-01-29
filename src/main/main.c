#include <stdio.h>
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "rgb_controll.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Analog refers to an actual potentiometer here
// While digital refers to Home Assistant or something like that which will come at a later stage

void app_main(void)
{
    setupChannels();

    uint8_t analog_selected_color = 0;
    uint8_t previous_button_state = 0;

    int analog_previous_adc = 0;
    bool analog_mode = true;
    
    ledc_channel_t led_channels[3] = { CHANNEL_RED, CHANNEL_GREEN, CHANNEL_BLUE };
    uint8_t digital_colors[3] = {0,0,0};

    unsigned long last_change_time = esp_timer_get_time() / 1000;

    while(true)
    {
        unsigned long current_time = esp_timer_get_time() / 1000;
    if(current_time - last_change_time >= 60000) {
        printf("\nSleeping...\n");

        esp_sleep_enable_ext1_wakeup(1ULL << COLOR_BUTTON, !ESP_EXT1_WAKEUP_ANY_HIGH);

        esp_light_sleep_start();

        last_change_time = esp_timer_get_time() / 1000;
    }

        int adc_raw;
        esp_err_t r = adc_oneshot_read(adc1_handle, POTENTIOMETER_ADC_CHANNEL, &adc_raw);
        if (r != ESP_OK) {
            adc_raw = 0;
            printf("failed to read adc");
        }

        if(analog_mode)
        {
            if(abs(adc_raw - analog_previous_adc) >= 10)
            {
                last_change_time = esp_timer_get_time() / 1000;
                printf("New adc: %d\n", adc_raw);
                uint32_t duty = (adc_raw * 255) / 4095;
                analog_previous_adc = adc_raw;
                setDutyCycle(led_channels[analog_selected_color], duty);
            }        
        }else
        {
            setColorByRGB(digital_colors, led_channels);
        }

        int pressed = (gpio_get_level(COLOR_BUTTON) == 0);
        if (pressed && !previous_button_state) {
            last_change_time = esp_timer_get_time() / 1000;
            analog_selected_color = (analog_selected_color + 1) % 3;
            printf("Selected color: %d\n", analog_selected_color);
        }
        previous_button_state = pressed;

        vTaskDelay(pdMS_TO_TICKS(10));
    }

}
