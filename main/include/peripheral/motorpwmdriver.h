#pragma once
#ifndef _MOTOR_PWM_DRIVER_H_
#define _MOTOR_PWM_DRIVER_H_

#include "driver/mcpwm_timer.h"
#include "driver/mcpwm_oper.h"
#include "driver/mcpwm_cmpr.h"
#include "driver/mcpwm_gen.h"

class CMotorPWMDriver
{
public:
    CMotorPWMDriver();
    ~CMotorPWMDriver();

    bool initialize(int gpio_num, int group_id = 0);
    bool release();

    bool set_enable();
    bool set_disable();
    
    bool set_duty_ratio(float ratio);

private:
    mcpwm_timer_handle_t m_mcpwm_timer;
    mcpwm_oper_handle_t m_mcpwm_operator;
    mcpwm_cmpr_handle_t m_mcpwm_comparator;
    mcpwm_gen_handle_t m_mcpwm_generator;

    int m_gpio_num;
    int m_group_id;
    uint32_t m_timer_resolution_hz;
    uint32_t m_pwm_frequency_hz;
    uint32_t m_pwm_count_tick;

    bool create_mcpwm_timer();
    bool create_mcpwm_operator();
    bool create_mcpwm_comparator();
    bool create_mcpwm_generator();
};

#endif