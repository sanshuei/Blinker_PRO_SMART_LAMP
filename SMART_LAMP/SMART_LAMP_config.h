#ifndef BLINKER_LAMP_CONFIG_H
#define BLINKER_LAMP_CONFIG_H

#include <Arduino.h>

#define BLINKER_WS2812_PIN          4
#define BLINKER_WS2812_COUNT        9

#define BLINKER_POWER_3V3_PIN       14
#define BLINKER_POWER_5V_PIN        15

#define BLINKER_IIC_SCK_PIN         2
#define BLINKER_IIC_SDA_PIN         0

#define BLINKER_BAT_POWER_LOW       3.5
#define BLINKER_BAT_POWER_HIGH      4.0
#define BLINKER_BAT_POWER_USEUP     0.2
#define BLINKER_BAT_CHECK_TIME      10000UL

#define BLINKER_LAMP_TYPE_COUNT     3

#define BLINKER_LAMP_RAINBOW        0
#define BLINKER_LAMP_RAINBOW_CYCLE  1
#define BLINKER_LAMP_BREATH         2

#define BLINKER_LAMP_SPEED_DEFUALT  5000UL

typedef void (*callback_with_uint32_arg_t)(uint32_t data);

#endif