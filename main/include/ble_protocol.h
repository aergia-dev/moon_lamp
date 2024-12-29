#ifndef BLE_PROTOCOL
#define BLE_PROTOCOL

enum
{
    RESET = 0x0101,
    WRITE_STATUS = 0x0102,
    READ_STATUS = 0x0103,
    TEST_COLOR = 0x0104,

    WRITE_DEV_NAME = 0x0201,
    WRITE_PASSKEY = 0x0202,
};

typedef struct
{
    uint16_t cmd;
    uint8_t *data;
    uint8_t len;
} handler_req_t;

#define HANDLER_RSP_SZ 1024
typedef struct
{
    bool is_success;
    uint8_t data[HANDLER_RSP_SZ];
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

typedef struct
{
    char dev_name[15];
} write_dev_name_t;

typedef struct
{
    uint32_t passkey;
} write_passkey_t;

void process_command(handler_req_t *req, handler_rsp_t *rsp);

#endif