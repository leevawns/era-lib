#ifndef INC_ERA_VERSION_HPP_
#define INC_ERA_VERSION_HPP_

#define ERA_MAJOR                   1
#define ERA_MINOR                   2
#define ERA_PATCH                   0

#define ERA_VERSION_NUMBER          10200

#define ERA_VERSION                 "1.2.0"
#define ERA_VERSION_1_2_0

#define ERA_VERSION_VAL(major, minor, patch)   \
            ((major * 10000) + (minor * 100) + (patch))

#if !defined(ERA_FIRMWARE_VERSION)
    #define ERA_FIRMWARE_VERSION    ERA_VERSION
#endif

#endif /* INC_ERA_VERSION_HPP_ */
