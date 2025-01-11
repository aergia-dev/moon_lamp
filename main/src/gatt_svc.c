/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* Includes */
#include "gatt_svc.h"
#include "common.h"
#include "embedded_led.h"
#include "ble_protocol.h"
#include "esp_ota_ops.h"

/* Private function declarations */
static int sleep_light_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                                  struct ble_gatt_access_ctxt *ctxt, void *arg);

static handler_rsp_t rsp = {
    .is_success = false,
    .data = {
        0,
    },
    .len = 0,
};

// light service
static const ble_uuid128_t sleep_light_svc_uuid =
    BLE_UUID128_INIT(0xA1, 0xB2, 0xC3, 0xD4,
                     0x00, 0x00, 0x10, 0x00,
                     0x80, 0x00, 0x00, 0x80,
                     0x5F, 0x9B, 0x34, 0xFB);

static uint16_t sleep_light_chr_val_handle;

static const ble_uuid128_t sleep_light_chr_uuid =
    BLE_UUID128_INIT(0xA1, 0xB2, 0xC3, 0xD5,
                     0x00, 0x00, 0x10, 0x00,
                     0x80, 0x00, 0x00, 0x80,
                     0x5F, 0x9B, 0x34, 0xFB);

static uint16_t sleep_light_chr_conn_handle = 0;
static bool sleep_light_chr_conn_handle_inited = false;
static bool sleep_light_ind_status = false;
// OTA service
static const ble_uuid128_t gatt_svr_svc_ota_uuid =
    BLE_UUID128_INIT(0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15,
                     0xDE, 0xEF, 0x12, 0x12, 0x30, 0x15, 0x00, 0x00);

static const ble_uuid128_t gatt_svr_chr_ota_control_uuid =
    BLE_UUID128_INIT(0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15,
                     0xDE, 0xEF, 0x12, 0x12, 0x30, 0x15, 0x01, 0x00);

static const ble_uuid128_t gatt_svr_chr_ota_data_uuid =
    BLE_UUID128_INIT(0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15,
                     0xDE, 0xEF, 0x12, 0x12, 0x30, 0x15, 0x02, 0x00);

static esp_ota_handle_t ota_handle = 0;
static const esp_partition_t *update_partition = NULL;
static bool ota_in_progress = false;

enum ota_control
{
    OTA_START = 0x01,
    OTA_DATA = 0x02,
    OTA_END = 0x03,
};

static int gatt_svr_chr_ota_control_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR)
    {
        uint8_t command;
        memcpy(&command, ctxt->om->om_data, sizeof(command));

        switch (command)
        {
        case OTA_START:
            if (!ota_in_progress)
            {
                update_partition = esp_ota_get_next_update_partition(NULL);
                ESP_ERROR_CHECK(esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle));
                ota_in_progress = true;
                ESP_LOGI(TAG, "OTA start");
            }
            break;
        case OTA_END:
            if (ota_in_progress)
            {
                ESP_ERROR_CHECK(esp_ota_end(ota_handle));
                ESP_ERROR_CHECK(esp_ota_set_boot_partition(update_partition));
                ota_in_progress = false;
                ESP_LOGI(TAG, "OTA end");
                esp_restart();
            }
        }
    }
    return 0;
}

static int gatt_svr_chr_ota_data_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR)
    {
        if (ota_in_progress)
        {
            ESP_ERROR_CHECK(esp_ota_write(ota_handle, ctxt->om->om_data, OS_MBUF_PKTLEN(ctxt->om)));
            ESP_LOGI(TAG, "OTA: Recevied %d bytes", OS_MBUF_PKTLEN(ctxt->om));
        }
    }
    return 0;
}

/* GATT services table */
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = &sleep_light_svc_uuid.u,
     .characteristics =
         (struct ble_gatt_chr_def[]){
             {.uuid = &sleep_light_chr_uuid.u,
              .access_cb = sleep_light_chr_access,
              .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
              //  | BLE_GATT_CHR_F_READ_ENC |
              // BLE_GATT_CHR_F_WRITE_ENC,
              .val_handle = &sleep_light_chr_val_handle},
             {
                 0, /* No more characteristics in this service. */
             }}},
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &gatt_svr_svc_ota_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]){
            {
                .uuid = &gatt_svr_chr_ota_control_uuid.u,
                .access_cb = gatt_svr_chr_ota_control_cb,
                .flags = BLE_GATT_CHR_F_WRITE,
            },
            {
                .uuid = &gatt_svr_chr_ota_data_uuid.u,
                .access_cb = gatt_svr_chr_ota_data_cb,
                .flags = BLE_GATT_CHR_F_WRITE,
            },
            {
                0, /* No more characteristics in this service. */
            },
        },
    },
    {
        0, /* No more services. */
    },
};

