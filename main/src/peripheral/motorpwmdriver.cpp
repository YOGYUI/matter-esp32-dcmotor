#include "motorpwmdriver.h"
#include "logger.h"
#include <functional>

CMotorPWMDriver::CMotorPWMDriver()
{
    m_mcpwm_timer = nullptr;
    m_mcpwm_operator = nullptr;
    m_mcpwm_comparator = nullptr;
    m_mcpwm_generator = nullptr;

    m_gpio_num = 0;
    m_group_id = 0;
    m_timer_resolution_hz = 10000000;   // 10MHz
    m_pwm_frequency_hz = 25000;         // 25 KHz (above audible frequency)
    m_pwm_count_tick = 1;
}

CMotorPWMDriver::~CMotorPWMDriver()
{
}

bool CMotorPWMDriver::initialize(int gpio_num, int group_id/*=0*/)
{
    m_gpio_num = gpio_num;
    m_group_id = group_id;
    m_pwm_count_tick = m_timer_resolution_hz / m_pwm_frequency_hz;

    if (!create_mcpwm_timer())
        return false;
    if (!create_mcpwm_operator())
        return false;
    if (!create_mcpwm_comparator())
        return false;
    if (!create_mcpwm_generator())
        return false;

    return true;
}

static bool on_timer_full(mcpwm_timer_handle_t timer, const mcpwm_timer_event_data_t *edata, void *user_ctx)
{
    return true;
}

static bool on_timer_empty(mcpwm_timer_handle_t timer, const mcpwm_timer_event_data_t *edata, void *user_ctx)
{
    return true;
}

static bool on_timer_stop(mcpwm_timer_handle_t timer, const mcpwm_timer_event_data_t *edata, void *user_ctx)
{
    return true;
}

bool CMotorPWMDriver::create_mcpwm_timer()
{
    mcpwm_timer_config_t config;
    config.group_id = m_group_id;                   // Specify from which group to allocate the MCPWM timer
    config.clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT;   // MCPWM timer clock source
    config.resolution_hz = m_timer_resolution_hz;   // Counter resolution in Hz The step size of each count tick equals to (1 / resolution_hz) seconds
    config.count_mode = MCPWM_TIMER_COUNT_MODE_UP;  // Count mode
    config.period_ticks = m_pwm_count_tick;         // Number of count ticks within a period
    config.intr_priority = 0;                       // MCPWM timer interrupt priority, 
                                                    // if set to 0, the driver will try to allocate an interrupt with a relative low priority (1,2,3)
    config.flags.update_period_on_empty = false;    // Whether to update period when timer counts to zero
    config.flags.update_period_on_sync = false;     // Whether to update period on sync event
    
    esp_err_t ret = mcpwm_new_timer(&config, &m_mcpwm_timer);
    if (ret != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to create mcpwm timer (ret: %d)", ret);
        return false;
    }

    mcpwm_timer_event_callbacks_t callbacks;
    callbacks.on_full = on_timer_full;
    callbacks.on_empty = on_timer_empty;
    callbacks.on_stop = on_timer_stop;
    ret = mcpwm_timer_register_event_callbacks(m_mcpwm_timer, &callbacks, this);
    if (ret != ESP_OK) {
        GetLogger(eLogType::Warning)->Log("Failed to register timer event callback (ret: %d)", ret);
    }

    return true;
}

bool CMotorPWMDriver::create_mcpwm_operator()
{
    mcpwm_operator_config_t config;
    config.group_id = m_group_id;                   // Specify from which group to allocate the MCPWM operator
    config.intr_priority = 0;                       // MCPWM operator interrupt priority, 
                                                    // if set to 0, the driver will try to allocate an interrupt with a relative low priority (1,2,3)
    config.flags.update_gen_action_on_tez = true;   // Whether to update generator action when timer counts to zero
    config.flags.update_gen_action_on_tep = false;  // Whether to update generator action when timer counts to peak
    config.flags.update_gen_action_on_sync = false; // Whether to update generator action on sync event
    config.flags.update_dead_time_on_tez = false;   // Whether to update dead time when timer counts to zero
    config.flags.update_dead_time_on_tep = false;   // Whether to update dead time when timer counts to peak
    config.flags.update_dead_time_on_sync = false;  // Whether to update dead time on sync event

    esp_err_t ret = mcpwm_new_operator(&config, &m_mcpwm_operator);
    if (ret != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to create mcpwm operator (ret: %d)", ret);
        return false;
    }

    ret = mcpwm_operator_connect_timer(m_mcpwm_operator, m_mcpwm_timer);
    if (ret != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to connect operator to timer (ret: %d)", ret);
        return false;
    }

    return true;
}

