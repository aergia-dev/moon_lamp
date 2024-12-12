#ifndef BLE_PROTOCOL
#define BLE_PROTOCOL

enum
{
    RESET = 0x0101,
    WRITE_STATUS = 0x0102,
    READ_STATUS = 0x0103,
    TEST_COLOR = 0x0104,
};

typedef struct
{
    uint16_t cmd;
    uint8_t *data;
    uint8_t len;
} handler_req_t;

typedef struct
{
    bool is_success;
    uint8_t *data;
    uint8_t len;
} handler_rsp_t;

typedef struct
{
    bool is_on;
    uint8_t brightness;
    uint8_t reserved_0;
    uint8_t reserved_1;
    uint32_t color; // argb
} led_status_t;

void process_command(handler_req_t *req, handler_rsp_t *rsp);

#endif