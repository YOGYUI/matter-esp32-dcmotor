#include "device_fanctrl_dcmotor.h"
#include "logger.h"
#include "system.h"
#include "definition.h"

CDeviceFanControlDCMotor::CDeviceFanControlDCMotor()
{
    m_motor_pwm_driver = new CMotorPWMDriver();
    if (m_motor_pwm_driver)
        m_motor_pwm_driver->initialize(GPIO_PIN_MOTOR_PWM);
}

CDeviceFanControlDCMotor::~CDeviceFanControlDCMotor()
{
    if (m_motor_pwm_driver) {
        m_motor_pwm_driver->release();
        delete m_motor_pwm_driver;
    }
}

bool CDeviceFanControlDCMotor::matter_add_endpoint()
{
    esp_matter::node_t *root = GetSystem()->get_root_node();
    esp_matter::endpoint::fan::config_t config_endpoint;
    config_endpoint.fan_control.fan_mode = 0;

    config_endpoint.fan_control.fan_mode_sequence = 0;
    config_endpoint.fan_control.percent_setting = (uint8_t)0;
    config_endpoint.fan_control.percent_current = 0;
    uint8_t flags = esp_matter::ENDPOINT_FLAG_DESTROYABLE;
    m_endpoint = esp_matter::endpoint::fan::create(root, &config_endpoint, flags, nullptr);
    if (!m_endpoint) {
        GetLogger(eLogType::Error)->Log("Failed to create endpoint");
        return false;
    }

    return CDevice::matter_add_endpoint();
}

bool CDeviceFanControlDCMotor::matter_init_endpoint()
{
    esp_err_t ret;
    esp_matter::cluster_t *cluster;
    uint32_t attribute_id;
    esp_matter::attribute_t *attribute;
    bool enable_multi_speed = false;
    bool enable_auto = false;
    bool enable_rocking = false;
    bool enable_wind = false;

    cluster = esp_matter::cluster::get(m_endpoint, chip::app::Clusters::FanControl::Id);
    if (cluster) {
        attribute_id = chip::app::Clusters::Globals::Attributes::FeatureMap::Id;
        attribute = esp_matter::attribute::get(cluster, attribute_id);
        if (attribute) {
            esp_matter_attr_val_t value = esp_matter_invalid(nullptr);
            esp_matter::attribute::get_val(attribute, &value);
            value.val.u32 = 0;
            if (enable_multi_speed)
                value.val.u32 |= 0x01;  // SPD (Multi-Speed)
            if (enable_auto)
                value.val.u32 |= 0x02;  // AUT (Auto)
            if (enable_rocking)
                value.val.u32 |= 0x04;  // RCK (Rocking)
            if (enable_wind)
                value.val.u32 |= 0x08;  // WND (Wind)
            ret = esp_matter::attribute::set_val(attribute, &value);
            if (ret != ESP_OK) {
                GetLogger(eLogType::Warning)->Log("Failed to modify feature map (%d)", ret);
            }
        }

        uint8_t flags;
        if (enable_multi_speed) {
            // add speed max attribute
            attribute_id = chip::app::Clusters::FanControl::Attributes::SpeedMax::Id;
            flags = esp_matter::attribute_flags::ATTRIBUTE_FLAG_NONVOLATILE | esp_matter::attribute_flags::ATTRIBUTE_FLAG_WRITABLE;
            attribute = esp_matter::attribute::create(cluster, attribute_id, flags, esp_matter_uint8(100));
            if (!attribute) {
                GetLogger(eLogType::Warning)->Log("Failed to add speed max attribute");
            }
            // add speed setting attribute
            attribute_id = chip::app::Clusters::FanControl::Attributes::SpeedSetting::Id;
            flags = esp_matter::attribute_flags::ATTRIBUTE_FLAG_WRITABLE;
            attribute = esp_matter::attribute::create(cluster, attribute_id, flags, esp_matter_uint8(0));
            if (!attribute) {
                GetLogger(eLogType::Warning)->Log("Failed to add speed setting attribute");
            }
            // add speed current attribute
            attribute_id = chip::app::Clusters::FanControl::Attributes::SpeedCurrent::Id;
            flags = esp_matter::attribute_flags::ATTRIBUTE_FLAG_NONE;
            attribute = esp_matter::attribute::create(cluster, attribute_id, flags, esp_matter_uint8(0));
            if (!attribute) {
                GetLogger(eLogType::Warning)->Log("Failed to add speed current attribute");
            }
        }

        if (enable_rocking) {
            // todo
        }
        
        if (enable_wind) {
            // todo
        }
    }
    
    return true;
}

void CDeviceFanControlDCMotor::matter_on_change_attribute_value(esp_matter::attribute::callback_type_t type, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *value)
{
    if (type == esp_matter::attribute::callback_type_t::PRE_UPDATE) {
        if (cluster_id == chip::app::Clusters::FanControl::Id) {
            if (attribute_id == chip::app::Clusters::FanControl::Attributes::FanMode::Id) {
                GetLogger(eLogType::Info)->Log("cluster: FanControl(0x%04X), attribute: FanMode(0x%04X), value: %u", cluster_id, attribute_id, value->val.u8);
                if (!m_updating_clus_fancontrol_attr_fanmode) {
                    m_state_fan_mode = value->val.u8;
                } else {
                    m_updating_clus_fancontrol_attr_fanmode = false;
                }
            } else if (attribute_id == chip::app::Clusters::FanControl::Attributes::PercentSetting::Id) {
                GetLogger(eLogType::Info)->Log("cluster: FanControl(0x%04X), attribute: PercentSetting(0x%04X), value: %d", cluster_id, attribute_id, value->val.u8);
                if (!m_updating_clus_fancontrol_attr_percentsetting) {
                    m_state_fan_percent = value->val.u8;
                    m_motor_pwm_driver->set_duty_ratio((float)m_state_fan_percent);
                } else {
                    m_updating_clus_fancontrol_attr_percentsetting = false;
                }
            } else if (attribute_id == chip::app::Clusters::FanControl::Attributes::PercentCurrent::Id) {
                GetLogger(eLogType::Info)->Log("cluster: FanControl(0x%04X), attribute: PercentCurrent(0x%04X), value: %d", cluster_id, attribute_id, value->val.u8);
                if (!m_updating_clus_fancontrol_attr_percentcurrent) {
                    // do nothing
                } else {
                    m_updating_clus_fancontrol_attr_percentcurrent = false;
                }
            }
        } 
    }
}

void CDeviceFanControlDCMotor::matter_update_all_attribute_values()
{
    matter_update_clus_fancontrol_attr_fanmode();
    matter_update_clus_fancontrol_attr_percentsetting();
    matter_update_clus_fancontrol_attr_percentcurrent();
}

