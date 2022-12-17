#ifndef INC_ERA_ZIGBEE_CONFIG_HPP_
#define INC_ERA_ZIGBEE_CONFIG_HPP_

#ifndef ZIGBEE_BAUDRATE
    #define ZIGBEE_BAUDRATE     115200
#endif

#ifndef ZIGBEE_RXD_Pin
    #if defined(ARDUINO_ESP32C3_DEV)
        #define ZIGBEE_RXD_Pin      6
    #elif defined(ARDUINO_ESP32S2_DEV)
        #define ZIGBEE_RXD_Pin      21
    #elif defined(ARDUINO_ESP32S3_DEV)
        #define ZIGBEE_RXD_Pin      5
    #elif defined(ARDUINO_ESP32_S3_BOX)
        #define ZIGBEE_RXD_Pin      5
    #elif defined(ARDUINO_ARCH_ESP32)
        #define ZIGBEE_RXD_Pin      32
    #else
        #define ZIGBEE_RXD_Pin      32
    #endif
#endif

#ifndef ZIGBEE_TXD_Pin
    #if defined(ARDUINO_ESP32C3_DEV)
        #define ZIGBEE_TXD_Pin      7
    #elif defined(ARDUINO_ESP32S2_DEV)
        #define ZIGBEE_TXD_Pin      17
    #elif defined(ARDUINO_ESP32S3_DEV)
        #define ZIGBEE_TXD_Pin      4
    #elif defined(ARDUINO_ESP32_S3_BOX)
        #define ZIGBEE_TXD_Pin      4
    #elif defined(ARDUINO_ARCH_ESP32)
        #define ZIGBEE_TXD_Pin      33
    #else
        #define ZIGBEE_TXD_Pin      33
    #endif
#endif

#define ZIGBEE_BUFFER_SIZE      1024

#if !defined(ERA_ZIGBEE_YIELD)
    #if !defined(ERA_ZIGBEE_YIELD_MS)
        #define ERA_ZIGBEE_YIELD_MS 10
    #endif
    #if !defined(ERA_NO_YIELD)
        #define ERA_ZIGBEE_YIELD() { ERaDelay(ERA_ZIGBEE_YIELD_MS); }
    #else
        #define ERA_ZIGBEE_YIELD() {}
    #endif
#endif

#endif /* INC_ERA_ZIGBEE_CONFIG_HPP_ */
