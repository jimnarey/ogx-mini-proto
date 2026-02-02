#include "Board/Config.h"
#include "Board/Display.h"

#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <string>
#include "TaskQueue/TaskQueue.h"
#include "u8g2.h"


static u8g2_t u8g2;

static uint8_t u8x8_gpio_and_delay_pico(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    (void)u8x8;
    switch (msg) {
        case U8X8_MSG_GPIO_AND_DELAY_INIT:
            gpio_init(I2C_SDA_PIN);
            gpio_init(I2C_SCL_PIN);
            gpio_set_dir(I2C_SDA_PIN, GPIO_OUT);
            gpio_set_dir(I2C_SCL_PIN, GPIO_OUT);
            gpio_put(I2C_SDA_PIN, 1);
            gpio_put(I2C_SCL_PIN, 1);
            return 1;
        case U8X8_MSG_DELAY_MILLI:
            sleep_ms(static_cast<uint32_t>(arg_int));
            return 1;
        case U8X8_MSG_GPIO_I2C_CLOCK:
            gpio_put(I2C_SCL_PIN, arg_int);
            return 1;
        case U8X8_MSG_GPIO_I2C_DATA:
            gpio_put(I2C_SDA_PIN, arg_int);
            return 1;
        default:
            return 0;
    }
}

namespace Display {

void initialize()
{

    // Change this to use hardware i2c
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_sw_i2c, u8x8_gpio_and_delay_pico);
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
