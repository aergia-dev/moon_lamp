#ifndef LED_EVENT_LOOP
#define LED_EVENT_LOOP


#include "esp_event.h"
#include "esp_timer.h"


#ifdef __cplusplus
extern "C" {
#endif

#define TASK_ITERATIONS_COUNT 10
#define TASK_PERIOD 500

ESP_EVENT_DECLARE_BASE(TASK_EVENTS);

enum {
    TASK_ITERATION_EVENT
};


void reg_led_event(void);
#ifdef __cplusplus
}
#endif

#endif