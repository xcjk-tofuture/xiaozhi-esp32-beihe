/*
 * SPDX-FileCopyrightText: 2022 atanisoft (github.com/atanisoft)
 *
 * SPDX-License-Identifier: MIT
 */

#include <driver/gpio.h>
#include <esp_check.h>
#include <esp_lcd_panel_commands.h>
#include <esp_lcd_panel_interface.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_log.h>
#include <esp_rom_gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <memory.h>
#include <stdlib.h>
#include <sys/cdefs.h>

static const char *TAG = "ili9486";

typedef struct
{
    uint8_t cmd;
    uint8_t data[16];
    uint8_t data_bytes;
} lcd_init_cmd_t;

typedef struct
{
    esp_lcd_panel_t base;
    esp_lcd_panel_io_handle_t io;
    int reset_gpio_num;
    bool reset_level;
    int x_gap;
    int y_gap;
    uint8_t memory_access_control;
    uint8_t color_mode;
    size_t buffer_size;
    uint8_t *color_buffer;
} ili9486_panel_t;

enum ili9486_constants
{
    ILI9486_POWER_CTL_THREE = 0xC2,
    ILI9486_VCOM_CTL = 0xC5,
    ILI9486_POSITIVE_GAMMA_CTL = 0xE0,
    ILI9486_NEGATIVE_GAMMA_CTL = 0xE1,

    ILI9486_COLOR_MODE_16BIT = 0x55,
    ILI9486_COLOR_MODE_18BIT = 0x66,

    ILI9486_INIT_LENGTH_MASK = 0x1F,
    ILI9486_INIT_DONE_FLAG = 0xFF
};

static esp_err_t panel_ili9486_del(esp_lcd_panel_t *panel)
{
    ili9486_panel_t *ili9486 = __containerof(panel, ili9486_panel_t, base);

    if (ili9486->reset_gpio_num >= 0)
    {
        gpio_reset_pin(ili9486->reset_gpio_num);
    }

    if (ili9486->color_buffer != NULL)
    {
        heap_caps_free(ili9486->color_buffer);
    }

    ESP_LOGI(TAG, "del ili9486 panel @%p", ili9486);
    free(ili9486);
    return ESP_OK;
}

