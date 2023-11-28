#pragma once
#ifndef _DEVICE_FANCTRL_DCMOTOR_H_
#define _DEVICE_FANCTRL_DCMOTOR_H_

#include "device.h"
#include "motorpwmdriver.h"

class CDeviceFanControlDCMotor : public CDevice
{
public:
    CDeviceFanControlDCMotor();
    ~CDeviceFanControlDCMotor();

    bool matter_add_endpoint() override;
    bool matter_init_endpoint() override;
    void matter_on_change_attribute_value(
        esp_matter::attribute::callback_type_t type,
        uint32_t cluster_id,
        uint32_t attribute_id,
        esp_matter_attr_val_t *value
    ) override;
    void matter_update_all_attribute_values() override;

private:
    CMotorPWMDriver *m_motor_pwm_driver;
};

#endif