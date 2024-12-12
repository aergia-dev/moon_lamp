
#include "led_strip_encoder.h"
#include "sleep_light.h"
#include "esp_log.h"
#include "nvs_storage.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/rmt_tx.h"
#include "freertos/FreeRTOS.h"
#include "nvs_storage.h"

static const char *TAG = "sleep-light";
#define LED_CNT 15 // 23
#define CONT_STEP 20
#define LED_GPIO_NUM 21
int using_led_cnt = 0;
ARGB current_color;

#define RMT_LED_STRIP_RESOLUTION_HZ 10000000 // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
static bool light_state = false;
static uint8_t led_strip_pixels[LED_CNT * 3];
static rmt_encoder_handle_t led_encoder = NULL;
static led_strip_encoder_config_t encoder_config = {
    .resolution = RMT_LED_STRIP_RESOLUTION_HZ,
};
static rmt_transmit_config_t tx_config = {
    .loop_count = 0, // no transfer loop
};
static rmt_channel_handle_t led_chan = NULL;

static uint8_t brightness = 100;

uint8_t get_brightness(void)
{
    return brightness;
}

void set_brightness(uint8_t v)
{
    brightness = v;
}

bool get_light_state(void)
{
    return light_state;
}
void set_light_state(bool is_on)
{
    light_state = is_on;
}

uint32_t get_saved_color_uint32(void)
{
    ARGB color = {.code = read_color_nvs()};
    return color.code;
}

void update_led_strip()
{
    ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_strip_pixels, sizeof(led_strip_pixels), &tx_config));
    ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));
}

void change_color(ARGB color, int led_cnt)
{
    for (int j = 0; j < led_cnt * 3; j += 3)
    {
        led_strip_pixels[j + 0] = color.argb.green;
        led_strip_pixels[j + 1] = color.argb.red;
        led_strip_pixels[j + 2] = color.argb.blue;
    }
    update_led_strip();
}

void change_color_uint32(uint32_t c)
{
    ARGB color = {.code = c};
    change_color(color, LED_CNT);
}
void limit_val(ARGB *dst, const ARGB_float *src, ARGB limit, bool is_inc)
{
#define UPPER_LIMIT(x, limit) ((x > limit) ? limit : x)
#define LOWER_LIMIT(x, limit) ((x < limit) ? limit : x)

    if (is_inc)
    {
        dst->argb.red = UPPER_LIMIT(src->red, limit.argb.red);
        dst->argb.blue = UPPER_LIMIT(src->blue, limit.argb.blue);
        dst->argb.green = UPPER_LIMIT(src->green, limit.argb.green);
        dst->argb.alpha = UPPER_LIMIT(src->alpha, limit.argb.alpha);
    }
    else
    {
        dst->argb.red = LOWER_LIMIT(src->red, limit.argb.red);
        dst->argb.blue = LOWER_LIMIT(src->blue, limit.argb.blue);
        dst->argb.green = LOWER_LIMIT(src->green, limit.argb.green);
        dst->argb.alpha = LOWER_LIMIT(src->alpha, limit.argb.alpha);
    }
}

void light_chage_color_dimming(const int step, const int duration, ARGB from_color, ARGB to_color, bool is_turn_on)
{
    const int DELAY_TIME = duration / step;
    const int MARGIN = step * 20 / 100;
    ARGB_float argb_step = {.red = 0.0, .blue = 0.0, .green = 0, .alpha = 0};

    argb_step.red = (to_color.argb.red - from_color.argb.red) / (float)step;
    argb_step.blue = (to_color.argb.blue - from_color.argb.blue) / (float)step;
    argb_step.green = (to_color.argb.green - from_color.argb.green) / (float)step;
    argb_step.alpha = (to_color.argb.alpha - from_color.argb.alpha) / (float)step;

    ESP_LOGI(TAG, "from: r: %d, g: %d, b: %d", from_color.argb.red, from_color.argb.green, from_color.argb.blue);
    ESP_LOGI(TAG, "to: r: %d, g: %d, b: %d", to_color.argb.red, to_color.argb.green, to_color.argb.blue);
    ESP_LOGI(TAG, "step - r: %f, g: %f, b: %f", argb_step.red, argb_step.green, argb_step.blue);

    ARGB_float argb_accum = {
        .red = (float)from_color.argb.red,
        .blue = (float)from_color.argb.blue,
        .green = (float)from_color.argb.green,
        .alpha = (float)from_color.argb.alpha};

    ARGB cur_color;
    cur_color.code = from_color.code;

    for (int i = 1; i <= step + MARGIN; i++)
    {
        argb_accum.red += argb_step.red;
        argb_accum.blue += argb_step.blue;
        argb_accum.green += argb_step.green;
        argb_accum.alpha += argb_step.alpha;

        limit_val(&cur_color, &argb_accum, to_color, is_turn_on);

        change_color(cur_color, LED_CNT);
        esp_rom_delay_us(DELAY_TIME);
    }

    change_color(to_color, LED_CNT);
}

