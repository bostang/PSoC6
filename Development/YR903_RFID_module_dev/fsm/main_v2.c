// menggunakan tiga buah state 

/*******************************************************************************
* File Name:   main.c
*
* Description: This code example demonstrates the use of GPIO configured as an
*              input pin to generate interrupts.
*
* Related Document: See README.md
*
*
********************************************************************************
* Copyright 2019-2023, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"


/*******************************************************************************
* Macros
*******************************************************************************/
#define DELAY_SHORT_MS          (250)   /* milliseconds */
#define DELAY_LONG_MS           (500)   /* milliseconds */
#define LED_BLINK_COUNT         (4)
#define GPIO_INTERRUPT_PRIORITY (7u)

#define STATE0 0
//#define STATE_DEBOUNCE_0 1
#define STATE1 1
#define STATE2 2
//#define STATE_DEBOUNCE_1 3

#define DEBOUNCE_DURATION 200u

/*******************************************************************************
* Global Variables
*******************************************************************************/
volatile bool gpio_intr_flag = false;
cyhal_gpio_callback_data_t gpio_btn_callback_data;

// FSM State
int state = STATE0;
int prev_state = STATE1;

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
static void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event);


/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
* Function Name: handle_error
********************************************************************************
* Summary:
* User defined error handling function
*
* Parameters:
*  uint32_t status - status indicates success or failure
*
* Return:
*  void
*
*******************************************************************************/
void handle_error(uint32_t status)
{
    if (status != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
}


/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  System entrance point. This function configures and initializes the GPIO
*  interrupt, update the delay on every GPIO interrupt, blinks the LED and enter
*  in deepsleep mode.
*
* Return: int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;
//    uint32_t count = 0;
//    uint32_t delay_led_blink = DELAY_LONG_MS;

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    /* Board init failed. Stop program execution */
    handle_error(result);

    /* Initialize retarget-io to use the debug UART port */
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                                 CY_RETARGET_IO_BAUDRATE);
    /* Retarget-io init failed. Stop program execution */
    handle_error(result);

    /* Initialize the user LED */
    result = cyhal_gpio_init(CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT,
                    CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    /* User LED init failed. Stop program execution */
    handle_error(result);

    /* Initialize the user button */
//    result = cyhal_gpiinit(CYBSP_USER_BTN, CYHAL_GPIO_DIR_INPUT,
//                    CYBSP_o_USER_BTN_DRIVE, CYBSP_BTN_OFF);
    result = cyhal_gpio_init(P13_4, CYHAL_GPIO_DIR_INPUT,
    		CYHAL_GPIO_DRIVE_PULL_NONE , 0);

    /* User button init failed. Stop program execution */
    handle_error(result);

    /* Configure GPIO interrupt */
    gpio_btn_callback_data.callback = gpio_interrupt_handler;
    cyhal_gpio_register_callback(P13_4,
                                 &gpio_btn_callback_data);
//    cyhal_gpio_enable_event(P13_4, CYHAL_GPIO_IRQ_FALL,
//                                 GPIO_INTERRUPT_PRIORITY, true);
    cyhal_gpio_enable_event(P13_4, CYHAL_GPIO_IRQ_RISE,
                                     GPIO_INTERRUPT_PRIORITY, true);

    /* Enable global interrupts */
    __enable_irq();

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");
    printf("**************** HAL: GPIO Interrupt ****************\r\n");

    for (;;)
    {
        /* Check the interrupt status */
    	// Saat button ditekan
        if (true == gpio_intr_flag)
        {
            gpio_intr_flag = false;

            if (state == STATE0)
            {
//            	state = STATE_DEBOUNCE_0; // next state
            	state = STATE1;
            	printf("State1\r\n");
				cyhal_gpio_write(CYBSP_USER_LED, 1);
				cyhal_system_delay_ms(DEBOUNCE_DURATION);
            }
//            else if (state == STATE_DEBOUNCE_0)
//            {
//            	cyhal_system_delay_ms(DEBOUNCE_DURATION);
//            	state = STATE1; // next state
//            }
            else if (state == STATE1)
            {
//            	state = STATE_DEBOUNCE_1;
				state = STATE2;
            	printf("State2\r\n");
            	cyhal_gpio_write(CYBSP_USER_LED, 0);
            	cyhal_system_delay_ms(DEBOUNCE_DURATION);
            }

            else
            {
//            	state = STATE_DEBOUNCE_1;
				state = STATE0;
            	printf("State0\r\n");
            	cyhal_gpio_write(CYBSP_USER_LED, 0);
            	cyhal_system_delay_ms(DEBOUNCE_DURATION);
            }
//            else // state_debounce_1
//            {
//            	cyhal_system_delay_ms(DEBOUNCE_DURATION);
//            	state = STATE0; // next state
//            }

//            /* Update LED toggle delay */
//            if (DELAY_LONG_MS == delay_led_blink)
//            {
//                delay_led_blink = DELAY_SHORT_MS;
//            }
//            else
//            {
//                delay_led_blink = DELAY_LONG_MS;
//            }
//        }
//
//        /* Blink LED four times */
//        for (count = 0; count < LED_BLINK_COUNT; count++)
//        {
//            cyhal_gpio_write(CYBSP_USER_LED, CYBSP_LED_STATE_ON);
//            cyhal_system_delay_ms(delay_led_blink);
//            cyhal_gpio_write(CYBSP_USER_LED, CYBSP_LED_STATE_OFF);
//            cyhal_system_delay_ms(delay_led_blink);
//        }

        /* Enter deep sleep mode */
        cyhal_syspm_deepsleep();
        }
    }
}


/*******************************************************************************
* Function Name: gpio_interrupt_handler
********************************************************************************
* Summary:
*   GPIO interrupt handler.
*
* Parameters:
*  void *handler_arg (unused)
*  cyhal_gpio_event_t (unused)
*
*******************************************************************************/
static void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event)
{
    gpio_intr_flag = true;
}


/* [] END OF FILE */
