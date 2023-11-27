#include "device.h"
#include "logger.h"
#include "system.h"

CDevice::CDevice()
{
    m_endpoint = nullptr;
    m_endpoint_id = 0;
    m_state_onoff = false;
    m_state_brightness = 0;
    m_state_hue = 0;
    m_state_saturation = 0;
    m_state_fan_mode = 0;
    m_state_fan_percent = 0;
    m_updating_clus_onoff_attr_onoff = false;
    m_updating_clus_levelcontrol_attr_currentlevel = false;
    m_updating_clus_colorcontrol_attr_currenthue = false;
    m_updating_clus_colorcontrol_attr_currentsaturation = false;
    m_updating_clus_fancontrol_attr_fanmode = false;
    m_updating_clus_fancontrol_attr_percentsetting = false;
    m_updating_clus_fancontrol_attr_percentcurrent = false;
}

CDevice::~CDevice()
{
}

bool CDevice::matter_add_endpoint()
{
    esp_err_t ret;
    
    if (m_endpoint != nullptr) {
        matter_init_endpoint();

        // get endpoint id
        m_endpoint_id = esp_matter::endpoint::get_id(m_endpoint);

        ret = esp_matter::endpoint::enable(m_endpoint);  // should be called after esp_matter::start()
        if (ret != ESP_OK) {
            GetLogger(eLogType::Error)->Log("Failed to enable endpoint (%d, ret=%d)", m_endpoint_id, ret);
            matter_destroy_endpoint();
            return false;
        }
    } else {
        GetLogger(eLogType::Error)->Log("endpoint instance is null!");
        return false;
    }

    return true;
}

bool CDevice::matter_init_endpoint()
{
    return true;
}

bool CDevice::matter_destroy_endpoint()
{
    esp_err_t ret;

    esp_matter::node_t *root = GetSystem()->get_root_node();
    ret = esp_matter::endpoint::destroy(root, m_endpoint);
    if (ret != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to destroy endpoint (%d)", ret);
        return false;
    }

    return true;
}

esp_matter::endpoint_t* CDevice::matter_get_endpoint() 
{ 
    return m_endpoint; 
}

uint16_t CDevice::matter_get_endpoint_id()
{ 
    return m_endpoint_id; 
}

void CDevice::matter_on_change_attribute_value(esp_matter::attribute::callback_type_t type, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *value)
{
    
}

void CDevice::matter_update_all_attribute_values()
{

}

void CDevice::matter_update_cluster_attribute_common(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t target_value, bool* updating_flag)
{
    *updating_flag = true;
    GetSystem()->matter_update_attribute(endpoint_id, cluster_id, attribute_id, &target_value);
}

void CDevice::matter_update_clus_onoff_attr_onoff()
{
    esp_matter_attr_val_t target_value = esp_matter_bool((bool)m_state_onoff);
    matter_update_cluster_attribute_common(
        m_endpoint_id,
        chip::app::Clusters::OnOff::Id,
        chip::app::Clusters::OnOff::Attributes::OnOff::Id,
        target_value,
        &m_updating_clus_onoff_attr_onoff
    );
}

void CDevice::matter_update_clus_levelcontrol_attr_currentlevel()
{
    esp_matter_attr_val_t target_value = esp_matter_uint8(m_state_brightness);
    matter_update_cluster_attribute_common(
        m_endpoint_id,
        chip::app::Clusters::LevelControl::Id,
        chip::app::Clusters::LevelControl::Attributes::CurrentLevel::Id,
        target_value,
        &m_updating_clus_levelcontrol_attr_currentlevel
    );
}

void CDevice::matter_update_clus_colorcontrol_attr_currenthue()
{
    esp_matter_attr_val_t target_value = esp_matter_uint8(m_state_hue);
    matter_update_cluster_attribute_common(
        m_endpoint_id,
        chip::app::Clusters::ColorControl::Id,
        chip::app::Clusters::ColorControl::Attributes::CurrentHue::Id,
        target_value,
        &m_updating_clus_colorcontrol_attr_currenthue
    );
}

void CDevice::matter_update_clus_colorcontrol_attr_currentsaturation()
{
    esp_matter_attr_val_t target_value = esp_matter_uint8(m_state_saturation);
    matter_update_cluster_attribute_common(
        m_endpoint_id,
        chip::app::Clusters::ColorControl::Id,
        chip::app::Clusters::ColorControl::Attributes::CurrentSaturation::Id,
        target_value,
        &m_updating_clus_colorcontrol_attr_currentsaturation
    );
}

void CDevice::matter_update_clus_fancontrol_attr_fanmode()
{
    esp_matter_attr_val_t target_value = esp_matter_enum8(m_state_fan_mode);
    matter_update_cluster_attribute_common(
        m_endpoint_id,
        chip::app::Clusters::FanControl::Id,
        chip::app::Clusters::FanControl::Attributes::FanMode::Id,
        target_value,
        &m_updating_clus_fancontrol_attr_fanmode
    );
}

void CDevice::matter_update_clus_fancontrol_attr_percentsetting()
{
    esp_matter_attr_val_t target_value = esp_matter_nullable_uint8(m_state_fan_percent);
    matter_update_cluster_attribute_common(
        m_endpoint_id,
        chip::app::Clusters::FanControl::Id,
        chip::app::Clusters::FanControl::Attributes::PercentSetting::Id,
        target_value,
        &m_updating_clus_fancontrol_attr_percentsetting
    );
}

void CDevice::matter_update_clus_fancontrol_attr_percentcurrent()
{
    esp_matter_attr_val_t target_value = esp_matter_uint8(m_state_fan_percent);
    matter_update_cluster_attribute_common(
        m_endpoint_id,
        chip::app::Clusters::FanControl::Id,
        chip::app::Clusters::FanControl::Attributes::PercentCurrent::Id,
        target_value,
        &m_updating_clus_fancontrol_attr_percentcurrent
    );
}
