/*************************************************************
  Download latest ERa library here:
    https://github.com/eoh-jsc/era-lib/releases/latest
    https://www.arduino.cc/reference/en/libraries/era
    https://registry.platformio.org/libraries/eoh-ltd/ERa/installation

    ERa website:                https://e-ra.io
    ERa blog:                   https://iotasia.org
    ERa forum:                  https://forum.eoh.io
    Follow us:                  https://www.fb.com/EoHPlatform
 *************************************************************/

// Enable debug console
// Set CORE_DEBUG_LEVEL = 3 first
// #define ERA_DEBUG
// #define ERA_SERIAL Serial

/* Select ERa host location (VN: Viet Nam, SG: Singapore) */
#define ERA_LOCATION_VN
// #define ERA_LOCATION_SG

/* Define setting button */
// #define BUTTON_PIN              0

#if defined(BUTTON_PIN)
    // Active low (false), Active high (true)
    #define BUTTON_INVERT       false
    #define BUTTON_HOLD_TIMEOUT 5000UL
#endif

#include <Arduino.h>
#include <ERa.hpp>
#if defined(BUTTON_PIN)
    #include <pthread.h>
    #include <ERa/ERaButton.hpp>
#endif

#if defined(BUTTON_PIN)
    ERaButton button;
    pthread_t pthreadButton;

    static void* handlerButton(void* arg) {
        for (;;) {
            button.run();
            ERaDelay(10);
        }
        pthread_exit(NULL);
    }

    static void eventButton(ButtonEventT event) {
        if (event != ButtonEventT::BUTTON_ON_HOLD) {
            return;
        }
        ERa.switchToConfig(true);
    }

    void initButton() {
        pinMode(BUTTON_PIN, INPUT);
        button.setButton(BUTTON_PIN, digitalRead, eventButton,
                        BUTTON_INVERT).onHold(BUTTON_HOLD_TIMEOUT);
        pthread_create(&pthreadButton, NULL, handlerButton, NULL);
    }
#endif

/* This function will run every time ERa is connected */
ERA_CONNECTED() {
    ERA_LOG("ERa", "ERa connected!");
}

/* This function will run every time ERa is disconnected */
ERA_DISCONNECTED() {
    ERA_LOG("ERa", "ERa disconnected!");
}

/* This function print uptime every second */
void timerEvent() {
    ERA_LOG("Timer", "Uptime: %d", ERaMillis() / 1000L);
}

void setup() {
    /* Setup debug console */
#if defined(ERA_DEBUG)
    Serial.begin(115200);
#endif

#if defined(BUTTON_PIN)
    /* Initializing button. */
    initButton();
#endif

    /* Set board id */
    // ERa.setBoardID("Board_1");
    /* Initializing the ERa library. */
    ERa.begin();

    /* Setup timer called function every second */
    ERa.addInterval(1000L, timerEvent);
}

void loop() {
    ERa.run();
}