/* Private functions */
static int sleep_light_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                                  struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    /* Local variables */
    int rc;

    /* Handle access events */
    switch (ctxt->op)
    {

    /* Read characteristic event */
    case BLE_GATT_ACCESS_OP_READ_CHR:
        /* Verify connection handle */
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE)
        {
            ESP_LOGI(TAG, "characteristic read; conn_handle=%d attr_handle=%d",
                     conn_handle, attr_handle);
        }
        else
        {
            ESP_LOGI(TAG, "characteristic read by nimble stack; attr_handle=%d",
                     attr_handle);
        }

        /* Verify attribute handle */
        if (attr_handle == sleep_light_chr_val_handle)
        {
            ESP_LOGI(TAG, "read - attr_handle == sleep_light_chr_val_handle");
            ESP_LOGI(TAG, "received: %x, %x, %x, %x", (int)ctxt->om->om_data[0], (int)ctxt->om->om_data[1], (int)ctxt->om->om_data[2], (int)ctxt->om->om_data[3]);

            if (ctxt->om->om_data[0] == 0x0B && ctxt->om->om_data[1] == 0x00)
            {
                handler_req_t req = {
                    .cmd = ctxt->om->om_data[2] << 8 | ctxt->om->om_data[3],
                    .data = &ctxt->om->om_data[4],
                    .len = ctxt->om->om_len - 2,
                };

                if (req.cmd == READ_STATUS)
                {
                    ESP_LOGI(TAG, "read status");
                    rc = os_mbuf_append(ctxt->om, &rsp.is_success,
                                        1);
                    rc = os_mbuf_append(ctxt->om, &rsp.data,
                                        rsp.len);
                }
                else
                {
                    ESP_LOGE(TAG, "cmd is not support %x", req.cmd);
                    rc = 0;
                }
            }
            else
            {
                // int result = 0;
                // rc = os_mbuf_append(ctxt->om, &result,
                // 1);
                rc = 0;
                ESP_LOGE(TAG, "read cmd should be 0x0B00xxxx 0x%x%x", ctxt->om->om_data[0], ctxt->om->om_data[1]);
            }

            ESP_LOGI(TAG, "read - rc: %d", rc);

            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        goto error;
    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE)
        {
            ESP_LOGI(TAG, "characteristic write; conn_handle=%d attr_handle=%d",
                     conn_handle, attr_handle);
        }
        else
        {
            ESP_LOGI(TAG, "characteristic write by nimble stack; attr_handle=%d",
                     attr_handle);
        }

        /* Verify attribute handle */
        if (attr_handle == sleep_light_chr_val_handle)
        {
            ESP_LOGI(TAG, "write - attr_handle == sleep_light_chr_val_handle");
            ESP_LOGI(TAG, "write received: %x, %x, %x, %x", (int)ctxt->om->om_data[0], (int)ctxt->om->om_data[1], (int)ctxt->om->om_data[2], (int)ctxt->om->om_data[3]);

            handler_req_t req = {
                .cmd = ctxt->om->om_data[0] << 8 | ctxt->om->om_data[1],
                .data = &ctxt->om->om_data[2],
                .len = ctxt->om->om_len - 2,
            };

            process_command(&req, &rsp);

            ESP_LOGI(TAG, "rsp->len: %d", rsp.len);

            rc = os_mbuf_append(ctxt->om, &rsp.is_success,
                                1);

            ESP_LOGI(TAG, "write - rc: %d", rc);

            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }

        goto error;
    /* Unknown event */
    default:
        goto error;
    }

error:
    ESP_LOGE(
        TAG,
        "unexpected access operation to light cont characteristic, opcode: %d",
        ctxt->op);
    return BLE_ATT_ERR_UNLIKELY;
}

/* Public functions */
void sleep_light_indication(void)
{
    if (sleep_light_ind_status && sleep_light_chr_conn_handle_inited)
    {
        ble_gatts_indicate(sleep_light_chr_conn_handle,
                           sleep_light_chr_val_handle);
        ESP_LOGI(TAG, "sleep light indication sent!");
    }
}

/*
 *  Handle GATT attribute register events
 *      - Service register event
 *      - Characteristic register event
 *      - Descriptor register event
 */
void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg)
{
    /* Local variables */
    char buf[BLE_UUID_STR_LEN];

    /* Handle GATT attributes register events */
    switch (ctxt->op)
    {

    /* Service register event */
    case BLE_GATT_REGISTER_OP_SVC:
        ESP_LOGD(TAG, "registered service %s with handle=%d",
                 ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                 ctxt->svc.handle);
        break;

    /* Characteristic register event */
    case BLE_GATT_REGISTER_OP_CHR:
        ESP_LOGD(TAG,
                 "registering characteristic %s with "
                 "def_handle=%d val_handle=%d",
                 ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                 ctxt->chr.def_handle, ctxt->chr.val_handle);
        break;

    /* Descriptor register event */
    case BLE_GATT_REGISTER_OP_DSC:
        ESP_LOGD(TAG, "registering descriptor %s with handle=%d",
                 ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                 ctxt->dsc.handle);
        break;

    /* Unknown event */
    default:
        assert(0);
        break;
    }
}

/*
 *  GATT server subscribe event callback
 */

void gatt_svr_subscribe_cb(struct ble_gap_event *event)
{
    /* Check connection handle */
    if (event->subscribe.conn_handle != BLE_HS_CONN_HANDLE_NONE)
    {
        ESP_LOGI(TAG, "subscribe event; conn_handle=%d attr_handle=%d",
                 event->subscribe.conn_handle, event->subscribe.attr_handle);
    }
    else
    {
        ESP_LOGI(TAG, "subscribe by nimble stack; attr_handle=%d",
                 event->subscribe.attr_handle);
    }

    /* Check attribute handle */
    if (event->subscribe.attr_handle == sleep_light_chr_val_handle)
    {
        sleep_light_chr_conn_handle = event->subscribe.conn_handle;
        sleep_light_chr_conn_handle_inited = true;
        sleep_light_ind_status = event->subscribe.cur_indicate;
    }
}

/*
 *  GATT server initialization
 *      1. Initialize GATT service
 *      2. Update NimBLE host GATT services counter
 *      3. Add GATT services to server
 */
int gatt_svc_init(void)
{
    /* Local variables */
    int rc;

    /* 1. GATT service initialization */
    ble_svc_gatt_init();

    /* 2. Update GATT services counter */
    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0)
    {
        return rc;
    }

    /* 3. Add GATT services */
    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0)
    {
        return rc;
    }

    return 0;
}