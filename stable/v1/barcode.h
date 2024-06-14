/*******************************************************************************
* File Name:   barcode.h (WMS v4.1)
* Description: implementasi Barcode sebagai bagian dari sub-sistem data acquisition
* 			   Mengirim perintah baca ke barcode scanner, menunggu byte pertama datang,
* 			   melakukan pembacaan dan penyimpanan ke array epc.
* Programmer: Bostang
* Tanggal: 19 Mei 2024, 30 Mei 2024
*******************************************************************************/

// HEADER GUARD
#ifndef BARCODE_H
#define BARCODE_H

/*******************************************************************************
* Header Files
*******************************************************************************/
#include <stdlib.h>
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "utils.h"
#include "fsm.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define BAR_RX 					(P9_0)
#define BAR_TX 					(P9_1)
#define RX_BUF_SIZE     		(14)
#define TX_BUF_SIZE     		(14)
#define BAR_TX_BUF_SIZE     	(6)

#define DATA_BITS_8     		(8)
#define STOP_BITS_1     		(1)
#define BAUD_RATE       		(115200)

#define UART_DELAY				(10u)
/*******************************************************************************
* Global Variables
*******************************************************************************/
cy_rslt_t result;

cyhal_uart_t uart_obj;
uint32_t     actualbaud;
uint8_t read_data; /* Variable to store the received character
					* through terminal */

uint8_t barcode_tx_buf[BAR_TX_BUF_SIZE] = {0x04, 0xE4, 0x04, 0x00, 0xFF, 0x14}; // cmd_read byte array
uint8_t barcode_wakeup[1] = {0x00}; // cmd_read byte array

size_t barcode_tx_length = BAR_TX_BUF_SIZE;
size_t barcode_wakeup_length = 1;

static bool barcode_initialized = false;

/* Timer object used */
cy_rslt_t rslt;
uint32_t read_val;
/* Timer object used */
cyhal_timer_t timer_obj;
cyhal_timer_t timer_obj;
const cyhal_timer_cfg_t timer_cfg =
{
    .compare_value = 0,                 /* Timer compare value, not used */
    .period = 20000,                    /* Timer period set to a large enough value
                                         * compared to event being measured */
    .direction = CYHAL_TIMER_DIR_UP,    /* Timer counts up */
    .is_compare = false,                /* Don't use compare mode */
    .is_continuous = false,             /* Do not run timer indefinitely */
    .value = 0                          /* Initial value of counter */
};


/*******************************************************************************
* Function Prototypes
********************************************************************************/
uint8_t* data_acquisition_barcode();

/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
 * Function Name: data_acquisition_barcode
 *******************************************************************************
 * Summary:
 *  Task used to retrieve data from qrcode and rfid.
 *
 * Parameters:
 *  void *args : Task parameter defined during task creation (unused).
 *
 * Return:
 *  uint8_t
 *
 *******************************************************************************/
uint8_t* data_acquisition_barcode()
{
    if (!barcode_initialized)
    {
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

		// Initialize the UART Block
		result = cyhal_uart_init(&uart_obj, BAR_TX, BAR_RX, NC, NC, NULL,
							   &uart_config);
		handle_error(result);

		// Set the baud rate
		result = cyhal_uart_set_baud(&uart_obj, 115200, &actualbaud);
		handle_error(result);

        barcode_initialized = true; // Set the flag to true indicating UART initialization is done
    }

    __enable_irq();
    // Clear the receive buffer of the UART object (untuk iterasi kedua dan seterusnya)
    cyhal_uart_clear(&uart_obj);

    // Membangunkan barcode scanner (0x00 50ms, datasheet hlm. 56 (Appendix 6))
    cyhal_uart_write(&uart_obj, (void*)barcode_wakeup, &barcode_wakeup_length); // wake up command for 50 ms
    vTaskDelay(pdMS_TO_TICKS(50));

    cyhal_uart_write(&uart_obj, (void*)barcode_tx_buf, &barcode_tx_length);
    cyhal_system_delay_ms(UART_DELAY); // kalau tidak diberi delay, data acknowledgement akan terdeteksi

    uint8_t* data_barcode = malloc(TX_BUF_SIZE * sizeof(uint8_t));

    if (data_barcode == NULL)
    {
        return NULL;
    }

    // melakukan pembacaan UART dari barcode scanner dan menyalinnya pada variabel data_barcode
    size_t i = 0;
    bool flag_bar_detected = false;

    // menunggu sampai data pertama tiba data
    while (!flag_bar_detected)
    {
    	// data pertama tiba
    	if (CY_RSLT_SUCCESS == cyhal_uart_getc(&uart_obj,&read_data, 0))
    	{
            if (isalnum(read_data))  // Check if the character is alphanumeric
            {
				data_barcode[i]=read_data;
				i++;
				flag_bar_detected = true;
            }
    	}
    }
    // lanjut baca data kedua sampai akhir
    while (i<TX_BUF_SIZE)
    {
    	if (CY_RSLT_SUCCESS == cyhal_uart_getc(&uart_obj,&read_data, 0))
		{
			data_barcode[i]=read_data;
			i++;
		}
    }
    return data_barcode;
}

#endif // BARCODE_H
/* [] END OF FILE */