void light_on()
{
    ARGB to_color = {.code = read_color_nvs()};
    change_color(to_color, LED_CNT);
    light_state = true;
}

void light_off()
{
    ARGB color = {.code = Black};
    change_color(color, LED_CNT);
    light_state = false;
}

void light_on_dimming()
{
    ARGB to_color = {.code = read_color_nvs()};
    ARGB from_color = {.code = 0};

    const int step = 100;
    const int duration_us = 500000;
    light_chage_color_dimming(step, duration_us, from_color, to_color, true);
    light_state = true;
}

void light_off_dimming()
{
    ARGB from_color = {.code = 0};
    from_color.code = read_color_nvs();
    ARGB to_color = {.code = 0};

    to_color.code = Black;
    const int step = 100;
    const int duration_us = 500000;
    light_chage_color_dimming(step, duration_us, from_color, to_color, false);
    light_state = false;
}

static void cont_brightness(uint8_t brightness)
{
}

bool ble_cont_light(led_status_t *status)
{
    if (status->is_on)
    {
        light_off_dimming();
    }
    else
    {
        light_on_dimming();
    }
    if (status->brightness > 0)
    {
        cont_brightness(status->brightness);
    }

    if (status->color > 0)
    {
        ARGB color = {.code = status->color};
        change_color(color, LED_CNT);
    }

    return true;
}

void toggle_light()
{
    if (light_state)
    {
        light_off_dimming();
    }
    else
    {
        light_on_dimming();
    }
}

// void darker_light()
// {
//     int cnt = using_led_cnt - CONT_STEP;

//     if(cnt < 0)
//         cnt = 0;

//     using_led_cnt = cnt;

//     ARGB color;
//     color.code = White;

//     printf("less light : %d\n", using_led_cnt);
//     light_chage_color(color, cnt);
// }

// void brighter_light()
// {
//     int cnt = using_led_cnt + CONT_STEP;

//     if(cnt > LED_CNT - 1 )
//         cnt = LED_CNT -1;

//     using_led_cnt = cnt;

//     printf("more light : %d\n", using_led_cnt);
//     ARGB color;
//     color.code = White;

//     light_chage_color(color, cnt);
// }

void light_init()
{
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT, // select source clock
        .gpio_num = LED_GPIO_NUM,
        .mem_block_symbols = 64, // increase the block size can make the LED less flickering
        .resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ,
        .trans_queue_depth = 4, // set the number of transactions that can be pending in the background
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));

    ESP_LOGI(TAG, "Install led strip encoder");
    ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &led_encoder));

    ESP_LOGI(TAG, "Enable RMT TX channel");
    ESP_ERROR_CHECK(rmt_enable(led_chan));
}

// void get_current_color(uint8_t* color)
// {
//     color[0] = current_color.argb.alpha;
//     color[1] = current_color.argb.red;
//     color[2] = current_color.argb.green;
//     color[3] = current_color.argb.blue;
// }

// bool get_light_on_off()
// {
//     return light_state;
// }

// ARGB fromRGB(uint8_t r, uint8_t g, uint8_t b)
// {
//     ARGB color = {.argb.alpha=0, .argb.red=r, .argb.blue=b, .argb.green=g, };

//     return color;
// }

void led_strip_hsv2rgb(uint32_t h, uint32_t s, uint32_t v, uint32_t *r, uint32_t *g, uint32_t *b)
{
    h %= 360; // h -> [0,360]
    uint32_t rgb_max = v * 2.55f;
    uint32_t rgb_min = rgb_max * (100 - s) / 100.0f;

    uint32_t i = h / 60;
    uint32_t diff = h % 60;

    // RGB adjustment amount by hue
    uint32_t rgb_adj = (rgb_max - rgb_min) * diff / 60;

    switch (i)
    {
    case 0:
        *r = rgb_max;
        *g = rgb_min + rgb_adj;
        *b = rgb_min;
        break;
    case 1:
        *r = rgb_max - rgb_adj;
        *g = rgb_max;
        *b = rgb_min;
        break;
    case 2:
        *r = rgb_min;
        *g = rgb_max;
        *b = rgb_min + rgb_adj;
        break;
    case 3:
        *r = rgb_min;
        *g = rgb_max - rgb_adj;
        *b = rgb_max;
        break;
    case 4:
        *r = rgb_min + rgb_adj;
        *g = rgb_min;
        *b = rgb_max;
        break;
    default:
        *r = rgb_max;
        *g = rgb_min;
        *b = rgb_max - rgb_adj;
        break;
    }
}
