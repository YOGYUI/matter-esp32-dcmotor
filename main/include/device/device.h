#pragma once
#ifndef _DEVICE_H_
#define _DEVICE_H_

#include <stdint.h>
#include "definition.h"
#include <esp_matter.h>
#include <esp_matter_core.h>

class CDevice
{
public:
    CDevice();
    virtual ~CDevice();

protected:
    esp_matter::endpoint_t *m_endpoint;
    uint16_t m_endpoint_id;

    bool m_state_onoff;
    uint8_t m_state_brightness;
    uint8_t m_state_hue;
    uint8_t m_state_saturation;
    uint8_t m_state_fan_mode;
    uint8_t m_state_fan_percent;

    virtual bool matter_init_endpoint();

public:
    virtual bool matter_add_endpoint();
    bool matter_destroy_endpoint();
    esp_matter::endpoint_t* matter_get_endpoint();
    uint16_t matter_get_endpoint_id();
    virtual void matter_on_change_attribute_value(
        esp_matter::attribute::callback_type_t type,
        uint32_t cluster_id,
        uint32_t attribute_id,
        esp_matter_attr_val_t *value
    );
    virtual void matter_update_all_attribute_values();

protected:
    bool m_updating_clus_onoff_attr_onoff;
    bool m_updating_clus_levelcontrol_attr_currentlevel;
    bool m_updating_clus_colorcontrol_attr_currenthue;
    bool m_updating_clus_colorcontrol_attr_currentsaturation;
    bool m_updating_clus_fancontrol_attr_fanmode;
    bool m_updating_clus_fancontrol_attr_percentsetting;
    bool m_updating_clus_fancontrol_attr_percentcurrent;

    void matter_update_cluster_attribute_common(
        uint16_t endpoint_id, 
        uint32_t cluster_id, 
        uint32_t attribute_id,
        esp_matter_attr_val_t target_value,
        bool* updating_flag
    );
    virtual void matter_update_clus_onoff_attr_onoff();
    virtual void matter_update_clus_levelcontrol_attr_currentlevel();
    virtual void matter_update_clus_colorcontrol_attr_currenthue();
    virtual void matter_update_clus_colorcontrol_attr_currentsaturation();
    virtual void matter_update_clus_fancontrol_attr_fanmode();
    virtual void matter_update_clus_fancontrol_attr_percentsetting();
    virtual void matter_update_clus_fancontrol_attr_percentcurrent();
};

#endif