bool CMotorPWMDriver::create_mcpwm_comparator()
{
    mcpwm_comparator_config_t config;
    config.intr_priority = 0;                   // MCPWM comparator interrupt priority, 
                                                // if set to 0, the driver will try to allocate an interrupt with a relative low priority (1,2,3)
    config.flags.update_cmp_on_tez = true;      // Whether to update compare value when timer count equals to zero (tez)
    config.flags.update_cmp_on_tep = false;     // Whether to update compare value when timer count equals to peak (tep)
    config.flags.update_cmp_on_sync = false;    // Whether to update compare value on sync event

    esp_err_t ret = mcpwm_new_comparator(m_mcpwm_operator, &config, &m_mcpwm_comparator);
    if (ret != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to create mcpwm comparator (ret: %d)", ret);
        return false;
    }

    mcpwm_comparator_set_compare_value(m_mcpwm_comparator, 0);

    return true;
}

bool CMotorPWMDriver::create_mcpwm_generator()
{
    mcpwm_generator_config_t config;
    config.gen_gpio_num = m_gpio_num;   // The GPIO number used to output the PWM signal
    config.flags.invert_pwm = false;    // Whether to invert the PWM signal (done by GPIO matrix)
    config.flags.io_loop_back = false;  // For debug/test, the signal output from the GPIO will be fed to the input path as well
    config.flags.io_od_mode = false;    // Configure the GPIO as open-drain mode
    config.flags.pull_up = false;       // Whether to pull up internally
    config.flags.pull_down = true;      // Whether to pull down internally

    esp_err_t ret = mcpwm_new_generator(m_mcpwm_operator, &config, &m_mcpwm_generator);
    if (ret != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to create mcpwm generator (ret: %d)", ret);
        return false;
    }

    mcpwm_generator_set_actions_on_timer_event(
        m_mcpwm_generator, 
        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH),
        MCPWM_GEN_TIMER_EVENT_ACTION_END());
    mcpwm_generator_set_actions_on_compare_event(
        m_mcpwm_generator,
        MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, m_mcpwm_comparator, MCPWM_GEN_ACTION_LOW),
        MCPWM_GEN_COMPARE_EVENT_ACTION_END());
    // mcpwm_generator_set_action_on_brake_event
    // mcpwm_generator_set_action_on_fault_event
    // mcpwm_generator_set_action_on_sync_event

    return true;
}

bool CMotorPWMDriver::release()
{
    bool result = true;
    esp_err_t ret = mcpwm_del_generator(m_mcpwm_generator);
    if (ret != ESP_OK) {
        GetLogger(eLogType::Warning)->Log("Failed to delete mcpwm generator (ret: %d)", ret);
        result = false;
    }

    ret = mcpwm_del_comparator(m_mcpwm_comparator);
    if (ret != ESP_OK) {
        GetLogger(eLogType::Warning)->Log("Failed to delete mcpwm comparator (ret: %d)", ret);
        result = false;
    }

    ret = mcpwm_del_operator(m_mcpwm_operator);
    if (ret != ESP_OK) {
        GetLogger(eLogType::Warning)->Log("Failed to delete mcpwm operator (ret: %d)", ret);
        result = false;
    }

    ret = mcpwm_del_timer(m_mcpwm_timer);
    if (ret != ESP_OK) {
        GetLogger(eLogType::Warning)->Log("Failed to delete mcpwm timer (ret: %d)", ret);
        result = false;
    }

    return result;
}

bool CMotorPWMDriver::set_enable()
{
    esp_err_t ret = mcpwm_timer_enable(m_mcpwm_timer);
    if (ret != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to enable timer (ret: %d)", ret);
        return false;
    }

    ret = mcpwm_timer_start_stop(m_mcpwm_timer, MCPWM_TIMER_START_NO_STOP);
    if (ret != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to start timer (ret: %d)", ret);
        return false;
    }

    return true;
}

bool CMotorPWMDriver::set_disable()
{
    esp_err_t ret = mcpwm_timer_start_stop(m_mcpwm_timer, MCPWM_TIMER_STOP_EMPTY);
    if (ret != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to stop timer (ret: %d)", ret);
        return false;
    }

    ret = mcpwm_timer_disable(m_mcpwm_timer);
    if (ret != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to disable timer (ret: %d)", ret);
        return false;
    }

    return true;
}

bool CMotorPWMDriver::set_duty_ratio(float ratio)
{
    if (ratio < 0.f)
        ratio = 0.f;
    else if (ratio > 100.f)
        ratio = 100.f;
    
    uint32_t tick = (uint32_t)((float)m_pwm_count_tick * ratio / 100.f);
    esp_err_t ret = mcpwm_comparator_set_compare_value(m_mcpwm_comparator, tick);
    if (ret != ESP_OK) {
        GetLogger(eLogType::Error)->Log("Failed to comparator set compare value (ret: %d)", ret);
        return false;
    }

    return true;
}