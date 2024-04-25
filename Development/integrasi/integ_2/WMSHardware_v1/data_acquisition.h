/*******************************************************************************
* File Name:   data_acquisition.h
* Description: implementasi Barcode sebagai bagian dari sub-sistem data acquisition
* 				apabila button ditekan, maka akan melakukan uart scanning (menunggu karakter input dari barcode scanner dan kemudian menampilkan bacaan pada terminal
* 				dan juga OLED
* Programmer: Bostang
* Tanggal awal : 23 April 2024
*******************************************************************************/

// HEADER GUARD
#ifndef DATA_ACQUISITION_H
#define DATA_ACQUISITION_H

/*******************************************************************************
* Header Files
*******************************************************************************/
#include <stdlib.h>
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define BAR_RX P9_0
#define BAR_TX P9_1
#define RX_BUF_SIZE     4
//#define TX_BUF_SIZE     14 // panjang tag = 14
#define TX_BUF_SIZE     12 // panjang tag = 14 (OLD)

/*******************************************************************************
* Function Prototypes
********************************************************************************/
uint8_t* data_acquisition_task();

/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
 * Function Name: data_acquisition_task
 *******************************************************************************
 * Summary:
 *  Task used to retrieve data from qrcode and rfid.
 *
 * Parameters:
 *  void *args : Task parameter defined during task creation (unused).
 *
 * Return:
 *  void
 *
 *******************************************************************************/

uint8_t* data_acquisition_task()
{
    cyhal_uart_t uart_obj;
    uint32_t     actualbaud;
    uint8_t      rx_buf[RX_BUF_SIZE];

    // Initialize the UART configuration structure
    const cyhal_uart_cfg_t uart_config =
    {
        .data_bits      = 8,
        .stop_bits      = 1,
        .parity         = CYHAL_UART_PARITY_NONE,
        .rx_buffer      = rx_buf,
        .rx_buffer_size = RX_BUF_SIZE
    };

    uint8_t read_data; /* Variable to store the received character
                        * through terminal */

    // Initialize the UART Block
    cyhal_uart_init(&uart_obj, BAR_TX, BAR_RX, NC, NC, NULL,
                           &uart_config);

    // Set the baud rate
    cyhal_uart_set_baud(&uart_obj, 115200, &actualbaud);

    __enable_irq();
    uint8_t* data_barcode = malloc(TX_BUF_SIZE * sizeof(uint8_t));

    if (data_barcode == NULL)
    {
        return NULL;
    }

    // melakukan pembacaan UART dari barcode scanner dan menyalinnya pada variabel data_barcode
    for (size_t i = 0;i<TX_BUF_SIZE;i++)
    {
    	if (CY_RSLT_SUCCESS == cyhal_uart_getc(&uart_obj,
    											   &read_data, 0))
		{
			data_barcode[i]=read_data;
		}
    }
    return data_barcode;
}

#endif // DATA_ACQUISITION_H
/* [] END OF FILE */