static esp_err_t panel_ili9486_reset(esp_lcd_panel_t *panel)
{
    ili9486_panel_t *ili9486 = __containerof(panel, ili9486_panel_t, base);
    esp_lcd_panel_io_handle_t io = ili9486->io;

    if (ili9486->reset_gpio_num >= 0)
    {
        ESP_LOGI(TAG, "Setting GPIO:%d to %d", ili9486->reset_gpio_num,
                 ili9486->reset_level);
        // perform hardware reset
        gpio_set_level(ili9486->reset_gpio_num, ili9486->reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
        ESP_LOGI(TAG, "Setting GPIO:%d to %d", ili9486->reset_gpio_num,
                 !ili9486->reset_level);
        gpio_set_level(ili9486->reset_gpio_num, !ili9486->reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    else
    {
        ESP_LOGI(TAG, "Sending SW_RESET to display");
        esp_lcd_panel_io_tx_param(io, LCD_CMD_SWRESET, NULL, 0);
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    return ESP_OK;
}

static esp_err_t panel_ili9486_init(esp_lcd_panel_t *panel)
{
    ili9486_panel_t *ili9486 = __containerof(panel, ili9486_panel_t, base);
    esp_lcd_panel_io_handle_t io = ili9486->io;

    lcd_init_cmd_t ili9486_init[] =
    {
         { 0xF1, { 0x36,0x04,0x00,0x3C,0x0F,0x8F }, 6 },
        { 0xF2, { 0x18,0xA3,0x12,0x02,0xB2,0x12,0xFF,0x10,0x00 }, 9 },
         { 0xF8, { 0x21,0x04 }, 2 },
        { 0xF9, { 0x00,0x08 }, 2 },
        { ILI9486_POWER_CTL_THREE, { 0x44 }, 1 },
        { ILI9486_VCOM_CTL, { 0x00, 0x00, 0x00, 0x00 }, 4 },
        {
            ILI9486_POSITIVE_GAMMA_CTL,
            {
                0x0F, 0x1F, 0x1C, 0x0C, 0x0F,
                0x08, 0x48, 0x98, 0x37, 0x0A,
                0x13, 0x04, 0x11, 0x0D, 0x00
            },
            15
            },
        {
            ILI9486_NEGATIVE_GAMMA_CTL,
            {
                0x0F, 0x32, 0x2E, 0x0B, 0x0D,
                0x05, 0x47, 0x75, 0x37, 0x06,
                0x10, 0x03, 0x24, 0x20, 0x00
            },
            15
            },

        { LCD_CMD_MADCTL, { ili9486->memory_access_control }, 1 },
        { LCD_CMD_COLMOD, { ili9486->color_mode }, 1 },
        { LCD_CMD_INVOFF, { 0 }, 0 },
        { LCD_CMD_NOP, { 0 }, ILI9486_INIT_DONE_FLAG },
    };

    ESP_LOGI(TAG, "Initializing ILI9486");
    int cmd = 0;
    while (ili9486_init[cmd].data_bytes != ILI9486_INIT_DONE_FLAG)
    {
        ESP_LOGD(TAG, "Sending CMD: %02x, len: %d", ili9486_init[cmd].cmd,
                 ili9486_init[cmd].data_bytes & ILI9486_INIT_LENGTH_MASK);
        esp_lcd_panel_io_tx_param(
            io, ili9486_init[cmd].cmd, ili9486_init[cmd].data,
            ili9486_init[cmd].data_bytes & ILI9486_INIT_LENGTH_MASK);
        cmd++;
    }

    // Take the display out of sleep mode.
    esp_lcd_panel_io_tx_param(io, LCD_CMD_SLPOUT, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(120));

    // Turn on the display.
    esp_lcd_panel_io_tx_param(io, LCD_CMD_DISPON, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_LOGI(TAG, "Initialization complete");

    return ESP_OK;
}

#define SEND_COORDS(start, end, io, cmd)                \
    esp_lcd_panel_io_tx_param(io, cmd, (uint8_t[]) {    \
        (start >> 8) & 0xFF,                            \
        start & 0xFF,                                   \
        ((end - 1) >> 8) & 0xFF,                        \
        (end - 1) & 0xFF,                               \
    }, 4)

static esp_err_t panel_ili9486_draw_bitmap(
    esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end,
    const void *color_data)
{
    ili9486_panel_t *ili9486 = __containerof(panel, ili9486_panel_t, base);
    assert((x_start < x_end) && (y_start < y_end) &&
            "starting position must be smaller than end position");
    esp_lcd_panel_io_handle_t io = ili9486->io;

    x_start += ili9486->x_gap;
    x_end += ili9486->x_gap;
    y_start += ili9486->y_gap;
    y_end += ili9486->y_gap;

    size_t color_data_len = (x_end - x_start) * (y_end - y_start);

    SEND_COORDS(x_start, x_end, io, LCD_CMD_CASET);
    SEND_COORDS(y_start, y_end, io, LCD_CMD_RASET);

    // When the ILI9486 is used in 18-bit color mode we need to convert the
    // incoming color data from RGB565 (16-bit) to RGB666.
    if (ili9486->color_mode == ILI9486_COLOR_MODE_18BIT)
    {
        uint8_t *buf = ili9486->color_buffer;
        uint16_t *raw_color_data = (uint16_t *) color_data;
        for (uint32_t i = 0, pixel_index = 0; i < color_data_len; i++)
        {
            buf[pixel_index++] = (uint8_t) (((raw_color_data[i] & 0xF800) >> 8) |
                                            ((raw_color_data[i] & 0x8000) >> 13));
            buf[pixel_index++] = (uint8_t) ((raw_color_data[i] & 0x07E0) >> 3);
            buf[pixel_index++] = (uint8_t) (((raw_color_data[i] & 0x001F) << 3) |
                                            ((raw_color_data[i] & 0x0010) >> 2));
        }

        esp_lcd_panel_io_tx_color(io, LCD_CMD_RAMWR, buf, color_data_len * 3);
    }
    else
    {
        esp_lcd_panel_io_tx_color(io, LCD_CMD_RAMWR, color_data, color_data_len * 2);
    }

    return ESP_OK;
}

#undef SEND_COORDS

static esp_err_t panel_ili9486_invert_color(
    esp_lcd_panel_t *panel, bool invert_color_data)
{
    ili9486_panel_t *ili9486 = __containerof(panel, ili9486_panel_t, base);
    esp_lcd_panel_io_handle_t io = ili9486->io;

    if (invert_color_data)
    {
        esp_lcd_panel_io_tx_param(io, LCD_CMD_INVON, NULL, 0);
    }
    else
    {
        esp_lcd_panel_io_tx_param(io, LCD_CMD_INVOFF, NULL, 0);
    }

    return ESP_OK;
}

static esp_err_t panel_ili9486_mirror(
    esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y)
{
    ili9486_panel_t *ili9486 = __containerof(panel, ili9486_panel_t, base);
    esp_lcd_panel_io_handle_t io = ili9486->io;
    if (mirror_x)
    {
        ili9486->memory_access_control &= ~LCD_CMD_MX_BIT;
    }
    else
    {
        ili9486->memory_access_control |= LCD_CMD_MX_BIT;
    }
    if (mirror_y)
    {
        ili9486->memory_access_control |= LCD_CMD_MY_BIT;
    }
    else
    {
        ili9486->memory_access_control &= ~LCD_CMD_MY_BIT;
    }
    esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, &ili9486->memory_access_control, 1);
    return ESP_OK;
}

static esp_err_t panel_ili9486_swap_xy(esp_lcd_panel_t *panel, bool swap_axes)
{
    ili9486_panel_t *ili9486 = __containerof(panel, ili9486_panel_t, base);
    esp_lcd_panel_io_handle_t io = ili9486->io;
    if (swap_axes)
    {
        ili9486->memory_access_control |= LCD_CMD_MV_BIT;
    }
    else
    {
        ili9486->memory_access_control &= ~LCD_CMD_MV_BIT;
    }
    esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, &ili9486->memory_access_control, 1);
    return ESP_OK;
}

static esp_err_t panel_ili9486_set_gap(
    esp_lcd_panel_t *panel, int x_gap, int y_gap)
{
    ili9486_panel_t *ili9486 = __containerof(panel, ili9486_panel_t, base);
    ili9486->x_gap = x_gap;
    ili9486->y_gap = y_gap;
    return ESP_OK;
}

static esp_err_t panel_ili9486_disp_on_off(esp_lcd_panel_t *panel, bool on_off)
{
    ili9486_panel_t *ili9486 = __containerof(panel, ili9486_panel_t, base);
    esp_lcd_panel_io_handle_t io = ili9486->io;

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
    on_off = !on_off;
#endif

    if (on_off)
    {
        esp_lcd_panel_io_tx_param(io, LCD_CMD_DISPON, NULL, 0);
    }
    else
    {
        esp_lcd_panel_io_tx_param(io, LCD_CMD_DISPOFF, NULL, 0);
    }

    vTaskDelay(pdMS_TO_TICKS(100));

    return ESP_OK;
}

esp_err_t esp_lcd_new_panel_ili9486(
    const esp_lcd_panel_io_handle_t io,
    const esp_lcd_panel_dev_config_t *panel_dev_config,
    const size_t buffer_size,
    esp_lcd_panel_handle_t *ret_panel)
{
    esp_err_t ret = ESP_OK;
    ili9486_panel_t *ili9486 = NULL;
    ESP_GOTO_ON_FALSE(io && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG,
                      err, TAG, "invalid argument");
    ili9486 = (ili9486_panel_t *)(calloc(1, sizeof(ili9486_panel_t)));
    ESP_GOTO_ON_FALSE(ili9486, ESP_ERR_NO_MEM, err, TAG, "no mem for ili9486 panel");

    if (panel_dev_config->reset_gpio_num >= 0)
    {
        gpio_config_t cfg;
        memset(&cfg, 0, sizeof(gpio_config_t));
        esp_rom_gpio_pad_select_gpio(panel_dev_config->reset_gpio_num);
        cfg.pin_bit_mask = BIT64(panel_dev_config->reset_gpio_num);
        cfg.mode = GPIO_MODE_OUTPUT;
        ESP_GOTO_ON_ERROR(gpio_config(&cfg), err, TAG,
                          "configure GPIO for RESET line failed");
    }

    if (panel_dev_config->bits_per_pixel == 16)
    {
        ili9486->color_mode = ILI9486_COLOR_MODE_16BIT;
    }
    else
    {
        ESP_GOTO_ON_FALSE(buffer_size > 0, ESP_ERR_INVALID_ARG, err, TAG,
                          "Color conversion buffer size must be specified");
        ili9486->color_mode = ILI9486_COLOR_MODE_18BIT;

        // Allocate DMA buffer for color conversions
        ili9486->color_buffer =
            (uint8_t *)heap_caps_malloc(buffer_size * 3, MALLOC_CAP_DMA);
        ESP_GOTO_ON_FALSE(ili9486->color_buffer, ESP_ERR_NO_MEM, err, TAG,
                          "Failed to allocate DMA color conversion buffer");
    }

    ili9486->memory_access_control = LCD_CMD_MX_BIT | LCD_CMD_BGR_BIT;

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(6, 0, 0)
    switch (panel_dev_config->color_space)
    {
        case ESP_LCD_COLOR_SPACE_RGB:
            ESP_LOGI(TAG, "Configuring for RGB color order");
            ili9486->memory_access_control &= ~LCD_CMD_BGR_BIT;
            break;
        case ESP_LCD_COLOR_SPACE_BGR:
            ESP_LOGI(TAG, "Configuring for BGR color order");
            break;
        default:
            ESP_GOTO_ON_FALSE(false, ESP_ERR_INVALID_ARG, err, TAG,
                              "Unsupported color mode!");
    }
#else
    switch (panel_dev_config->rgb_ele_order)
    {
        case LCD_RGB_ELEMENT_ORDER_RGB:
            ESP_LOGI(TAG, "Configuring for RGB color order");
            ili9486->memory_access_control &= ~LCD_CMD_BGR_BIT;
            break;
        case LCD_RGB_ELEMENT_ORDER_BGR:
            ESP_LOGI(TAG, "Configuring for BGR color order");
            break;
        default:
            ESP_GOTO_ON_FALSE(false, ESP_ERR_INVALID_ARG, err, TAG,
                              "Unsupported color mode!");
    }
#endif

    ili9486->io = io;
    ili9486->reset_gpio_num = panel_dev_config->reset_gpio_num;
    ili9486->reset_level = panel_dev_config->flags.reset_active_high;
    ili9486->base.del = panel_ili9486_del;
    ili9486->base.reset = panel_ili9486_reset;
    ili9486->base.init = panel_ili9486_init;
    ili9486->base.draw_bitmap = panel_ili9486_draw_bitmap;
    ili9486->base.invert_color = panel_ili9486_invert_color;
    ili9486->base.set_gap = panel_ili9486_set_gap;
    ili9486->base.mirror = panel_ili9486_mirror;
    ili9486->base.swap_xy = panel_ili9486_swap_xy;
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
    ili9486->base.disp_off = panel_ili9486_disp_on_off;
#else
    ili9486->base.disp_on_off = panel_ili9486_disp_on_off;
#endif
    *ret_panel = &(ili9486->base);
    ESP_LOGI(TAG, "new ili9486 panel @%p", ili9486);

    return ESP_OK;

err:
    if (ili9486)
    {
        if (panel_dev_config->reset_gpio_num >= 0)
        {
            gpio_reset_pin(panel_dev_config->reset_gpio_num);
        }
        if (ili9486->color_buffer != NULL)
        {
            heap_caps_free(ili9486->color_buffer);
        }
        free(ili9486);
    }
    return ret;
}
