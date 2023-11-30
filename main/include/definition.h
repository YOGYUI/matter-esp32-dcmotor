#ifndef _DEFINITION_H_
#define _DEFINITION_H_
#pragma once

#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif
#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#define PRODUCT_NAME            "YOGYUI-MATTER-FAN"

#define GPIO_PIN_DEFAULT_BTN    0
#define GPIO_PIN_MOTOR_PWM      16

#define TASK_STACK_DEPTH        4096
#define TASK_PRIORITY_WS2812    2

typedef enum
{
    FANMODE_OFF     = 0,
    FANMODE_LOW     = 1,
    FANMODE_MEDIUM  = 2,
    FANMODE_HIGH    = 3,
    FANMODE_ON      = 4,
    FANMODE_AUTO    = 5,
    FANMODE_SMART   = 6
} MATTER_FAN_FANMODE;

typedef enum
{
    PERCENT_MODE_OFF      = 0,
    PERCENT_MODE_LOW      = 30,
    PERCENT_MODE_MEDIUM   = 60,
    PERCENT_MODE_HIGH     = 100,
    PERCENT_MODE_MAX      = 255,
} MATTER_FAN_MODE_PERCENT_MAP;

#endif