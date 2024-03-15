#include <stdlib.h>
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "dataAcquisition.h"

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

//uint8_t data_acquisition_task(void *arg)
//uint8_t* data_acquisition_task(void *arg)
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
    cyhal_uart_init(&uart_obj, P12_1, P12_0, NC, NC, NULL,
                           &uart_config);

    // Set the baud rate
    cyhal_uart_set_baud(&uart_obj, 115200, &actualbaud);

    __enable_irq();
//    uint8_t data_barcode[13];
    uint8_t* data_barcode = malloc(TX_BUF_SIZE * sizeof(uint8_t));

    if (data_barcode == NULL)
    {
        // Handle memory allocation failure
        // For example, print an error message and return NULL
//        printf("Memory allocation failed!");
        return NULL;
    }

//    int i=0;
    	// for(;;) menyebabkan infinite loop sehingga return tidak bisa tercapai
//    for (;;)
//    {
//    while(true)
//    {
//
//    }
    for (size_t i = 0;i<TX_BUF_SIZE;i++)
    {
    	if (CY_RSLT_SUCCESS == cyhal_uart_getc(&uart_obj,
    											   &read_data, 0))
		{
			data_barcode[i]=read_data;
//			i++;
//			if (i == 12)
//			{
//				i = 0;
//			}
		}
    }
//    }
//    return (uint8_t)data_barcode;
//    return 0;
    return data_barcode;
}

/* [] END OF FILE */
