#ifndef TOUCH_EVENTS_H
#define TOUCH_EVENTS_H

#include <stdint.h>

typedef enum {
    TOUCH_EVENT_SHORT_PRESS,     
    TOUCH_EVENT_LONG_PRESS,      
    TOUCH_EVENT_BRIGHTNESS_STEP,
    TOUCH_EVENT_BRIGHTNESS_EXIT  
} touch_event_type_t;

typedef struct {
    touch_event_type_t type;
    uint32_t duration_ms;        
    uint8_t brightness_level;   
} touch_event_t;

typedef void (*touch_event_callback_t)(touch_event_t *event);

void touch_register_callback(touch_event_callback_t callback);

#endif