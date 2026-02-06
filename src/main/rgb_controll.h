#pragma once
#include <stdio.h>
#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"

#define PIN_RED GPIO_NUM_21
#define CHANNEL_RED LEDC_CHANNEL_0
#define PIN_GREEN GPIO_NUM_19
#define CHANNEL_GREEN LEDC_CHANNEL_1
#define PIN_BLUE GPIO_NUM_18
#define CHANNEL_BLUE LEDC_CHANNEL_2

#define SLEEP_SWITCH GPIO_NUM_33
#define POTENTIOMETER GPIO_NUM_34
#define POTENTIOMETER_ADC_CHANNEL ADC_CHANNEL_6
#define COLOR_BUTTON GPIO_NUM_25

#define FREQUENCY 5000
#define RESOLUTION LEDC_TIMER_8_BIT

extern adc_oneshot_unit_handle_t adc1_handle;
extern ledc_channel_t led_channels[3];

void setupChannels(void);
void setDutyCycle(ledc_channel_t channel, uint32_t duty_cycle);
void setColorByRGB(uint8_t colors[3], ledc_channel_t led_channels[3], uint8_t brightness, bool light_on);