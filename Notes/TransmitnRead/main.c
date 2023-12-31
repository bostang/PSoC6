/******************************************************************************
* File Name: main.c
*
* Description: This example demonstrates the UART transmit and receive
*              operation using HAL APIs
*
* Related Document: See Readme.md 
*
*******************************************************************************
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

#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"


/*******************************************************************************
* Function Name: handle_error
********************************************************************************
* Summary:
* User defined error handling function.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void handle_error(void)
{
     /* Disable all interrupts. */
    __disable_irq();

    CY_ASSERT(0);
}

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function.
* Reads one byte from the serial terminal and echoes back the read byte.
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
    #if defined(CY_DEVICE_SECURE)
        cyhal_wdt_t wdt_obj;
        /* Clear watchdog timer so that it doesn't trigger a reset */
        result = cyhal_wdt_init(&wdt_obj, cyhal_wdt_get_max_timeout_ms());
        CY_ASSERT(CY_RSLT_SUCCESS == result);
        cyhal_wdt_free(&wdt_obj);
    #endif

    uint8_t read_data; /* Variable to store the received character
                        * through terminal */

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    if (result != CY_RSLT_SUCCESS)
    {
        handle_error();
    }

    /* Initialize retarget-io to use the debug UART port */
#ifdef XMC72_EVK
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                                 CY_RETARGET_IO_BAUDRATE);

    if (result != CY_RSLT_SUCCESS)
    {
        handle_error();
    }
#else
    cyhal_uart_t uart_obj;
    cyhal_uart_t uart_obj_barcode;
	#define TX_BUF_SIZE     32
    size_t tx_length = TX_BUF_SIZE;

//    result = cy_retarget_io_init_fc(P9_1,
//                                    P9_0,
//                                    CYBSP_DEBUG_UART_CTS,
//                                    CYBSP_DEBUG_UART_RTS,
//                                    CY_RETARGET_IO_BAUDRATE);

    /* Initialize the UART configuration structure */
//    const cyhal_uart_cfg_t uart_config =
//    {
//        .data_bits = DATA_BITS_8,
//        .stop_bits = STOP_BITS_1,
//        .parity = CYHAL_UART_PARITY_NONE,
//        .rx_buffer = rx_buf,
//        .rx_buffer_size = RX_BUF_SIZE
//    };

    // mendeklarasikan uart object untuk barcode reader (sender)
    result = cyhal_uart_init(&uart_obj_barcode, CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,NC,NC, NULL, NULL);
    if (result != CY_RSLT_SUCCESS)
    {
        handle_error();
    }

    // mendeklarasikan uart object untuk USB-TTL (receiver)
    result = cyhal_uart_init(&uart_obj, P9_1, P9_0,NC,NC, NULL, NULL);
    if (result != CY_RSLT_SUCCESS)
    {
        handle_error();
    }
#endif
    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
//    printf("\x1b[2J\x1b[;H");

    // printf() menjadi tidak bisa dipakai
//    printf("***********************************************************\r\n");
//    printf("HAL: UART Transmit and Receive\r\n");
//    printf("***********************************************************\r\n\n");
//    printf(">> Start typing to see the echo on the screen \r\n\n");
//    uint8_t tx_buf[TX_BUF_SIZE] = {'1','2','3','4'};
//    uint8_t tx_buf[TX_BUF_SIZE] = "Starting Program\n============\n";
    char tx_buf[TX_BUF_SIZE] = "Starting Program";
    char msg1[15] = "----------";
    size_t len_msg1 = sizeof(msg1);

    cyhal_uart_write(&uart_obj,(void*) tx_buf,&tx_length);
    cyhal_uart_putc(&uart_obj,'\n');
    cyhal_uart_write(&uart_obj,(void*) msg1,&len_msg1);
    cyhal_uart_putc(&uart_obj,'\n');

    __enable_irq();
 
    for (;;)
    {
        if (CY_RSLT_SUCCESS == cyhal_uart_getc(&uart_obj_barcode,
                                               &read_data, 0))
        {
            if (CY_RSLT_SUCCESS != cyhal_uart_putc(&uart_obj,
                                                   read_data))
            {
                handle_error();
            }
        }
        else
        {
            handle_error();
        }
    }
}

/* [] END OF FILE */
