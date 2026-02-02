#include "Board/Config.h"
#include "Board/Display.h"

#if defined(I2C_OLED_SDA_PIN)

#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <string>
#include "TaskQueue/TaskQueue.h"
#include "u8g2.h"

// Use hardware I2C for better performance
#define USE_HARDWARE_I2C 1

static u8g2_t u8g2;

// Determine which RP2040 I2C controller the OLED SDA pin maps to
#if (I2C_OLED_SDA_PIN == 2) || \
    (I2C_OLED_SDA_PIN == 6) || \
    (I2C_OLED_SDA_PIN == 10) || \
    (I2C_OLED_SDA_PIN == 14) || \
    (I2C_OLED_SDA_PIN == 15) || \
    (I2C_OLED_SDA_PIN == 18) || \
    (I2C_OLED_SDA_PIN == 26)
    #define I2C_OLED_PORT i2c1
#else
    #define I2C_OLED_PORT i2c0
#endif

#if !USE_HARDWARE_I2C
static uint8_t u8x8_gpio_and_delay_pico_sw(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    (void)u8x8;
    switch (msg) {
        case U8X8_MSG_GPIO_AND_DELAY_INIT:
            gpio_init(I2C_OLED_SDA_PIN);
            gpio_init(I2C_OLED_SCL_PIN);
            gpio_set_dir(I2C_OLED_SDA_PIN, GPIO_OUT);
            gpio_set_dir(I2C_OLED_SCL_PIN, GPIO_OUT);
            gpio_put(I2C_OLED_SDA_PIN, 1);
            gpio_put(I2C_OLED_SCL_PIN, 1);
            return 1;
        case U8X8_MSG_DELAY_MILLI:
            sleep_ms(static_cast<uint32_t>(arg_int));
            return 1;
        case U8X8_MSG_GPIO_I2C_CLOCK:
            gpio_put(I2C_OLED_SCL_PIN, arg_int);
            return 1;
        case U8X8_MSG_GPIO_I2C_DATA:
            gpio_put(I2C_OLED_SDA_PIN, arg_int);
            return 1;
        default:
            return 0;
    }
}
#endif

static uint8_t u8x8_byte_hw_i2c_pico(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    static uint8_t buffer[32];
    static uint8_t buf_idx;
    uint8_t *data;

    switch (msg) {
        case U8X8_MSG_BYTE_INIT:
            // Initialize hardware I2C for OLED
            i2c_init(I2C_OLED_PORT, 400 * 1000); // 400kHz
            gpio_set_function(I2C_OLED_SDA_PIN, GPIO_FUNC_I2C);
            gpio_set_function(I2C_OLED_SCL_PIN, GPIO_FUNC_I2C);
            gpio_pull_up(I2C_OLED_SDA_PIN);
            gpio_pull_up(I2C_OLED_SCL_PIN);
            break;

        case U8X8_MSG_BYTE_START_TRANSFER:
            buf_idx = 0;
            break;

        case U8X8_MSG_BYTE_SEND:
            data = (uint8_t *)arg_ptr;
            for (uint8_t i = 0; i < arg_int; i++) {
                buffer[buf_idx++] = data[i];
            }
            break;

        case U8X8_MSG_BYTE_END_TRANSFER:
            i2c_write_blocking(I2C_OLED_PORT, u8x8_GetI2CAddress(u8x8) >> 1, buffer, buf_idx, false);
            break;

        default:
            return 0;
    }
    return 1;
}

static uint8_t u8x8_gpio_and_delay_pico_hw(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    (void)u8x8;
    (void)arg_ptr;
    
    switch (msg) {
        case U8X8_MSG_GPIO_AND_DELAY_INIT:
            break;
        case U8X8_MSG_DELAY_MILLI:
            sleep_ms(static_cast<uint32_t>(arg_int));
            break;
        case U8X8_MSG_DELAY_10MICRO:
            sleep_us(10);
            break;
        case U8X8_MSG_DELAY_100NANO:
            sleep_us(1); // Minimum delay on Pico
            break;
        default:
            return 0;
    }
    return 1;
}

namespace Display {

void initialize()
{
#if USE_HARDWARE_I2C
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_hw_i2c_pico, u8x8_gpio_and_delay_pico_hw);
#else
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_sw_i2c, u8x8_gpio_and_delay_pico_sw);
#endif
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);

    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
    u8g2_DrawStr(&u8g2, 0, 12, "OGXMini");
    u8g2_SendBuffer(&u8g2);
}

void enable(bool en)
{
    (void)en;
}

void printf(const char* fmt, ...)
{
    (void)fmt;
}

void render_now()
{
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
    u8g2_DrawStr(&u8g2, 0, 12, "OGXMini");
    u8g2_SendBuffer(&u8g2);
}

}

#else // no I2C pins defined - provide no-op implementations so build succeeds

#include <pico/stdlib.h>

namespace Display {

void initialize() { }
void enable(bool) { }
void printf(const char*, ...) { }
void render_now() { }

}

#endif // defined(I2C_OLED_SDA_PIN)
