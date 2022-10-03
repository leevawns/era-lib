#ifndef INC_MVP_PIN_DEFINE_HPP_
#define INC_MVP_PIN_DEFINE_HPP_

#include <stdint.h>

typedef struct __PinConfig_t {
    uint8_t pin;
    uint8_t pinMode;
    unsigned int configId;
    struct __PWMConfig_t {
        __PWMConfig_t()
            : channel(0)
            , resolution(8)
            , frequency(1000)
        {}
        int8_t channel;
        uint8_t resolution;
        uint32_t frequency;
    } pwm;
    struct __ReportConfig_t {
        __ReportConfig_t()
            : interval(100)
            , minInterval(100)
            , maxInterval(60000)
            , reportableChange(1.0f)
        {}
        __ReportConfig_t(unsigned long _interval, unsigned long _minInterval,
                        unsigned long _maxInterval, float _reportableChange)
            : interval(_interval)
            , minInterval(_minInterval)
            , maxInterval(_maxInterval)
            , reportableChange(_reportableChange)
        {}
        unsigned long interval;
        unsigned long minInterval;
        unsigned long maxInterval;
        float reportableChange;
    } report;
} PinConfig_t;

#endif /* INC_MVP_PIN_DEFINE_HPP_ */