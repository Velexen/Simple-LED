#include "rgb_controll.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "mqtt.h"

adc_oneshot_unit_handle_t adc1_handle;
ledc_channel_t led_channels[3] = { CHANNEL_RED, CHANNEL_GREEN, CHANNEL_BLUE };

// Static configuration structures - only used within this file
static ledc_timer_config_t timer_config = {
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .timer_num =  LEDC_TIMER_0,
    .duty_resolution = RESOLUTION,
    .freq_hz = FREQUENCY,
    .clk_cfg = LEDC_AUTO_CLK
};

static ledc_channel_config_t channel_config_red = {
    .gpio_num   = PIN_RED,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .channel    = CHANNEL_RED,
    .timer_sel  = LEDC_TIMER_0,
    .duty       = 0,
    .hpoint     = 0
};

static ledc_channel_config_t channel_config_green = {
    .gpio_num   = PIN_GREEN,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .channel    = LEDC_CHANNEL_1,
    .timer_sel  = LEDC_TIMER_0,
    .duty       = 0,
    .hpoint     = 0
};

static ledc_channel_config_t channel_config_blue = {
    .gpio_num   = PIN_BLUE,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .channel    = LEDC_CHANNEL_2,
    .timer_sel  = LEDC_TIMER_0,
    .duty       = 0,
    .hpoint     = 0
};

static adc_oneshot_unit_init_cfg_t init_cfg = {
    .unit_id = ADC_UNIT_1,
};

static adc_oneshot_chan_cfg_t config = {
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
    
    printf("\nChannels Setup...\n");

}

void setDutyCycle(ledc_channel_t channel, uint32_t duty_cycle)
{
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, channel, duty_cycle);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, channel);
}


void setColorByRGB(uint8_t colors[3], ledc_channel_t led_channels[3])
{
    setDutyCycle(led_channels[0], colors[0]);
    setDutyCycle(led_channels[1], colors[1]);
    setDutyCycle(led_channels[2], colors[2]);

    char json[64];
    snprintf(json, sizeof(json),
    "{\"r\":%d,\"g\":%d,\"b\":%d}", colors[0], colors[1], colors[2]);

    mqttPublishState(
        "home/esp32/rgb/state",
        json,
        true
    );
}