/*******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the Empty Application Example
*              for ModusToolbox.
*
* Related Document: See README.md
*
*
********************************************************************************
* Copyright 2021-2023, Cypress Semiconductor Corporation (an Infineon company) or
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
#define RFID_rx	P12_0
#define RFID_tx P12_1
//#define button 13

#define DEBOUNCE_DURATION 300
#define READ_MODE 0
#define STOP_MODE 1

#define DELAY_SHORT_MS          (250)   /* milliseconds */
#define DELAY_LONG_MS           (500)   /* milliseconds */
#define GPIO_INTERRUPT_PRIORITY (7u)

//COMMAND
//command untuk membaca device address
const uint8_t cmd_readDevice[] = { 0xAA, 0xAA, 0xFF, 0x06, 0x05, 0x01, 0xFF, 0x1B, 0xA2 };
//command untuk setting frequency region 920.125~924.875MHz
const uint8_t cmd_setFreqRegion[] = { 0xAA, 0xAA, 0xFF, 0x06, 0x30, 0x00, 0x01, 0x08, 0x17 };
//command untuk setting work frequency channel
const uint8_t cmd_setFreqCh[] = { 0xAA, 0xAA, 0xFF, 0x06, 0x32, 0x00, 0x00, 0x76, 0x56 };
//command untuk turn Automatic Frequency Hopping Mode on
const uint8_t cmd_autoFreqHop[] = { 0xAA, 0xAA, 0xFF, 0x06, 0x37, 0x00, 0xFF, 0x83, 0x56 };
//command untuk Set RF Emission Power Capacity 30 dBm
const uint8_t cmd_PowEmmCap[] = { 0xAA, 0xAA, 0xFF, 0x07, 0x3B, 0x00, 0x0B, 0xB8, 0xEB, 0x5E };
// command untuk single tag inventory
const uint8_t cmd_startSingleRead[] = {0xAA,0xAA,0xFF,0x05,0xC8,0x00,0x3A,0x5E};
// command untuk set working parameters of the antenna
// jumlah antenna 1, port 1, polling open, power 30 dBm, batas kali bacaan per antenna 200
const uint8_t cmd_setWorkParam[] = { 0xAA, 0xAA, 0xFF, 0x0F, 0x3F, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x0B, 0xB8, 0x00, 0xC8, 0x74, 0xAF };
// command untuk start multiple tags in inventory
const uint8_t cmd_startRead[] = { 0xAA, 0xAA, 0xFF, 0x08, 0xC1, 0x00, 0x08, 0x00, 0x00, 0x60, 0x4A };
//menghentikan multiple read
const uint8_t cmd_stopRead[] = { 0xAA, 0xAA, 0xFF, 0x05, 0xC0, 0x00, 0xB3, 0xF7 };

size_t l_cmd_readDevice = sizeof(cmd_readDevice);

/*******************************************************************************
* Global Variables
*******************************************************************************/
volatile bool gpio_intr_flag = false;
cyhal_gpio_callback_data_t gpio_btn_callback_data;

// inisiasi variabel
int mode = STOP_MODE;
int countButtonPressed = 0;

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
* This is the main function for CPU. It switches between read and stop mode of RFID interface with trigger of button pressing.
*
*    1.
*    2.
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;

#if defined (CY_DEVICE_SECURE)
    cyhal_wdt_t wdt_obj;

    /* Clear watchdog timer so that it doesn't trigger a reset */
    result = cyhal_wdt_init(&wdt_obj, cyhal_wdt_get_max_timeout_ms());
    CY_ASSERT(CY_RSLT_SUCCESS == result);
    cyhal_wdt_free(&wdt_obj);
#endif

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

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
   result = cyhal_gpio_init(CYBSP_USER_BTN, CYHAL_GPIO_DIR_INPUT,
				   CYBSP_USER_BTN_DRIVE, CYBSP_BTN_OFF);

   /* User button init failed. Stop program execution */
   handle_error(result);

   /* Configure GPIO interrupt */
   gpio_btn_callback_data.callback = gpio_interrupt_handler;
   cyhal_gpio_register_callback(CYBSP_USER_BTN,
								&gpio_btn_callback_data);
   cyhal_gpio_enable_event(CYBSP_USER_BTN, CYHAL_GPIO_IRQ_FALL,
								GPIO_INTERRUPT_PRIORITY, true);


   /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
   printf("\x1b[2J\x1b[;H");
   printf("**************** HAL: GPIO Interrupt ****************\r\n");

   /* Enable global interrupts */
   __enable_irq();

    for (;;)
    {
    	/* Check the interrupt status */
		if (true == gpio_intr_flag)
		{
			gpio_intr_flag = false;
			countButtonPressed++;

			/* debug start*/
			printf("panjang cmd_readDevice : %d \r\n",l_cmd_readDevice);
			// mencetak byte per byte
			//printf("%02X\r\n",cmd_readDevice); // SALAH karena langsung kirim array
			printf("Satu per satu dengan cyhal_uart_putc:\r\n");
			for (size_t i = 0; i < l_cmd_readDevice; i++)
			{
//			    printf("%02X ", cmd_readDevice[i]);
				cyhal_uart_putc(&cy_retarget_io_uart_obj, cmd_readDevice[i]);
			}
			printf("\r\n");
			printf("Langsung dengan cyhal_uart_write:\r\n");
			cyhal_uart_write(&cy_retarget_io_uart_obj, (void*) cmd_readDevice, &l_cmd_readDevice);
			printf("\r\n");

			// kita peroleh kedua metode (write dan for loop + putc) sama hasilnya


			/*	debug end */
			if (mode == READ_MODE)
			{
			  mode = STOP_MODE;
			  printf("%d. getting in READ_MODE\r\n",countButtonPressed);
			  cyhal_gpio_write(CYBSP_USER_LED, CYBSP_LED_STATE_ON);
			}
			else
			{
			  mode = READ_MODE;
			  printf("%d. getting in STOP_MODE\r\n",countButtonPressed);
			  cyhal_gpio_write(CYBSP_USER_LED, CYBSP_LED_STATE_OFF);
			}
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
