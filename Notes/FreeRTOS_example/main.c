// Program FreeRTOSExample
  // blinking LED dengan membuat task baru

// referensi :
  // https://github.com/Infineon/freertos/blob/master/README.md
  // https://github.com/Infineon/freertos/blob/master/Source/portable/COMPONENT_CM4/FreeRTOSConfig.h

// Include the required header files:
#include "cyhal.h"
#include "cybsp.h"
#include "FreeRTOS.h"
#include "task.h"

// Specify the GPIO port and pin for the LED:
#define LED_GPIO (CYBSP_USER_LED)

// Add the function to toggle the LED:
void blinky(void * arg)
{
    (void)arg;

    /* Initialize the LED GPIO pin */
    cyhal_gpio_init(LED_GPIO, CYHAL_GPIO_DIR_OUTPUT,
                    CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);

    for(;;)
    {
        /* Toggle the LED periodically */
        cyhal_gpio_toggle(LED_GPIO);
        vTaskDelay(500);
    }
}

// Create a task using the previously added LED toggle function and start the task scheduler:

int main(void)
{
    BaseType_t retval;
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    __enable_irq();

    retval = xTaskCreate(blinky, "blinky", configMINIMAL_STACK_SIZE, NULL, 5, NULL);

    if (pdPASS == retval)
    {
        vTaskStartScheduler();
    }

    for (;;)
    {
        /* vTaskStartScheduler never returns */
    }
}

