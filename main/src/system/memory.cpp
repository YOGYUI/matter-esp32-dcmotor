#include "memory.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "logger.h"
#include "cJSON.h"

#define MEMORY_NAMESPACE "yogyui"

CMemory* CMemory::_instance;

CMemory::CMemory()
{
}

CMemory::~CMemory()
{
}

CMemory* CMemory::Instance()
{
    if (!_instance) {
        _instance = new CMemory();
    }

    return _instance;
}

void CMemory::Release()
{
    if (_instance) {
        delete _instance;
        _instance = nullptr;
    }
}

bool CMemory::read_nvs(const char *key, void *out, size_t data_size)
{
    nvs_handle handle;

    esp_err_t err = nvs_open(MEMORY_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to open nvs (ret=%d)", err);
        return false;
    }

    size_t temp = data_size;
    err = nvs_get_blob(handle, key, out, &temp);
    if (err != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to get blob (%s, ret=%d)", key, err);
        nvs_close(handle);
        return false;
    }

    nvs_close(handle);
    return true;
}

bool CMemory::write_nvs(const char *key, const void *data, const size_t data_size)
{
    nvs_handle handle;

    esp_err_t err = nvs_open(MEMORY_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to open nvs (ret=%d)", err);
        return false;
    }

    err = nvs_set_blob(handle, key, data, data_size);
    if (err != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to set nvs blob (%s, ret=%d)", key, err);
        nvs_close(handle);
        return false;
    }

    err = nvs_commit(handle);
    if (err != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to commit nvs (ret=%d)", err);
        nvs_close(handle);
        return false;
    }

    nvs_close(handle);
    return true;
}

bool CMemory::load_motor_pwm_percent(uint8_t *percent)
{
    uint8_t temp;
    if (read_nvs("motor_pwm", &temp, sizeof(uint8_t))) {
        GetLogger(eLogType::Info)->Log("load <motor pwm percent> from memory: %d", temp);
        *percent = temp;
    } else{
        return false;
    }

    return true;
}

bool CMemory::save_motor_pwm_percent(const uint8_t percent)
{
    if (write_nvs("motor_pwm", &percent, sizeof(uint8_t))) {
        GetLogger(eLogType::Info)->Log("save <motor pwm percent> to memory: %d", percent);
    } else {
        return false;
    }

    return true;
}
