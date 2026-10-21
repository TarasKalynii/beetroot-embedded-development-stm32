
#include "app.h"
#include "main.h"
#include "usbd_cdc_if.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// Settings
#define BLINK_INTERVAL_MS 500U
#define DEBOUNCE_MS 50U

// Variables
static char buf[64];

static volatile uint8_t button_flag = 0;

static bool blink_enabled = true;
static uint32_t last_blink_time = 0;

// USB CDC logging
static int serial_print(char *ptr, int len)
{
    CDC_Transmit_FS((uint8_t *)ptr, (uint16_t)len);
    HAL_Delay(1);

    return len;
}

// Called once after hardware initialization
void setup(void)
{
    blink_enabled = true;
    button_flag = 0;

    last_blink_time = HAL_GetTick();

    HAL_GPIO_WritePin(
        LED_GPIO_Port,
        LED_Pin,
        GPIO_PIN_RESET
    );
}

// Called continuously from main()
void loop(void)
{
    // Handle button press
    if (button_flag == 1)
    {
        button_flag = 0;

        // Switch LED mode
        blink_enabled = !blink_enabled;

        if (blink_enabled)
        {
            // Resume blinking
            last_blink_time = HAL_GetTick();

            HAL_GPIO_WritePin(
                LED_GPIO_Port,
                LED_Pin,
                GPIO_PIN_RESET
            );

            int len = snprintf(
                buf,
                sizeof(buf),
                "Button pushed. Mode: BLINK\r\n"
            );

            serial_print(buf, len);
        }
        else
        {
            // LED always ON
            HAL_GPIO_WritePin(
                LED_GPIO_Port,
                LED_Pin,
                GPIO_PIN_SET
            );

            int len = snprintf(
                buf,
                sizeof(buf),
                "Button pushed. Mode: ON\r\n"
            );

            serial_print(buf, len);
        }
    }

    // Non-blocking LED blinking
    if (blink_enabled)
    {
        uint32_t current_time = HAL_GetTick();

        if ((uint32_t)(current_time - last_blink_time) >= BLINK_INTERVAL_MS)
        {
            last_blink_time = current_time;

            HAL_GPIO_TogglePin(
                LED_GPIO_Port,
                LED_Pin
            );
        }
    }
}

// Button interrupt callback
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    static uint32_t last_interrupt_time = 0;

    uint32_t current_time = HAL_GetTick();

    if (GPIO_Pin == BTN_Pin)
    {
        if ((uint32_t)(current_time - last_interrupt_time) > DEBOUNCE_MS)
        {
            button_flag = 1;
            last_interrupt_time = current_time;
        }
    }
}
