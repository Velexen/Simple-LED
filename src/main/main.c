#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"


#define PIN_RED GPIO_NUM_21
#define CHANNEL_RED LEDC_CHANNEL_0
#define PIN_GREEN GPIO_NUM_19
#define CHANNEL_GREEN LEDC_CHANNEL_1
#define PIN_BLUE GPIO_NUM_18
#define CHANNEL_BLUE LEDC_CHANNEL_2

#define POTENTIOMETER GPIO_NUM_34
#define POTENTIOMETER_ADC_CHANNEL ADC_CHANNEL_6
#define COLOR_BUTTON GPIO_NUM_25

#define FREQUENCY 5000
#define RESOLUTION LEDC_TIMER_8_BIT

//First configure the timer
ledc_timer_config_t timer_config = {
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .timer_num =  LEDC_TIMER_0,
    .duty_resolution = RESOLUTION,
    .freq_hz = FREQUENCY,
    .clk_cfg = LEDC_AUTO_CLK
};

//Then configure the chanel
ledc_channel_config_t channel_config_red = {
    .gpio_num   = PIN_RED,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .channel    = CHANNEL_RED,
    .timer_sel  = LEDC_TIMER_0,
    .duty       = 0,
    .hpoint     = 0
};

ledc_channel_config_t channel_config_green = {
    .gpio_num   = PIN_GREEN,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .channel    = LEDC_CHANNEL_1,
    .timer_sel  = LEDC_TIMER_0,
    .duty       = 0,
    .hpoint     = 0
};

ledc_channel_config_t channel_config_blue = {
    .gpio_num   = PIN_BLUE,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .channel    = LEDC_CHANNEL_2,
    .timer_sel  = LEDC_TIMER_0,
    .duty       = 0,
    .hpoint     = 0
};

adc_oneshot_unit_handle_t adc1_handle;
adc_oneshot_unit_init_cfg_t init_cfg = {
    .unit_id = ADC_UNIT_1,
};

adc_oneshot_chan_cfg_t config = {
    .atten = ADC_ATTEN_DB_12,
    .bitwidth = ADC_BITWIDTH_DEFAULT,
};

void setupChannels()
{
    //Setup Potentiometer
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg, &adc1_handle));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, POTENTIOMETER_ADC_CHANNEL, &config));


    gpio_set_direction(COLOR_BUTTON, GPIO_MODE_INPUT);
    gpio_set_pull_mode(COLOR_BUTTON, GPIO_FLOATING);

    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));
    ESP_ERROR_CHECK(ledc_channel_config(&channel_config_red));
    ESP_ERROR_CHECK(ledc_channel_config(&channel_config_green));
    ESP_ERROR_CHECK(ledc_channel_config(&channel_config_blue));
}

void setDutyCycle(ledc_channel_t channel, uint32_t duty_cycle)
{
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, channel, duty_cycle);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, channel);
}


void app_main(void)
{
    setupChannels();
    uint8_t selected_color = 0;
    uint8_t previous_button_state = 0;
    int previous_adc = 0;
    while(true)
    {
        int adc_raw;
        esp_err_t r = adc_oneshot_read(adc1_handle, POTENTIOMETER_ADC_CHANNEL, &adc_raw);
        if (r != ESP_OK) {
            adc_raw = 0;
            printf("failed to read adc");
        }
        if(adc_raw != previous_adc)
        {
            printf("New adc: %d\n", adc_raw);
            previous_adc = adc_raw;
        } 

        uint32_t duty = (adc_raw * 255) / 4095;

        int pressed = (gpio_get_level(COLOR_BUTTON) == 0);
        if (pressed && !previous_button_state) {
            selected_color = (selected_color + 1) % 3;
            printf("Selected color: %d\n", selected_color);
        }
        previous_button_state = pressed;

        ledc_channel_t led_channels[3] = { CHANNEL_RED, CHANNEL_GREEN, CHANNEL_BLUE };
        setDutyCycle(led_channels[selected_color], duty);

        vTaskDelay(pdMS_TO_TICKS(10));
    }

}
