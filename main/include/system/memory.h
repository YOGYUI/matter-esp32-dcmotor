#ifndef _MEMORY_H_
#define _MEMORY_H_
#pragma once

#include <stdint.h>
#include <strings.h>
#include "definition.h"
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

class CMemory
{
public:
    CMemory();
    virtual ~CMemory();

public:
    static CMemory* Instance();
    static void Release();

    bool load_motor_pwm_percent(uint8_t *percent);
    bool save_motor_pwm_percent(const uint8_t percent);

private:
    static CMemory* _instance;

    bool read_nvs(const char *key, void *out, size_t data_size);
    bool write_nvs(const char *key, const void *data, const size_t data_size);
};

inline CMemory* GetMemory() {
    return CMemory::Instance();
}

inline void ReleaseMemory() {
    CMemory::Release();
}

#ifdef __cplusplus
}
#endif
#endif